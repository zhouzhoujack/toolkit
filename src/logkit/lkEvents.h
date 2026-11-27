#ifndef _HDR_lkEvents
#define _HDR_lkEvents

#include "lkObject.h"
#include "lkException.h"

#include <vector>
#include <algorithm>

namespace toolkit {
namespace logkit {

/**
 *  @brief A helper function to handle event exceptions of Exception type
 */
void handle_event_exception (Exception &ex);

/**
 *  @brief A helper function to handle event exceptions of std::exception type
 */
void handle_event_exception (std::exception &ex);

/**
 *  @brief A framework of observer and observables
 *
 *  This framework competes with the signal/event concept of Qt. It is provided because
 *  the latter has some weaknesses, namely:
 *
 *   - Signals/slots cannot be a defined for templates
 *   - The involved objects need to be derived from QObject
 *   - The Qt concept requires the meta object compiler
 *
 *  the framework provided herein is based on C++ meta programming alone and avoids
 *  some of these issues.
 *
 *  Events are used the following way:
 *   - Instantiate an event object inside the observable class
 *   - Expose this event object
 *   - Use add/remove to attach or detach receivers
 *
 *  @code
 *
 *  //  Object with event
 *  class Observed
 *  {
 *  public:
 *    event<Observed *, int> &event () { return m_event; }
 *    void trigger_event ()
 *    {
 *      //  issue event
 *      m_event (this, -1);
 *    }
 *  private:
 *    event<Observed *, int> m_event;
 *  };
 *
 *  //  Observer
 *  class Observer : public Object
 *  {
 *  public:
 *    void receives_event (Observed *, int) { ... }
 *  };
 *
 *  //  Connect this
 *  Observed x;
 *  Observer y;
 *  x.event ().add (&y, &Observed::receives_event);
 *  x.trigger_event ();
 *
 *  @endcode
 *
 *  Events can be given a disambiguation or client data parameter:
 *
 *  @code
 *
 *  //  Observer
 *  class Observer : public Object
 *  {
 *  public:
 *    //  The first parameter is the client data parameter which is given as
 *    //  third parameter in "add"
 *    void receives_event (int, Observed *, int) { ... }
 *  };
 *
 *  //  Connect this
 *  Observed x;
 *  Observer y;
 *  x.event ().add (&y, &Observed::receives_event, 1);
 *  x.trigger_event ();
 *
 *  @endcode
 */

template <class A1 = void, class A2 = void, class A3 = void, class A4 = void, class A5 = void> class event_function_base;
template <class T, class A1 = void, class A2 = void, class A3 = void, class A4 = void, class A5 = void> class event_function;
template <class T, class D, class A1 = void, class A2 = void, class A3 = void, class A4 = void, class A5 = void> class event_function_with_data;
template <class T, class A1 = void, class A2 = void, class A3 = void, class A4 = void, class A5 = void> class generic_event_function;
template <class T, class D, class A1 = void, class A2 = void, class A3 = void, class A4 = void, class A5 = void> class generic_event_function_with_data;
template <class A1 = void, class A2 = void, class A3 = void, class A4 = void, class A5 = void> class event;
typedef event<> Event;

#define  _COUNT 0
#define _TMPLARGS
#define _TMPLARGLIST
#define _TMPLARGLISTP void, void, void, void, void
#define _CALLARGS
#define _CALLARGLIST
#define _CALLARGPTRS  0

#include "lkEventsVar.h"

#undef _VOIDLIST
#undef _CALLARGPTRS
#undef _CALLARGLIST
#undef _CALLARGS
#undef _TMPLARGLISTP
#undef _TMPLARGLIST
#undef _TMPLARGS
#undef _COUNT

#define  _COUNT 1
#define _TMPLARGS     class A1
#define _TMPLARGLIST  A1
#define _TMPLARGLISTP A1, void, void, void, void
#define _CALLARGS     a1
#define _CALLARGLIST  A1 a1
#define _CALLARGPTRS  (void *)&a1

#include "lkEventsVar.h"

#undef _VOIDLIST
#undef _CALLARGPTRS
#undef _CALLARGLIST
#undef _CALLARGS
#undef _TMPLARGLISTP
#undef _TMPLARGLIST
#undef _TMPLARGS
#undef _COUNT

#define  _COUNT 2
#define _TMPLARGS     class A1, class A2
#define _TMPLARGLIST  A1, A2
#define _TMPLARGLISTP A1, A2, void, void, void
#define _CALLARGS     a1, a2
#define _CALLARGLIST  A1 a1, A2 a2
#define _VOIDLIST     void, void, void
#define _CALLARGPTRS  (void *)&a1, (void *)&a2

#include "lkEventsVar.h"

#undef _VOIDLIST
#undef _CALLARGPTRS
#undef _CALLARGLIST
#undef _CALLARGS
#undef _TMPLARGLISTP
#undef _TMPLARGLIST
#undef _TMPLARGS
#undef _COUNT

#define  _COUNT 3
#define _TMPLARGS     class A1, class A2, class A3
#define _TMPLARGLIST  A1, A2, A3
#define _TMPLARGLISTP A1, A2, A3, void, void
#define _CALLARGS     a1, a2, a3
#define _CALLARGLIST  A1 a1, A2 a2, A3 a3
#define _VOIDLIST     void, void
#define _CALLARGPTRS  (void *)&a1, (void *)&a2, (void *)&a3

#include "lkEventsVar.h"

#undef _VOIDLIST
#undef _CALLARGPTRS
#undef _CALLARGLIST
#undef _CALLARGS
#undef _TMPLARGLISTP
#undef _TMPLARGLIST
#undef _TMPLARGS
#undef _COUNT

#define  _COUNT 4
#define _TMPLARGS     class A1, class A2, class A3, class A4
#define _TMPLARGLIST  A1, A2, A3, A4
#define _TMPLARGLISTP A1, A2, A3, A4, void
#define _CALLARGS     a1, a2, a3, a4
#define _CALLARGLIST  A1 a1, A2 a2, A3 a3, A4 a4
#define _VOIDLIST     void
#define _CALLARGPTRS  (void *)&a1, (void *)&a2, (void *)&a3, (void *)&a4

#include "lkEventsVar.h"

#undef _VOIDLIST
#undef _CALLARGPTRS
#undef _CALLARGLIST
#undef _CALLARGS
#undef _TMPLARGLISTP
#undef _TMPLARGLIST
#undef _TMPLARGS
#undef _COUNT

} // namespace logkit
} // namespace toolkit

#endif
