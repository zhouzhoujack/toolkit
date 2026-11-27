#ifndef HDR_lkException
#define HDR_lkException

#include <exception>

#include "lkVariant.h"

namespace toolkit {
namespace logkit {

/**
 *  @brief The unspecific exception class
 *
 *  This class is the base class for all exceptions in this 
 *  framework. It does not carry further information except 
 *  a message string that can be created through different 
 *  constructor methods.
 */

class Exception : public std::exception 
{
public:
  Exception (const std::string &msg)
    : m_msg (msg)
  { }

  Exception (const std::string &fmt, const std::vector<Variant> &a)
  {
    init (fmt, a);
  }

  Exception (const std::string &fmt, const Variant &a1)
  {
    std::vector<Variant> a;
    a.push_back (a1);
    init (fmt, a);
  }

  Exception (const std::string &fmt, const Variant &a1, const Variant &a2)
  {
    std::vector<Variant> a;
    a.push_back (a1);
    a.push_back (a2);
    init (fmt, a);
  }

  Exception (const std::string &fmt, const Variant &a1, const Variant &a2, const Variant &a3)
  {
    std::vector<Variant> a;
    a.push_back (a1);
    a.push_back (a2);
    a.push_back (a3);
    init (fmt, a);
  }

  Exception (const std::string &fmt, const Variant &a1, const Variant &a2, const Variant &a3, const Variant &a4)
  {
    std::vector<Variant> a;
    a.push_back (a1);
    a.push_back (a2);
    a.push_back (a3);
    a.push_back (a4);
    init (fmt, a);
  }

  Exception (const std::string &fmt, const Variant &a1, const Variant &a2, const Variant &a3, const Variant &a4, const Variant &a5)
  {
    std::vector<Variant> a;
    a.push_back (a1);
    a.push_back (a2);
    a.push_back (a3);
    a.push_back (a4);
    a.push_back (a5);
    init (fmt, a);
  }

  Exception (const std::string &fmt, const Variant &a1, const Variant &a2, const Variant &a3, const Variant &a4, const Variant &a5, const Variant &a6)
  {
    std::vector<Variant> a;
    a.push_back (a1);
    a.push_back (a2);
    a.push_back (a3);
    a.push_back (a4);
    a.push_back (a5);
    a.push_back (a6);
    init (fmt, a);
  }

  Exception (const std::string &fmt, const Variant &a1, const Variant &a2, const Variant &a3, const Variant &a4, const Variant &a5, const Variant &a6, const Variant &a7)
  {
    std::vector<Variant> a;
    a.push_back (a1);
    a.push_back (a2);
    a.push_back (a3);
    a.push_back (a4);
    a.push_back (a5);
    a.push_back (a6);
    a.push_back (a7);
    init (fmt, a);
  }

  virtual ~Exception () { }

  virtual std::string msg () const { return m_msg; }

  const char *what () const noexcept override { return m_msg.c_str (); }

private:
  std::string m_msg;
  void init (const std::string &fmt, const std::vector<Variant> &a);
};

} // namespace logkit
} // namespace toolkit

#endif

