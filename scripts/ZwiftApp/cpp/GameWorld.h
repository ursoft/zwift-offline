#pragma once //READY for testing
enum WORLD_ID { WID_NONE, WID_WATOPIA, WID_RICHMOND, WID_LONDON, WID_NYC, WID_INNSBRUCK, WID_BOLOGNA, WID_YORKSHIRE, WID_CRIT_CITY, WID_JAPAN, WID_FRANCE, WID_PARIS, WID_GRAVEL_MOUNTAIN, WID_ERR = 0xFF };
struct TextureOverrides {
    std::string m_texName[6];
};
struct GpsInfo { //0x18 bytes
    float m_lonOffset = 0.0f, m_eleOffset = 0.0f, m_latOffset = 0.0f, m_latDegDist = 0.0f, m_lonDegDist = 0.0f;
    bool m_useLegacy = true;
};
typedef void (*nearbyRiderFn)(void);
struct GameWorld { //0x28 bytes
    time_t m_roadXmlDt = 0;
    WORLD_ID m_WorldID;
    int m_roadVersion = 0, m_roadStyleVersion = 0, m_field_C = 1;
    struct WaterSettings {
        float m_seaLevel = 9000.0f, m_defWaterLevel = 9000.0f, m_waterLevel = 9000.0f;
        bool m_showOcean = true, m_allowCaustics = true;
        void Load(tinyxml2::XMLElement *el);
    };
    struct CommonSettings { //0x28 bytes
        std::vector<VEC3> m_fireworks;
        int m_worldId_net = 0, m_color = 0;
        bool m_availableViaPrefs = false;
    };
    struct StringList { //0xE0
        std::string m_worldName, m_worldNameCaps, m_worldNameCapsFull, m_worldNameUnitTest, m_worldStringID, m_sportFormatString, m_loadingText;
    };
    struct RouteDecision { //7 bytes
        RouteDecision(tinyxml2::XMLElement *el);
        int m_markerId = 0;
        enum Turn { T_RIGHT = 0, T_LEFT = 1, T_FWD = 3 } m_turn = T_RIGHT;
        bool m_forward = false, m_stub = false;
    };
    struct RoadInfo { //8 bytes
        float m_startLineRoadTime = 0.0f;
        uint32_t m_defaultRoad = 0;
    };
    struct RouteSportInfo { //0x30 bytes
        std::string m_routeKey;
        int m_routeDefault = 0, m_routeDefaultAlt = 0, m_routeInitial = 0;
    };
    struct RouteInfo { //0x60 bytes
        std::vector<RouteDecision> m_mainEvent, m_runningOnlyEvents;
        std::string m_branchPreferenceKey;
        RouteSportInfo *m_cyclingRouteInfo = nullptr, *m_runRouteInfo = nullptr;
        ~RouteInfo() { delete m_cyclingRouteInfo; delete m_runRouteInfo; }
    };
    struct CameraInfo { //0x18 bytes
        VEC3 m_titlePos{}, m_titleTarget{};
    };
    struct FileList { //0x20 bytes
        std::string m_worldIcon;
    };
    struct MiniMapInfo { //0x38 bytes
        std::string m_minimapXml;
        VEC2 m_anchor{};
        float m_tileScale = 1.0f, m_invTileScale = 1.0f;
        bool m_rotateMinimap = false, m_rotateRouteSelect = false;
    };
    struct TerrainParameters { //0x60 bytes
        std::vector<int> m_tileShaders;
        std::vector<VEC4> m_tileParms1, m_tileParms2, m_tileParms3;
    };
    struct TileConfig {
        int m_sea = 0, m_top = 0, m_vertical = 0, field_14 = 0;
        void *field_C = nullptr;
    };
    struct EventTags { //QUEST: where used
        std::string field_0, field_20;
        std::vector<std::string> field_40, field_58, field_70, field_88;
    };
    struct WorldDef { //0x1E8 bytes
        WorldDef(WORLD_ID aWorldID = WID_NONE) : m_WorldID(aWorldID) {}
        ~WorldDef() {
            delete m_gpsInfo;
            delete m_settings;
            delete m_routeInfo;
            delete m_sl;
            delete m_roadInfo;
            delete m_minimap;
            delete m_cameraInfo;
            delete m_terrain;
            delete m_fileList;
            delete m_eventTags;
        }
        std::unordered_map<int, TextureOverrides> m_texOverrides;
        std::unordered_map<int, int> m_field_1A8;
        EventTags *m_eventTags = nullptr;
        nearbyRiderFn m_nbyFunc = nullptr;
        std::string m_basePath, m_HeightMapPath, m_AOMapPath, m_heightMapOffset, m_heightMapLength, m_skyName, m_videoTextures;
        StringList *m_sl = nullptr;
        WaterSettings m_ws;
        GpsInfo *m_gpsInfo = nullptr;
        RouteInfo *m_routeInfo = nullptr;
        CommonSettings *m_settings = nullptr;
        RoadInfo *m_roadInfo = nullptr;
        MiniMapInfo *m_minimap = nullptr;
        CameraInfo *m_cameraInfo = nullptr;
        FileList *m_fileList = nullptr;
        TerrainParameters *m_terrain = nullptr;
        std::vector<std::string> m_shotList;
        WORLD_ID m_WorldID;
        int m_skyHandle = -1, m_version = 0;
    };
    GameWorld();
    static bool Load(int);
    ~GameWorld();
    WORLD_ID WorldID();
    WorldDef *GetWorldDef();
    float GetSeaLevel();
    static void LoadWorldsData();
    static const char *GetWorldName(int a1, bool a2, bool a3);
    static void LoadShotList(std::vector<std::string> *ret, tinyxml2::XMLElement *el);
    static WorldDef *LoadWorldDefs(tinyxml2::XMLElement *el, bool a2);
    WORLD_ID GetWorldIDForWorldNetworkID(int wni /*WORLD_NETWORK_IDS*/);
    static int GetWorldNetworkIDForWorldID(WORLD_ID wid);
    static GpsInfo *LoadGpsInfo(tinyxml2::XMLElement *src);
    static CommonSettings *LoadCommonSettings(tinyxml2::XMLElement *el);
    static RouteInfo *LoadRouteInfo(tinyxml2::XMLElement *el);
    static RoadInfo *LoadRoadInfo(tinyxml2::XMLElement *el);
    static MiniMapInfo *LoadMiniMapInfo(tinyxml2::XMLElement *el);
    static CameraInfo *LoadCameraInfo(tinyxml2::XMLElement *el);
    static StringList *LoadStringList(tinyxml2::XMLElement *el);
    static FileList *LoadFileList(tinyxml2::XMLElement *el);
    static TerrainParameters *LoadTerrainConfigFile(const std::string &fileName);
    static void LoadTerrainTextureCombinations();
    static void LoadTextureOverrides(std::unordered_map<int, TextureOverrides> *);
    static std::string AddSavePrefix(const char *);
    static WorldDef *GetWorldDef(int worldId);
    static const char *GetWorldNameFull(int worldId);
    static void LoadHeightMap(tinyxml2::XMLElement *, GameWorld::WorldDef *);
    static void RenderEntities();
    static void RenderEntities2D();
    static void RenderRoad();
    static void Update(float dt);

