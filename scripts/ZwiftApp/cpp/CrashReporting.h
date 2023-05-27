#pragma once
struct EventSystem;
enum EVENT_ID;
struct CrashReporting { //432 bytes, OMIT
    static inline std::unique_ptr<CrashReporting> g_CrashReportingUPtr;
    CrashReporting(EventSystem *ptr);
    void HandleEvent(EVENT_ID e, va_list va);
    ~CrashReporting();
    static void Initialize(EventSystem *ptr);
    static bool IsInitialized() { return g_CrashReportingUPtr.get() != nullptr; }
    static CrashReporting *Instance() { zassert(g_CrashReportingUPtr.get() != nullptr); return g_CrashReportingUPtr.get(); }
    static void Shutdown() { g_CrashReportingUPtr.reset(); }
};