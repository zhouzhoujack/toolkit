#include "tlLog.h"

#include "tlString.h"
#include "tlEnv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

namespace toolkit { namespace logkit
{

// ------------------------------------------------
//  Verbosity implementation

static int default_verbosity ()
{
  //  Level 10 is where the "log" channel becomes visible, so LOGI works out of
  //  the box. KLAYOUT_VERBOSITY still overrides the initial level.
  int verbosity = 10;
  std::string verbosity_str = get_env ("KLAYOUT_VERBOSITY");
  if (! verbosity_str.empty ()) {
    try {
      from_string (verbosity_str, verbosity);
    } catch (...) {
    }
  }
  return verbosity;
}

static int m_verbosity_level = default_verbosity ();

void
verbosity (int level)
{
  m_verbosity_level = level;
}

int
verbosity ()
{
  return m_verbosity_level;
}

// ------------------------------------------------
//  Channel implementation

Channel::Channel ()
  : m_no_endl (false), m_active (false), m_in_yield (false), m_at_line_start (true)
{
  //  .. nothing else ..
}

Channel::~Channel ()
{
  // .. nothing yet ..
}

ChannelProxy
Channel::issue_proxy ()
{
  m_no_endl = false;
  m_lock.lock ();
  if (! m_active) {
    begin ();
    m_active = true;
  }
  return ChannelProxy (this);
}

void
Channel::release_proxy ()
{
  if (! m_no_endl) {
    endl (); //  this helps remembering that there is something implicit going on here ..
    mark_line_end ();
  }
  end ();
  m_active = false;
  m_no_endl = false;
  bool in_yield = m_in_yield;
  m_in_yield = true;
  m_lock.unlock ();

  //  after we have released the lock we give the receivers an opportunity to process events. Note that we allow only one thread to yield
  //  at the same time and no recursive yields.
  if (! in_yield) {
    yield ();
    //  this should be atomic, but execution reordering may place it before yield(). Hence the lock.
    m_lock.lock ();
    m_in_yield = false;
    m_lock.unlock ();
  }
}

ChannelEndl endl;
ChannelNoendl noendl;

// ------------------------------------------------
//  LogTee implementation

LogTee::LogTee ()
{
  //  .. nothing yet ..
}

LogTee::LogTee (Channel *first, bool owned)
{
  add (first, owned);
}

void
LogTee::add (Channel *other, bool owned)
{
  m_lock.lock ();
  m_channels.push_back (other);
  if (owned) {
    m_owned_channels.push_back (other);
  }
  m_lock.unlock ();
}

void
LogTee::prepend (Channel *other, bool owned)
{
  m_lock.lock ();
  m_channels.insert (m_channels.begin (), other);
  if (owned) {
    m_owned_channels.push_back (other);
  }
  m_lock.unlock ();
}

void
LogTee::clear ()
{
  m_lock.lock ();
  m_channels.clear ();
  m_owned_channels.clear ();
  m_lock.unlock ();
}

void
LogTee::puts (const char *s)
{
  try {
    for (weak_collection<Channel>::iterator c = m_channels.begin (); c != m_channels.end (); ++c) {
      c->puts (s);
    }
  } catch (...) {
    //  ignore exceptions here
  }
}

void
LogTee::yield ()
{
  try {
    for (weak_collection<Channel>::iterator c = m_channels.begin (); c != m_channels.end (); ++c) {
      c->yield ();
    }
  } catch (...) {
    //  ignore exceptions here
  }
}

void
LogTee::endl ()
{
  try {
    for (weak_collection<Channel>::iterator c = m_channels.begin (); c != m_channels.end (); ++c) {
      c->endl ();
    }
  } catch (...) {
    //  ignore exceptions here
  }
}

void
LogTee::end ()
{
  try {
    for (weak_collection<Channel>::iterator c = m_channels.begin (); c != m_channels.end (); ++c) {
      c->end ();
    }
  } catch (...) {
    //  ignore exceptions here
  }
}

void
LogTee::begin ()
{
  try {
    for (weak_collection<Channel>::iterator c = m_channels.begin (); c != m_channels.end (); ++c) {
      c->begin ();
    }
  } catch (...) {
    //  ignore exceptions here
  }
}

void
LogTee::mark_line_end ()
{
  try {
    for (weak_collection<Channel>::iterator c = m_channels.begin (); c != m_channels.end (); ++c) {
      c->mark_line_end ();
    }
  } catch (...) {
    //  ignore exceptions here
  }
}

// ------------------------------------------------
//  ChannelProxy implementation

ChannelProxy::ChannelProxy (Channel *channel)
  : mp_channel (channel)
{
}

ChannelProxy::~ChannelProxy ()
{
  if (mp_channel) {
    mp_channel->release_proxy ();
    mp_channel = 0;
  }
}

ChannelProxy &
ChannelProxy::operator= (const ChannelProxy &d)
{
  if (mp_channel != d.mp_channel) {
    mp_channel = d.mp_channel;
    // transfer channel to the assigned object
    (const_cast<ChannelProxy &> (d)).mp_channel = 0;
  }
  return *this;
}

ChannelProxy::ChannelProxy (const ChannelProxy &d)
  : mp_channel (d.mp_channel)
{
  // transfer channel to the assigned object
  (const_cast<ChannelProxy &> (d)).mp_channel = 0;
}

// ------------------------------------------------
//  LogAt implementation

/**
 *  @brief Builds the "[HH:MM:SS.mmm file.cc:123] " prefix
 */
static std::string format_log_prefix (const char *file, int line)
{
  struct timespec ts;
  clock_gettime (CLOCK_REALTIME, &ts);

  struct tm tmv;
  localtime_r (&ts.tv_sec, &tmv);

  char stamp [32];
  snprintf (stamp, sizeof (stamp), "%02d:%02d:%02d.%03d",
            tmv.tm_hour, tmv.tm_min, tmv.tm_sec, int (ts.tv_nsec / 1000000));

  const char *base = file ? file : "?";
  const char *slash = strrchr (base, '/');
  if (slash) {
    base = slash + 1;
  }

  std::string prefix ("[");
  prefix += stamp;
  prefix += " ";
  prefix += base;
  prefix += ":";
  prefix += to_string (line);
  prefix += "] ";

  return prefix;
}

LogAt::LogAt (Channel &channel, const char *file, int line)
  : m_proxy (channel << format_log_prefix (file, line).c_str () << noendl)
{
  //  the prefix must not suppress the implicit end of line of the message itself
  m_proxy.mp_channel->noendl (false);
}

// ------------------------------------------------
//  Some utilities for colorized terminal output

static bool
can_colorize (FILE *stream)
{
  return isatty (fileno (stream));
}

#define ANSI_RED "\033[31;1m"
#define ANSI_BLUE "\033[34m"
#define ANSI_RESET "\033[0m"

// ------------------------------------------------
//  InfoChannel implementation

/**
 *  @brief A special implementation for information messages to stdout
 */

class InfoChannel : public Channel
{
public:
  InfoChannel (int verbosity, const char *level);
  ~InfoChannel ();

protected:
  virtual void puts (const char *s);
  virtual void endl ();
  virtual void end ();
  virtual void begin ();
  virtual void yield () { }

private:
  int m_verbosity;
  const char *m_level;
};

InfoChannel::InfoChannel (int verbosity, const char *level)
  : m_verbosity (verbosity), m_level (level)
{
}

InfoChannel::~InfoChannel ()
{
  //  .. nothing yet ..
}

void
InfoChannel::puts (const char *s)
{
  if (verbosity () >= m_verbosity) {
    if (take_line_start ()) {
      fputs (m_level, stdout);
      if (*s != '[') {
        fputs (" ", stdout);
      }
    }
    fprintf (stdout, "%s", s);
  }
}

void
InfoChannel::endl ()
{
  if (verbosity () >= m_verbosity) {
    fprintf (stdout, "\n");
  }
}

void
InfoChannel::end ()
{
  if (verbosity () >= m_verbosity) {
    fflush (stdout);
  }
}

void
InfoChannel::begin ()
{
  //  the info channel is intentionally not colorized
}

// ------------------------------------------------
//  WarningChannel implementation

/**
 *  @brief A special implementation for warnings to stdout
 */

class WarningChannel : public Channel
{
public:
  WarningChannel ();
  ~WarningChannel ();

protected:
  virtual void puts (const char *s);
  virtual void endl ();
  virtual void end ();
  virtual void begin ();
  virtual void yield () { }

private:
  bool m_colorized;
};

WarningChannel::WarningChannel ()
{
  m_colorized = can_colorize (stdout);
}

WarningChannel::~WarningChannel ()
{
  //  .. nothing yet ..
}

void
WarningChannel::puts (const char *s)
{
  if (take_line_start ()) {
    fputs ("[warn]", stdout);
    if (*s != '[') {
      fputs (" ", stdout);
    }
  }
  fputs (s, stdout);
}

void
WarningChannel::endl ()
{
  fputs ("\n", stdout);
}

void
WarningChannel::end ()
{
  if (m_colorized) {
    fputs (ANSI_RESET, stdout);
  }
  fflush (stdout);
}

void
WarningChannel::begin ()
{
  if (m_colorized) {
    fputs (ANSI_BLUE, stdout);
  }
}


// ------------------------------------------------
//  ErrorChannel implementation

/**
 *  @brief A special implementation for errors to stderr
 */

class ErrorChannel : public Channel
{
public:
  ErrorChannel ();
  ~ErrorChannel ();

protected:
  virtual void puts (const char *s);
  virtual void endl ();
  virtual void end ();
  virtual void begin ();
  virtual void yield () { }

private:
  bool m_colorized;
};

ErrorChannel::ErrorChannel ()
{
  m_colorized = can_colorize (stderr);
}

ErrorChannel::~ErrorChannel ()
{
  //  .. nothing yet ..
}

void
ErrorChannel::puts (const char *s)
{
  if (take_line_start ()) {
    fputs ("[error]", stderr);
    if (*s != '[') {
      fputs (" ", stderr);
    }
  }
  fputs (s, stderr);
}

void
ErrorChannel::endl ()
{
  fputs ("\n", stderr);
}

void
ErrorChannel::end ()
{
  if (m_colorized) {
    fputs (ANSI_RESET, stderr);
  }
  fflush (stderr);
}

void
ErrorChannel::begin ()
{
  if (m_colorized) {
    fputs (ANSI_RED, stderr);
  }
}

// ------------------------------------------------
//  FatalChannel implementation

/**
 *  @brief A special implementation for fatal errors to stderr
 *
 *  Unlike the error channel, the process is aborted after the message has been
 *  flushed (when the statement that wrote it ends).
 */

class FatalChannel : public Channel
{
public:
  FatalChannel ();
  ~FatalChannel ();

protected:
  virtual void puts (const char *s);
  virtual void endl ();
  virtual void end ();
  virtual void begin ();
  virtual void yield () { }

private:
  bool m_colorized;
};

FatalChannel::FatalChannel ()
{
  m_colorized = can_colorize (stderr);
}

FatalChannel::~FatalChannel ()
{
  //  .. nothing yet ..
}

void
FatalChannel::puts (const char *s)
{
  if (take_line_start ()) {
    fputs ("[fatal]", stderr);
    if (*s != '[') {
      fputs (" ", stderr);
    }
  }
  fputs (s, stderr);
}

void
FatalChannel::endl ()
{
  fputs ("\n", stderr);
}

void
FatalChannel::end ()
{
  if (m_colorized) {
    fputs (ANSI_RESET, stderr);
  }
  fflush (stderr);
  //  a fatal message is not a catchable error: terminate the process
  std::abort ();
}

void
FatalChannel::begin ()
{
  if (m_colorized) {
    fputs (ANSI_RED, stderr);
  }
}

// ------------------------------------------------
//  The instances of the log channels

TL_EXPORT LogTee fatal (new FatalChannel (), true);
TL_EXPORT LogTee warn (new WarningChannel (), true);
TL_EXPORT LogTee info (new InfoChannel (0, "[info]"), true);
TL_EXPORT LogTee log (new InfoChannel (10, "[log]"), true);
TL_EXPORT LogTee error (new ErrorChannel (), true);

} } // namespace toolkit::logkit

