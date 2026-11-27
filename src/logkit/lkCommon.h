#ifndef HDR_lkCommon_h
#define HDR_lkCommon_h


# if defined(__GNUC__) || defined(__clang__)
#  define LK_EXPORT __attribute__((visibility("default")))
# else
#  define LK_EXPORT
# endif


#endif
