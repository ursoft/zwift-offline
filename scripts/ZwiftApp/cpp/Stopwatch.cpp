#include "Stopwatch.h" //READY for testing
SteadyClock g_steadyClock;

TEST(SmokeTest, Stopwatch100) {
    Stopwatch w;
    Sleep(100);
    auto ms = w.elapsedInMilliseconds();
    EXPECT_NEAR(110, ms, 10);
}