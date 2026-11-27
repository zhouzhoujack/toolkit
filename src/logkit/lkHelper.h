#ifndef HDR_lkHelper
#define HDR_lkHelper

#include <string>

#include "lkCommon.h"

namespace toolkit {
namespace logkit {

// ---------------------------------------------------------------------------
// Assertions (formerly lkAssert.h)
// ---------------------------------------------------------------------------

/**
 *  @brief Reports an assertion failure and never returns
 *
 *  Under normal circumstances this logs the failure and throws an Exception
 *  derived class. Exported so that code using the lk_assert macro from outside
 *  the library can link.
 */
LK_EXPORT [[noreturn]] void assertion_failed (const char *filename, unsigned int line,
                                              const char *condition);

//  assertion_failed never returns, so lk_assert's body ends with the call
//  The name is fully qualified so the macro works from any namespace.
#define lk_assert(COND) if (!(COND)) { \
  ::toolkit::logkit::assertion_failed (__FILE__, __LINE__, #COND); \
}

// ---------------------------------------------------------------------------
// Environment variables (formerly lkEnv.h)
// ---------------------------------------------------------------------------

/**
 *  @brief Gets the value for the given environment variable
 *
 *  If the environment variable is not set, the default value will be returned.
 */
std::string get_env (const std::string &name, const std::string &def_value = std::string ());

/**
 *  @brief Gets the value if the given environment variable is set
 */
bool has_env (const std::string &name);

} // namespace logkit
} // namespace toolkit

#endif
