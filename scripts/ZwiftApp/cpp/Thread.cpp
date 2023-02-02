#include "ZwiftApp.h"
bool g_threadNamingEnabled;
bool thread_local g_CurrentThreadNameSet;
void Thread::SetThreadNamingEnabled(bool en) { g_threadNamingEnabled = en; }
bool Thread::SetCurrentThreadNameOnce(LPCWSTR name) {
    bool ret = false;
    if (g_CurrentThreadNameSet == false) {
        g_CurrentThreadNameSet = true;
        ret = Thread::SetCurrentThreadName(name);
    }
    return ret;
}
bool Thread::SetCurrentThreadName(LPCWSTR name) {
    bool ret = false;
    if (g_threadNamingEnabled)
        ret = !FAILED(SetThreadDescription(GetCurrentThread(), name));
    return ret;
}
void Thread::Initialize(Experimentation *exp) {
    exp->IsEnabled(FID_SETTHRE, [](ExpVariant val) {
        if (val == EXP_ENABLED) {
            Thread::SetThreadNamingEnabled(true);
            Thread::SetCurrentThreadNameOnce(L"main");
        } else {
            Thread::SetThreadNamingEnabled(false);
        }
        });
}
