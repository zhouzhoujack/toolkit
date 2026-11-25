#include <gtest/gtest.h>

#include "tlGLog.h"

#include <regex>
#include <string>

//  LOGI / LOGW / LOGE / LOGF write to the tl log channels and prefix each message
//  with "[HH:MM:SS.mmm file:line]" (macros are defined in tlGLog.h).

TEST (LogMacros, LogiGoesToTheInfoChannel)
{
  //  LOGI is backed by the info channel, so it is visible even at verbosity 0.
  toolkit::logkit::verbosity (0);

  testing::internal::CaptureStdout ();
  LOGI << "info message";
  std::string out = testing::internal::GetCapturedStdout ();

  EXPECT_NE (out.find ("[info]["), std::string::npos);
  EXPECT_NE (out.find ("] info message"), std::string::npos);
}

TEST (LogMacros, MessageIsPrefixedWithTimeAndLocation)
{
  testing::internal::CaptureStdout ();
  LOGW << "located message";
  std::string out = testing::internal::GetCapturedStdout ();

  //  [warn][21:41:08.261 tlLog_test.cpp:42] located message
  EXPECT_NE (out.find ("[warn]["), std::string::npos);
  EXPECT_NE (out.find (" tlLog_test.cpp:"), std::string::npos);
  EXPECT_NE (out.find ("] located message"), std::string::npos);
  EXPECT_TRUE (std::regex_search (out, std::regex ("\\[warn\\]\\[\\d\\d:\\d\\d:\\d\\d\\.\\d\\d\\d [^\\]]*tlLog_test\\.cpp:\\d+\\] ")))
      << out;
}

TEST (LogMacros, LogwAndLogeGoToTheirChannels)
{
  testing::internal::CaptureStdout ();
  LOGW << "warn text";
  std::string out = testing::internal::GetCapturedStdout ();
  EXPECT_NE (out.find ("[warn]["), std::string::npos);
  EXPECT_NE (out.find ("warn text"), std::string::npos);

  testing::internal::CaptureStderr ();
  LOGE << "error text";
  std::string err = testing::internal::GetCapturedStderr ();
  EXPECT_NE (err.find ("[error]["), std::string::npos);
  EXPECT_NE (err.find ("error text"), std::string::npos);
}

TEST (LogMacros, LogfPrintsThenAborts)
{
  //  LOGF flushes the message and terminates the process.
  EXPECT_DEATH (LOGF << "fatal " << 42, "\\[fatal\\].*fatal 42");
}

// ---------------------------------------------------------------------------
//  Custom types: values are converted with toolkit::logkit::to_string (t).
//  1) a to_string() member is used by the default implementation
//  2) for types that cannot be changed, to_string can be specialised
// ---------------------------------------------------------------------------

namespace test_types
{

struct Celsius
{
  double value;
  std::string to_string () const
  {
    return toolkit::logkit::to_string (value) + "C";
  }
};

struct Point
{
  int x, y;
};

} // namespace test_types

namespace toolkit { namespace logkit
{

template <>
std::string to_string (const test_types::Point &p)
{
  return "(" + to_string (p.x) + "," + to_string (p.y) + ")";
}

} }

TEST (LogMacros, CustomTypeWithToStringMember)
{
  testing::internal::CaptureStdout ();
  LOGI << "temp " << test_types::Celsius { 21.5 };
  std::string out = testing::internal::GetCapturedStdout ();
  EXPECT_NE (out.find ("temp 21.5C"), std::string::npos) << out;
}

TEST (LogMacros, CustomTypeWithToStringSpecialisation)
{
  testing::internal::CaptureStdout ();
  LOGI << "point " << test_types::Point { 3, 4 };
  std::string out = testing::internal::GetCapturedStdout ();
  EXPECT_NE (out.find ("point (3,4)"), std::string::npos) << out;
}
