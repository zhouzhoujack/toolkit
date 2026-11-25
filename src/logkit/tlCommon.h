#if !defined(HDR_tlCommon_h)
# define HDR_tlCommon_h

//  Linux-only build: symbols use the default visibility, so the
//  dllexport/dllimport and visibility attribute machinery is gone.
//  TL_PUBLIC / TL_PUBLIC_TEMPLATE are kept as empty markers for the existing
//  declarations.
# define TL_PUBLIC
# define TL_PUBLIC_TEMPLATE


#endif
