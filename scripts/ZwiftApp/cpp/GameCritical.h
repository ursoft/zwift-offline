#pragma once //READY for testing
struct GameCritical {
    static int s_jobCount;
    static bool s_shouldAbortJobs;
    static std::mutex s_mutex;
    static uint32_t s_threadWithLock;
    bool m_locked = false;
    static int GetJobCount() { return s_jobCount; }
    static void AbortJobs() { s_shouldAbortJobs = true; }
    static void PermitJobs() { s_shouldAbortJobs = false; }
    static bool ShouldAbortJobs() { return s_shouldAbortJobs; }
    GameCritical();
    bool CanRun() { return m_locked; }
    ~GameCritical();
};
