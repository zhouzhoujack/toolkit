
#include "lkException.h"
#include "lkString.h"

namespace toolkit {
namespace logkit {

void
Exception::init (const std::string &fmt, const std::vector<Variant> &a)
{ 
  m_msg = sprintf (fmt, a);
}

} // namespace logkit
} // namespace toolkit

