#pragma once

enum WorldIDs { WID_0, WID_1, WID_2 }; //TODO

class GameWorld { //0x28 bytes
    WorldIDs m_WorldID;
public:
    struct WorldDef { //0x30 bytes
        WorldIDs m_WorldID;
        WorldDef(WorldIDs aWorldID) : m_WorldID(aWorldID) {
        }
        float m_seaLevel = 0.0;
    };
    GameWorld();
    ~GameWorld();
    WorldIDs WorldID();
    WorldDef *GetWorldDef();
    float GetSeaLevel();
};

extern int g_WORLD_COUNT;
extern float g_WorldTime;
extern GameWorld *g_pGameWorld;
extern std::map<int, GameWorld::WorldDef *> g_allWorlds;