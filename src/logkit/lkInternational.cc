#include "lkInternational.h"

#include "lkString.h"

#include <memory>
#include <iostream>
#include <locale.h>
#include <stdio.h>
#include <langinfo.h>

namespace toolkit { namespace logkit
{

std::string system_to_string (const std::string &s)
{
  return to_string_from_local (s.c_str ());
}

std::string string_to_system (const std::string &s)
{
  return to_local (s);
}

std::string tr (const char *s)
{
  //  TODO: this is a fallback implementation without translation
  return std::string (s);
}

} }
