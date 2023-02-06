#include "ZwiftApp.h"
enum { ZM_CNT = 256 };
bool g_CriticalSectionsAvailable[ZM_CNT];
CRITICAL_SECTION g_CriticalSections[ZM_CNT];
const char *g_MutexNames[ZM_CNT];
void ZwiftLeaveCriticalSection(int idx) {
    if (idx >= 0) {
        zassert(!g_CriticalSectionsAvailable[idx]);
        LeaveCriticalSection(g_CriticalSections + idx);
    }
}
bool ZwiftEnterCriticalSection(int idx) {
    if (idx >= 0) {
        zassert(!g_CriticalSectionsAvailable[idx]);
        EnterCriticalSection(g_CriticalSections + idx);
    }
    return true;
}
void ZMUTEX_SystemInitialize() {
    memset(g_CriticalSectionsAvailable, 1, sizeof(g_CriticalSectionsAvailable));
}
uint8_t g_LastMutexIdx;
int g_nMutexes;
int ZMUTEX_FindFree() {
    for (int i = 0; i < ZM_CNT; ++i) {
        uint8_t tryIdx = uint8_t(g_LastMutexIdx + i);
        if (g_CriticalSectionsAvailable[tryIdx]) {
            ++g_nMutexes;
            g_LastMutexIdx++;
            g_CriticalSectionsAvailable[tryIdx] = false;
            return int(tryIdx);
        }
    }
    zassert(0);
    return -1;
}
int ZMUTEX_Create(const char *name) {
    auto free = ZMUTEX_FindFree();
    if (free >= 0) {
        InitializeCriticalSection(g_CriticalSections + free);
        g_MutexNames[free] = name;
    }
    return free;
}
void ZMUTEX_Delete(int handle) {
    if (handle >= 0) {
        zassert(g_CriticalSectionsAvailable[handle] == false);
        DeleteCriticalSection(g_CriticalSections + handle);
        g_MutexNames[handle] = nullptr;
        g_CriticalSectionsAvailable[handle] = true;
    }
}