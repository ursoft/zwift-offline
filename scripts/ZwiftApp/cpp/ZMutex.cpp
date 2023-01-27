#include "ZwiftApp.h"

bool g_CriticalSectionsAvailable[256];
CRITICAL_SECTION g_CriticalSections[256];

void ZwiftLeaveCriticalSection(int idx) {
    if (idx >= 0) {
        assert(g_CriticalSectionsAvailable[idx]);
        LeaveCriticalSection(g_CriticalSections + idx);
    }
}

bool ZwiftEnterCriticalSection(int idx) {
    if (idx >= 0) {
        assert(g_CriticalSectionsAvailable[idx]);
        EnterCriticalSection(g_CriticalSections + idx);
    }
    return true;
}