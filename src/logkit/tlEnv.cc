
/*

  KLayout Layout Viewer
  Copyright (C) 2006-2021 Matthias Koefferlein

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/


#include "tlEnv.h"
#include "tlString.h"

#include <string>

#include <unistd.h>

namespace toolkit { namespace logkit
{

std::string get_env (const std::string &name, const std::string &def_value)
{
  char *env = getenv (name.c_str ());
  if (env) {
    return system_to_string (env);
  } else {
    return def_value;
  }
}

bool has_env (const std::string &name)
{
  char *env = getenv (name.c_str ());
  return env != 0;
}

bool app_flag (const std::string &name)
{
  std::string env_name = std::string ("KLAYOUT_") + replaced (to_upper_case (name), "-", "_");

  int v = 0;
  std::string vs = get_env (env_name);
  Extractor ex (vs.c_str ());
  return ex.try_read (v) && v != 0;
}

} } // namespace toolkit::logkit


