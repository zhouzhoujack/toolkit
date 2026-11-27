


#ifndef HDR_lkGlobPattern
#define HDR_lkGlobPattern

#include <string>
#include <vector>

#include "lkCommon.h"

namespace toolkit {
namespace logkit {

class GlobPatternOp;

/** 
 *  @brief A class representing a glob pattern 
 */

class GlobPattern
{
public:
  /**
   *  @brief The default constructor
   */
  GlobPattern ();

  /**
   *  @brief The constructor
   *
   *  Creates a glob pattern form the given string
   */
  GlobPattern (const std::string &p);

  /**
   *  @brief Copy constructor
   */
  GlobPattern (const GlobPattern &other);

  /**
   *  @brief Assignment
   */
  GlobPattern &operator= (const GlobPattern &other);

  /**
   *  @brief Assignment of a string
   */
  GlobPattern &operator= (const std::string &s);

  /**
   *  @brief Equality
   */
  bool operator== (const GlobPattern &other) const
  {
    return m_p == other.m_p;
  }

  /**
   *  @brief Less
   */
  bool operator< (const GlobPattern &other) const
  {
    return m_p < other.m_p;
  }

  /**
   *  @brief Pattern is empty
   */
  bool empty () const
  {
    return m_p.empty ();
  }

  /**
   *  @brief Sets a value indicating whether to treat the match case sensitive
   */
  void set_case_sensitive (bool f);

  /**
   *  @brief Gets a value indicating whether to treat the match case sensitive
   */
  bool case_sensitive () const;

  /**
   *  @brief Sets a value indicating whether to match exact (without brackets and wildcards)
   */
  void set_exact (bool f);

  /**
   *  @brief Gets a value indicating whether to match exact (without brackets and wildcards)
   */
  bool exact () const;

  /**
   *  @brief Sets a value indicating whether to allow trailing characters in the subject
   */
  void set_header_match (bool f);

  /**
   *  @brief Gets a value indicating whether to allow trailing characters in the subject
   */
  bool header_match () const;

  /**
   *  @brief Get the pattern string
   */
  const std::string &pattern () const
  {
    return m_p;
  }

  /**
   *  @brief Returns true, if the pattern is a catchall expression ("*")
   */
  bool is_catchall () const;

  /**
   *  @brief Returns true, if the pattern is a const string
   */
  bool is_const () const;

  /**
   *  @brief Match the given string 
   *
   *  Returns true, if the given subject string matches the glob pattern 
   */
  bool match (const std::string &s) const;

  /**
   *  @brief Match the given string and extract the bracket expressions into the vector of substrings
   */
  bool match (const std::string &s, std::vector<std::string> &e) const;

  /**
   *  @brief Match the given string 
   *
   *  Returns true, if the given subject string matches the glob pattern 
   */
  bool match (const char *s) const;

  /**
   *  @brief Match the given string and extract the bracket expressions into the vector of substrings
   */
  bool match (const char *s, std::vector<std::string> &e) const;

private:
  std::string m_p;
  GlobPatternOp *mp_op;
  bool m_case_sensitive;
  bool m_exact;
  bool m_header_match;
  bool m_needs_compile;

  void do_compile ();
  void needs_compile ();
  GlobPatternOp *op () const;
};

} // namespace logkit
} // namespace toolkit

#endif

