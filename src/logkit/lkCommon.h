#ifndef HDR_lkCommon_h
#define HDR_lkCommon_h

//  Linux-only build.
//
//  The library is compiled with -fvisibility=hidden (see src/CMakeLists.txt),
//  so only symbols explicitly marked LK_EXPORT are visible to consumers of
//  liblogkit.so; every other symbol (and any inline function) stays internal.
//
//  Mark ONLY the public API with LK_EXPORT:
//    - the channel classes / helpers in lkLog.h and their global instances
//    - Object (the polymorphic base class of Channel)
//    - the to_string() family reached from the "<<"/LOGI macros
//
//  LK_EXPORT is the ONLY export marker. The old TL_PUBLIC / TL_PUBLIC_TEMPLATE
//  no-op markers have been removed; internal declarations just carry no macro.
# if defined(__GNUC__) || defined(__clang__)
#  define LK_EXPORT __attribute__((visibility("default")))
# else
#  define LK_EXPORT
# endif


#endif
