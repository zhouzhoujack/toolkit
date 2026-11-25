
/*

  KLayout Layout Viewer
  Copyright (C) 2006-2021 Matthias Koefferlein

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/



#ifndef HDR_tlException
#define HDR_tlException

#include "tlCommon.h"

#include "tlInternational.h"
#include "tlVariant.h"

namespace toolkit { namespace logkit
{

/**
 *  @brief The unspecific exception class
 *
 *  This class is the base class for all exceptions in this 
 *  framework. It does not carry further information except 
 *  a message string that can be created through different 
 *  constructor methods.
 */

class TL_PUBLIC Exception 
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

private:
  std::string m_msg;
  void init (const std::string &fmt, const std::vector<Variant> &a);
};

/**
 *  @brief A "neutral" exception thrown to terminate some operation
 *  This exception is not shown.
 */
struct TL_PUBLIC CancelException
  : public Exception
{
  CancelException ()
    : Exception (to_string (tr ("Operation cancelled")))
  { }
};

/**
 *  @brief A special "internal" exception class used by tl_assert
 */
struct TL_PUBLIC InternalException
  : public Exception
{
  InternalException (const char *file, int line, const char *cond)
    : Exception (to_string (tr ("Internal error: %s:%d %s was not true")).c_str (), file, line, cond)
  { }
};

} } // namespace toolkit::logkit

#endif

