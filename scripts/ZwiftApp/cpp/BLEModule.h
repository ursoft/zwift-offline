#pragma once
struct BLEModule : public EventObject {
    inline static std::unique_ptr<BLEModule> g_BLEModule;
    BLEModule(Experimentation *exp);
    static void Initialize(Experimentation *exp);
    static bool IsInitialized() { return g_BLEModule.get() != nullptr; }
    static BLEModule *Instance() { zassert(g_BLEModule.get() != nullptr); return g_BLEModule.get(); }
    static void Shutdown();
    void HandleEvent(EVENT_ID, va_list) override;
    void StopScan();
};
