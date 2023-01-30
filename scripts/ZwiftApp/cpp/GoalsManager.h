#pragma once
class GoalsManager {
public:
    GoalsManager();
    static void Shutdown() { s_pSelf.reset(); }
    inline static std::unique_ptr<GoalsManager> s_pSelf;
};