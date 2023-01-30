#pragma once
namespace Thread {
    bool SetCurrentThreadName(LPWSTR name);
    void SetThreadNamingEnabled(bool en);
    bool SetCurrentThreadNameOnce(LPWSTR name);
}
