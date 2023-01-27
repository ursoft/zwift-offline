#include "ZwiftApp.h"
#include "GameWorld.h"

int g_WORLD_COUNT;
GameWorld *g_pGameWorld;

inline GameWorld::GameWorld() {
    //TODO
}

GameWorld::~GameWorld() { //onGameWorldDtr
    //TODO
}

WorldIDs GameWorld::WorldID() {
    zassert(m_WorldID < g_WORLD_COUNT);
    return m_WorldID;
}

std::map<int, GameWorld::WorldDef *> g_allWorlds; 
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
		return 0.0;
}
