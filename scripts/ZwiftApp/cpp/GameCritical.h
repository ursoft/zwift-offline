#pragma once
class GameCritical {
    static int s_jobCount;
    static bool s_shouldAbortJobs;
    static std::mutex s_mutex;
    static uint32_t s_threadWithLock;
    bool m_locked = false;
public:
    static int GetJobCount() { return s_jobCount; }
    static void AbortJobs() { s_shouldAbortJobs = true; }
    static void PermitJobs() { s_shouldAbortJobs = false; }
    static bool ShouldAbortJobs() { return s_shouldAbortJobs; }

    GameCritical();
    bool CanRun() { return m_locked; }
    ~GameCritical();
};
template<class T> void FreeAndNil(T &ptr) {
    auto copy = ptr;
    ptr = nullptr;
    delete copy;
}
