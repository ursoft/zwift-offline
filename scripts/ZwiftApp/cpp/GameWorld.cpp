#include "ZwiftApp.h"
inline GameWorld::GameWorld() {
    //TODO
}
GameWorld::~GameWorld() { //onGameWorldDtr
    //TODO
}
WORLD_ID GameWorld::WorldID() {
    zassert(m_WorldID < g_WORLD_COUNT);
    return m_WorldID;
}
GameWorld::WorldDef *GameWorld::GetWorldDef() {
    auto ret = g_allWorlds.find(m_WorldID);
    if (ret == g_allWorlds.end()) {
        auto newWd = new WorldDef(m_WorldID);
        g_allWorlds[(int)m_WorldID] = newWd;
        return newWd;
    }
    return ret->second;
}
float GameWorld::GetSeaLevel() {
    auto wd = GetWorldDef();
    if (wd)
        return wd->m_seaLevel;
    else
        return 0.0f;
}
void GameWorld::LoadWorldsData() {
    //TODO
}
float IsUnderWater(const VEC3 &pos) {
    if (g_pGameWorld) {
        auto wd = g_pGameWorld->GetWorldDef(); // GetDefaultWaterPlaneLevel inlined
        float wl = wd ? wd->m_defWaterLevel : 0.0f;
        auto v5 = std::clamp((pos.m_data[1] - wl) * 0.004f + 2.0f, 0.0f, 1.0f);
        VEC3 v{ -74762.0f - pos.m_data[0], 8600.0f - pos.m_data[1], 106316.0f - pos.m_data[2] };
        auto v7 = (v.len() - 14200.0f) * 0.002f + 1.0f;
        return (1.0f - v5) * std::clamp(v7, 0.0f, 1.0f);
    }
    return 0.0f;
}