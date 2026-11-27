#ifndef HDR_lkFileUtils
#define HDR_lkFileUtils

#include "lkCommon.h"
#include "lkString.h"

namespace toolkit {
namespace logkit {

/**
 *  @brief Returns a value indicating whether the parent path is a parent directory of the path (version with std::string)
 */
bool is_parent_path (const std::string &parent, const std::string &path);

/**
 *  @brief Returns true if s is an absolute path
 */
bool is_absolute (const std::string &s);

/**
 *  @brief Recursively remove the given directory, the files from that directory and all sub-directories (version with std::string)
 *  @return True, if successful. false otherwise.
 */
bool rm_dir_recursive (const std::string &path);

/**
 *  @brief Creates the given path as far it does not exist yet
 *  @return True, if successful. false otherwise.
 */
bool mkpath (const std::string &path);

/**
 *  @brief Recursively remove the given directory, the files from that directory and all sub-directories (version with std::string)
 *  @return True, if successful. false otherwise.
 */
bool cp_dir_recursive (const std::string &source, const std::string &target);

/**
 *  @brief Gets the absolute path for a given file path
 *  This will deliver the directory of the file as absolute path.
 */
std::string absolute_path (const std::string &s);

/**
 *  @brief Gets the absolute file path for a given file path
 *  This will deliver the directory and the file part as absolute path.
 */
std::string absolute_file_path (const std::string &s);

/**
 *  @brief Gets the directory name for a given file path
 */
std::string dirname (const std::string &s);

/**
 *  @brief Gets the filename for a given file path (file name without directory part)
 */
std::string filename (const std::string &s);

/**
 *  @brief Gets the basename for a given file path (file name without any extensions)
 *  This will strip all extensions (i.e. "archive.tar.gz" will become "archive").
 */
std::string basename (const std::string &s);

/**
 *  @brief Gets the complete extension for a given file path
 */
std::string extension (const std::string &s);

/**
 *  @brief Gets the last extension for a given file path
 */
std::string extension_last (const std::string &s);

/**
 *  @brief Returns true, if the given path exists
 *  If the path is a directory, file_exists will return true, if the directory exists.
 */
bool file_exists (const std::string &s);

/**
 *  @brief Returns true, if the given path is writable
 */
bool is_writable (const std::string &s);

/**
 *  @brief Returns true, if the given path is readable
 */
bool is_readable (const std::string &s);

/**
 *  @brief Returns true, if the given path is a directory
 */
bool is_dir (const std::string &s);

/**
 *  @brief Gets the directory entries for the given directory
 *  This method will NEVER return the ".." entry.
 *  @param with_files Includes all files
 *  @param with_dirs Include all directories
 *  @param without_dotfiles Exclude all ".*" files
 */
std::vector<std::string> dir_entries (const std::string &s, bool with_files = true, bool with_dirs = true, bool without_dotfiles = false);

/**
 *  @brief Rename the given file
 */
bool rename_file (const std::string &path, const std::string &new_name);

/**
 *  @brief Removes the given file and returns true on success
 */
bool rm_file (const std::string &path);

/**
 *  @brief Removes the given directory and returns true on success
 */
bool rm_dir (const std::string &path);

/**
 *  @brief Returns true, if the given path is the same directory of file than the other one
 */
bool is_same_file (const std::string &a, const std::string &b);

/**
 *  @brief Gets the relative path of p vs. base
 *  If p cannot be resolved relative to base, p is returned.
 */
std::string relative_path (const std::string &base, const std::string &p);

/**
 *  @brief Normalizes the path
 *  This function will remove duplicate "/" or "\" and strip any trailing
 *  "/" or "\".
 */
std::string normalize_path (const std::string &s);

/**
 *  @brief Combines the two path components into one path
 *  If "always_join" is true, the path is also built if p2 is empty. This will
 *  essentially add a slash or backslash to p1.
 */
std::string combine_path (const std::string &p1, const std::string &p2, bool always_join = false);

/**
 *  @brief Gets the current directory
 */
std::string current_dir ();

/**
 *  @brief This function splits the path into it's components
 *  On Windows, the first component may be the drive prefix ("C:") or
 *  UNC server name ("\\server").
 *  The components will keep their path separator, so joining the
 *  parts will render the original path. A trailing empty element is
 *  added if the path terminates with a separator (like "C:\" or "/home/user/").
 *  The idea is that the last element is the file name part.
 *  If "keep_last" is true, the last part will be kept even if it's empty.
 *  With this, a path like "/hello/" becomes "/hello"+"/".
 */
std::vector<std::string> split_path (const std::string &p, bool keep_last = false);

/**
 *  @brief Gets the path of the currently running process
 */
std::string get_inst_path ();

/**
 *  @brief Gets the absolute path of the module (DLL/.so) which contains the given address
 *  "address" is supposed to be the address of a function inside the module.
 */
std::string get_module_path (void *addr);

/**
 *  @brief Gets the line separator (CRLF on windows, LF on linux)
 */
const char *line_separator ();

} // namespace logkit
} // namespace toolkit

#endif
