#pragma once
enum UnitType { UT_IMPERIAL, UT_METRIC, UT_CNT };
class UnitTypeManager : public EventObject {
    inline static std::unique_ptr<UnitTypeManager> g_pUnitTypeManagerUPtr;
    bool m_readonly;
    UnitType m_unitType = UT_METRIC;
public:
    UnitTypeManager(EventSystem *, bool);
    static void Shutdown() { g_pUnitTypeManagerUPtr.reset(); }
    static UnitTypeManager *Instance() { zassert(g_pUnitTypeManagerUPtr.get() != nullptr); return g_pUnitTypeManagerUPtr.get(); }
    UnitType GetUnitType() { return m_unitType; }
    static bool IsInitialized() { return g_pUnitTypeManagerUPtr.get() != nullptr; }
    bool GetUseMetric() { return m_unitType == UT_METRIC; }
    void SetUseMetric(bool m);
    static void Initialize(EventSystem *ev, bool b);
    void HandleEvent(EVENT_ID, va_list) override;
    ~UnitTypeManager();
};