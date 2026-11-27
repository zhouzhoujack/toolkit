#include "lkEvents.h"

#include "lkLog.h"
#include "lkCommon.h"

namespace toolkit {
namespace logkit {

void handle_event_exception (std::exception &ex)
{
  //  Exceptions have unpredictable side effects for events so we just log
  //  them here.
  warn << ex.what ();
}

} // namespace logkit
} // namespace toolkit
