#ifndef HDR_tlThreads
#define HDR_tlThreads

#include "tlCommon.h"

#include <limits>

//  atomics taken from https://github.com/mbitsnbites/atomic
#include "atomic/spinlock.h"

namespace toolkit { namespace logkit
{

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
 *  @brief A wait condition implementation
 */

class WaitConditionPrivate;

class WaitCondition
{
public:
  WaitCondition ();
  ~WaitCondition ();
  bool wait (Mutex * /*mutex*/, unsigned long /*time*/ = std::numeric_limits<unsigned long>::max ());
  void wakeAll ();
  void wakeOne ();

private:
  WaitConditionPrivate *mp_data;
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

/**
 *  @brief A thread implementation
 */

class ThreadPrivateData;

class Thread
{
public:
  Thread ();
  virtual ~Thread ();

  void exit (int /*returnCode*/ = 0);
  bool isFinished () const;
  bool isRunning () const;
  void quit ();
  void start ();
  void terminate ();
  bool wait (unsigned long /*time*/ = std::numeric_limits<unsigned long>::max ());

protected:
  virtual void run () { }

private:
  friend void *start_thread (void *);
  ThreadPrivateData *mp_data;
  void do_run ();
};

/**
 *  @brief A thread local storage implementation
 */

class ThreadStorageHolderBase
{
public:
  ThreadStorageHolderBase (void *obj) : mp_obj (obj) { }
  virtual ~ThreadStorageHolderBase () { }

protected:
  void *obj () { return mp_obj; }
  void *mp_obj;
};

template <class T>
class ThreadStorageHolder
  : public ThreadStorageHolderBase
{
public:
  ThreadStorageHolder (T *t) : ThreadStorageHolderBase ((void *) t) { }
  ~ThreadStorageHolder () { delete data (); }
  T *data () { return (T *) obj (); }
};

class ThreadStorageBase
{
public:
  ThreadStorageBase ();

protected:
  void add (ThreadStorageHolderBase *holder);
  ThreadStorageHolderBase *holder ();

  const ThreadStorageHolderBase *holder () const
  {
    return (const_cast<ThreadStorageBase *> (this)->holder ());
  }
};

//  TODO: this is the non-threaded dummy implementation
template <class T>
class ThreadStorage
  : public ThreadStorageBase
{
public:
  ThreadStorage () : ThreadStorageBase () { }

  bool hasLocalData () const
  {
    return dynamic_cast<const ThreadStorageHolder<T> *> (holder ()) != 0;
  }

  T &localData ()
  {
    return *(dynamic_cast<ThreadStorageHolder<T> *> (holder ())->data ());
  }

  T localData () const
  {
    return *(dynamic_cast<const ThreadStorageHolder<T> *> (holder ())->data ());
  }

  void setLocalData (const T &data)
  {
    add (new ThreadStorageHolder<T> (new T (data)));
  }
};

} }

#endif
