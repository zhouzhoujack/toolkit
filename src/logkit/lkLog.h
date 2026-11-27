
#ifndef HDR_lkLog
#define HDR_lkLog

#include "lkHelper.h"

#include "lkString.h"
#include "lkThreads.h"
#include "lkObjectCollection.h"

namespace toolkit {
namespace logkit {

/**
 *  @brief Set the verbosity level
 *
 *  Predefined levels are:
 *    0: none
 *    10: basic
 *    11: basic timing
 *    20: detailed 
 *    21: detailed timing
 *    30: verbose
 *    31: verbose timing
 *    100+: very verbose 
 *
 *  The initial level is 10; KLAYOUT_VERBOSITY overrides it.
 */
LK_EXPORT void verbosity (int level);

/**
 *  @brief Get the verbosity level
 */
LK_EXPORT int verbosity ();

/**
 *  @brief A "endl" tag class
 *
 *  This class is supposed to issue a end-of-line.
 */

struct LK_EXPORT ChannelEndl 
{
  //  just a "tag"
};

extern LK_EXPORT ChannelEndl endl;

/**
 *  @brief A "noendl" tag class
 *
 *  This class is supposed to suppress the implicit end-of-line.
 */

struct LK_EXPORT ChannelNoendl 
{
  //  just a "tag"
};

extern LK_EXPORT ChannelNoendl noendl;


class LK_EXPORT ChannelProxy;

/**
 *  @brief A basic channel
 *
 *  Channels are supposed to be derived by subclasses providing 
 *  a special implementation for the channels.
 */

class LK_EXPORT Channel
  : public Object
{
public:
  /**
   *  @brief Construct a channel 
   */
  Channel ();

  /**
   *  @brief Destructor
   */
  virtual ~Channel ();

  /**
   *  @brief Output "something"
   *  
   *  A proxy object to the original channel is returned that does
   *  locking of the channel and reference counting such that the
   *  channel is freed again once it is no longer used.
   */
  template <class T> 
  ChannelProxy operator<< (const T &t);

  /**
   *  @brief Output a const char *
   *
   *  For the return object see the generic operator<<
   */
  ChannelProxy operator<< (const char *s);

  /**
   *  @brief A end-of-line output
   */
  ChannelProxy operator<< (ChannelEndl);
  
  /**
   *  @brief Suppress the implicit end of line at the end
   */
  ChannelProxy operator<< (ChannelNoendl);
  
protected:
  //  this is the interface implemented by the subclasses
  virtual void puts (const char *s) = 0;
  virtual void endl () = 0;
  virtual void end () = 0;
  virtual void begin () = 0;
  virtual void yield () = 0;

  /**
   *  @brief Returns true if this "puts" starts a new physical line
   *
   *  Used by the channels to write the level tag ("[warn]" etc.) once per
   *  line only -- not again for continuation lines produced with noendl.
   */
  bool take_line_start ()
  {
    bool at_start = m_at_line_start;
    m_at_line_start = false;
    return at_start;
  }

  /**
   *  @brief Marks the beginning of a new physical line
   */
  virtual void mark_line_end ()
  {
    m_at_line_start = true;
  }

  Mutex m_lock;

private:
  friend class ChannelProxy;
  friend class LogTee;
  friend class LogAt;

  ChannelProxy issue_proxy ();
  void release_proxy ();

  void noendl (bool flag)
  {
    m_no_endl = flag;
  }

  bool m_no_endl;
  bool m_active;
  bool m_in_yield;
  bool m_at_line_start;
};

/**
 *  @brief A channel proxy
 *
 *  The proxy objects are used to control when the channel is to
 *  be released.
 */

class LK_EXPORT ChannelProxy
{
public:
  /**
   *  @brief Construct a channel proxy to a channel
   */
  ChannelProxy (Channel *channel);

  /**
   *  @brief Destructor
   */
  ~ChannelProxy ();

  /**
   *  @brief Output "something"
   */
  template <class T> 
  ChannelProxy &operator<< (const T &t)
  {
    mp_channel->puts (to_string (t).c_str ());
    return *this;
  }

  /**
   *  @brief Output a const char *
   */
  ChannelProxy &operator<< (const char *s)
  {
    mp_channel->puts (s);
    return *this;
  }

