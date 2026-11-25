// Unit tests for tlGlobPattern.h.
#include <string>

#include <gtest/gtest.h>

#include "tlGlobPattern.h"

TEST(GlobPattern, Wildcards) {
    EXPECT_TRUE(toolkit::logkit::GlobPattern("a*b").match("ab"));
    EXPECT_TRUE(toolkit::logkit::GlobPattern("a*b").match("axxxb"));
    EXPECT_FALSE(toolkit::logkit::GlobPattern("a*b").match("axxxc"));

    EXPECT_TRUE(toolkit::logkit::GlobPattern("a?c").match("abc"));
    EXPECT_TRUE(toolkit::logkit::GlobPattern("a?c").match("a c"));
    EXPECT_FALSE(toolkit::logkit::GlobPattern("a?c").match("ac"));
}

TEST(GlobPattern, CharClassAndAlternatives) {
    EXPECT_TRUE(toolkit::logkit::GlobPattern("a[bc]d").match("abd"));
    EXPECT_TRUE(toolkit::logkit::GlobPattern("a[bc]d").match("acd"));
    EXPECT_FALSE(toolkit::logkit::GlobPattern("a[bc]d").match("aed"));

    EXPECT_TRUE(toolkit::logkit::GlobPattern("a{b,c}d").match("abd"));
    EXPECT_TRUE(toolkit::logkit::GlobPattern("a{b,c}d").match("acd"));
    EXPECT_FALSE(toolkit::logkit::GlobPattern("a{b,c}d").match("aed"));
}

TEST(GlobPattern, CatchallAndConst) {
    EXPECT_TRUE(toolkit::logkit::GlobPattern("*").is_catchall());
    EXPECT_TRUE(toolkit::logkit::GlobPattern("*").match("anything at all"));
    EXPECT_FALSE(toolkit::logkit::GlobPattern("a*").is_catchall());

    EXPECT_TRUE(toolkit::logkit::GlobPattern("abc").is_const());
    EXPECT_FALSE(toolkit::logkit::GlobPattern("ab*").is_const());
}

TEST(GlobPattern, CaseSensitivity) {
    toolkit::logkit::GlobPattern p("ABC");
    EXPECT_TRUE(p.case_sensitive());
    EXPECT_TRUE(p.match("ABC"));
    EXPECT_FALSE(p.match("abc"));

    p.set_case_sensitive(false);
    EXPECT_FALSE(p.case_sensitive());
    EXPECT_TRUE(p.match("abc"));
}

TEST(GlobPattern, PatternAndEmpty) {
    EXPECT_TRUE(toolkit::logkit::GlobPattern().empty());
    EXPECT_EQ(toolkit::logkit::GlobPattern("x").pattern(), "x");
    EXPECT_FALSE(toolkit::logkit::GlobPattern("x").empty());
}
