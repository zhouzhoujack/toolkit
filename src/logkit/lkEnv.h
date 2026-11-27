#ifndef HDR_lkAppFlags
#define HDR_lkAppFlags

#include "lkCommon.h"

#include <string>

namespace toolkit { namespace logkit
{

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

/**
 *  @brief Gets an application flag with the given name
 *
 *  This feature is supposed to deliver special flags for debugging etc.
 *  By using a central access point for these settings, it will be easy to provide
 *  different implementations later.
 *
 *  Currently, application flags are derived by checking whether a corresponding
 *  environment variable exists. Names like "a-b" are translated into "KLAYOUT_A_B"
 *  for the environment variable.
 *
 *  If the corresponding variable exists and does not have a value of "0", true
 *  is returned from this function.
 */
bool app_flag (const std::string &name);

} }

#endif

