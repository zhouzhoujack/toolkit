#include "lkString.h"

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <cctype>
#include <limits>
#include <memory>
#include <vector>
#include <sstream>
#include <algorithm>

namespace toolkit {
namespace logkit {

static std::locale c_locale ("C");

// -------------------------------------------------------------------------
//  lower and upper case for wchar_t and uint32_t
//
//  Only ASCII is folded. This is sufficient for the one remaining purpose
//  (case-insensitive user class name lookup, see lkVariant.cc); the Unicode
//  code page tables that used to live in utf_casefolding.h are gone.

wchar_t wdowncase (wchar_t c)
{
  if (c >= L'A' && c <= L'Z') {
    return c + (L'a' - L'A');
  }
  return c;
}

wchar_t wupcase (wchar_t c)
{
  if (c >= L'a' && c <= L'z') {
    return c - (L'a' - L'A');
  }
  return c;
}

uint32_t utf32_downcase (uint32_t c32)
{
  if (sizeof (wchar_t) == 2 && c32 >= 0x10000) {
    return c32;
  } else {
    return uint32_t (wdowncase (wchar_t (c32)));
  }
}

uint32_t utf32_upcase (uint32_t c32)
{
  if (sizeof (wchar_t) == 2 && c32 >= 0x10000) {
    return c32;
  } else {
    return uint32_t (wupcase (wchar_t (c32)));
  }
}

// -------------------------------------------------------------------------
//  Conversion of UTF8 to wchar_t

uint32_t utf32_from_utf8 (const char *&cp, const char *cpe)
{
  uint32_t c32 = (unsigned char) *cp++;
  if (c32 >= 0xf0 && ((cpe && cp + 2 < cpe) || (! cpe && cp [0] && cp [1] && cp [2]))) {
    c32 = ((c32 & 0x7) << 18) | ((uint32_t (cp [0]) & 0x3f) << 12) | ((uint32_t (cp [1]) & 0x3f) << 6) | (uint32_t (cp [2]) & 0x3f);
    cp += 3;
  } else if (c32 >= 0xe0 && ((cpe && cp + 1 < cpe) || (! cpe && cp [0] && cp [1]))) {
    c32 = ((c32 & 0xf) << 12) | ((uint32_t (cp [0]) & 0x3f) << 6) | (uint32_t (cp [1]) & 0x3f);
    cp += 2;
  } else if (c32 >= 0xc0 && ((cpe && cp < cpe) || (! cpe && cp [0]))) {
    c32 = ((c32 & 0x1f) << 6) | (uint32_t (*cp) & 0x3f);
    ++cp;
  }

  return c32;
}

std::wstring to_wstring (const std::string &s)
{
  std::wstring ws;

  const char *cpe = s.c_str () + s.size ();
  for (const char *cp = s.c_str (); cp < cpe; ) {

    uint32_t c32 = utf32_from_utf8 (cp, cpe);

    if (sizeof (wchar_t) == 2 && c32 >= 0x10000) {
      c32 -= 0x10000;
      ws += wchar_t (0xd800 + (c32 >> 10));
      ws += wchar_t (0xdc00 + (c32 & 0x3ff));
    } else {
      ws += wchar_t (c32);
    }

  }

  return ws;
}

std::string to_string (const std::wstring &ws)
{
  std::string s;

  for (std::wstring::const_iterator c = ws.begin (); c != ws.end (); ++c) {

    uint32_t c32 = *c;
    if (sizeof (wchar_t) == 2 && c32 >= 0xd800 && c + 1 < ws.end ()) {
      ++c;
      c32 = (c32 & 0x3ff) << 10;
      c32 |= uint32_t (*c) & 0x3ff;
      c32 += 0x10000;
    }

    if (c32 >= 0x10000) {

      s.push_back (0xf0 | ((c32 >> 18) & 0x7));
      s.push_back (0x80 | ((c32 >> 12) & 0x3f));
      s.push_back (0x80 | ((c32 >> 6) & 0x3f));
      s.push_back (0x80 | (c32 & 0x3f));

    } else if (c32 >= 0x800) {

      s.push_back (0xe0 | ((c32 >> 12) & 0xf));
      s.push_back (0x80 | ((c32 >> 6) & 0x3f));
      s.push_back (0x80 | (c32 & 0x3f));

    } else if (c32 >= 0x80) {

      s.push_back (0xc0 | ((c32 >> 6) & 0x1f));
      s.push_back (0x80 | (c32 & 0x3f));

    } else {
      s.push_back (char (c32));
    }

  }

  return s;
}

// -------------------------------------------------------------------------
//  safe versions (assertion-less) of safe_isdigit, safe_isprint, safe_isalpha, safe_isalnum
//  (required for debug mode of MSVC)

inline bool safe_isdigit (char c)
{
  return c != 0 && static_cast<unsigned char> (c) < 0x80 && isdigit (c);
}

inline bool safe_isalnum (char c)
{
  return c != 0 && static_cast<unsigned char> (c) < 0x80 && isalnum (c);
}

inline bool safe_isalpha (char c)
{
  return c != 0 && static_cast<unsigned char> (c) < 0x80 && isalpha (c);
}

inline bool safe_isprint (char c)
{
  return c != 0 && static_cast<unsigned char> (c) < 0x80 && isprint (c);
}

inline bool safe_isspace (char c)
{
  return c != 0 && static_cast<unsigned char> (c) < 0x80 && isspace (c);
}

// -------------------------------------------------------------------------
//  Utility: a strtod version that is independent of the locale

static std::string micron_format ("%.5f");
static std::string dbu_format ("%.2f");

void set_micron_resolution (unsigned int ndigits)
{
  micron_format = "%." + to_string (ndigits) + "f";
}

void set_db_resolution (unsigned int ndigits)
{
  dbu_format = "%." + to_string (ndigits) + "f";
}

std::string micron_to_string (double d)
{
  char buffer [64];
  ::snprintf (buffer, sizeof (buffer), micron_format.c_str (), d);
  return buffer;
}

std::string db_to_string (double d)
{
  char buffer [64];
  ::snprintf (buffer, sizeof (buffer), dbu_format.c_str (), d);
  return buffer;
}

std::string to_upper_case (const std::string &s)
{
  std::wstring ws = to_wstring (s);
  for (std::wstring::iterator c = ws.begin (); c != ws.end (); ++c) {
    *c = wupcase (*c);
  }
  return to_string (ws);
}

std::string to_lower_case (const std::string &s)
{
  std::wstring ws = to_wstring (s);
  for (std::wstring::iterator c = ws.begin (); c != ws.end (); ++c) {
    *c = wdowncase (*c);
  }
  return to_string (ws);
}

std::string to_local (const std::string &s)
{
  std::unique_ptr<char> buffer (new char [MB_CUR_MAX]); //  MB_CUR_MAX isn't a constant
  std::string ls;

  std::wstring ws = to_wstring (s);
  for (std::wstring::const_iterator c = ws.begin (); c != ws.end (); ++c) {
    int length = wctomb (buffer.get (), *c);
    for (int i = 0; i < length; ++i) {
      ls += buffer.get ()[i];
    }
  }

  return ls;
}

std::string system_to_string (const std::string &s)
{
  return to_string_from_local (s.c_str ());
}

std::string string_to_system (const std::string &s)
{
  return to_local (s);
}

std::string to_string_from_local (const char *cp)
{
  mbstate_t state;
  memset ((void *) &state, 0, sizeof (mbstate_t));

  std::wstring ws;

  size_t max = strlen (cp);

  while (max > 0) {
    wchar_t wc;
    //  NOTE: mbrtowc uses the current LOCALE, hence "local"
    int length = int (mbrtowc (&wc, cp, max, &state));
    if (length < 1) {
      break;
    }
    ws += wc;
    cp += length;
    max -= length;
  }

  return to_string (ws);
}

// -------------------------------------------------------------------------
//  Utility: a strtod version that is independent of the locale

static double local_strtod (const char *cp, const char *&cp_new)
{
  const char *cp0 = cp;

  //  Extract sign
  double s = 1.0;
  if (*cp == '-') {
    s = -1.0;
    ++cp;
  /*
  } else if (*cp == '+') {
    ++cp;
  */
  }

  //  Extract upper digits
  int exponent = 0;
  double mant = 0.0;
  while (safe_isdigit (*cp)) {
    mant = mant * 10.0 + double (*cp - '0');
    ++cp;
  }

  //  Extract lower digits
  if (*cp == '.') {
    ++cp;
    while (safe_isdigit (*cp)) {
      mant = mant * 10.0 + double (*cp - '0');
      ++cp;
      --exponent;
    }
  }

  //  Extract exponent (unless we're at the beginning)
  if (cp != cp0 && (*cp == 'e' || *cp == 'E')) {
    ++cp;
    bool epos = true;
    if (*cp == '-') {
      epos = false;
      ++cp;
    } else if (*cp == '+') {
      ++cp;
    }
    int en = 0;
    while (safe_isdigit (*cp)) {
      en = en * 10 + int (*cp - '0');
      ++cp;
    }
    if (! epos) {
      en = -en;
    }
    exponent += en;
  }

  cp_new = cp;

  return s * mant * pow(10.0, exponent);
}

// -------------------------------------------------------------------------
//  Implementation

std::string
to_string (double d, int prec)
{
  //  For small values less than 1e-(prec) simply return "0" to avoid ugly values like "1.2321716e-14".
  if (fabs (d) < pow (10.0, -prec)) {
    return "0";
  }

  std::ostringstream os;
  os.imbue (c_locale);
  os.precision (prec);
  os.setf (std::ios_base::fmtflags (0), std::ios::basefield);
  os.setf (std::ios_base::fmtflags (0), std::ios::floatfield);
  os << d;
  return os.str ();
}

std::string
to_string (float d, int prec)
{
  //  For small values less than 1e-(prec) simply return "0" to avoid ugly values like "1.2321716e-14".
  if (fabs (d) < pow (10.0, -prec)) {
    return "0";
  }

  std::ostringstream os;
  os.imbue (c_locale);
  os.precision (prec);
  os.setf (std::ios_base::fmtflags (0), std::ios::basefield);
  os.setf (std::ios_base::fmtflags (0), std::ios::floatfield);
  os << d;
  return os.str ();
}

template <>
std::string
to_string (const int &d)
{
  std::ostringstream os;
  os.imbue (c_locale);
  os << d;
  return os.str ();
}

template <>
std::string
to_string (const unsigned int &d)
{
  std::ostringstream os;
  os.imbue (c_locale);
  os << d;
  return os.str ();
}

template <>
std::string
to_string (const long &d)
{
  std::ostringstream os;
  os.imbue (c_locale);
  os << d;
  return os.str ();
}

template <>
std::string
to_string (const long long &d)
{
  std::ostringstream os;
  os.imbue (c_locale);
  os << d;
  return os.str ();
}

template <>
std::string
to_string (const unsigned long &d)
{
  std::ostringstream os;
  os.imbue (c_locale);
  os << d;
  return os.str ();
}

template <>
std::string
to_string (const unsigned long long &d)
{
  std::ostringstream os;
  os.imbue (c_locale);
  os << d;
  return os.str ();
}

template <>
std::string
to_string (char * const &cp)
{
  return std::string (cp);
}

template <>
std::string
to_string (const char * const &cp)
{
  return std::string (cp);
}

template <>
std::string
to_string (unsigned char * const &cp)
{
  return std::string ((const char *) cp);
}

template <>
std::string
to_string (const unsigned char * const &cp)
{
  return std::string ((const char *) cp);
}

std::string
to_string (const char *cp, int length)
{
  return std::string (cp, length);
}

std::string
to_string (const unsigned char *cp, int length)
{
  return std::string ((const char *) cp, length);
}

template <>
std::string
to_string (const bool &b)
{
  return b ? "true" : "false";
}

int
edit_distance (const std::string &a, const std::string &b)
{
  std::vector<int> row0, row1;
  row0.resize (a.size () + 1, 0);
  row1.resize (a.size () + 1, 0);

  for (int i = 0; i <= int (a.size ()); ++i) {
    row0[i] = i;
  }

  for (int i = 0; i < int (b.size ()); ++i) {

    row1[0] = i + 1;

    for (int j = 0; j < int (a.size ()); ++j) {
      int cost = (b[i] == a[j] ? 0 : 1);
      row1[j + 1] = std::min (row0[j] + cost, std::min (row0[j + 1], row1[j]) + 1);
    }

    row0.swap (row1);

  }

  return row0 [a.size ()];
}

std::string
to_quoted_string (const std::string &s)
{
  std::string r;
  r.reserve (s.size () + 2);
  r += '\'';
  for (const char *c = s.c_str (); *c; ++c) {
    if (*c == '\'' || *c == '\\') {
      r += '\\';
      r += *c;
    } else if (*c == '\n') {
      r += "\\n";
    } else if (*c == '\r') {
      r += "\\r";
    } else if (*c == '\t') {
      r += "\\t";
    } else if (! safe_isprint (*c) || (unsigned char) *c >= 0x80) {
      char b [20];
      ::sprintf (b, "\\%03o", int ((unsigned char) *c));
      r += b;
    } else {
      r += *c;
    }
  }
  r += '\'';
  return r;
}

std::string
escape_string (const std::string &s)
{
  std::string r;
  for (const char *c = s.c_str (); *c; ++c) {
    if (*c == '\\') {
      r += '\\';
      r += *c;
    } else if (*c == '\n') {
      r += "\\n";
    } else if (*c == '\r') {
      r += "\\r";
    } else if (*c == '\t') {
      r += "\\t";
    } else if (! safe_isprint (*c)) {
      char b [20];
      ::sprintf (b, "\\%03o", int ((unsigned char) *c));
      r += b;
    } else {
      r += *c;
    }
  }
  return r;
}

inline char unescape_char (const char * &cp)
{
  if (safe_isdigit (*cp)) {
    int c = 0;
    while (*cp && safe_isdigit (*cp)) {
      c = c * 8 + int (*cp - '0');
      ++cp;
    }
    --cp;
    return char (c);
  } else if (*cp == 'r') {
    return '\r';
  } else if (*cp == 'n') {
    return '\n';
  } else if (*cp == 't') {
    return '\t';
  } else {
    return *cp;
  }
}

std::string
unescape_string (const std::string &value)
{
  std::string r;
  for (const char *cp = value.c_str (); *cp; ++cp) {
    if (*cp == '\\' && cp[1]) {
      ++cp;
      r += unescape_char (cp);
    } else {
      r += *cp;
    }
  }
  return r;
}

std::string
to_word_or_quoted_string (const std::string &s, const char *non_term)
{
  //  If the string does not contain non_term characters, we may simply keep it.
  //  Otherwise we need to quote it.
  const char *cp = s.c_str ();
  if (*cp && (safe_isalpha (*cp) || strchr (non_term, *cp) != NULL)) {
    ++cp;
    for ( ; *cp && (safe_isalnum (*cp) || strchr (non_term, *cp) != NULL); ++cp) {
      ;
    }
  }
  if (*cp || s.empty ()) {
    return to_quoted_string (s);
  } else {
    return s;
  }
}

void
escape_to_html (std::string &out, const std::string &in, bool replace_newlines)
{
  for (const char *cp = in.c_str (); *cp; ++cp) {
    if (*cp == '<') {
      out += "&lt;";
    } else if (*cp == '>') {
      out += "&gt;";
    } else if (*cp == '&') {
      out += "&amp;";
    } else if (*cp == '\"') {
      out += "&quot;";
    } else if (replace_newlines && *cp == '\n') {
      out += "<br/>";
    } else {
      out += *cp;
    }
  }
}

std::string
escaped_to_html (const std::string &in, bool replace_newlines)
{
  std::string s;
  escape_to_html (s, in, replace_newlines);
  return s;
}

std::string
replicate (const std::string &s, unsigned int n)
{
  if (n == 0) {
    return std::string ();
  }

  std::string res;
  res.reserve (s.size () * n);
  while (n > 0) {
    res += s;
    --n;
  }
  return res;
}

std::string
pad_string_right (unsigned int columns, const std::string &text)
{
  std::string s = text;
  s.reserve (columns);
  while (s.size () < size_t (columns)) {
    s += " ";
  }
  return s;
}

std::string
pad_string_left (unsigned int columns, const std::string &text)
{
  std::string s;
  s.reserve (columns);
  while (s.size () + text.size () < size_t (columns)) {
    s += " ";
  }
  s += text;
  return s;
}

std::string
replaced (const std::string &subject, const std::string &before, const std::string &after)
{
  if (before.empty ()) {
    return subject;
  }

  std::string s;

  std::string::size_type pos;
  std::string::size_type last = 0;
  while ((pos = subject.find (before, last)) != std::string::npos) {
    if (pos > last) {
       s += std::string (subject, last, pos - last);
    }
    s += after;
    last = pos + before.size ();
  }

  if (last < subject.size ()) {
    s += std::string (subject, last, subject.size () - last);
  }

  return s;
}

void
from_string (const std::string &s, const char * &result)
{
  result = s.c_str ();
}

void
from_string (const std::string &s, const unsigned char * &result)
{
  result = (unsigned char *) s.c_str ();
}

void
from_string (const std::string &s, double &v)
{
  const char *cp = s.c_str ();
  while (safe_isspace (*cp)) {
    ++cp;
  }
  if (! *cp) {
    throw Exception (to_string (std::string ("Got empty string where a real number was expected")));
  }
  const char *cp_end = cp;
  v = local_strtod (cp, cp_end);
  while (safe_isspace (*cp_end)) {
    ++cp_end;
  }
  if (*cp_end) {
    throw Exception (to_string (std::string ("Not a number: ")) + s);
  }
}

template <class T>
void
convert_string_to_int (const std::string &s, T &v)
{
  double x;
  // HACK: this should be some real string-to-int conversion
  from_string (s, x);
  if (x < std::numeric_limits <T>::min ()) {
    throw Exception (to_string (std::string ("Range underflow: ")) + s);
  }
  if (x > std::numeric_limits <T>::max ()) {
    throw Exception (to_string (std::string ("Range overflow: ")) + s);
  }
  v = T (x);
  if (x != v) {
    throw Exception (to_string (std::string ("Number cannot be represented precisely: ")) + s);
  }
}

void
from_string (const std::string &s, int &v)
{
  convert_string_to_int (s, v);
}

void
from_string (const std::string &s, long &v)
{
  convert_string_to_int (s, v);
}

void
from_string (const std::string &s, long long &v)
{
  convert_string_to_int (s, v);
}

void
from_string (const std::string &s, unsigned int &v)
{
  convert_string_to_int (s, v);
}

void
from_string (const std::string &s, unsigned long &v)
{
  convert_string_to_int (s, v);
}

void
from_string (const std::string &s, unsigned long long &v)
{
  convert_string_to_int (s, v);
}

void
from_string (const std::string &s, bool &b)
{
  std::string t (trim (s));
  if (t == "true") {
    b = true;
  } else if (t == "false") {
    b = false;
  } else if (t == "1") {
    b = true;
  } else if (t == "0") {
    b = false;
  } else {
    throw Exception (to_string (std::string ("Invalid boolean value: ")) + s);
  }
}

std::string
join (const std::vector <std::string> &vv, const std::string &s)
{
  std::ostringstream r;

  bool first = true;
  for (std::vector <std::string>::const_iterator i = vv.begin (); i != vv.end (); ++i) {
    if (!first) {
      r << s;
    }
    first = false;
    r << *i;
  }

  return r.str ();
}

std::vector<std::string>
split (const std::string &t, const std::string &s)
{
  std::vector<std::string> r;

  size_t p = 0;
  for (size_t pp = 0; (pp = t.find (s, p)) != std::string::npos; p = pp + s.size ()) {
    r.push_back (std::string (t, p, pp - p));
  }

  r.push_back (std::string (t, p));

  return r;
}

std::string
trim (const std::string &s)
{
  const char *cp = s.c_str ();
  while (safe_isspace (*cp)) {
    ++cp;
  }

  const char *cq = s.c_str () + s.size ();
  while (cq > cp && safe_isspace (cq [-1])) {
    --cq;
  }

  return std::string (cp, cq - cp);
}

} // namespace logkit
} // namespace toolkit
