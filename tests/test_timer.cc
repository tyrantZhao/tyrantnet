#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"

#include <chrono>
#include <memory>

#include <tyrant/timer/timer.h>

TEST_CASE("Timers are computed", "[timer]") {
    auto timerMgr = std::make_shared<tyrant::TimerMgr>();

    int upvalue = 0;
    auto timer = timerMgr->addTimer(std::chrono::seconds(1), [&upvalue]() {
        upvalue++;
    });
    timer = timerMgr->addTimer(std::chrono::seconds(1), [&upvalue]() {
        upvalue++;
    });
    auto leftTime = timerMgr->nearLeftTime();
    REQUIRE_FALSE(leftTime > std::chrono::seconds(1));
    REQUIRE(timerMgr->isEmpty() == false);
    while (!timerMgr->isEmpty())
    {
        timerMgr->schedule();
    }
    REQUIRE(upvalue == 2);

    timer = timerMgr->addTimer(std::chrono::seconds(1), [&upvalue]() {
        upvalue++;
    });
    timer.lock()->cancel();
    REQUIRE(timerMgr->isEmpty() == false);
    while (!timerMgr->isEmpty())
    {
        timerMgr->schedule();
    }

    REQUIRE(upvalue == 2);
}


