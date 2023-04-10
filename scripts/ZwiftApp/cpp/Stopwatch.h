#pragma once
struct SteadyClock {
    inline static double g_perfPeriod;
    SteadyClock() {
        assert(g_perfPeriod == 0.0); // second object???
        LARGE_INTEGER v1;
        BOOL result = QueryPerformanceFrequency(&v1);
        assert(result);
        g_perfPeriod = 1000000000.0 / v1.LowPart;
    }
    uint64_t now() {
        LARGE_INTEGER PerformanceCount;
        QueryPerformanceCounter(&PerformanceCount);
        uint64_t ret = (uint32_t)(int32_t)(PerformanceCount.LowPart * g_perfPeriod); //QUEST: why 32 bit
        return ret;
    }
};
extern SteadyClock g_steadyClock; //one per app is OK, no need for shared ptr
struct Stopwatch {
    uint64_t m_start, m_stop;
    Stopwatch() { start(); }
    uint64_t elapsed() {
        if (m_stop == m_start)
            stop();
        return m_stop - m_start;
    }
    uint64_t elapsedInMilliseconds() { return elapsed() / 1'000; }
    uint64_t elapsedInSeconds() { return elapsed() / 1'000'000; }
    uint64_t partial() { return g_steadyClock.now() - m_start; }
    void start() { m_start = m_stop = g_steadyClock.now(); }
    void stop() { m_stop = g_steadyClock.now(); }
};