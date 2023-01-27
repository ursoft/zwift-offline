#include "Thread.h"

bool g_threadNamingEnabled;
bool thread_local g_CurrentThreadNameSet;

bool Thread::SetCurrentThreadNameOnce(LPWSTR name) {
    bool ret = false;
    if (g_CurrentThreadNameSet == false) {
        g_CurrentThreadNameSet = true;
        ret = Thread::SetCurrentThreadName(name);
    }
    return ret;
}

bool Thread::SetCurrentThreadName(LPWSTR name) {
    bool ret = false;
    if (g_threadNamingEnabled)
        ret = !FAILED(SetThreadDescription(GetCurrentThread(), name));
    return ret;
}