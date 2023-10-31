#include "ZwiftApp.h" //READY for testing
GameWorld::GameWorld() { //inlined to GAME_LoadLevel_MiddlePart
    m_WorldID = (WORLD_ID)g_allWorldIds.size();
}
GameWorld::~GameWorld() { //onGameWorldDtr
    auto wd = GetWorldDef();
    Sky::Destroy(&wd->m_skyHandle);
    EntityManager::ClearAllRenderLists();
    RoadManager::Destroy();
    HeightMapManager::Destroy();
    EntityManager::Destroy();
    EntityFactory::Destroy();
    ReleaseVideoMaterials();
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
        return wd->m_ws.m_seaLevel;
    else
        return 0.0f;
}
WORLD_ID GameWorld::GetWorldIDForWorldNetworkID(int wni /*WORLD_NETWORK_IDS*/) {
    auto ret = g_allWorldIds.find(wni);
    if (ret == g_allWorldIds.end())
        return WID_ERR;
    return ret->second;
}
int GameWorld::GetWorldNetworkIDForWorldID(WORLD_ID wid) {
    auto ret = g_allWorlds.find(wid);
    if (ret == g_allWorlds.end()) {
        zassert(0);
        return 6;
    }
    return ret->second->m_settings->m_worldId_net;
}
void GameWorld::LoadWorldsData() {
    XMLDoc v16;
    auto worldListLoaded = v16.Load("data/Worlds/worldlist.xml", nullptr);
    if (!worldListLoaded) {
        LogTyped(LOG_ERROR, "The world list file is missing.  Is your data stale? path=%s", "data/Worlds/worldlist.xml");
        zassert(worldListLoaded);
    }
    int v2 = 0;
    g_allWorldIds.clear();
    while (auto i = v16.FindNextElement("worlds\\world", false, false)) {
        auto wd = GameWorld::LoadWorldDefs(i, false);
        g_allWorldIds[wd->m_settings->m_worldId_net] = (WORLD_ID)v2;
        g_allWorlds[v2++] = wd;
    }
    g_WORLD_COUNT = (int)g_allWorlds.size();
}
GameWorld::RouteSportInfo *LoadRouteSportInfo(tinyxml2::XMLElement *el) {
    auto ret = new GameWorld::RouteSportInfo();
    const char *val = "";
    el->QueryStringAttribute("routeKey", &val);
    if (*val)
        ret->m_routeKey = val;
    el->QueryIntAttribute("routeDefault", &ret->m_routeDefault);
    el->QueryIntAttribute("routeDefaultAlt", &ret->m_routeDefaultAlt);
    el->QueryIntAttribute("routeInitial", &ret->m_routeInitial);
    return ret;
}
void LoadEventData(std::vector<GameWorld::RouteDecision> *ret, tinyxml2::XMLElement *src) {
    auto el = src->FirstChildElement();
    ret->clear();
    for (int idx = 0; el; idx++) {
        zassert(el->Name() == "routeDecision" + std::to_string(idx));
        ret->emplace_back(GameWorld::RouteDecision(el));
        el = el->NextSiblingElement();
    }
}
GameWorld::RouteInfo *GameWorld::LoadRouteInfo(tinyxml2::XMLElement *el) {
    auto ret = new RouteInfo;
    auto ch = el->FirstChildElement("mainEvent");
    if (ch)
        LoadEventData(&ret->m_mainEvent, ch);
    ch = el->FirstChildElement("runningOnlyEvents");
    if (ch)
        LoadEventData(&ret->m_runningOnlyEvents, ch);
    ch = el->FirstChildElement("branchPreferenceKey");
    if (ch)
        ret->m_branchPreferenceKey = ch->Value();
    ch = el->FirstChildElement("cyclingRouteInfo");
    if (ch)
        ret->m_cyclingRouteInfo = LoadRouteSportInfo(ch);
    ch = el->FirstChildElement("runRouteInfo");
    if (ch)
        ret->m_runRouteInfo = LoadRouteSportInfo(ch);
    return ret;
}
std::map<std::string, nearbyRiderFn> g_nearbyRiderFnMap {
    { "ConstructNearbyRiderGroupsWatopia"s,  RoadManager::ConstructNearbyRiderGroupsWatopia },
    { "ConstructNearbyRiderGroupsRichmond"s, RoadManager::ConstructNearbyRiderGroupsRichmond },
    { "ConstructNearbyRiderGroupsLondon"s,   RoadManager::ConstructNearbyRiderGroupsLondon },
    { "ConstructNearbyRiderGroupsNewYork"s,  RoadManager::ConstructNearbyRiderGroupsNewYork },
    { "ConstructNearbyRiderGroupsEmpty"s,    RoadManager::ConstructNearbyRiderGroupsEmpty },
};
GameWorld::WorldDef *GameWorld::LoadWorldDefs(tinyxml2::XMLElement *el, bool withHeights) {
    auto wdRet = new WorldDef();
    auto ver = el->FirstChildElement("worldVersion");
    if (ver)
        wdRet->m_version = ver->IntText(1);
    auto bp = el->FirstChildElement("basePath");
    if (bp)
        wdRet->m_basePath = bp->GetText();
    auto hmp = el->FirstChildElement("heightMapPath");
    if (hmp)
        wdRet->m_HeightMapPath = hmp->GetText();
    auto ap = el->FirstChildElement("AOMapPath");
    if (ap)
        wdRet->m_AOMapPath = ap->GetText();
    auto sky = el->FirstChildElement("sky");
    if (sky) {
        auto s = ap->GetText();
        wdRet->m_skyName = s;
        wdRet->m_skyHandle = Sky::Create(s, -1);
    }
    zassert(!wdRet->m_HeightMapPath.empty());
    zassert(!wdRet->m_AOMapPath.empty());
    if (withHeights && wdRet->m_HeightMapPath.size() && wdRet->m_AOMapPath.size())
        LoadHeightMap(el, wdRet);
    auto vt = el->FirstChildElement("videoTextures");
    if (vt) {
        wdRet->m_videoTextures = vt->GetText();
        auto parts = ZStringUtil::Split(wdRet->m_videoTextures, ',');
        if (parts.size() == 2)
            AddVideoMaterial(parts[0].c_str(), GAMEPATH(parts[1].c_str()));
    }
    wdRet->m_gpsInfo = LoadGpsInfo(el);
    wdRet->m_settings = LoadCommonSettings(el);
    wdRet->m_sl = LoadStringList(el);
    wdRet->m_routeInfo = LoadRouteInfo(el);
    wdRet->m_roadInfo = LoadRoadInfo(el);
    wdRet->m_cameraInfo = LoadCameraInfo(el);
    wdRet->m_fileList = LoadFileList(el);
    LoadShotList(&wdRet->m_shotList, el);
    wdRet->m_ws.Load(el);
    auto tc_path = wdRet->m_basePath + "terrainconfig.xml"s;
    wdRet->m_terrain = LoadTerrainConfigFile(tc_path);
    LoadTerrainTextureCombinations(/*arg not used*/);
    if (withHeights) {
        LoadTextureOverrides(&wdRet->m_texOverrides);
        // GameWorld::LoadTerrainConfigIntoTiles inlined
        auto &tiles = HeightMapManager::GetInst()->m_heightMapTiles;
        for (int ti = 0; ti < tiles.size(); ++ti) {
            auto tile = tiles[ti];
            if (wdRet->m_terrain) { // HeightMapTile::SetTerrainConfig inlined
                tile->m_shaderId = wdRet->m_terrain->m_tileShaders[ti];
                tile->m_terrainVec1 = wdRet->m_terrain->m_tileParms1[ti];
                tile->m_terrainVec2 = wdRet->m_terrain->m_tileParms2[ti];
                tile->m_terrainVec3 = wdRet->m_terrain->m_tileParms3[ti];
                HeightMapTile::g_useTerrainConfigData = true;
            } else {
                tile->m_shaderId = 0;
                tile->m_terrainVec1 = VEC4{ 4.0f, 8.0f, 1.0f, 9700.0f };
                tile->m_terrainVec2 = VEC4{ 2.93f, 0.1f, 0.75f, 200800.0f };
                tile->m_terrainVec3 = VEC4{ 80000.0f, 0.533f, 0.1f, 0.75f };
                HeightMapTile::g_useTerrainConfigData = false;
            }
        }
    }
    auto nb = el->FirstChildElement("nearbyRiderCreationFunction");
    if (nb) {
        auto nbyFunc = nb->GetText();
        if (*nbyFunc) {
            auto fptr = g_nearbyRiderFnMap.find(nbyFunc);
            if (fptr != g_nearbyRiderFnMap.end()) {
                wdRet->m_nbyFunc = fptr->second; //also params, but they are 0's
            }
        }
    }
    return wdRet;
}
GameWorld::FileList *GameWorld::LoadFileList(tinyxml2::XMLElement *el) {
    auto ret = new GameWorld::FileList();
    auto ch = el->FirstChildElement("worldIcon");
    if (ch)
        ret->m_worldIcon = ch->GetText();
    return ret;
}
GameWorld::CommonSettings *GameWorld::LoadCommonSettings(tinyxml2::XMLElement *el) {
    auto ret = new CommonSettings();
    auto ni = el->FirstChildElement("networkID");
    if (ni)
        ni->QueryIntText(&ret->m_worldId_net);
    auto ce = el->FirstChildElement("color");
    if (ce)
        ce->QueryIntText(&ret->m_color);
    auto p = el->FirstChildElement("availableViaPrefs");
    if (p)
        p->QueryBoolText(&ret->m_availableViaPrefs);
    if (ret->m_worldId_net == 10 || ret->m_worldId_net == 12 || ret->m_worldId_net == 16)
        ret->m_availableViaPrefs = false;
    auto fw = el->FirstChildElement("fireworkLocations");
    if (fw)
        XMLDoc::GetVEC3Array(fw, "fireworkLocation%d", &ret->m_fireworks);
    return ret;
}
GpsInfo *GameWorld::LoadGpsInfo(tinyxml2::XMLElement *el) {
    auto ret = new GpsInfo;
    auto ul = el->FirstChildElement("useLegacy");
    if (ul)
        ul->QueryBoolText(&ret->m_useLegacy);
    auto lo = el->FirstChildElement("lonOffset");
    if (lo)
        lo->QueryFloatText(&ret->m_lonOffset);
    auto eo = el->FirstChildElement("eleOffset");
    if (eo)
        eo->QueryFloatText(&ret->m_eleOffset);
    auto lao = el->FirstChildElement("latOffset");
    if (lao)
        lao->QueryFloatText(&ret->m_latOffset);
    auto ld = el->FirstChildElement("latDegDist");
    if (ld)
        ld->QueryFloatText(&ret->m_latDegDist);
    auto lod = el->FirstChildElement("lonDegDist");
    if (lod)
        lod->QueryFloatText(&ret->m_lonDegDist);
    return ret;
}
void LoadShotListData(std::vector<std::string> *ret, tinyxml2::XMLElement *src, const char *name) {
    auto el = src->FirstChildElement();
    ret->clear();
    for (int idx = 0; el; idx++) {
        zassert(el->Name() == name + std::to_string(idx));
        ret->push_back(el->Value());
        el = el->NextSiblingElement();
    }
}
void GameWorld::LoadShotList(std::vector<std::string> *ret, tinyxml2::XMLElement *el) {
    auto ds = el->FirstChildElement("defaultShots"), cs = el->FirstChildElement("cyclingShots");
    if (ds && cs == nullptr)
        LoadShotListData(ret, ds, "defaultShot");
    if (cs)
        LoadShotListData(ret, cs, "cyclingShot");
}
const char *GameWorld::GetWorldName(int wid, bool caps, bool translated) {
    zassert(wid < g_allWorlds.size())
    auto wd = g_allWorlds.find(wid);
    if (caps) {
        if (translated) {
            return GetText(wd->second->m_sl->m_worldNameCaps.c_str());
        } else {
            return GetText(wd->second->m_sl->m_worldNameCaps.c_str(), LOC_ENG);
        }
    } else {
        if (translated)
            return GetText(wd->second->m_sl->m_worldName.c_str());
        else
            return GetText(wd->second->m_sl->m_worldName.c_str(), LOC_ENG);
    }
}
float IsUnderWater(const VEC3 &pos) {
    if (g_pGameWorld) {
        auto wd = g_pGameWorld->GetWorldDef(); // GetDefaultWaterPlaneLevel inlined
        float wl = wd ? wd->m_ws.m_defWaterLevel : 0.0f;
        auto v5 = std::clamp((pos.m_data[1] - wl) * 0.004f + 2.0f, 0.0f, 1.0f);
        VEC3 v{ -74762.0f - pos.m_data[0], 8600.0f - pos.m_data[1], 106316.0f - pos.m_data[2] };
        auto v7 = (v.len() - 14200.0f) * 0.002f + 1.0f;
        return (1.0f - v5) * std::clamp(v7, 0.0f, 1.0f);
    }
    return 0.0f;
}
void GameWorld::WaterSettings::Load(tinyxml2::XMLElement *el) {
    auto sl = el->FirstChildElement("seaLevel");
    if (sl)
        sl->QueryFloatText(&m_seaLevel);
    auto wpl = el->FirstChildElement("waterPlaneLevel");
    if (wpl) {
        wpl->QueryFloatText(&m_defWaterLevel);
        m_waterLevel = m_defWaterLevel;
    }
    auto ac = el->FirstChildElement("allowCaustics");
    if (ac)
        ac->QueryBoolText(&m_allowCaustics);
    auto so = el->FirstChildElement("showOcean");
    if (so)
        so->QueryBoolText(&m_showOcean);
}
GameWorld::RouteDecision::RouteDecision(tinyxml2::XMLElement *el) {
    el->QueryIntAttribute("markerId", &m_markerId);
    const char *sturn = "0";
    el->QueryStringAttribute("turn", &sturn);
    if (!_stricmp(sturn, "Forward")) {
        m_turn = T_FWD;
    } else if (!_stricmp(sturn, "Left")) {
        m_turn = T_LEFT;
    } else {
        m_turn = (Turn)_stricmp(sturn, "Right");
        if (m_turn)
            sscanf_s(sturn, "%i", &m_turn);
    }
    el->QueryAttribute("forward", &m_forward);
    el->QueryAttribute("stub", &m_stub);
}
GameWorld::RoadInfo *GameWorld::LoadRoadInfo(tinyxml2::XMLElement *el) {
    auto ret = new GameWorld::RoadInfo;
    auto ch = el->FirstChildElement("startLineRoadTime");
    if (ch)
        ch->QueryFloatText(&ret->m_startLineRoadTime);
    ch = el->FirstChildElement("defaultRoad");
    if (ch)
        ch->QueryUnsignedText(&ret->m_defaultRoad);
    return ret;
}
GameWorld::MiniMapInfo *GameWorld::LoadMiniMapInfo(tinyxml2::XMLElement *el) {
    auto ret = new MiniMapInfo;
    auto ch = el->FirstChildElement("rotateMinimap");
    if (ch)
        ch->QueryBoolText(&ret->m_rotateMinimap);
    ch = el->FirstChildElement("rotateRouteSelect");
    if (ch)
        ch->QueryBoolText(&ret->m_rotateRouteSelect);
    ch = el->FirstChildElement("anchor");
    if (ch)
        XMLDoc::GetVEC2(ch, &ret->m_anchor);
    ch = el->FirstChildElement("tileScale");
    if (ch)
        ch->QueryFloatText(&ret->m_tileScale);
    ret->m_invTileScale = 1.0f / ret->m_tileScale;
    ch = el->FirstChildElement("minimapXml");
    if (ch)
        ret->m_minimapXml = ch->GetText();
    return ret;
}
GameWorld::CameraInfo *GameWorld::LoadCameraInfo(tinyxml2::XMLElement *el) {
    auto ret = new CameraInfo;
    auto ch = el->FirstChildElement("titlePos");
    if (ch)
        XMLDoc::GetVEC3(ch, &ret->m_titlePos);
    ch = el->FirstChildElement("titleTarget");
    if (ch)
        XMLDoc::GetVEC3(ch, &ret->m_titleTarget);
    return ret;
}
GameWorld::StringList *GameWorld::LoadStringList(tinyxml2::XMLElement *el) {
    auto ret = new StringList;
    auto ch = el->FirstChildElement("worldName");
    if (ch)
        ret->m_worldName = ch->Value();
    ch = el->FirstChildElement("worldNameCaps");
    if (ch)
        ret->m_worldNameCaps = ch->Value();
    ch = el->FirstChildElement("worldNameCapsFull");
    if (ch)
        ret->m_worldNameCapsFull = ch->Value();
    ch = el->FirstChildElement("worldNameUnitTest");
    if (ch)
        ret->m_worldNameUnitTest = ch->Value();
    ch = el->FirstChildElement("sportFormatString");
    if (ch)
        ret->m_sportFormatString = ch->Value();
    ch = el->FirstChildElement("loadingText");
    if (ch)
        ret->m_loadingText = ch->Value();
    return ret;
}
GameWorld::TerrainParameters *GameWorld::LoadTerrainConfigFile(const std::string &fileName) {
    XMLDoc xdoc;
    if (xdoc.Load(fileName.c_str(), nullptr)) {
        auto ch = xdoc.FindNextElement("terrainconfig", false, false);
        if (ch) {
            auto ret = new TerrainParameters();
            auto ch1 = ch->FirstChildElement("tileShaders");
            if (ch1) {
                auto ch11 = ch1->FirstChildElement("parmData");
                for (int i = 0; ch11; i++) {
                    ret->m_tileShaders.push_back(ch11->IntAttribute("tileValue"));
                    ch11 = ch11->NextSiblingElement("parmData");
                }
            }
            auto ch2 = ch->FirstChildElement("tileParms1");
            if (ch2) {
                auto ch21 = ch2->FirstChildElement("parmData");
                const VEC4 def{ 4.0f, 8.0f, 1.0f, 9700.0f };
                ret->m_tileParms1.reserve(16);
                for (int i = 0; ch21; i++) {
                    ret->m_tileParms1.resize(i + 1);
                    XMLDoc::GetVEC4(ch21->FindAttribute("tileValue"), &ret->m_tileParms1[i], def, "%f %f %f %f");
                    ch21 = ch21->NextSiblingElement("parmData");
                }
            }
            auto ch3 = ch->FirstChildElement("tileParms2");
            if (ch3) {
                auto ch31 = ch3->FirstChildElement("parmData");
                const VEC4 def{ 2.93f, 0.1f, 0.75f, 200800.0f };
                ret->m_tileParms2.reserve(16);
                for (int i = 0; ch31; i++) {
                    ret->m_tileParms2.resize(i + 1);
                    XMLDoc::GetVEC4(ch31->FindAttribute("tileValue"), &ret->m_tileParms2[i], def, "%f %f %f %f");
                    ch31 = ch31->NextSiblingElement("parmData");
                }
            }
            auto ch4 = ch->FirstChildElement("tileParms3");
            if (ch4) {
                auto ch41 = ch4->FirstChildElement("parmData");
                const VEC4 def{ 80000.0f, 0.533f, 0.1f, 0.75f };
                ret->m_tileParms3.reserve(16);
                for (int i = 0; ch41; i++) {
                    ret->m_tileParms3.resize(i + 1);
                    XMLDoc::GetVEC4(ch41->FindAttribute("tileValue"), &ret->m_tileParms3[i], def, "%f %f %f %f");
                    ch41 = ch41->NextSiblingElement("parmData");
                }
            }
        }
    }
    return nullptr;
}
void ListTgaFiles(const char *path, std::vector<std::string> *dest) {
    namespace fs = std::filesystem;
    for (const auto &entry : fs::directory_iterator(path)) {
        if (!entry.is_regular_file() || entry.path().extension().compare(".tga")) continue;
        auto fn = entry.path().filename();
        dest->push_back(fn.string());
    }
}
void GameWorld::LoadTerrainTextureCombinations() {
    s_Textures.clear();
    s_TexturePairsInUse.clear();
    s_TileConfigsInUse.clear();
    auto terfn = "data/Worlds/terraintextures.xml"s;
    if (LOADER_FileExists(terfn)) {
        XMLDoc v157;
        v157.Load(terfn.c_str(), nullptr);
        auto ch = v157.FindNextElement("tileTextures", false, false);
        if (ch) {
            auto ts = ch->FirstChildElement("textureSets");
            if (ts) {
                auto tx = ts->FirstChildElement("texture");
                while (tx) {
                    const char *diffuse = nullptr, *normal = nullptr;
                    tx->QueryStringAttribute("diffuse", &diffuse);
                    tx->QueryStringAttribute("normal", &normal);
                    std::string snormal(normal);
                    s_TexturePairsInUse.push_back({normal, diffuse});
                    auto fnd = std::ranges::find(s_Textures, snormal);
                    if (fnd == s_Textures.end())
                        s_Textures.push_back(snormal);
                    tx = tx->NextSiblingElement("texture");
                }
            }
            auto tcs = ch->FirstChildElement("tileConfigs");
            if (tcs) {
                auto tc = tcs->FirstChildElement("tileConfig");
                while (tc) {
                    TileConfig tilec;
                    tc->QueryIntAttribute("top", &tilec.m_top);
                    tc->QueryIntAttribute("sea", &tilec.m_sea);
                    tc->QueryIntAttribute("vertical", &tilec.m_vertical);
                    s_TileConfigsInUse.push_back(tilec);
                    tc = tc->NextSiblingElement("tileConfig");
                }
            }
        }
    }
    auto String = "Environment/Ground/"s;
    std::vector<std::string> tgas;
    ListTgaFiles("data/Environment/Ground/", &tgas);
    for (auto &tga : tgas) {
        auto texName = "Environment/Ground/"s + tga;
        auto fnd = std::ranges::find(s_Textures, texName);
        if (fnd == s_Textures.end())
            s_Textures.push_back(texName);
    }
}
void GameWorld::LoadTextureOverrides(std::unordered_map<int, TextureOverrides> *dst) {
    dst->clear();
    auto &tiles = HeightMapManager::GetInst()->m_heightMapTiles;
    auto pathName = "data/Worlds/worldlist.xml"s;
    if (LOADER_FileExists(pathName)) {
        XMLDoc v79;
        v79.Load(pathName.c_str(), nullptr);
        auto ws = v79.FindNextElement("worlds", false, false);
        if (ws) {
            int v7 = 0;
            auto fc = ws->FirstChildElement("world");
            if (fc) {
                while (v7 != g_pGameWorld->m_WorldID) {
                    ++v7;
                    fc = ws->NextSiblingElement("world");
                    if (!fc)
                        break;
                }
                if (fc) {
                    fc = fc->FirstChildElement("heightMapTextures");
                    if (fc) {
                        auto heightMapTextures = fc->GetText();
                        if (heightMapTextures) {
                            auto parts = ZStringUtil::Split(heightMapTextures, ',');
                            if (tiles.size() == parts.size()) {
                                for (int v30 = 0; v30 < tiles.size(); ++v30) {
                                    auto v34 = tiles[v30];
                                    auto &v36 = s_TileConfigsInUse[_strtoi64(parts[v30].c_str(), nullptr, 10)];
                                    auto &v37 = s_TexturePairsInUse[v36.m_sea];
                                    auto &v38 = s_TexturePairsInUse[v36.m_top];
                                    auto &v39 = s_TexturePairsInUse[v36.m_vertical];
                                    auto over = (*dst)[v34->m_texOverrideKey];
                                    over.m_texName[0] = v37.first;
                                    over.m_texName[3] = v37.second;
                                    over.m_texName[1] = v38.first;
                                    over.m_texName[4] = v38.second;
                                    over.m_texName[2] = v39.first;
                                    over.m_texName[5] = v39.second;
                                    v34->LoadOverridenTextures(over);
                                }
                                return; //no need for fallback
                            }
                        }
                    }
                }
            }
        }
    }
    //fallback lambda
    for (int i = 0; i < tiles.size(); i++) {
        dst->emplace(tiles[i]->m_texOverrideKey, TextureOverrides{ {
            "Environment/Ground/grass.tga"s, "Environment/Ground/sand.tga"s, "Environment/Ground/Rock.tga"s,
            "Environment/Ground/grass_NRM.tga"s, "Environment/Ground/sand_NRM.tga"s, "Environment/Ground/Rock_NRM.tga"s } });
    }
}
std::string GameWorld::AddSavePrefix(const char *path) {
    return "ZWIFT\\CONFIG\\"s + path;
}
GameWorld::WorldDef *GameWorld::GetWorldDef(int worldId) { //GetWorldDef_0
    auto ret = g_allWorlds.find(worldId);
    if (ret == g_allWorlds.end()) {
        zassert(0);
        return nullptr;
    }
    return ret->second;
}
const char *GameWorld::GetWorldNameFull(int worldId) {
    auto wd = GetWorldDef(worldId);
    if (!wd) return "nullptr";
    /*never called with 0 if (!caps) {
        return GetText(wd->m_sl->m_worldName.c_str());
    }*/
    if (wd->m_sl->m_worldNameCapsFull.size())
        return GetText(wd->m_sl->m_worldNameCapsFull.c_str());
    else
        return GetText(wd->m_sl->m_worldNameCaps.c_str());
}
bool GameWorld::Load(int worldId) {
    g_pGameWorld->m_WorldID = (WORLD_ID)worldId;
    char buf[0x400];
    sprintf_s(buf, "assets/Worlds/world%d/data.wad", worldId);
    g_WADManager.LoadWADFile(buf);
    sprintf_s(buf, "assets/Worlds/world%d/data_1.wad", worldId);
    g_WADManager.LoadWADFile(buf);
    XMLDoc xdoc;
    xdoc.Load("data/Worlds/worldlist.xml", nullptr);
    auto xw = xdoc.FindElement("worlds\\world", false);
    WorldDef *wd = nullptr;
    for (int i = 0; xw; i++) {
        if (i == worldId) {
            wd = LoadWorldDefs(xw, true);
            auto wdOld = g_allWorlds.find(worldId);
            if (wdOld != g_allWorlds.end())
                delete wdOld->second;
            g_allWorlds[worldId] = wd;
            break;
        }
        xw = xw->NextSiblingElement("world");
    }
    zassert(wd != nullptr);
    if (wd) {
        g_startLineRoadTime = wd->m_roadInfo->m_startLineRoadTime;
        XMLDoc xroad, xroadStyle, xents;
        sprintf_s(buf, "%s%s", wd->m_basePath.c_str(), "road.xml");
        if (xroad.Load(buf, &g_pGameWorld->m_roadXmlDt)) {
            g_pGameWorld->m_roadVersion = xroad.m_version;
            sprintf_s(buf, "%s%s", wd->m_basePath.c_str(), "roadstyle.xml");
            if (xroadStyle.Load(buf, &g_pGameWorld->m_roadXmlDt)) {
                g_pGameWorld->m_roadStyleVersion = xroadStyle.m_version;
                g_pRoadManager->Load(&xroad, &xroadStyle);
                OCCLUSION_Initialize();
                auto seed_save = g_seed;
                g_seed = 703710;
                sprintf_s(buf, "%s%s", wd->m_basePath.c_str(), "entities.xml"); //EntityFileManager::LoadXMLFile
                if (xents.Load(buf, nullptr)) {
                    EntityFileManager::ParseEntityFile(&xents);
                } else {
                    zassert(false);
                }
                XMLDoc xprefabs; // PrefabFileManager::LoadPrefabs
                if (xprefabs.Load("data/Worlds/prefabs.xml", nullptr))
                    PrefabFileManager::LoadPrefabsXML(&xprefabs);
                g_seed = seed_save;
                return true;
            }
        }
    }
    return false;
}
void GameWorld::LoadHeightMap(tinyxml2::XMLElement *el, GameWorld::WorldDef *wd) {
    auto heightmapPath = ZStringUtil::Split(wd->m_HeightMapPath, ',');
    auto AOMapPath = ZStringUtil::Split(wd->m_AOMapPath, ',');
    auto hml = el->FirstChildElement("heightMapLength");
    wd->m_heightMapLength = hml->GetText();
    auto lengths = ZStringUtil::Split(wd->m_heightMapLength, ',');
    auto hmo = el->FirstChildElement("heightMapOffset");
    wd->m_heightMapOffset = hmo->GetText();
    auto offsets = ZStringUtil::Split(wd->m_heightMapOffset, ',');
    zassert(heightmapPath.size() == lengths.size() && lengths.size() == offsets.size() && offsets.size() == AOMapPath.size() && "GameWorld::Load Asserted; worlds file contains invalid heightmap information");
    if (heightmapPath.size()) {
        for (int v46 = 0; v46 < heightmapPath.size(); ++v46) {
            auto &v48 = offsets[v46];
            auto positionOffset = ZStringUtil::Split(v48, ' ');
            zassert(positionOffset.size() == 3 && "GameWorld::Load Asserted; worlds file contains incorrect formatted map offset vector");
            if (positionOffset.size() != 3)
                break;
            char bakedfn[MAX_PATH], aofn[MAX_PATH];
            sprintf_s(bakedfn, "%sbaked_%s", wd->m_basePath.c_str(), heightmapPath[v46].c_str());
            sprintf_s(aofn, "%s%s", wd->m_basePath.c_str(), AOMapPath[v46].c_str());
            if (!HeightMapManager::GetInst()->LoadHeightMap(bakedfn, aofn,
                    VEC3{ std::stof(positionOffset[0]), std::stof(positionOffset[1]), std::stof(positionOffset[2])},
                    _strtoi64(lengths[v46].c_str(), nullptr, 10)))
                break;
        }
    } else {
        auto lods = el->FirstChildElement("tileLods");
        auto &tiles = HeightMapManager::GetInst()->m_heightMapTiles;
        if (lods) {
            std::vector<int> v150;
            for (int v73 = 0; v73 < tiles.size(); v73++)
                v150.push_back(tiles[v73]->m_texOverrideKey);
            if (v150.size() > 1)
                std::ranges::sort(v150);
            if (lods->FirstChildElement("tileLod0")) {
                char name[32];
                for (int v83 = 0; v83 < heightmapPath.size(); v83++) {
                    sprintf_s(name, "tileLod%d", v83);
                    auto v92 = lods->FirstChildElement(name);
                    if (v92) {
                        auto v94 = tiles[v83];
                        auto v95 = v92->GetText();
                        v94->m_field_3D8 = true;
                        v94->m_field_3DC = strtol(v95, nullptr, 10);
                        wd->m_field_1A8[v150[v83]] = v94->m_field_3DC;
                    }
                }
            } else {
                auto lodsp = ZStringUtil::Split(lods->GetText(), ',');
                for (int v115 = 0; v115 < lodsp.size(); v115++)
                    wd->m_field_1A8[v150[v115]] = std::stoi(lodsp[v115]);
            }
        }
        for (int v129 = 0; v129 < tiles.size(); ++v129) {
            auto v134 = tiles[v129];
            v134->m_field_3D8 = true; // HeightMapTile::LoadLodClampValues inlined
            v134->m_field_3DC = wd->m_field_1A8[v134->m_texOverrideKey];
        }
        HeightMapManager::GetInst()->DoneLoading();
    }
}
void GameWorld::RenderEntities() {
    EntityManager::GetInst()->Render();
}
void GameWorld::RenderEntities2D() {
    EntityManager::GetInst()->Render2D();
}
void GameWorld::RenderRoad() {
    g_pRoadManager->Render();
}
void GameWorld::Update(float dt) {
    static float g_GameWorldRenderNextDt = (HIWORD(g_seed) & 0x7FFF) * 0.00021362957f + 3.0f;
    g_GameWorldRenderNextDt -= dt;
    if (g_GameWorldRenderNextDt < 0.0f) {
        g_seed = 214013 * g_seed + 2531011;
        g_GameWorldRenderNextDt = (HIWORD(g_seed) & 0x7FFF) * 0.00036622211f + 8.0f;
        g_CameraManager.GetSelectedCamera();
        g_pGameWorld->GetWorldDef();
    }
    Sky::Update(dt);
    EntityManager::GetInst()->Update(dt);
    g_pRoadManager->Update(dt);
}
