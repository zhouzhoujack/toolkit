#ifndef HDR_lkInternational
#define HDR_lkInternational

#include <string>

namespace toolkit {
namespace logkit {

/**
 *  @brief Convert a system encoding std::string to a UTF8 std::string
 */
std::string system_to_string (const std::string &s);

/**
 *  @brief Convert a UTF8 string to a system encoding string
 */
std::string string_to_system (const std::string &s);

} // namespace logkit
} // namespace toolkit

#endif

