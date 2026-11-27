#ifndef HDR_lkThreads
#define HDR_lkThreads

//  atomics taken from https://github.com/mbitsnbites/atomic
#include "atomic/spinlock.h"

namespace toolkit {
namespace logkit {

/**
 *  @brief A mutex implementation
 */

class Mutex
{
public:
  Mutex () : m_spinlock () { }
  void lock() { m_spinlock.lock(); }
  void unlock() { m_spinlock.unlock(); }
private:
  atomic::spinlock m_spinlock;
};

/**
 *  @brief A RAII-based Mutex locker
 */
class MutexLocker
{
public:
  MutexLocker (Mutex *mutex)
    : mp_mutex (mutex)
  {
    mp_mutex->lock ();
  }

  ~MutexLocker ()
  {
    mp_mutex->unlock ();
  }

private:
  Mutex *mp_mutex;
};

} // namespace logkit
} // namespace toolkit

#endif