    static inline std::vector<std::string> s_Textures;
    static inline std::vector<std::pair<std::string, std::string>> s_TexturePairsInUse;
    static inline std::vector<TileConfig> s_TileConfigsInUse;
};
inline int g_WORLD_COUNT;
inline float g_WorldTime;
inline GameWorld *g_pGameWorld;
inline std::map<int, GameWorld::WorldDef *> g_allWorlds;
inline std::map<int /*WORLD_NETWORK_IDS*/, WORLD_ID> g_allWorldIds;

float IsUnderWater(const VEC3 &pos);
/*
//inlined GameWorld::AddTexture(std::string const&)
//inlined GameWorld::BeginDataLoad(int)
//inlined GameWorld::GetAllowWaterCaustics(void)
//inlined GameWorld::GetBasePath(void)
//inlined GameWorld::GetDefaultWaterPlaneLevel(void)
//inlined GameWorld::GetGpsInfo(void)
//inlined GameWorld::GetInst(void)
//inlined GameWorld::GetSaveShots(int,std::vector<std::string> &)
//inlined GameWorld::GetShowOcean(void)
//inlined GameWorld::GetSkyHandle(void)
//inlined GameWorld::GetTextureOverrides(void)
//inlined GameWorld::GetTextures(void)
//inlined GameWorld::GetWaterPlaneLevel(void)
//inlined GameWorld::GetWorldByStringID(char const*)
//inlined GameWorld::GetWorldIDs(void)
//inlined GameWorld::GetWorldVersion(void)
//empty GameWorld::HandleEvent(EVENT_ID,std::__va_list)
//inlined GameWorld::LoadTerrainConfigIntoTiles(GameWorld::WorldDef::TerrainParameters *)
//inlined GameWorld::SetWaterPlaneLevel(float,bool)
//not found GameWorld::UpdateTextures(int,int,std::string const&)
//inlined GameWorld::UpdateWorldIDForWorldNetworkID(void)
*/