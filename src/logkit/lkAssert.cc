#include "lkAssert.h"

#include "lkException.h"
#include "lkGLog.h"
#include "lkInternational.h"

namespace toolkit {
namespace logkit {

namespace {

//  A special "internal" exception class used by lk_assert. Kept local to this
//  translation unit (anonymous namespace); it derives from the public Exception
//  so a throwing assert is still catchable as Exception&.
struct InternalException
  : public Exception
{
  InternalException (const char *file, int line, const char *cond)
    : Exception (tr ("Internal error: %s:%d %s was not true"), file, line, cond)
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

} // namespace logkit
} // namespace toolkit

