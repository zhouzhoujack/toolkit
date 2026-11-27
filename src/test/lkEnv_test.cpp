// Unit tests for lkEnv.h.
#include <cstdlib>
#include <string>

#include <gtest/gtest.h>

#include "lkEnv.h"

namespace {

const char *ENV_NAME = "LOGKIT_TEST_ENV";
const char *FLAG_NAME = "KLAYOUT_TEST_FLAG";

class EnvTest : public ::testing::Test {
protected:
    void TearDown() override {
        unsetenv(ENV_NAME);
        unsetenv(FLAG_NAME);
    }
};

}  // namespace

TEST_F(EnvTest, UnsetUsesDefault) {
    unsetenv(ENV_NAME);
    EXPECT_FALSE(toolkit::logkit::has_env(ENV_NAME));
    EXPECT_EQ(toolkit::logkit::get_env(ENV_NAME), "");
    EXPECT_EQ(toolkit::logkit::get_env(ENV_NAME, "fallback"), "fallback");
}

TEST_F(EnvTest, SetValueIsRead) {
    setenv(ENV_NAME, "hello", 1);
    EXPECT_TRUE(toolkit::logkit::has_env(ENV_NAME));
    EXPECT_EQ(toolkit::logkit::get_env(ENV_NAME), "hello");
    EXPECT_EQ(toolkit::logkit::get_env(ENV_NAME, "fallback"), "hello");
}

TEST_F(EnvTest, EmptyValueStillCountsAsSet) {
    setenv(ENV_NAME, "", 1);
    EXPECT_TRUE(toolkit::logkit::has_env(ENV_NAME));
    EXPECT_EQ(toolkit::logkit::get_env(ENV_NAME), "");
    EXPECT_EQ(toolkit::logkit::get_env(ENV_NAME, "fallback"), "");
}

TEST_F(EnvTest, AppFlagFromEnvironment) {
    unsetenv(FLAG_NAME);
    EXPECT_FALSE(toolkit::logkit::app_flag("test-flag"));

    setenv(FLAG_NAME, "0", 1);
    EXPECT_FALSE(toolkit::logkit::app_flag("test-flag"));

    setenv(FLAG_NAME, "1", 1);
    EXPECT_TRUE(toolkit::logkit::app_flag("test-flag"));
}
