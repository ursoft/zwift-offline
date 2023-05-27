#pragma once
struct Powerups {
    Powerups();
    static inline std::unique_ptr<Powerups> g_pPowerups;
};
