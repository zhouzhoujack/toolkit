#include "lkEnv.h"

#include "lkString.h"

#include <string>

namespace toolkit {
namespace logkit {

std::string get_env (const std::string &name, const std::string &def_value)
{
  char *env = getenv (name.c_str ());
  if (env) {
    return system_to_string (env);
  } else {
    return def_value;
  }
}

bool has_env (const std::string &name)
{
  char *env = getenv (name.c_str ());
  return env != 0;
}

} // namespace logkit
} // namespace toolkit


