#pragma once
enum WORLD_ID { WID_NONE, WID_WATOPIA, WID_RICHMOND, WID_LONDON, WID_NYC, WID_INNSBRUCK, WID_BOLOGNA, WID_YORKSHIRE, WID_CRIT_CITY, WID_JAPAN, WID_FRANCE, WID_PARIS, WID_GRAVEL_MOUNTAIN };
struct TextureOverrides {
    std::string m_texName[6];
};
struct GameWorld { //0x28 bytes
    WORLD_ID m_WorldID;
public:
    struct WorldDef { //> 0x30 bytes
        WorldDef(WORLD_ID aWorldID) : m_WorldID(aWorldID) {}
        std::unordered_map<int, TextureOverrides> m_texOverrides;
        WORLD_ID m_WorldID;
        float m_seaLevel = 0.0f, m_defWaterLevel = 0.0f, m_waterLevel = 0.0f;
        bool m_allowWaterCaustics = false;
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

float IsUnderWater(const VEC3 &pos);
/*GameWorld::AddSavePrefix(char const*)
GameWorld::AddTexture(std::string const&)
GameWorld::BeginDataLoad(int)
GameWorld::GetAllowWaterCaustics(void)
GameWorld::GetBasePath(void)
GameWorld::GetDefaultWaterPlaneLevel(void)
GameWorld::GetGpsInfo(void)
GameWorld::GetInst(void)
GameWorld::GetSaveShots(int,std::vector<std::string> &)
GameWorld::GetShowOcean(void)
GameWorld::GetSkyHandle(void)
GameWorld::GetTextureOverrides(void)
GameWorld::GetTextures(void)
GameWorld::GetWaterPlaneLevel(void)
GameWorld::GetWorldByStringID(char const*)
GameWorld::GetWorldDef(int)
GameWorld::GetWorldIDForWorldNetworkID(WORLD_NETWORK_IDS)
GameWorld::GetWorldIDs(void)
GameWorld::GetWorldName(int,bool,bool)
GameWorld::GetWorldNameFull(int,bool,bool)
GameWorld::GetWorldNetworkIDForWorldID(int)
GameWorld::GetWorldVersion(void)
GameWorld::HandleEvent(EVENT_ID,std::__va_list)
GameWorld::Load(int)
GameWorld::LoadCameraInfo(tinyxml2::XMLElement *)
GameWorld::LoadCommonSettings(tinyxml2::XMLElement *)
GameWorld::LoadFileList(tinyxml2::XMLElement *)
GameWorld::LoadGpsInfo(tinyxml2::XMLElement *)
GameWorld::LoadHeightMap(tinyxml2::XMLElement *,tinyxml2::XMLElement *&,GameWorld::WorldDef *)
GameWorld::LoadMiniMapInfo(tinyxml2::XMLElement *)
GameWorld::LoadRoadInfo(tinyxml2::XMLElement *)
GameWorld::LoadRouteInfo(tinyxml2::XMLElement *)
GameWorld::LoadShotList(tinyxml2::XMLElement *)
GameWorld::LoadStringList(tinyxml2::XMLElement *)
GameWorld::LoadTerrainConfigFile(std::string)
GameWorld::LoadTerrainConfigIntoTiles(GameWorld::WorldDef::TerrainParameters *)
GameWorld::LoadTerrainTextureCombinations(std::vector<std::string> const&)
GameWorld::LoadTextureOverrides(std::unordered_map<int,GameWorld::WorldDef::TextureOverrides> &)
GameWorld::LoadWorldDefs(tinyxml2::XMLElement *,bool)
GameWorld::RenderEntities(void)
GameWorld::RenderEntities2D(void)
GameWorld::RenderRoad(void)
GameWorld::SetWaterPlaneLevel(float,bool)
GameWorld::Update(float)
GameWorld::UpdateTextures(int,int,std::string const&)
GameWorld::UpdateWorldIDForWorldNetworkID(void)
GameWorld::WorldDef::EventTags::~EventTags()
GameWorld::WorldDef::RouteInfo::~RouteInfo()
GameWorld::WorldDef::StringList::~StringList()
GameWorld::WorldDef::TextureOverrides::~TextureOverrides()
GameWorld::WorldDef::WaterSettings::Load(tinyxml2::XMLElement *)
GameWorld::WorldDef::~WorldDef()
*/