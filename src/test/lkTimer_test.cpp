// Unit tests for lkTimer.h.
#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "lkTimer.h"

TEST(Timer, WallClockCoversSleep) {
    toolkit::logkit::Timer timer;
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    timer.stop();

    EXPECT_GE(timer.sec_wall(), 0.01);
}

TEST(Timer, ZeroBeforeMeasurement) {
    toolkit::logkit::Timer timer;
    EXPECT_EQ(timer.sec_wall(), 0.0);
    EXPECT_EQ(timer.sec_user(), 0.0);
    EXPECT_EQ(timer.sec_sys(), 0.0);
}

TEST(Timer, TakeDoesNotStop) {
    toolkit::logkit::Timer timer;
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    timer.take();
    const double first = timer.sec_wall();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    timer.take();
    EXPECT_GE(timer.sec_wall(), first);
}
