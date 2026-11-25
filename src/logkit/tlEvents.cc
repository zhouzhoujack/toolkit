#include "tlEvents.h"

#include "tlLog.h"
#include "tlCommon.h"

namespace toolkit { namespace logkit
{

void handle_event_exception (Exception &ex)
{
  //  Exceptions have unpredictable side effects for events so we just log
  //  them here.
  warn << ex.msg ();
}

void handle_event_exception (std::exception &ex)
{
  //  Exceptions have unpredictable side effects for events so we just log
  //  them here.
  warn << ex.what ();
}

} }
