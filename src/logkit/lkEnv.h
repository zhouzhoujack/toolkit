#ifndef HDR_lkAppFlags
#define HDR_lkAppFlags

#include <string>

namespace toolkit {
namespace logkit {

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

