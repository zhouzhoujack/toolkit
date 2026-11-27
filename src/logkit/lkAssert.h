#ifndef HDR_lkAssert
#define HDR_lkAssert

namespace toolkit {
namespace logkit {

[[noreturn]] void assertion_failed (const char *filename, unsigned int line, const char *condition);

//  the throw int(0) instruction will tell the compiler that the assertion will not return
#define lk_assert(COND) if (!(COND)) { \
  assertion_failed (__FILE__, __LINE__, #COND); \
}

} // namespace logkit
} // namespace toolkit

#endif

