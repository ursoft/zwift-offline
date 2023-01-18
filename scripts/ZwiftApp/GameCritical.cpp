#include "ZwiftApp.h"

bool GameCritical::s_shouldAbortJobs;
int GameCritical::s_jobCount;
std::mutex GameCritical::s_mutex;
DWORD GameCritical::s_threadWithLock;

GameCritical::GameCritical() {
    if (!s_shouldAbortJobs && s_mutex.try_lock()) {
        s_threadWithLock = GetCurrentThreadId();
        m_locked = true;
        s_jobCount++;
    }
}
GameCritical::~GameCritical() {
    if (m_locked) {
        --s_jobCount;
        s_threadWithLock = 0;
        s_mutex.unlock();
    }
}