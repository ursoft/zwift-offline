#pragma once
enum WORLD_ID { WID_NONE, WID_WATOPIA, WID_RICHMOND, WID_LONDON, WID_NYC, WID_INNSBRUCK, WID_BOLOGNA, WID_YORKSHIRE, WID_CRIT_CITY, WID_JAPAN, WID_FRANCE, WID_PARIS, WID_GRAVEL_MOUNTAIN };
struct GameWorld { //0x28 bytes
    WORLD_ID m_WorldID;
public:
    struct WorldDef { //0x30 bytes
        WORLD_ID m_WorldID;
        WorldDef(WORLD_ID aWorldID) : m_WorldID(aWorldID) {
        }
        float m_seaLevel = 0.0;
    };
    GameWorld();
    ~GameWorld();
    WORLD_ID WorldID();
    WorldDef *GetWorldDef();
    float GetSeaLevel();
    static void LoadWorldsData();
};
inline int g_WORLD_COUNT;
inline float g_WorldTime;
inline GameWorld *g_pGameWorld;
inline std::map<int, GameWorld::WorldDef *> g_allWorlds;