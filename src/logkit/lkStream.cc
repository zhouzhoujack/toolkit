#include "lkStream.h"

#include <stddef.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <zlib.h>
#include <unistd.h>

#include "lkDeflate.h"
#include "lkAssert.h"
#include "lkFileUtils.h"
#include "lkLog.h"

#include "lkException.h"
#include "lkString.h"
#include "lkUri.h"

namespace toolkit {
namespace logkit {

// ---------------------------------------------------------------------------------
//  Some exception classes

class FileWriteErrorException
  : public Exception
{
public:
  FileWriteErrorException (const std::string &f, int en)
    : Exception (to_string (std::string ("Write error on file: %s (errno=%d)")), f, en)
  { }
};

class FileReadErrorException
  : public Exception
{
public:
  FileReadErrorException (const std::string &f, int en)
    : Exception (to_string (std::string ("Read error on file: %s (errno=%d)")), f, en)
  { }
};

class ZLibWriteErrorException
  : public Exception
{
public:
  ZLibWriteErrorException (const std::string &f, const char *em)
    : Exception (to_string (std::string ("Write error on file in decompression library: %s (message=%s)")), f, em)
  { }
};

class ZLibReadErrorException
  : public Exception
{
public:
  ZLibReadErrorException (const std::string &f, const char *em)
    : Exception (to_string (std::string ("Read error on file in decompression library: %s (message=%s)")), f, em)
  { }
};

class FileOpenErrorException
  : public Exception
{
public:
  FileOpenErrorException (const std::string &f, int en)
    : Exception (to_string (std::string ("Unable to open file: %s (errno=%d)")), f, en)
  { }
};

class FilePOpenErrorException
  : public Exception
{
public:
  FilePOpenErrorException (const std::string &f, int en)
    : Exception (to_string (std::string ("Unable to get input from command through pipe: %s (errno=%d)")), f, en)
  { }
};

class FilePReadErrorException
  : public Exception
{
public:
  FilePReadErrorException (const std::string &f, int en)
    : Exception (to_string (std::string ("Read error on pipe from command: %s (errno=%d)")), f, en)
  { }
};

class FilePWriteErrorException
  : public Exception
{
public:
  FilePWriteErrorException (const std::string &f, int en)
    : Exception (to_string (std::string ("Write error on pipe from command: %s (errno=%d)")), f, en)
  { }
};

// ---------------------------------------------------------------

class ZLibFilePrivate
{
public:
  ZLibFilePrivate () : zs (NULL) { }
  gzFile zs;
};

// ---------------------------------------------------------------
//  InputStream implementation

InputStream::InputStream (InputStreamBase &delegate)
  : m_pos (0), mp_bptr (0), mp_delegate (&delegate), m_owns_delegate (false), mp_inflate (0)
{ 
  m_bcap = 4096; // initial buffer capacity
  m_blen = 0;
  mp_buffer = new char [m_bcap];
}

InputStream::InputStream (InputStreamBase *delegate)
  : m_pos (0), mp_bptr (0), mp_delegate (delegate), m_owns_delegate (true), mp_inflate (0)
{
  m_bcap = 4096; // initial buffer capacity
  m_blen = 0;
  mp_buffer = new char [m_bcap];
}

InputStream::InputStream (const std::string &abstract_path)
  : m_pos (0), mp_bptr (0), mp_delegate (0), m_owns_delegate (false), mp_inflate (0)
{ 
  m_bcap = 4096; // initial buffer capacity
  m_blen = 0;
  mp_buffer = 0;

  Extractor ex (abstract_path.c_str ());

  if (ex.test (":")) {

    throw Exception (to_string (std::string ("Qt not enabled - resource paths are not available")));

  } else if (ex.test ("pipe:")) {

    mp_delegate = new InputPipe (ex.get ());

  } else {

    URI uri (abstract_path);

    if (uri.scheme () == "http" || uri.scheme () == "https") {
      throw Exception (to_string (std::string ("HTTP support not enabled - HTTP/HTTPS paths are not available")));
    } else if (uri.scheme () == "file") {
      mp_delegate = new InputZLibFile (uri.path ());
    } else if (! uri.scheme ().empty ()) {
      throw Exception (to_string (std::string ("URI scheme not supported: ")) + uri.scheme ());
    } else {
      mp_delegate = new InputZLibFile (abstract_path);
    }

  }

  if (! mp_buffer) {
    mp_buffer = new char [m_bcap];
  }

  m_owns_delegate = true;
}

std::string InputStream::absolute_path (const std::string &abstract_path)
{
  //  TODO: align this implementation with InputStream ctor
  Extractor ex (abstract_path.c_str ());
  if (ex.test ("pipe:")) {
    return abstract_path;
  } else if (ex.test ("file:")) {
    URI uri (abstract_path);
    return absolute_path (uri.path ());
  } else {
    return absolute_file_path (abstract_path);
  }
}

InputStream::~InputStream ()
{
  if (mp_delegate && m_owns_delegate) {
    delete mp_delegate;
    mp_delegate = 0;
  }
  if (mp_inflate) {
    delete mp_inflate;
    mp_inflate = 0;
  } 
  if (mp_buffer) {
    delete[] mp_buffer;
    mp_buffer = 0;
  }
}

const char * 
InputStream::get (size_t n, bool bypass_inflate)
{
  //  if deflating, employ the deflate filter to get the data
  if (mp_inflate && ! bypass_inflate) {
    if (! mp_inflate->at_end ()) {

      const char *r = mp_inflate->get (n);
      lk_assert (r != 0);  //  since deflate did not report at_end()
      return r;

    } else {
      delete mp_inflate;
      mp_inflate = 0;
    }
  } 

  if (m_blen < n) {

    //  to keep move activity low, allocate twice as much as required
    if (m_bcap < n * 2) {

      while (m_bcap < n) {
        m_bcap *= 2;
      }

      char *buffer = new char [m_bcap];
      if (m_blen > 0) {
        memcpy (buffer, mp_bptr, m_blen);
      }
      delete [] mp_buffer;
      mp_buffer = buffer;

    } else if (m_blen > 0) {
      memmove (mp_buffer, mp_bptr, m_blen);
    }

    if (mp_delegate) {
      m_blen += mp_delegate->read (mp_buffer + m_blen, m_bcap - m_blen);
    }
    mp_bptr = mp_buffer;

  }

  if (m_blen >= n) {
    const char *r = mp_bptr;
    mp_bptr += n;
    m_blen -= n;
    m_pos += n;
    return r;
  } else {
    return 0;
  }
}

void
InputStream::unget (size_t n)
{
  if (mp_inflate) {
    mp_inflate->unget (n);
  } else {
    mp_bptr -= n;
    m_blen += n;
    m_pos -= n;
  }
}

std::string
InputStream::read_all (size_t max_count) 
{
  std::string str;
  while (max_count > 0) {
    size_t n = std::min (max_count, std::max (size_t (1), m_blen));
    const char *b = get (n);
    if (b) {
      str += std::string (b, n);
      max_count -= n;
    } else {
      break;
    }
  } 
  return str;
}

std::string
InputStream::read_all () 
{
  std::string str;
  while (true) {
    size_t n = std::max (size_t (1), m_blen);
    const char *b = get (n);
    if (b) {
      str += std::string (b, n);
    } else {
      break;
    }
  } 
  return str;
}

void InputStream::copy_to (OutputStream &os)
{
  const size_t chunk = 65536;
  char b [chunk];
  size_t read;
  while (mp_delegate && (read = mp_delegate->read (b, sizeof (b))) > 0) {
    os.put (b, read);
  }
}

void
InputStream::inflate ()
{
  lk_assert (mp_inflate == 0);
  mp_inflate = new InflateFilter (*this);
}

void
InputStream::close ()
{
  if (mp_delegate) {
    mp_delegate->close ();
  }
}

void 
InputStream::reset ()
{
  //  stop inflate
  if (mp_inflate) {
    delete mp_inflate;
    mp_inflate = 0;
  } 

  //  optimize for a reset in the first m_bcap bytes
  //  -> this reduces the reset calls on mp_delegate which may not support this
  if (m_pos < m_bcap) {

    m_blen += m_pos;
    mp_bptr = mp_buffer;
    m_pos = 0;

  } else {

    mp_delegate->reset ();
    m_pos = 0;

    if (mp_buffer) {
      delete[] mp_buffer;
      mp_buffer = 0;
    }

    mp_bptr = 0;
    m_blen = 0;
    mp_buffer = new char [m_bcap];

  }
}

// ---------------------------------------------------------------
//  TextInputStream implementation

TextInputStream::TextInputStream (InputStream &stream)
  : m_line (1), m_next_line (1), m_at_end (false), m_stream (stream)
{ 
  if (m_stream.get (1) == 0) {
    m_at_end = true;
  } else {
    m_stream.unget (1);
  }
}

std::string
TextInputStream::read_all ()
{
  return read_all (std::numeric_limits<size_t>::max ());
}

std::string
TextInputStream::read_all (size_t max_count)
{
  std::string text;

  while (! at_end () && max_count > 0) {
    char c = get_char ();
    if (c == 0) {
      break;
    } else {
      --max_count;
      text += c;
    }
  }

  return text;
}

const std::string &
TextInputStream::get_line ()
{
  size_t line = m_next_line;
  m_line_buffer.clear ();

  while (! at_end ()) {
    char c = get_char ();
    if (c == '\n') {
      //  set at_end if there is nothing after this terminal LF -> this will avoid
      //  emitting an empty dummy line as the last one
      if (peek_char () == 0) {
        m_at_end = true;
      }
      break;
    } else if (c == 0) {
      break;
    } else {
      m_line_buffer += c;
    }
  }

  m_line = line;
  return m_line_buffer;
}

char 
TextInputStream::get_char ()
{
  while (true) {
    m_line = m_next_line;
    const char *c = m_stream.get (1);
    if (c == 0) {
      m_at_end = true;
      return 0;
    } else if (*c != '\r' && *c) {
      if (*c == '\n') {
        ++m_next_line;
      }
      return *c;
    }
  }
}

char 
TextInputStream::peek_char ()
{
  while (true) {
    m_line = m_next_line;
    const char *c = m_stream.get (1);
    if (c == 0) {
      return 0;
    } else if (*c != '\r' && *c) {
      char cc = *c;
      m_stream.unget (1);
      return cc;
    }
  }
}

char 
TextInputStream::skip ()
{
  char c = 0;
  while (! at_end () && isspace (c = peek_char ())) {
    get_char ();
  }
  return at_end () ? 0 : c;
}

void
TextInputStream::reset ()
{
  m_stream.reset ();

  m_line = 1;
  m_next_line = 1;

  if (m_stream.get (1) == 0) {
    m_at_end = true;
  } else {
    m_at_end = false;
    m_stream.unget (1);
  }
}

// ---------------------------------------------------------------
//  InputFile implementation

InputFile::InputFile (const std::string &path)
  : m_fd (-1)
{
  m_source = path;
  int fd = open (path.c_str (), O_RDONLY);
  if (fd < 0) {
    throw FileOpenErrorException (m_source, errno);
  }
  m_fd = fd;
}

InputFile::~InputFile ()
{
  close ();
}

void
InputFile::close ()
{
  if (m_fd >= 0) {
    ::close (m_fd);
    m_fd = -1;
  }  
}

size_t 
InputFile::read (char *b, size_t n)
{
  lk_assert (m_fd >= 0);
  ptrdiff_t ret = ::read (m_fd, b, (unsigned int) n);
  if (ret < 0) {
    throw FileReadErrorException (m_source, errno);
  }
  return size_t (ret);
}

void 
InputFile::reset ()
{
  if (m_fd >= 0) {
    lseek (m_fd, 0, SEEK_SET);
  }
}

std::string
InputFile::absolute_path () const
{
  return absolute_file_path (m_source);
}

std::string
InputFile::filename () const
{
  return logkit::filename (m_source);
}

// ---------------------------------------------------------------
//  InputZLibFile implementation

InputZLibFile::InputZLibFile (const std::string &path)
  : mp_d (new ZLibFilePrivate ())
{
  m_source = path;
  mp_d->zs = gzopen (string_to_system (path).c_str (), "rb");
  if (mp_d->zs == NULL) {
    throw FileOpenErrorException (m_source, errno);
  }
}

InputZLibFile::~InputZLibFile ()
{
  close ();
  delete mp_d;
  mp_d = 0;
}

void
InputZLibFile::close ()
{
  if (mp_d->zs != NULL) {
    gzclose (mp_d->zs);
    mp_d->zs = NULL;
  }  
}

size_t 
InputZLibFile::read (char *b, size_t n)
{
  lk_assert (mp_d->zs != NULL);
  int ret = gzread (mp_d->zs, b, (unsigned int) n);
  if (ret < 0) {
    int gz_err = 0;
    const char *em = gzerror (mp_d->zs, &gz_err);
    if (gz_err == Z_ERRNO) {
      throw FileReadErrorException (m_source, errno);
    } else {
      throw ZLibReadErrorException (m_source, em);
    }
  }

  return ret;
}

void 
InputZLibFile::reset ()
{
  if (mp_d->zs != NULL) {
    gzrewind (mp_d->zs);
  }
}

std::string
InputZLibFile::absolute_path () const
{
  return absolute_file_path (m_source);
}

std::string
InputZLibFile::filename () const
{
  return logkit::filename (m_source);
}

// ---------------------------------------------------------------
//  OutputStream implementation

OutputStream::OutputStream (OutputStreamBase &delegate, bool as_text)
  : m_pos (0), mp_delegate (&delegate), m_owns_delegate (false), m_as_text (as_text)
{ 
  m_buffer_capacity = 16384;
  m_buffer_pos = 0;
  mp_buffer = new char[m_buffer_capacity];
}

OutputStream::OutputStream (OutputStreamBase *delegate, bool as_text)
  : m_pos (0), mp_delegate (delegate), m_owns_delegate (true), m_as_text (as_text)
{
  m_buffer_capacity = 16384;
  m_buffer_pos = 0;
  mp_buffer = new char[m_buffer_capacity];
}

OutputStream::OutputStreamMode
OutputStream::output_mode_from_filename (const std::string &abstract_path, OutputStream::OutputStreamMode om)
{
  if (om == OM_Auto) {
    if (match_filename_to_format (abstract_path, "(*.gz *.gzip *.GZ *.GZIP)")) {
      om = OM_Zlib;
    } else {
      om = OM_Plain;
    }
  }

  return om;
}

static
OutputStreamBase *create_file_stream (const std::string &path, OutputStream::OutputStreamMode om, int keep_backups)
{
  if (om == OutputStream::OM_Zlib) {
    return new OutputZLibFile (path, keep_backups);
  } else {
    return new OutputFile (path, keep_backups);
  }
}

OutputStream::OutputStream (const std::string &abstract_path, OutputStreamMode om, bool as_text, int keep_backups)
  : m_pos (0), mp_delegate (0), m_owns_delegate (false), m_as_text (as_text), m_path (abstract_path)
{
  //  Determine output mode
  om = output_mode_from_filename (abstract_path, om);

  Extractor ex (abstract_path.c_str ());
  if (ex.test ("http:") || ex.test ("https:")) {
    throw Exception (to_string (std::string ("Cannot write to http:, https: or pipe: URL's")));
  } else if (ex.test ("pipe:")) {
    mp_delegate = new OutputPipe (ex.get ());
  } else if (ex.test ("file:")) {
    mp_delegate = create_file_stream (ex.get (), om, keep_backups);
  } else {
    mp_delegate = create_file_stream (abstract_path, om, keep_backups);
  }

  m_owns_delegate = true;

  m_buffer_capacity = 16384;
  m_buffer_pos = 0;
  mp_buffer = new char[m_buffer_capacity];
}

OutputStream::~OutputStream ()
{
  try {
    close ();
  } catch (...) {
    //  no recursive exceptions
  }
}

void
OutputStream::close ()
{
  try {

    flush ();

    if (mp_delegate && m_owns_delegate) {
      delete mp_delegate;
      mp_delegate = 0;
    }
    if (mp_buffer) {
      delete[] mp_buffer;
      mp_buffer = 0;
    }

  } catch (...) {

    if (mp_delegate && m_owns_delegate) {
      delete mp_delegate;
      mp_delegate = 0;
    }
    if (mp_buffer) {
      delete[] mp_buffer;
      mp_buffer = 0;
    }

    throw;

  }
}

void
OutputStream::set_as_text (bool f)
{
  m_as_text = f;
}

inline void fast_copy (char *t, const char *s, size_t n)
{
  if (n >= sizeof (unsigned long)) {

    unsigned long *tl = reinterpret_cast<unsigned long *> (t);
    const unsigned long *sl = reinterpret_cast<const unsigned long *> (s);

    while (n >= sizeof (unsigned long)) {
      *tl++ = *sl++;
      n -= sizeof (unsigned long);
    }

    t = reinterpret_cast<char *> (tl);
    s = reinterpret_cast<const char *> (sl);

  }

  while (n-- > 0) {
    *t++ = *s++;
  }
}

void
OutputStream::flush ()
{
  if (m_buffer_pos > 0 && mp_delegate) {
    mp_delegate->write (mp_buffer, m_buffer_pos);
    m_buffer_pos = 0;
  }
}

void
OutputStream::put (const char *b, size_t n)
{
  if (! mp_delegate) {
    return;
  }

  if (m_as_text) {
    //  skip CR, but replace LF by CRLF -> this will normalize the line terminators to CRLF
    while (n > 0) {
      if (*b == '\r') {
        ++b;
        --n;
      } else if (*b == '\n') {
        const char *ls = line_separator ();
        while (*ls) {
          put_raw (ls++, 1);
        }
        ++b;
        --n;
      } else {
        const char *b0 = b;
        while (n > 0 && *b != '\r' && *b != '\n') {
          ++b;
          --n;
        }
        put_raw (b0, b - b0);
      }
    }
  } else {
    put_raw (b, n);
  }
}

void
OutputStream::put_raw (const char *b, size_t n)
{
  m_pos += n;

  while (m_buffer_pos + n > m_buffer_capacity) {

    size_t nw = m_buffer_capacity - m_buffer_pos;
    if (nw) {
      n -= nw;
      fast_copy (mp_buffer + m_buffer_pos, b, nw);
      b += nw;
    }

    mp_delegate->write (mp_buffer, m_buffer_capacity);
    m_buffer_pos = 0;

  }

  if (n) {
    fast_copy (mp_buffer + m_buffer_pos, b, n);
    m_buffer_pos += n;
  }
} 

void
OutputStream::seek (size_t pos)
{
  flush ();

  if (mp_delegate) {
    mp_delegate->seek (pos);
  }
  m_pos = pos;
}

// ---------------------------------------------------------------
//  OutputFileBase implementation

OutputFileBase::OutputFileBase (const std::string &path, int keep_backups)
  : m_keep_backups (keep_backups), m_path (path), m_has_error (false)
{
  if (file_exists (path)) {
    m_backup_path = path + ".~backup";
    if (file_exists (m_backup_path)) {
      if (! rm_file (m_backup_path)) {
        warn << sprintf (to_string (std::string ("Could not create backup file: unable to remove existing file '%s'")), m_backup_path);
        m_backup_path = std::string ();
      }
    }
    if (! m_backup_path.empty ()) {
      if (! rename_file (path, m_backup_path)) {
        warn << sprintf (to_string (std::string ("Could not create backup file: unable to rename original file '%s' to backup file")), path, m_backup_path);
        m_backup_path = std::string ();
      }
    }
  }
}

OutputFileBase::~OutputFileBase ()
{
  if (! m_backup_path.empty ()) {

    if (m_has_error) {

      if (! rm_file (m_path)) {
        warn << sprintf (to_string (std::string ("Could not restore backup file: unable to remove file '%s'")), m_path);
      } else if (! rename_file (m_backup_path, m_path)) {
        warn << sprintf (to_string (std::string ("Could not restore backup file: unable to rename file '%s' back to '%s'")), m_backup_path, m_path);
      }

    } else {

      if (m_keep_backups == 0) {

        if (! rm_file (m_backup_path)) {
          warn << sprintf (to_string (std::string ("Could not remove backup file '%s'")), m_backup_path);
        }

      } else {

        //  shuffle backup files
        int n = 1;
        for ( ; m_keep_backups < 0 || n < m_keep_backups; ++n) {
          std::string p = m_path + "." + to_string (n);
          if (! file_exists (p)) {
            break;
          }
        }

        while (n > 0) {
          std::string p = m_path + "." + to_string (n);
          std::string pprev = n > 1 ? (m_path + "." + to_string (n - 1)) : m_backup_path;
          if (file_exists (p)) {
            if (! rm_file (p)) {
              warn << sprintf (to_string (std::string ("Error shuffling backup files: unable to remove file '%s'")), p);
            }
          }
          if (! rename_file (pprev, p)) {
            warn << sprintf (to_string (std::string ("Error shuffling backup files: unable to rename file '%s' to '%s'")), pprev, p);
          }
          --n;
        }

      }

    }
  }
}

void OutputFileBase::seek (size_t s)
{
  try {
    seek_file (s);
  } catch (...) {
    reject ();
    throw;
  }
}

void OutputFileBase::write (const char *b, size_t n)
{
  try {
    write_file (b, n);
  } catch (...) {
    reject ();
    throw;
  }
}

void OutputFileBase::reject ()
{
  m_has_error = true;
}

// ---------------------------------------------------------------
//  OutputFile implementation

OutputFile::OutputFile (const std::string &path, int keep_backups)
  : OutputFileBase (path, keep_backups), m_fd (-1)
{
  m_source = path;
  int fd = open (path.c_str (), O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd < 0) {
    throw FileOpenErrorException (m_source, errno);
  }
  m_fd = fd;
}

OutputFile::~OutputFile ()
{
  if (m_fd >= 0) {
    close (m_fd);
    m_fd = -1;
  }
}

void
OutputFile::seek_file (size_t s)
{
  lk_assert (m_fd >= 0);
  lseek (m_fd, s, SEEK_SET);
}

void
OutputFile::write_file (const char *b, size_t n)
{
  lk_assert (m_fd >= 0);
  ptrdiff_t ret = ::write (m_fd, b, (unsigned int) n);
  if (ret < 0) {
    throw FileWriteErrorException (m_source, errno);
  }
}

// ---------------------------------------------------------------
//  OutputZLibFile implementation

OutputZLibFile::OutputZLibFile (const std::string &path, int keep_backups)
  : OutputFileBase (path, keep_backups), mp_d (new ZLibFilePrivate ())
{
  m_source = path;
  mp_d->zs = gzopen (string_to_system (path).c_str (), "wb");
  if (mp_d->zs == NULL) {
    throw FileOpenErrorException (m_source, errno);
  }
}

OutputZLibFile::~OutputZLibFile ()
{
  if (mp_d->zs != NULL) {
    gzclose (mp_d->zs);
    mp_d->zs = NULL;
  }  
  delete mp_d;
  mp_d = 0;
}

void 
OutputZLibFile::write_file (const char *b, size_t n)
{
  lk_assert (mp_d->zs != NULL);
  int ret = gzwrite (mp_d->zs, (char *) b, (unsigned int) n);
  if (ret < 0) {
    int gz_err = 0;
    const char *em = gzerror (mp_d->zs, &gz_err);
    if (gz_err == Z_ERRNO) {
      throw FileWriteErrorException (m_source, errno);
    } else {
      throw ZLibWriteErrorException (m_source, em);
    }
  }
}

// ---------------------------------------------------------------
//  InputPipe delegate implementation

InputPipe::InputPipe (const std::string &path)
  : m_file (NULL)
{
  m_source = path;
  m_file = popen (string_to_system (path).c_str (), "r");
  if (m_file == NULL) {
    throw FilePOpenErrorException (m_source, errno);
  }
}

InputPipe::~InputPipe ()
{
  close ();
}

void
InputPipe::close ()
{
  wait ();
}

int InputPipe::wait ()
{
  int ret = 0;
  if (m_file != NULL) {
    ret = pclose (m_file);
    m_file = NULL;
  }
  return ret;
}

size_t 
InputPipe::read (char *b, size_t n)
{
  lk_assert (m_file != NULL);

  bool retry = true;
  size_t ret = 0;

  while (retry) {
    retry = false;
    ret = fread (b, 1, n, m_file);
    if (ret < n) {
      if (ferror (m_file)) {
        if (errno != EINTR) {
          throw FilePReadErrorException (m_source, errno);
        } else if (ret == 0) {
          retry = true;
          clearerr (m_file);
        }
      }
    }
  }

  return ret;
}

void 
InputPipe::reset ()
{
  throw Exception (to_string (std::string ("'reset' is not supported on pipeline input files")));
}

// ---------------------------------------------------------------
//  OutputPipe delegate implementation

OutputPipe::OutputPipe (const std::string &path)
  : m_file (NULL)
{
  m_source = path;
  m_file = popen (string_to_system (path).c_str (), "w");
  if (m_file == NULL) {
    throw FilePOpenErrorException (m_source, errno);
  }
}

OutputPipe::~OutputPipe ()
{
  if (m_file != NULL) {
    pclose (m_file);
    m_file = NULL;
  }  
}

void 
OutputPipe::write (const char *b, size_t n)
{
  lk_assert (m_file != NULL);

  size_t ret = fwrite (b, 1, n, m_file);
  if (ret < n) {
    if (ferror (m_file) && errno != EINTR) {
      throw FilePReadErrorException (m_source, errno);
    }
  }
}

// ---------------------------------------------------------------
//  match_filename_to_format implementation

bool 
match_filename_to_format (const std::string &fn, const std::string &fmt)
{
  const char *fp = fmt.c_str ();
  while (*fp && *fp != '(') {
    ++fp;
  }
  while (*fp && *fp != ')') {
    if (*++fp == '*') {
      ++fp;
    }
    const char *fpp = fp;
    while (*fpp && *fpp != ' ' && *fpp != ')') {
      ++fpp;
    }
    if (fn.size () > (unsigned int) (fpp - fp) && strncmp (fn.c_str () + fn.size () - (fpp - fp), fp, fpp - fp) == 0) {
      return true;
    }
    fp = fpp;
    while (*fp == ' ') {
      ++fp;
    }
  }
  return false;
}

} // namespace logkit
} // namespace toolkit

