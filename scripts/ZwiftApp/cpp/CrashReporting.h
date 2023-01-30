#pragma once
class EventSystem;
enum EVENT_ID;
class CrashReporting { //432 bytes, not implemented
    static inline std::unique_ptr<CrashReporting> g_CrashReportingUPtr;
public:
    CrashReporting(EventSystem *ptr);
    void HandleEvent(EVENT_ID e, va_list va);
    ~CrashReporting();
    static void Initialize(EventSystem *ptr);
    static bool IsInitialized() { return g_CrashReportingUPtr.get() != nullptr; }
    static CrashReporting *Instance() { zassert(g_CrashReportingUPtr.get() != nullptr); return g_CrashReportingUPtr.get(); }
    static void Shutdown() { g_CrashReportingUPtr.reset(); }
};