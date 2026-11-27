#include "lkHelper.h"

#include "lkException.h"
#include "lkGLog.h"
#include "lkString.h"

namespace toolkit {
namespace logkit {

// ---------------------------------------------------------------------------
// Assertions (formerly lkAssert.cc)
// ---------------------------------------------------------------------------

namespace {

//  A special "internal" exception class used by lk_assert. Kept local to this
//  translation unit (anonymous namespace); it derives from the public Exception
//  so a throwing assert is still catchable as Exception&.
struct InternalException
  : public Exception
{
  InternalException (const char *file, int line, const char *cond)
    : Exception (std::string ("Internal error: %s:%d %s was not true"), file, line, cond)
  { }
};

} // anonymous namespace

//  to allow setting a debug breakpoint to this function, we declare it "C"
extern "C"
void _assertion_failed (const char *filename, unsigned int line, const char *condition)
{
  LOGE << filename << "," << line << "," << condition;
}

void
assertion_failed (const char *filename, unsigned int line, const char *condition)
{
  _assertion_failed (filename, line, condition);
  throw InternalException (filename, line, condition);
}

// ---------------------------------------------------------------------------
// Environment variables (formerly lkEnv.cc)
// ---------------------------------------------------------------------------

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
