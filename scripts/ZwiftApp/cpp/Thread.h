#pragma once
namespace Thread {
    void Initialize(Experimentation *);
    bool SetCurrentThreadName(LPCWSTR name);
    void SetThreadNamingEnabled(bool en);
    bool SetCurrentThreadNameOnce(LPCWSTR name);
}
