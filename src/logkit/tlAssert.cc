#include "tlAssert.h"

#include "tlException.h"
#include "tlLog.h"

namespace toolkit { 

namespace logkit
{

//  to allow setting a debug breakpoint to this function, we declare it "C"
extern "C"
void _assertion_failed (const char *filename, unsigned int line, const char *condition)
{
  error << filename << "," << line << "," << condition; 
}

void 
assertion_failed (const char *filename, unsigned int line, const char *condition)
{
  _assertion_failed (filename, line, condition);
  throw InternalException (filename, line, condition); 
}

}   // namespace logkit

}   // namespace toolkit

