#ifndef HDR_tlInternational
#define HDR_tlInternational

#include "tlCommon.h"

#include <string>

namespace toolkit { namespace logkit
{

/**
 *  @brief Generic tr function
 */
std::string TL_PUBLIC tr (const char *s);

/**
 *  @brief Dummy conversion for convenience
 */
inline const std::string &to_string (const std::string &s)
{
  return s;
}

/**
 *  @brief Convert a system encoding std::string to a UTF8 std::string
 */
TL_PUBLIC std::string system_to_string (const std::string &s);

/**
 *  @brief Convert a UTF8 string to a system encoding string
 */
TL_PUBLIC std::string string_to_system (const std::string &s);

/**
 *  @brief Initialize the codecs
 */
TL_PUBLIC void initialize_codecs ();

} }

#endif

