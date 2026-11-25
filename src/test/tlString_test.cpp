// Unit tests for tlString.h.
#include <string>

#include <gtest/gtest.h>

#include "tlString.h"

TEST(ToString, Scalars) {
    EXPECT_EQ(toolkit::logkit::to_string(42), "42");
    EXPECT_EQ(toolkit::logkit::to_string(-7), "-7");
    EXPECT_EQ(toolkit::logkit::to_string(true), "true");
    EXPECT_EQ(toolkit::logkit::to_string(false), "false");
    EXPECT_EQ(toolkit::logkit::to_string(std::string("abc")), "abc");
}

TEST(CaseConversion, Ascii) {
    EXPECT_EQ(toolkit::logkit::to_upper_case("aBc"), "ABC");
    EXPECT_EQ(toolkit::logkit::to_lower_case("AbC"), "abc");
    EXPECT_EQ(toolkit::logkit::to_upper_case("1-_. "), "1-_. ");
    EXPECT_EQ(toolkit::logkit::to_upper_case(""), "");
}

TEST(StringHelpers, Replicate) {
    EXPECT_EQ(toolkit::logkit::replicate("ab", 3), "ababab");
    EXPECT_EQ(toolkit::logkit::replicate("x", 1), "x");
    EXPECT_EQ(toolkit::logkit::replicate("x", 0), "");
}

TEST(StringHelpers, Padding) {
    EXPECT_EQ(toolkit::logkit::pad_string_right(5, "ab"), "ab   ");
    EXPECT_EQ(toolkit::logkit::pad_string_left(5, "ab"), "   ab");
    // Padding never truncates.
    EXPECT_EQ(toolkit::logkit::pad_string_right(1, "abcd"), "abcd");
    EXPECT_EQ(toolkit::logkit::pad_string_left(1, "abcd"), "abcd");
}

TEST(StringHelpers, EditDistance) {
    EXPECT_EQ(toolkit::logkit::edit_distance("kitten", "sitting"), 3);
    EXPECT_EQ(toolkit::logkit::edit_distance("abc", "abc"), 0);
    EXPECT_EQ(toolkit::logkit::edit_distance("", "abc"), 3);
    EXPECT_EQ(toolkit::logkit::edit_distance("abc", ""), 3);
}
