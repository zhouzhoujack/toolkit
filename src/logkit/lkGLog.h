/**
 *  @brief glog-style logging macros backed by the tl log channels
 *
 *  This header used to be a thin wrapper around Google's glog. The library no
 *  longer depends on glog; LOGI/LOGW/LOGE/LOGF now write to the tl channels and
 *  prefix every message with a timestamp and the source location.
 *
 *  Each macro expands to a small level-bound class (toolkit::logkit::LogInfo, toolkit::logkit::LogWarn,
 *  toolkit::logkit::LogError, toolkit::logkit::LogFatal) which already carries its channel, so only the
 *  source location has to be passed in:
 *
 *      [info][HH:MM:SS.mmm file.cc:123] message
 *
 *      LOGI << "starting phase " << phase;   // -> toolkit::logkit::info   (stdout, always visible)
 *      LOGW << "value: " << v;               // -> toolkit::logkit::warn   (stdout, "[warn]" tag)
 *      LOGE << "cannot open " << path;       // -> toolkit::logkit::error  (stderr, "[error]" tag)
 *      LOGF << "unrecoverable error";        // -> toolkit::logkit::fatal  (stderr, "[fatal]" tag, then abort ())
 *
 *  Example output:
 *      [info][12:34:56.789 foo.cc:42] starting phase 3
 *      [warn][12:34:56.790 foo.cc:43] value: 7
 *      [error][12:34:56.791 foo.cc:44] cannot open x.txt
 *      [fatal][12:34:56.792 foo.cc:45] unrecoverable error
 *
 *  LOGF is not a catchable error: it flushes the message and calls abort ().
 *  A direct "toolkit::logkit::info << ..." (without a macro) is written as "[info] message"
 *  (level tag only, no time/location).
 */

#ifndef HDR_lkGLog
#define HDR_lkGLog

#include "lkLog.h"

#define LOGI toolkit::logkit::LogInfo (__FILE__, __LINE__)
#define LOGW toolkit::logkit::LogWarn (__FILE__, __LINE__)
#define LOGE toolkit::logkit::LogError (__FILE__, __LINE__)
#define LOGF toolkit::logkit::LogFatal (__FILE__, __LINE__)

#endif
