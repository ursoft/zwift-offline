#pragma once
class Powerups {
public:
    Powerups();
    static inline std::unique_ptr<Powerups> g_pPowerups;
};
