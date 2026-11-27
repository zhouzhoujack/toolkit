#include "lkHttpStream.h"

namespace toolkit {
namespace logkit {

// ---------------------------------------------------------------
//  HttpErrorException implementation

std::string
HttpErrorException::format_error (const std::string &em, int ec, const std::string &url, const std::string &body)
{
  std::string msg = sprintf (to_string (tr ("Error %d: %s, fetching %s")), ec, em, url);

  if (! body.empty ()) {

    msg += "\n\n";
    msg += to_string (tr ("Reply body:"));
    msg += "\n";

    if (body.size () > 1000) {
      msg += std::string (body.c_str (), 1000);
      msg += "...";
    } else {
      msg += body;
    }

  }

  return msg;
}

} // namespace logkit
} // namespace toolkit