  /**
   *  @brief A end-of-line output
   */
  ChannelProxy &operator<< (ChannelEndl)
  {
    mp_channel->endl ();
    mp_channel->mark_line_end ();
    return *this;
  }
  
  /**
   *  @brief A end-of-line output
   */
  ChannelProxy &operator<< (ChannelNoendl)
  {
    mp_channel->noendl (true);
    return *this;
  }
  
private:
  Channel *mp_channel; 

  //  copying only by the Channel class. This one knows what it does
  friend class Channel;
  friend class LogAt;
  ChannelProxy &operator= (const ChannelProxy &);
  ChannelProxy (const ChannelProxy &);
};

template <class T> 
inline ChannelProxy 
Channel::operator<< (const T &t)
{
  ChannelProxy p = issue_proxy ();
  puts (to_string (t).c_str ());
  return p;
}

inline ChannelProxy 
Channel::operator<< (const char *s)
{
  ChannelProxy p = issue_proxy ();
  puts (s);
  return p;
}

inline ChannelProxy 
Channel::operator<< (ChannelEndl)
{
  ChannelProxy p = issue_proxy ();
  endl ();
  return p;
}

/**
 *  @brief A log target that prefixes the message with time and source location
 *
 *  This is the generic form: channel and source location are constructor
 *  arguments.  The level-bound classes below (LogInfo, LogWarn, ...) embed the
 *  channel in the type, so a call site only passes the location:
 *
 *      #define LOGI LogInfo (__FILE__, __LINE__)
 *
 *  The output looks like
 *
 *      [HH:MM:SS.mmm file.cc:123] message
 *
 *  A direct "info << ..." carries no source location and is written without
 *  a prefix.
 */

class LK_EXPORT LogAt
{
public:
  LogAt (Channel &channel, const char *file, int line);

  template <class T>
  LogAt &operator<< (const T &t)
  {
    m_proxy << t;
    return *this;
  }

  LogAt &operator<< (const char *s)
  {
    m_proxy << s;
    return *this;
  }

  LogAt &operator<< (ChannelEndl e)
  {
    m_proxy << e;
    return *this;
  }

  LogAt &operator<< (ChannelNoendl e)
  {
    m_proxy << e;
    return *this;
  }

private:
  ChannelProxy m_proxy;
};

/**
 *  @brief A multi-cast log distribution object
 */

class LK_EXPORT LogTee : public Channel
{
public:
  LogTee ();
  LogTee (Channel *first, bool owned);

  void add (Channel *other, bool owned);
  void prepend (Channel *other, bool owned);
  void clear ();

protected:
  virtual void puts (const char *s);
  virtual void endl ();
  virtual void end ();
  virtual void begin ();
  virtual void yield ();
  virtual void mark_line_end ();

private:
  weak_collection<Channel> m_channels;
  shared_collection<Channel> m_owned_channels;
};

/// The static instance of the log channel
/// The log channel is identical to the info channel but is silent depending on the verbosity and 
/// the output mode. It should be used for general notifications like the beginning of a operation.
extern LK_EXPORT LogTee log;

/// The static instance of the info channel
extern LK_EXPORT LogTee info;

/// The static instance of the warning channel
extern LK_EXPORT LogTee warn;

/// The static instance of the error channel
extern LK_EXPORT LogTee error;

/// The static instance of the fatal channel
/// Messages go to stderr with a "FATAL: " prefix; the process is aborted once the
/// message has been flushed (i.e. at the end of the statement).
extern LK_EXPORT LogTee fatal;

/**
 *  @brief Level-bound log targets
 *
 *  Each class embeds the channel it writes to, so the call site only has to
 *  supply the source location.  The LOGI/LOGW/LOGE/LOGF macros (lkGLog.h) are
 *  thin aliases for these types:
 *
 *      LogInfo (__FILE__, __LINE__) << "message";
 */

class LogInfo : public LogAt
{
public:
  LogInfo (const char *file, int line)
    : LogAt (info, file, line) { }
};

class LogWarn : public LogAt
{
public:
  LogWarn (const char *file, int line)
    : LogAt (warn, file, line) { }
};

class LogError : public LogAt
{
public:
  LogError (const char *file, int line)
    : LogAt (error, file, line) { }
};

class LogFatal : public LogAt
{
public:
  LogFatal (const char *file, int line)
    : LogAt (fatal, file, line) { }
};


} // namespace logkit
} // namespace toolkit

#endif

