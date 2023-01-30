#pragma once
class UnitTypeManager {
public:
    UnitTypeManager();
    static void Shutdown() { g_pUnitTypeManagerUPtr.reset(); }
    inline static std::unique_ptr<UnitTypeManager> g_pUnitTypeManagerUPtr;
};