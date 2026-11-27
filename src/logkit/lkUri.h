#ifndef HDR_lkURI
#define HDR_lkURI

#include <string>
#include <map>

namespace toolkit {
namespace logkit {

/**
 *  @brief A class representing a URI
 *
 *  This class is able to parse the URI and deliver the parts.
 */

class URI
{
public:
  /**
   *  @brief Creates an empty URI
   */
  URI ();

  /**
   *  @brief Creates an URI from the given string
   */
  URI (const std::string &uri);

  /**
   *  @brief Returns the scheme part or an empty string if there is no scheme
   */
  const std::string &scheme () const
  {
    return m_scheme;
  }

  /**
   *  @brief Sets the scheme
   */
  void set_scheme (const std::string &s)
  {
    m_scheme = s;
  }

  /**
   *  @brief Returns the authority part or an empty string if there is no authority
   *  The leading slashes and not part of the authority.
   *  Percent escaping is undone already.
   */
  const std::string &authority () const
  {
    return m_authority;
  }

  /**
   *  @brief Sets the authority
   */
  void set_authority (const std::string &s)
  {
    m_authority = s;
  }

  /**
   *  @brief Returns the path part or an empty string if there is no path
   *  The path contains the leading slash if there is a path.
   *  Percent escaping is undone already.
   */
  const std::string &path () const
  {
    return m_path;
  }

  /**
   *  @brief Sets the path
   */
  void set_path (const std::string &s)
  {
    m_path = s;
  }

  /**
   *  @brief Returns the query part or an empty map if there is no query
   *  The map is a map of keys vs. values. Percent escaping is undone
   *  in the keys and values.
   */
  const std::map<std::string, std::string> &query () const
  {
    return m_query;
  }

  /**
   *  @brief Returns the query part or an empty map if there is no query (non-const version)
   */
  std::map<std::string, std::string> &query ()
  {
    return m_query;
  }

  /**
   *  @brief Returns the fragment or an empty string if there is none
   *  Percent escaping is undone on the fragment already.
   */
  const std::string &fragment () const
  {
    return m_fragment;
  }

  /**
   *  @brief Sets the fragment
   */
  void set_fragment (const std::string &s)
  {
    m_fragment = s;
  }

  /**
   *  @brief Turns the URI into a string
   *  Percent escaping is employed to escape special characters
   */
  std::string to_string () const;

  /**
   *  @brief Turns the URI into an "abstract path"
   *
   *  The "abstract path" is a concept provided by "InputStream".
   *  URIs with scheme "file", "http" and "https" are equivalent to their abstract path.
   *  URIs without a scheme turn into system file paths.
   *  Other schemes are not allowed.
   *
   *  Abstract paths are more powerful as they support pipes and Qt resource access.
   *  These modes are not supported by URIs.
   */
  std::string to_abstract_path () const;

  /**
   *  @brief Resolves an URI relative to this one
   */
  URI resolved (const URI &other) const;

private:
  std::string m_scheme;
  std::string m_authority;
  std::string m_path;
  std::map<std::string, std::string> m_query;
  std::string m_fragment;
};

} // namespace logkit
} // namespace toolkit

#endif

