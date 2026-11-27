
#ifndef HDR_lkString
#define HDR_lkString

#include "lkCommon.h"

#include <string>
#include <stdexcept>
#include <stdint.h>
#include <vector>

#include "lkException.h"

namespace toolkit {
namespace logkit {

/**
 *  @brief Another string class
 * 
 *  The purpose of this class is to provide a different string implementation
 *  that is optimized for special purposes. For example, the assign method
 *  and operator does reuse memory allocated so far. This way this object is
 *  better suited to string objects that frequently change.
 */

class string
{
public:
  typedef std::allocator<char> allocator_t;

  /**
   *  @brief The default constructor
   */
  string ()
    : m_size (0), m_capacity (0), mp_rep (0)
  {
    //  nothing yet ..
  }

  /**
   *  @brief The constructor from a const char *
   */
  string (const char *c);

  /**
   *  @brief The constructor from a const char * taking just a substring
   */
  string (const char *c, size_t from, size_t to);

  /**
   *  @brief The copy constructor
   */
  string (const string &s);

  /**
   *  @brief The copy constructor taking just a substring
   */
  string (const string &s, size_t from, size_t to);

  /**
   *  @brief The constructor creating a string from a STL string
   */
  string (const std::string &s);

  /**
   *  @brief The constructor creating a string from a STL string taking just a substring
   */
  string (const std::string &s, size_t from, size_t to);

  /**
   *  @brief Destructor
   */
  ~string ();

  /**
   *  @brief The assignment from a const char *
   */
  string &operator= (const char *c);

  /**
   *  @brief The assignment from a const char * taking just a substring
   */
  void assign (const char *c, size_t from, size_t to);

  /**
   *  @brief The assignment operator
   */
  string &operator= (const string &s);

  /**
   *  @brief The assignment taking just a substring
   */
  void assign (const string &s, size_t from, size_t to);

  /**
   *  @brief The assignment from a STL string
   */
  string &operator= (const std::string &s);

  /**
   *  @brief The assignment from a STL string taking just a substring
   */
  void assign (const std::string &s, size_t from, size_t to);

  /**
   *  @brief The length of the string
   */
  size_t size () const
  {
    return m_size;
  }

  /**
   *  @brief Reserve a certain number of characters
   */
  void reserve (size_t n);

  /**
   *  @brief The capacity of the string
   */
  size_t capacity () const
  {
    return m_capacity;
  }

  /**
   *  @brief Swap the string with another object
   */
  void swap (string &s)
  {
    std::swap (mp_rep, s.mp_rep);
    std::swap (m_size, s.m_size);
  }

  /**
   *  @brief Equality
   */
  bool operator== (const char *c) const;

  /**
   *  @brief Equality
   */
  bool operator== (const string &s) const;

  /**
   *  @brief Inequality
   */
  bool operator!= (const char *c) const;

  /**
   *  @brief Inequality
   */
  bool operator!= (const string &s) const;

  /**
   *  @brief Comparison operator
   */
  bool operator< (const char *c) const;
   
  /**
   *  @brief Comparison operator
   */
  bool operator< (const string &s) const;

  /**
   *  @brief Comparison operator
   */
  bool operator<= (const char *c) const;
   
  /**
   *  @brief Comparison operator
   */
  bool operator<= (const string &s) const;

  /**
   *  @brief Comparison operator
   */
  bool operator> (const char *c) const;
   
  /**
   *  @brief Comparison operator
   */
  bool operator> (const string &s) const;

  /**
   *  @brief Comparison operator
   */
  bool operator>= (const char *c) const;
   
  /**
   *  @brief Comparison operator
   */
  bool operator>= (const string &s) const;

  /**
   *  @brief Access to "C" representation
   */
  const char *c_str () const
  {
    return mp_rep == 0 ? "" : mp_rep;
  }
   
  /**
   *  @brief Access to STL representation
   */
  std::string std_str () const
  {
    if (mp_rep == 0) {
      return std::string ();
    } else {
      return std::string (mp_rep, 0, m_size);
    }
  }

