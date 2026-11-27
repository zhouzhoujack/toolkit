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

bool mkdir (const std::string &path)
{
  return ::mkdir (to_local (path).c_str (), 0777) == 0;
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

bool is_dir (const std::string &p)
{
  stat_struct st;
  if (stat_func (p, st) != 0) {
    return false;
  } else {
    return !S_ISREG (st.st_mode);
  }
}

} // namespace logkit
} // namespace toolkit
