#ifndef HDR_tlCommon_h
#define HDR_tlCommon_h

//  Linux-only build.
//
//  The library is compiled with -fvisibility=hidden (see src/CMakeLists.txt),
//  so only symbols explicitly marked TL_EXPORT are visible to consumers of
//  liblogkit.so; every other symbol (and any inline function) stays internal.
//
//  Mark ONLY the public API with TL_EXPORT:
//    - the channel classes / helpers in tlLog.h and their global instances
//    - Object (the polymorphic base class of Channel)
//    - the to_string() family reached from the "<<"/LOGI macros
//
//  TL_EXPORT is the ONLY export marker. The old TL_PUBLIC / TL_PUBLIC_TEMPLATE
//  no-op markers have been removed; internal declarations just carry no macro.
# if defined(__GNUC__) || defined(__clang__)
#  define TL_EXPORT __attribute__((visibility("default")))
# else
#  define TL_EXPORT
# endif


#endif
