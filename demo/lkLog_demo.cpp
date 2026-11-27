// Demo executable for the glog-style logging macros (lkGLog.h).
//
// Not installed -- it exists to eyeball the output:
//   ./compile.sh
//   ./build/demo/logkit_log_demo
//
// Each macro writes to its channel and prefixes the message with the level tag,
// a timestamp and the source location:
//
//   LOGI -> info  (stdout, always visible)
//   LOGW -> warn  (stdout, "[warn]" tag)
//   LOGE -> error (stderr, "[error]" tag)
//   LOGF -> fatal (stderr, "[fatal]" tag, then abort ())
//
// LOGI writes to the info channel, so it is always visible. The log verbosity
// (KLAYOUT_VERBOSITY, default 10) only affects the raw toolkit::logkit::log channel.
//
// Printing custom types
// ---------------------
// Every value is converted to text with toolkit::logkit::to_string (t).  The
// default implementation of that template calls t.to_string (), but there is no
// need to add such a member: the cleanest way (and the only one for types you
// cannot modify, e.g. QString) is to specialise toolkit::logkit::to_string.
// Both cases below use that mechanism.
//
// The specialisation has to be visible before the first use (same TU).

#include <toolkit/lkGLog.h>

#include <string>

// ---------------------------------------------------------------------------
//  1) Own type -- no member function, just a to_string specialisation
// ---------------------------------------------------------------------------

class Money
{
public:
  Money (int euros, int cents)
    : m_cents (euros * 100 + cents) { }

  int cents () const { return m_cents; }

private:
  int m_cents;
};

// ---------------------------------------------------------------------------
//  2) A type we cannot change (QString, simulated here) -- same mechanism
//     (the library itself does not use Qt)
// ---------------------------------------------------------------------------

namespace demo
{

class QString
{
public:
  QString () { }
  explicit QString (const std::string &utf8) : m_utf8 (utf8) { }
  const std::string &utf8 () const { return m_utf8; }

private:
  std::string m_utf8;
};

} // namespace demo

namespace toolkit { namespace logkit
{

//  own type

template <>
std::string to_string (const Money &m)
{
  return toolkit::logkit::to_string (m.cents () / 100) + "." +
         toolkit::logkit::to_string (m.cents () % 100) + " EUR";
}

//  third-party type

template <>
std::string to_string (const demo::QString &s)
{
  return "QString(\"" + s.utf8 () + "\")";
}

} }

int main() {

    LOGI << "LOGI -> toolkit::logkit::info";
    LOGW << "LOGW -> toolkit::logkit::warn";
    LOGE << "LOGE -> toolkit::logkit::error";
    // LOGF << "LOGF -> toolkit::logkit::fatal (would abort here)";

    // (1) custom type with a specialised to_string
    LOGI << "money: " << Money (12, 34);

    // (2) type we cannot modify, same mechanism
    demo::QString q ("hello from QString");
    LOGI << "custom: " << q;
    LOGW << "custom in warn: " << q;

    return 0;
}
