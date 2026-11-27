#include "lkFileUtils.h"
#include "lkStream.h"
#include "lkLog.h"

#include <cctype>

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>

namespace toolkit {
namespace logkit {

enum { OS_Auto, OS_Windows, OS_Linux } s_mode = OS_Auto;

static bool is_win ()
{
  if (s_mode == OS_Windows) {
    return true;
  } else if (s_mode == OS_Linux) {
    return false;
  } else {
    //  Windows support has been removed: the automatic mode is never Windows.
    return false;
  }
}

//  Secret mode switchers for testing
void file_utils_force_windows () { s_mode = OS_Windows; }
void file_utils_force_linux () { s_mode = OS_Linux; }
void file_utils_force_reset () { s_mode = OS_Auto; }

const char *line_separator ()
{
  return is_win () ? "\r\n" : "\n";
}

static bool is_drive (const std::string &part)
{
  return is_win () && (part.size () == 2 && isalpha (part[0]) && part[1] == ':');
}

static std::string normalized_part (const std::string &part)
{
  if (! is_win ()) {
    return part;
  }

  std::string p;
  p.reserve (part.size ());
  const char *cp = part.c_str ();
  while (*cp == '\\' || *cp == '/') {
    p += '\\';
    ++cp;
  }
  p += cp;
  return p;
}

static std::string trimmed_part (const std::string &part)
{
  const char *cp = part.c_str ();

  if (is_win ()) {
    while (*cp == '\\' || *cp == '/') {
      ++cp;
    }
  } else {
    while (*cp == '/') {
      ++cp;
    }
  }

  return std::string (cp);
}

static bool is_part_with_separator (const std::string &part)
{
  const char *cp = part.c_str ();
  if (is_win ()) {
    return (*cp == '\\' || *cp == '/');
  } else {
    return (*cp == '/');
  }
}

std::vector<std::string> split_path (const std::string &p, bool keep_last)
{
  std::vector<std::string> parts;

  bool first = true;

  if (is_win ()) {

    const char *cp = p.c_str ();
    if (*cp && isalpha (*cp) && cp[1] == ':') {

      //  drive name
      parts.push_back (std::string ());
      parts.back () += toupper (*cp);
      parts.back () += ":";

      cp += 2;

    } else if ((*cp == '\\' && cp[1] == '\\') || (*cp == '/' && cp[1] == '/')) {

      //  UNC server name
      const char *cp0 = cp;
      cp += 2;
      while (*cp && *cp != '\\' && *cp != '/') {
        ++cp;
      }
      parts.push_back (normalized_part (std::string (cp0, 0, cp - cp0)));

    } else if ((*cp == '\\' || *cp == '/') && cp[1] && isalpha (cp[1]) && cp[2] == ':') {

      //  drive name in the form "/c:" or "\c:"
      parts.push_back (std::string ());
      parts.back () += toupper (cp[1]);
      parts.back () += ":";

      cp += 3;

    }

    while (*cp) {

      const char *cp0 = cp;
      bool any = false;
      while (*cp && (!any || (*cp != '\\' && *cp != '/'))) {
        if (*cp != '\\' && *cp != '/') {
          any = true;
        } else {
          cp0 = cp;
        }
        ++cp;
      }

      if (any || first || keep_last) {
        first = false;
        parts.push_back (normalized_part (std::string (cp0, 0, cp - cp0)));
      }

    }

  } else {

    const char *cp = p.c_str ();
    while (*cp) {

      const char *cp0 = cp;
      bool any = false;
      while (*cp && (!any || *cp != '/')) {
        if (*cp != '/') {
          any = true;
        } else {
          cp0 = cp;
        }
        //  backslash escape
        if (*cp == '\\' && cp[1]) {
          ++cp;
        }
        ++cp;
      }

      if (any || first || keep_last) {
        first = false;
        parts.push_back (std::string (cp0, 0, cp - cp0));
      }

    }

  }

  return parts;
}

static std::vector<std::string> split_filename (const std::string &fn)
{
  std::vector<std::string> parts;

  const char *cp = fn.c_str ();

  while (*cp) {

    const char *cp0 = cp;
    ++cp;
    while (*cp && *cp != '.') {
      //  backslash escaping (ineffective on Windows because that is a path separator)
      if (*cp == '\\' && cp[1]) {
        ++cp;
      }
      ++cp;
    }

    parts.push_back (std::string (cp0, 0, cp - cp0));
    if (*cp) {
      ++cp;
    }

  }

  return parts;
}

std::string normalize_path (const std::string &s)
{
  return join (split_path (s), "");
}

std::string combine_path (const std::string &p1, const std::string &p2, bool always_join)
{
  if (! always_join && p2.empty ()) {
    return p1;
  } else if (is_win ()) {
    return p1 + "\\" + p2;
  } else {
    return p1 + "/" + p2;
  }
}

std::string dirname (const std::string &s)
{
  std::vector<std::string> parts = split_path (s, true /*keep last part*/);
  if (parts.size () > 0) {
    parts.pop_back ();
  }

  if (parts.empty ()) {
    return is_part_with_separator (s) ? "" : ".";
  } else {
    return join (parts, "");
  }
}

std::string filename (const std::string &s)
{
  std::vector<std::string> parts = split_path (s, true /*keep last part*/);
  if (parts.size () > 0) {
    return trimmed_part (parts.back ());
  } else {
    return std::string ();
  }
}

std::string basename (const std::string &s)
{
  std::vector<std::string> fnp = split_filename (filename (s));
  if (fnp.size () > 0) {
    return fnp.front ();
  } else {
    return std::string ();
  }
}

std::string extension (const std::string &s)
{
  std::vector<std::string> fnp = split_filename (filename (s));
  if (fnp.size () > 0) {
    fnp.erase (fnp.begin ());
  }
  return join (fnp, ".");
}

std::string extension_last (const std::string &s)
{
  std::vector<std::string> fnp = split_filename (filename (s));
  if (fnp.size () > 1) {
    return fnp.back ();
  } else {
    return std::string ();
  }
}

bool
is_parent_path (const std::string &parent, const std::string &path)
{
  if (! file_exists (parent)) {
    //  If the parent path does not exist, we always return false. This cannot be a parent.
    return false;
  }

  std::vector<std::string> parts = split_path (absolute_file_path (path));

  while (! parts.empty () && ! (parts.size () == 1 && is_drive (parts[0]))) {
    if (is_same_file (parent, join (parts, ""))) {
      return true;
    } else {
      parts.pop_back ();
    }
  }

  //  We did not find a match - now maybe the parent is root
  return (is_same_file (parent, combine_path (join (parts, ""), "", true /*always add slash*/)));
}

std::vector<std::string> dir_entries (const std::string &s, bool with_files, bool with_dirs, bool without_dotfiles)
{
  std::vector<std::string> ee;

  DIR *h = opendir (to_local (s).c_str ());
  if (h) {

    struct dirent *d;
    while ((d = readdir (h)) != NULL) {

      std::string e = to_string_from_local (d->d_name);
      if (e.empty () || e == "." || e == "..") {
        continue;
      }

      bool is_dir = (d->d_type == DT_DIR);
      if ((e[0] != '.' || !without_dotfiles) && ((is_dir && with_dirs) || (!is_dir && with_files))) {
        ee.push_back (e);
      }

    }

    closedir (h);

  }

  return ee;
}

bool mkdir (const std::string &path)
{
  return ::mkdir (to_local (path).c_str (), 0777) == 0;
}

bool mkpath (const std::string &p)
{
  std::vector<std::string> parts = split_path (absolute_file_path (p));

  size_t i = 0;
  std::string front;
  if (! parts.empty () && is_drive (parts.front ())) {
    front = parts.front ();
    ++i;
  }

  while (i < parts.size ()) {
    front += parts[i++];
    if (! file_exists (front)) {
      if (! mkdir (front)) {
        error << std::string ("Unable to create directory: ") << front;
        return false;
      }
    }
  }

  return true;
}

bool rename_file (const std::string &path, const std::string &new_name)
{
  //  resolve relative names in new_name
  std::string new_path = new_name;
  if (! is_absolute (new_path)) {
    new_path = combine_path (dirname (path), new_name);
  }

  return rename (to_local (path).c_str (), to_local (new_path).c_str ()) == 0;
}

bool rm_file (const std::string &path)
{
  return unlink (to_local (path).c_str ()) == 0;
}

bool rm_dir (const std::string &path)
{
  return rmdir (to_local (path).c_str ()) == 0;
}

bool rm_dir_recursive (const std::string &p)
{
  std::vector<std::string> entries;
  std::string path = absolute_file_path (p);

  if (! file_exists (path)) {
    //  already gone.
    return true;
  }

  entries = dir_entries (path, false /*without_files*/, true /*with_dirs*/);
  for (std::vector<std::string>::const_iterator e = entries.begin (); e != entries.end (); ++e) {
    if (! rm_dir_recursive (combine_path (path, *e))) {
      return false;
    }
  }

  entries = dir_entries (path, true /*with_files*/, false /*without_dirs*/);
  for (std::vector<std::string>::const_iterator e = entries.begin (); e != entries.end (); ++e) {
    std::string tc = combine_path (path, *e);
    if (! rm_file (tc)) {
      error << std::string ("Unable to remove file: ") << tc;
      return false;
    }
  }

  if (! rm_dir (path)) {
    error << std::string ("Unable to remove directory: ") << path;
    return false;
  }

  return true;
}

bool
cp_dir_recursive (const std::string &source, const std::string &target)
{
  std::vector<std::string> entries;
  std::string path = absolute_file_path (source);
  std::string path_to = absolute_file_path (target);

  entries = dir_entries (path, false /*without_files*/, true /*with_dirs*/);
  for (std::vector<std::string>::const_iterator e = entries.begin (); e != entries.end (); ++e) {
    std::string tc = combine_path (path_to, *e);
    if (! mkpath (tc)) {
      error << std::string ("Unable to create target directory: ") << tc;
      return false;
    }
    if (! cp_dir_recursive (combine_path (path, *e), tc)) {
      return false;
    }
  }

  entries = dir_entries (path, true /*with_files*/, false /*without_dirs*/);
  for (std::vector<std::string>::const_iterator e = entries.begin (); e != entries.end (); ++e) {

    //  TODO: leave symlinks symlinks? How to copy symlinks with Qt?

    //  copy the files
    try {

      OutputFile os_file (combine_path (path_to, *e));
      OutputStream os (os_file);
      InputFile is_file (combine_path (path, *e));
      InputStream is (is_file);
      is.copy_to (os);

    } catch (Exception &ex) {
      error << std::string ("Unable to copy file ") << combine_path (path_to, *e) << std::string (" to ") << combine_path (path, *e)
                << std::string ("(Error ") << ex.msg () << ")";
      return false;
    }

  }

  return true;
}

std::string absolute_path (const std::string &s)
{
  std::vector<std::string> parts = split_path (absolute_file_path (s));
  if (parts.size () > 0) {
    parts.pop_back ();
  }

  return join (parts, "");
}

std::string current_dir ()
{
  char *cwd = getcwd (NULL, 0);
  if (cwd == NULL) {
    return std::string ();
  } else {
    std::string cwds (to_string_from_local (cwd));
    free (cwd);
    return cwds;
  }
}


static std::pair<std::string, bool> absolute_path_of_existing (const std::string &s)
{
  char *fp;
  fp = realpath (to_local (s).c_str (), NULL);
  if (fp == NULL) {
    return std::make_pair (std::string (), false);
  } else {
    std::string fps (to_string_from_local (fp));
    free (fp);
    return std::make_pair (fps, true);
  }
}

bool is_absolute (const std::string &s)
{
  std::vector<std::string> parts = split_path (s);
  if (parts.size () > 1 && is_drive (parts [0])) {
    return is_part_with_separator (parts [1]);
  } else if (! parts.empty ()) {
    return is_part_with_separator (parts.front ());
  } else {
    return false;
  }
}

std::string absolute_file_path (const std::string &s)
{
  std::vector<std::string> parts = split_path (s);
  if (parts.empty ()) {
    return current_dir ();
  }

  std::pair<std::string, bool> known_part;
  std::vector<std::string> unknown_parts;

  while (! parts.empty () && ! (parts.size () == 1 && is_drive (parts[0]))) {
    known_part = absolute_path_of_existing (join (parts, ""));
    if (! known_part.second) {
      unknown_parts.push_back (parts.back ());
      parts.pop_back ();
    } else {
      break;
    }
  }

  std::reverse (unknown_parts.begin (), unknown_parts.end ());

  if (! known_part.second) {

    //  the top-level component is unknown. This can mean:
    //  1.) the path is already absolute, but the top-level entry does not exist
    //  2.) the path is relative, but the entry does not exist

    lk_assert (! unknown_parts.empty ());
    if (is_part_with_separator (unknown_parts.front ())) {

      //  case 1: return the full path as absolute
      return s;

    } else if (parts.size () == 1 && is_drive (parts[0])) {

      //  case 2 (for Windows): try to root on drive's working dir
      known_part = absolute_path_of_existing (parts[0]);
      if (! known_part.second) {
        //  drive is not known ... return the original path as fallback
        return s;
      } else {
        return combine_path (known_part.first, join (unknown_parts, ""));
      }

    } else {

      //  case 2 (for *nix): try to root on current working dir
      return combine_path (current_dir (), join (unknown_parts, ""));

    }

  } else {
    return combine_path (known_part.first, join (unknown_parts, ""));
  }
}

typedef struct stat stat_struct;

static int stat_func (const std::string &s, stat_struct &st)
{
  return stat (to_local (s).c_str (), &st);
}

bool file_exists (const std::string &p)
{
  stat_struct st;
  return stat_func (p, st) == 0;
}

bool is_writable (const std::string &p)
{
  stat_struct st;
  return stat_func (p, st) == 0 && (st.st_mode & S_IWUSR) != 0;
}

bool is_readable (const std::string &p)
{
  stat_struct st;
  return stat_func (p, st) == 0 && (st.st_mode & S_IRUSR) != 0;
}

bool is_dir (const std::string &p)
{
  stat_struct st;
  if (stat_func (p, st) != 0) {
    return false;
  } else {
    return !S_ISREG (st.st_mode);
  }
}

std::string relative_path (const std::string &base, const std::string &p)
{
  std::vector<std::string> rem;
  std::vector<std::string> parts = split_path (p);

  while (! parts.empty ()) {

    if (is_same_file (base, join (parts, ""))) {

      //  combine the remaining path
      std::reverse (rem.begin (), rem.end ());
      if (! rem.empty ()) {
        rem[0] = trimmed_part (rem.front ());
      }
      return join (rem, "");

    }

    rem.push_back (parts.back ());
    parts.pop_back ();

  }

  return p;
}

bool is_same_file (const std::string &a, const std::string &b)
{
  if (normalize_path (a) == normalize_path (b)) {
    return true;
  }

  stat_struct sta, stb;
  if (stat_func (a, sta) != 0 || stat_func (b, stb) != 0) {
    return false;
  }

  return sta.st_dev == stb.st_dev && sta.st_ino == stb.st_ino;
}

static std::string
get_inst_path_internal ()
{
  std::string pf = sprintf ("/proc/%d/exe", getpid ());
  if (file_exists (pf)) {
    return absolute_path (pf);
  }

  lk_assert (false);
}

std::string
get_inst_path ()
{
  static std::string s_inst_path;
  if (s_inst_path.empty ()) {
    s_inst_path = get_inst_path_internal ();
  }
  return s_inst_path;
}

std::string
get_module_path (void *addr)
{
  Dl_info info = { };
  if (dladdr (addr, &info)) {
    return absolute_file_path (to_string_from_local (info.dli_fname));
  } else {
    warn << to_string (std::string ("Unable to get path of db library (as basis for loading db_plugins)"));
    return std::string ();
  }
}

} // namespace logkit
} // namespace toolkit
