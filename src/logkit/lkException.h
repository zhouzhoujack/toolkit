#ifndef HDR_lkException
#define HDR_lkException

#include <exception>
#include <string>

namespace toolkit {
namespace logkit {

/**
 *  @brief The unspecific exception class
 *
 *  This class is the base for all exceptions in this framework. It carries
 *  nothing but a message string.
 */

class Exception : public std::exception
{
public:
  Exception (const std::string &msg)
    : m_msg (msg)
  { }

  virtual ~Exception () { }

  virtual std::string msg () const { return m_msg; }

  const char *what () const noexcept override { return m_msg.c_str (); }

private:
  std::string m_msg;
};

} // namespace logkit
} // namespace toolkit

#endif