  /**
   *  @brief Clear the string
   *
   *  Unlike assignment of an empty string, "clear" releases the memory allocated
   *  by this object.
   */
  void clear ();
   
private:
  size_t m_size;
  size_t m_capacity;
  char *mp_rep;
};

std::string to_upper_case (const std::string &s);
std::string to_lower_case (const std::string &s);

LK_EXPORT std::string to_string (double d, int prec);
LK_EXPORT std::string to_string (float d, int prec);
std::string to_string (const unsigned char *cp, int length);
std::string to_string (const char *cp, int length);
std::string to_string_from_local (const char *cp);
std::string to_local (const std::string &s);

//  system encoding <-> UTF-8 conversion (moved here from lkInternational)
std::string system_to_string (const std::string &s);
std::string string_to_system (const std::string &s);

template <class T> inline std::string to_string (const T &o) { return o.to_string (); }
template <> inline std::string to_string (const double &d) { return to_string (d, 12); }
template <> inline std::string to_string (const float &d) { return to_string (d, 6); }
template <> LK_EXPORT std::string to_string (const int &d);
template <> LK_EXPORT std::string to_string (const unsigned int &d);
template <> LK_EXPORT std::string to_string (const long &d);
template <> LK_EXPORT std::string to_string (const unsigned long &d);
template <> LK_EXPORT std::string to_string (const long long &d);
template <> LK_EXPORT std::string to_string (const unsigned long long &d);
template <> LK_EXPORT std::string to_string (const char * const &cp);
template <> LK_EXPORT std::string to_string (char * const &cp);
template <> LK_EXPORT std::string to_string (const unsigned char * const &cp);
template <> LK_EXPORT std::string to_string (unsigned char * const &cp);
template <> LK_EXPORT std::string to_string (const bool &b);
template <> inline std::string to_string (const std::string &s) { return s; }

/**
 *  @brief Converts UTF-8 to wide character string
 */
std::wstring to_wstring (const std::string &s);

/**
 *  @brief Converts a wide character string to UTF-8
 */
std::string to_string (const std::wstring &ws);

/**
 *  @brief Convert to a quoted string 
 *
 *  The string returned by this method can be read from a corresponding extractor with 
 *  the "read_quoted" method.
 */  
std::string to_quoted_string (const std::string &s);

/**
 *  @brief Escape special characters in a string
 *
 *  Special characters are replaced by escape sequences with a backslash.
 *  The format of the sequence is \xxx where x is the octal number of the 
 *  character and \r, \n and \t representing the CR, LF and TAB character.
 */
std::string escape_string (const std::string &value);

/**
 *  @brief Remove escape sequences from a string
 *
 *  Special characters are replaced by escape sequences with a backslash.
 *  The format of the sequence is \xxx where x is the octal number of the 
 *  character and \r, \n and \t representing the CR, LF and TAB character.
 */
std::string unescape_string (const std::string &value);

/**
 *  @brief Levenshtein distance
 *
 *  This function computes the edit distance ("Levenshtein distance") between two strings.
 */  
int edit_distance (const std::string &a, const std::string &b);

/**
 *  @brief Convert to a word or quoted string 
 *
 *  The string returned by this method can be read from a corresponding extractor with 
 *  the "read_word_or_quoted" method with the given "non_term" characters.
 */  
std::string to_word_or_quoted_string (const std::string &s, const char *non_term = "_.$");

/**
 *  @brief Escapes HTML (or XML) characters from in and adds the result to out
 *  If "replace_newlines" is true, "\n" will be replaced by "<br/>".
 */
void escape_to_html (std::string &out, const std::string &in, bool replace_newlines = true);

/**
 *  @brief Escapes HTML (or XML) characters from in and returns the resulting string
 *  Double quotes are substituted by "&quot;" which makes the resulting string usable for
 *  attributes (in double quotes) too.
 *  If "replace_newlines" is true, "\n" will be replaced by "<br/>".
 */
std::string escaped_to_html (const std::string &in, bool replace_newlines = true);

/**
 *  @brief Replaces the "before" string by "after" in the "subject" string and returns the new string
 *  All occurrences are replaced.
 */
std::string replaced (const std::string &subject, const std::string &before, const std::string &after);

/**
 *  @brief Replicates the given string n times
 */
std::string replicate (const std::string &s, unsigned int n);

/**
 *  @brief Fills the string (to the right) with blanks until the desired length is reached
 */
std::string pad_string_right (unsigned int n, const std::string &s);

/**
 *  @brief Fills the string (to the left) with blanks until the desired length is reached
 */
std::string pad_string_left (unsigned int n, const std::string &s);

/**
 *  @brief Set the number of digits resolution for a micron display
 */
void set_micron_resolution (unsigned int ndigits);

/**
 *  @brief A standard method to convert a micron value into a string for display
 */
std::string micron_to_string (double d);

/**
 *  @brief Set the number of digits resolution for a dbu display
 */
void set_db_resolution (unsigned int ndigits);

/**
 *  @brief A standard method to convert a coordinate in database units (pixel) into a string for display
 */
std::string db_to_string (double d);

/**
 *  @brief A standard method to convert a coordinate in database units (pixel) into a string for display
 */
inline std::string db_to_string (int32_t d) 
{
  return to_string (d);
}

/**
 *  @brief A standard method to convert a coordinate in database units (pixel) into a string for display
 */
inline std::string db_to_string (int64_t d) 
{
  return to_string (d);
}

void from_string (const std::string &s, const char * &result);
void from_string (const std::string &s, const unsigned char * &result);
void from_string (const std::string &s, double &v);
void from_string (const std::string &s, int &v);
void from_string (const std::string &s, long &v);
void from_string (const std::string &s, long long &v);
void from_string (const std::string &s, unsigned int &v);
void from_string (const std::string &s, unsigned long &v);
void from_string (const std::string &s, unsigned long long &v);
void from_string (const std::string &s, bool &b);

inline void from_string (const std::string &s, std::string &v) { v = s; }

std::string trim (const std::string &s);
std::vector<std::string> split (const std::string &s, const std::string &sep);
std::string join (const std::vector<std::string> &strings, const std::string &sep);

/**
 *  @brief Returns the lower-case character for a wchar_t
 */
wchar_t wdowncase (wchar_t c);

/**
 *  @brief Returns the upper-case character for a wchar_t
 */
wchar_t wupcase (wchar_t c);

/**
 *  @brief Returns the lower-case UTF32 character
 */
uint32_t utf32_downcase (uint32_t c32);

/**
 *  @brief Returns the upper-case UTF32 character
 */
uint32_t utf32_upcase (uint32_t c32);

/**
 *  @brief Parses the next UTF32 character from an UTF-8 string
 *  @param cp The input character's position, will be set to the next character.
 *  @param cpe The end of the string of 0 for "no end"
 */
uint32_t utf32_from_utf8 (const char *&cp, const char *cpe = 0);

} // namespace logkit
} // namespace toolkit

#endif
