#include "ZwiftApp.h"

void resize(GLFWwindow *wnd, int w, int h) {
    if (!w) w = 1;
    if (!h) h = 1;
    int rx_w, rx_h;
    glfwGetWindowSize(wnd, &rx_w, &rx_h);
    if (!rx_w) rx_w = 1;
    if (!rx_h) rx_h = 1;
    g_view_w = (float)rx_w;
    g_view_h = (float)rx_h;
    g_kwidth = g_view_w / (float)w;
    g_kheight = g_view_h / (float)h;
    Log("resize: %d x %d", w, h);
    g_width = rx_w;
    g_height = rx_h;
    g_view_x = 0.0f;
    g_view_y = 0.0f;
    if (g_view_w / g_view_h > 1.78) {
        g_view_w = g_UI_AspectRatio * g_view_h;
        g_view_x = ((float)g_width - g_view_w) * 0.5f;
    }
    if (auto pNoesisGUI = g_pNoesisGUI.lock()) {
        pNoesisGUI->sub_7FF6D4A23DC0(g_width, g_height, 0/*v7*/, 0/*rx_w*/);
    }
}
#include "optionparser.h"
enum optionIndex { UNKNOWN, LAUNCHER, TOKEN };
const option::Descriptor g_countOptsMetadata[] = { 
    {UNKNOWN,  0, "" , "",                 option::Arg::None,     "USAGE: ZwiftAdmin [options]\n\nOptions:"},
    {LAUNCHER, 0, "l", "launcher_version", option::Arg::Optional, "  --launcher_version=1.0.8     The version of the Launcher"},
    {TOKEN,    0, "t", "token",            option::Arg::Optional, "  --token=jsonToken     Login Token"},
    {UNKNOWN,  0, "" , "",                 option::Arg::None,     "\nExamples:\n  example --launcher_version=1.0.8 --token=jsonToken\n"}, {}};
std::unique_ptr<CrashReporting> g_sCrashReportingUPtr;
void LauncherUpdate(const std::string &launcherVersion) {
    //omit if launcherVersion < "1.1.1" -> restart and update
}
void ZwiftInitialize(const std::vector<std::string> &argv) {
    g_MainThread = GetCurrentThreadId();
    DWORD startTime = timeGetTime();
    auto evSysInst = EventSystem::GetInst();
    zassert(g_sCrashReportingUPtr.get() == nullptr);
    CrashReporting::Initialize(evSysInst);
    Experimentation::Initialize(evSysInst);
    auto exp = Experimentation::Instance();
    BLEModule::Initialize(exp);
    GameAssertHandler::Initialize();
    PlayerAchievementService::Initialize(evSysInst);
    NoesisPerfAnalytics::Initialize(exp);
    ClientTelemetry::Initialize(exp, evSysInst);
    GroupEvents::Initialize(exp);
    UnitTypeManager::Initialize(evSysInst, false);
    PlayerProfileCache::Initialize(evSysInst);
    GoalsManager::Initialize(evSysInst);
    ZNet::NetworkService::Initialize();
    //omit: v16 = (ZNet::NetworkService *)Cloud::CloudSyncManager::Initialize(Inst, v15);
    DataRecorder::Initialize(exp, ZNet::NetworkService::Instance(), evSysInst);
    Thread::Initialize(exp);
    //exam: --launcher_version=1.1.4 --token={"access_token":"_a_t_","expires_in":1000021600,"id_token":"_id_t","not-before-policy":1408478984,"refresh_expires_in":611975560,"refresh_token":"_r_t_","scope":"","session_state":"0846ab9a-765d-4c3f-a20c-6cac9e86e5f3","token_type":"bearer"}
    int useful_argc = __argc; char **myargv = __argv;
    useful_argc -= (useful_argc > 0); myargv += (useful_argc > 0); // skip program name argv[0] if present
    option::Stats stats(g_countOptsMetadata, useful_argc, myargv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(g_countOptsMetadata, useful_argc, myargv, &options[0], &buffer[0]);
    auto launcherVer = options[LAUNCHER].arg;
    if (launcherVer)
        LauncherUpdate(launcherVer);
    GFX_SetLoadedAssetMode(true);
    auto userPath = OS_GetUserPath();
    if (userPath) {
        char downloadPath[MAX_PATH] = {};
        sprintf_s(downloadPath, "%s/Zwift/", userPath);
        g_mDownloader.SetLocalPath(downloadPath);
    }
    g_mDownloader.SetServerURLPath("https://cdn.zwift.com/gameassets/");
    g_mDownloader.Download("MapSchedule_v2.xml", 0LL, Downloader::m_noFileTime, -1, GAME_onFinishedDownloadingMapSchedule);
    //omit: check GFX driver if no "<data>\Zwift\olddriver.ok" exist
    g_pDownloader->Update();
    ZMUTEX_SystemInitialize();
    LogInitialize(); //TODO: move up so logging early stages is available too
    OS_Initialize();
    u_setDataDirectory("data");
    InitICUBase();
    //omit watchdog init
    if (launcherVer) {
        Log("Launcher Version : %s", launcherVer);
        //OMIT g_launcherVer - it used only at PrepareAnalytics
    } else {
        Log("Legacy Launcher Used");
    }
    const float arrInitWorldTimes[] = { 4.f / 3.f, 16.f / 3.f, 11.f / 3.f };
    g_WorldTime = arrInitWorldTimes[timeGetTime() % _countof(arrInitWorldTimes)];
    SetupGameCameras();
    g_UserConfigDoc.UserLoad("prefs.xml");
    GAME_GetSuppressedLogTypes();
    GAME_SetUseErgModeInWorkouts(g_UserConfigDoc.GetBool("ZWIFT\\WORKOUTS\\USE_ERG", true, true));
    UnitTypeManager::Instance()->SetUseMetric(g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\METRIC", false, true));
    NotableMomentsManager::Init();
    evSysInst->TriggerEvent(EV_SENS_RECONN, 0);
    g_MaintainFullscreenForBroadcast = g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\MAINTAIN_FOCUS_FOR_BROADCAST", false, true);
    g_removeFanviewHints = g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\REMOVE_FANVIEW_HINTS", false, true);
    LOC_Init();
    g_ShowGraph = g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\SHOW_GRAPH", false, true);
    FitnessDeviceManager::Initialize();
    g_WADManager.LoadWADFile("assets/global.wad");
    g_WADManager.LoadWADFile("assets/Worlds/worlds.wad");
    g_WADManager.LoadWADFile("assets/Environment/Ground/textures.wad");
    ProfanityFilter::LoadProfanityList();
    g_WADManager.LoadWADFile("assets/UI/WhiteOrangeTheme/WhiteOrangeTheme.wad");
    g_WADManager.LoadWADFile("assets/UI/minimap/minimap.wad");
    g_WADManager.LoadWADFile("assets/UI/WhiteOrangeTheme/Scotty/Scotty.wad");
    g_WADManager.LoadWADFile("assets/UI/layouts/layouts.wad");
    g_WADManager.LoadWADFile("assets/Environment/GrassTextures/GrassTextures.wad");
    GameWorld::LoadWorldsData();
    g_RaceDictionary = RaceDictionary::Init();
    if (!g_RaceDictionary->LoadFromData())
        LogTyped(LOG_ERROR, "[ZwiftApp] Failed to load Race Dictionary!");
    SetupConsoleCommands();
    LOC_SetLanguageChangeCallback(LanguageChangeCallback);// GameLocalization::Init
    auto wadhLoc = g_WADManager.GetWadFileHeaderByItemName(
        "Localization/Localization.xml", WAD_ASSET_TYPE::GLOBAL, nullptr, nullptr);
    if (wadhLoc)
        LOC_Initialize(wadhLoc->FirstChar(), wadhLoc->m_fileLength, 0);
    LOC_LoadStringTable("data/Localization/Workouts/Loc_WOSelection.xml");
    /* not need, I think if (sub_7FF7196E5900(&v315, &crasher, &argv_) && v315 < 0xA)
    {
        Text_0 = GetText_0("LOC_UPDATE_REQUIRED");
        v303 = GetText_0("LOC_UPDATE_REQUIRED_MESSAGE_WINDOWS");
        MessageBoxW(0i64, v303, Text_0, 0);
        ZwiftExit(-1);
    }*/
    JM_Initialize();
    ANIM_PostInit();
    AUDIO_Init();
    GFX_Initialize();
    ZNETWORK_Initialize();
    ConnectionManager::Initialize();
    SaveActivityService::Initialize(&g_UserConfigDoc, exp, evSysInst);
    HoloReplayManager::Initialize(evSysInst, &g_UserConfigDoc);
    LanExerciseDeviceManager::Initialize();
    Log("Suceeded initializing graphics");
    VRAM_CreateRenderTarget(&g_RTPreviewWindow, 2048, 1024, true, false, true);
    VRAM_EndRenderTo(0);
    MATERIAL_Init();
    GFX_DrawInit();
    g_WADManager.LoadWADFile("assets/fonts/font.wad");
    g_ChatFontGW = &g_GiantFontW;
    g_ChatFontLW = &g_LargeFontW;
    g_GiantFontW.Load(FS_GIANTW);
    g_GiantFontW.SetScaleAndKerning(0.34108528, 0.93000001);
    ZWIFT_UpdateLoading(nullptr, false);
    switch (GFX_GetPerformanceGroup()) {
    case GPG_BASIC:
        Log("Using basic graphics profile");
        g_nSkipMipCount = 2;
        COMMAND_RunCommandsFromFile("basic");
        break;
    case GPG_MEDIUM:
        Log("Using medium graphics profile");
        g_nSkipMipCount = 1;
        COMMAND_RunCommandsFromFile("medium");
        break;
    case GPG_HIGH:
        Log("Using high graphics profile");
        g_nSkipMipCount = 0;
        COMMAND_RunCommandsFromFile("high");
        break;
    case GPG_ULTRA:
        Log("Using ultra graphics profile");
        g_nSkipMipCount = 0;
        COMMAND_RunCommandsFromFile("ultra");
        break;
    }
    if (GFX_GetPerformanceFlags() & 3) {
        GfxConfig::gLODBias = 1;
    }
    if (GFX_GetPerformanceFlags() & 4)
        g_nSkipMipCount = 1;
    if (GFX_GetPerformanceFlags() & 8)
        g_nSkipMipCount = 2;
    ZWIFT_UpdateLoading(nullptr, false);
    GFX_SetMaxFPSOnBattery(float(g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\BATTPREFS", 20, true)));
    auto s_urp = g_UserConfigDoc.GetCStr("ZWIFT\\CONFIG\\USER_RESOLUTION_PREF", nullptr, true);
    char cmdRes[32];
    const char *cmdRes_do = nullptr;
    if (s_urp) {
        sprintf_s(cmdRes, "res %s", s_urp);
        cmdRes_do = cmdRes;
    } else if (GFX_GetPerformanceFlags() & 0x80) {
        cmdRes_do = "res 1024x576";
    } else if (GFX_GetPerformanceFlags() & 0x100) {
        cmdRes_do = "res 1280x720";
    } else if (GFX_GetPerformanceFlags() & 0x800) {
        cmdRes_do = "res 960x720";
    } else if (GFX_GetPerformanceFlags() & 0x200) {
        cmdRes_do = "res 1334x750";
    } else if (GFX_GetPerformanceFlags() & 0x400) {
        cmdRes_do = "res 1536x864";
    } else if (GFX_GetPerformanceFlags() & 0x1000) {
        cmdRes_do = "res 1024x768";
    } else if (GFX_GetPerformanceFlags() & 0x2000) {
        cmdRes_do = "res 1280x960";
    } else if (GFX_GetPerformanceFlags() & 0x4000) {
        cmdRes_do = "res 1920x1080";
    } else if (GFX_GetPerformanceFlags() & 0x8000) {
        cmdRes_do = "res 1536x1152";
    } else if (GFX_GetPerformanceFlags() & 0x10000) {
        cmdRes_do = "res 1792x1344";
    } else if (GFX_GetPerformanceFlags() & 0x20000) {
        cmdRes_do = "res 2048x1536";
    } else if (GFX_GetPerformanceFlags() & 0x40000) {
        cmdRes_do = "res 3840x2160";
    }
    if (cmdRes_do) COMMAND_RunCommand(cmdRes_do);
    if (GFX_GetPerformanceFlags() & 0x10)
        COMMAND_RunCommand("sres 512x512");
    if (GFX_GetPerformanceFlags() & 0x20)
        COMMAND_RunCommand("sres 1024x1024");
    if (GFX_GetPerformanceFlags() & 0x40)
        COMMAND_RunCommand("sres 2048x2048");
    if (GFX_GetPerformanceFlags() & 0x100000)
        GFX_SetMaxFPSOnBattery(20.0);
    if (GFX_GetPerformanceFlags() & 0x80000) {
        GfxConfig::gFXAA = 0;
    }
    Log("%s %s initialized", GFX_GetVersion(), GFX_GetAPIName());
    auto v = GFX_GetVendorName();
    Log("Graphics Vendor: %s", v ? v : "UNKNOWN");
    auto r = GFX_GetRendererName();
    Log("Graphics Renderer: %s", r ? r : "UNKNOWN");
    if (GetPhysicallyInstalledSystemMemory(&g_TotalMemoryInKilobytes))
        Log("RAM: %dGB", g_TotalMemoryInKilobytes >> 20);
    int CPUInfo[4] = { -1 };
    unsigned   nExIds, i = 0;
    // Get the information associated with each extended ID.
    __cpuid(CPUInfo, 0x80000000);
    nExIds = CPUInfo[0];
    for (i = 0x80000000; i <= nExIds; ++i)
    {
        __cpuid(CPUInfo, i);
        // Interpret CPU brand string
        if (i == 0x80000002)
            memcpy(g_strCPU, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000003)
            memcpy(g_strCPU + 16, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000004)
            memcpy(g_strCPU + 32, CPUInfo, sizeof(CPUInfo));
    }
    //string includes manufacturer, model and clockspeed
    Log("CPU: %s", g_strCPU);
#if 0
    if (TotalMemoryInKilobytes <= 0x500000 && g_nSkipMipCount <= 1)
    {
        g_nSkipMipCount = 2;
        v220 = GFX_PerformanceFlags_2;
    LABEL_364:
        GFX_AddPerformanceFlags(v220);
        goto LABEL_365;
    }
    if (TotalMemoryInKilobytes <= 0x700000 && !g_nSkipMipCount)
    {
        g_nSkipMipCount = 1;
        v220 = GFX_PerformanceFlags_1;
        goto LABEL_364;
    }
LABEL_365:
    v400._Bx._Ptr = 0i64;
    v400._Mysize = 0i64;
    v400._Myres = 15i64;
    v401._Ptr = 0i64;
    *&v402 = 0i64;
    *(&v402 + 1) = 15i64;
    Bx._Ptr = 0i64;
    *&v404 = 0i64;
    *(&v404 + 1) = 15i64;
    v405._Ptr = 0i64;
    *&v406 = 0i64;
    *(&v406 + 1) = 15i64;
    v407._Bx._Ptr = 0i64;
    v407._Mysize = 0i64;
    v407._Myres = 15i64;
    v408 = 0i64;
    v409._Bx._Ptr = 0i64;
    v409._Mysize = 0i64;
    v409._Myres = 15i64;
    v410._Bx._Ptr = 0i64;
    v410._Mysize = 0i64;
    v410._Myres = 15i64;
    v411._Bx._Ptr = 0i64;
    v411._Mysize = 0i64;
    v411._Myres = 15i64;
    v412._Bx._Ptr = 0i64;
    v412._Mysize = 0i64;
    v412._Myres = 15i64;
    v221 = -1i64;
    do
        ++v221;
    while (a1321[v221]);
    string_assign(&v400, "1.32.1", v221);
    v222 = GFX_GetAPIName();
    v223 = zu::ToUpper(&coa2, v222);
    if (&Bx != v223)
    {
        if (*(&v404 + 1) >= 0x10ui64)
        {
            v224 = Bx._Ptr;
            if ((*(&v404 + 1) + 1i64) >= 0x1000)
            {
                v224 = *(Bx._Ptr - 1);
                if ((Bx._Ptr - v224 - 8) > 0x1F)
                    invalid_parameter_noinfo_noreturn();
            }
            j_j_free(v224);
        }
        *&v404 = 0i64;
        *(&v404 + 1) = 15i64;
        Bx._Buf[0] = 0;
        Bx = v223->_Bx;
        v404 = *&v223->_Mysize;
        v223->_Mysize = 0i64;
        v223->_Myres = 15i64;
        v223->_Bx._Buf[0] = 0;
    }
    string_dtr(&coa2.vptr);
    v225 = GFX_GetRendererName();
    v226 = zu::ToUpper(&coa2, v225);
    if (&v401 != v226)
    {
        if (*(&v402 + 1) >= 0x10ui64)
        {
            v227 = v401._Ptr;
            if ((*(&v402 + 1) + 1i64) >= 0x1000)
            {
                v227 = *(v401._Ptr - 1);
                if ((v401._Ptr - v227 - 8) > 0x1F)
                    invalid_parameter_noinfo_noreturn();
            }
            j_j_free(v227);
        }
        *&v402 = 0i64;
        *(&v402 + 1) = 15i64;
        v401._Buf[0] = 0;
        v401 = v226->_Bx;
        v402 = *&v226->_Mysize;
        v226->_Mysize = 0i64;
        v226->_Myres = 15i64;
        v226->_Bx._Buf[0] = 0;
    }
    string_dtr(&coa2.vptr);
    v228 = GFX_GetVendorName();
    v229 = zu::ToUpper(&coa2, v228);
    if (&v405 != v229)
    {
        if (*(&v406 + 1) >= 0x10ui64)
        {
            v230 = v405._Ptr;
            if ((*(&v406 + 1) + 1i64) >= 0x1000)
            {
                v230 = *(v405._Ptr - 1);
                if ((v405._Ptr - v230 - 8) > 0x1F)
                    invalid_parameter_noinfo_noreturn();
            }
            j_j_free(v230);
        }
        *&v406 = 0i64;
        *(&v406 + 1) = 15i64;
        v405._Buf[0] = 0;
        v405 = v229->_Bx;
        v406 = *&v229->_Mysize;
        v229->_Mysize = 0i64;
        v229->_Myres = 15i64;
        v229->_Bx._Buf[0] = 0;
    }
    string_dtr(&coa2.vptr);
    v231 = GFX_GetVersion();
    v232 = zu::ToUpper(&coa2, v231);
    string_set(&v407, v232);
    string_dtr(&coa2.vptr);
    PerformanceGroupName = GFX_GetPerformanceGroupName();
    v234 = zu::ToUpper(&coa2, PerformanceGroupName);
    string_set(&v409, v234);
    string_dtr(&coa2.vptr);
    v408 = *GFX_GetMemory(&coa1) >> 20;
    v235 = zu::ToUpper(&path, g_strCPU);
    string_set(&v410, v235);
    string_dtr(&path);
    DeviceName = HardwareInfo::GetDeviceName(&v396);
    string_set(&v411, DeviceName);
    string_dtr(&v396);
    HardwareInfo::GetOSNameVersioned(&path);
    string_set(&v412, &path);
    string_dtr(&path);
    v237 = EventSystem::GetInst();
    EventSystem::TriggerEvent(v237, EV_28, 1, &v400);
    ZWIFT_UpdateLoading(nullptr, false);
    v238 = time64(0i64);
    v239 = GameHolidayManager::Instance();
    GameHolidayManager::SetupCurrentHoliday(v239, v238);
    ANTRECEIVER_Initialize();
    ANTRECEIVER_Connect();
    v240 = "ANT USB receiver found";
    if (!ANTRECEIVER_IsConnected)
        v240 = "ANT USB receiver NOT found";
    LogTyped(LOG_ANT_IMPORTANT, v240);
    ZWIFT_UpdateLoading(nullptr, false);
    GAME_Initialize();
    LODWORD(v241) = sub_7FF719288A40(&g_UserConfigDoc, "ZWIFT\\CONFIG\    RAINER_EFFECT", COERCE_DOUBLE(1056964608i64), 1).m128_u32[0];
    GAME_SetTrainerSlopeModifier(v241);
    *(*&BikeManager::g_BikeManager->m_mainBike->gapC65[107] + 192i64) = XMLDoc::GetU32(// VirtualBikeComputer::SetTireSize
        &g_UserConfigDoc,
        "ZWIFT\\CONFIG\    IRE_CIRC",
        0x839u,
        v242);
    *&BikeManager::g_BikeManager->m_mainBike->gap498[1916] = SIG_CalcCaseInsensitiveSignature(
        "bikes/Wheels/Campagnolo_Bora_Ultra/Campagnolo_Bora_Ultra_Low_Front.gde");
    v243 = SIG_CalcCaseInsensitiveSignature("bikes/Wheels/Campagnolo_Bora_Ultra/Campagnolo_Bora_Ultra_Low_Rear.gde");
    v244 = BikeManager::g_BikeManager;
    *&BikeManager::g_BikeManager->m_mainBike->gap498[1920] = v243;
    *&v244->m_mainBike->gapC65[1419] = 0i64;
    v244->m_mainBike->gap498[876] = 1;
    v186 = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\POWERSMOOTHING", 1u, v245) == 0;
    v246 = *&BikeManager::g_BikeManager->m_mainBike->gapC65[107];
    *(v246 + 2208) = !v186;
    g_GlobalMouseOverSID = "Play_SFX_UI_MOUSEOVER_1";
    g_GlobalToggleOnSID = "Play_SFX_UI_RADIOBUTTON_ON_1";
    g_GlobalToggleOffSID = "Play_SFX_UI_RADIOBUTTON_OFF_1";
    g_GlobalSelectSID = "Play_SFX_UI_Menu_Select_1";
    GUI_Initialize(quick_exit, g_IsOnProductionServer == 0);
    GUI_SetDefaultFont(g_GiantFontW);
    ZWIFT_UpdateLoading(nullptr, false);
    g_vegetationWind[0] = 1.0;
    g_vegetationWind[3] = 0.2;
    timeGetTime();
    GameShaders::LoadAll();
    g_SimpleShaderHandle = GFX_CreateShaderFromFile("SimpleShader", 0xFFFFFFFF);
    g_WorldNoLightingHandle = GFX_CreateShaderFromFile("gde_nolighting", 0xFFFFFFFF);
    g_ShadowmapShaderHandle = GFX_CreateShaderFromFile("shadowmap", 0xFFFFFFFF);
    g_ShadowmapInstancedShaderHandle = GFX_CreateShaderFromFile("shadowmap_instanced", 0xFFFFFFFF);
    g_ShadowmapHairShaderHandle = GFX_CreateShaderFromFile("shadowmapHair", 0xFFFFFFFF);
    g_TestShaderHandle = GFX_CreateShaderFromFile("Glossy_shadowmapped", 0xFFFFFFFF);
    g_RoadShader = GFX_CreateShaderFromFile("Road", 0xFFFFFFFF);
    g_RoadAccessoryShader = GFX_CreateShaderFromFile("RoadAccessory", 0xFFFFFFFF);
    g_RoadWetShader = GFX_CreateShaderFromFile("RoadWet", 0xFFFFFFFF);
    g_RoadAccessoryWetShader = GFX_CreateShaderFromFile("RoadAccessoryWet", 0xFFFFFFFF);
    g_HeatHazeShader = GFX_CreateShaderFromFile("HeatHaze", 0xFFFFFFFF);
    g_CausticShader = GFX_CreateShaderFromFile("CausticPass", 0xFFFFFFFF);
    g_CrepuscularHandle = GFX_CreateShaderFromFile("crepuscular", 0xFFFFFFFF);
    GFX_CreateShaderFromFile("beam", 0xFFFFFFFF); // g_BeamShaderHandle
    GFX_CreateShaderFromFile("beam_inside", 0xFFFFFFFF);// g_InsideBeamShaderHandle
    g_WorldShaderHandle = GFX_CreateShaderFromFile("defaultWorld", 0xFFFFFFFF);
    g_WorldAOShaderHandle = GFX_CreateShaderFromFile("defaultWorldAO", 0xFFFFFFFF);
    g_WorldShaderBillboardedHandle = GFX_CreateShaderFromFile("defaultWorldBillboard", 0xFFFFFFFF);
    g_WorldShaderShinyHandle = GFX_CreateShaderFromFile("defaultWorldPhong", 0xFFFFFFFF);
    g_WorldShaderSimpleHandle = GFX_CreateShaderFromFile("defaultWorldSimple", 0xFFFFFFFF);
    g_HologramShader = GFX_CreateShaderFromFile("Hologram", 0xFFFFFFFF);
    GFX_CreateShaderFromFile("RainbowHologram", 0xFFFFFFFF);// g_RainbowHologramShader
    g_LockedItemShader = GFX_CreateShaderFromFile("Glossy_locked", 0xFFFFFFFF);
    g_LondonTerrainShader = GFX_CreateShaderFromFile("londonTerrainShader", 0xFFFFFFFF);
    g_LondonTerrainHeightMapShader = GFX_CreateShaderFromFile("londonTerrainShader_HeightMap", 0xFFFFFFFF);
    g_FranceTerrainShader = GFX_CreateShaderFromFile("franceTerrainShader", 0xFFFFFFFF);
    g_FranceTerrainHeightMapShader = GFX_CreateShaderFromFile("franceTerrainShader_HeightMap", 0xFFFFFFFF);
    g_BasicTerrainShader = GFX_CreateShaderFromFile("BasicTerrain", 0xFFFFFFFF);
    g_BasicTerrainHeightMapShader = GFX_CreateShaderFromFile("BasicTerrain_HeightMap", 0xFFFFFFFF);
    g_BasicTerrainNoSnowShader = GFX_CreateShaderFromFile("BasicTerrainNoSnow", 0xFFFFFFFF);
    g_BasicTerrainNoSnowHeightMapShader = GFX_CreateShaderFromFile("BasicTerrainNoSnow_HeightMap", 0xFFFFFFFF);
    g_InnsbruckTerrainHeightMapShader = GFX_CreateShaderFromFile("innsbruckTerrainShader_HeightMap", 0xFFFFFFFF);
    g_InnsbruckTerrainHandle = GFX_CreateShaderFromFile("innsbruckTerrainShader", 0xFFFFFFFF);
    g_BolognaTerrainHeightMapShader = GFX_CreateShaderFromFile("bolognaTerrainShader_HeightMap", 0xFFFFFFFF);
    g_BolognaTerrainShader = GFX_CreateShaderFromFile("bolognaTerrainShader", 0xFFFFFFFF);
    g_YorkshireTerrainShader = GFX_CreateShaderFromFile("yorkshireTerrainShader", 0xFFFFFFFF);
    g_YorkshireTerrainHeightMapShader = GFX_CreateShaderFromFile("yorkshireTerrainShader_HeightMap", 0xFFFFFFFF);
    g_RichmondTerrainShader = GFX_CreateShaderFromFile("richmondTerrainShader", 0xFFFFFFFF);
    g_RichmondTerrainHeightMapShader = GFX_CreateShaderFromFile("richmondTerrainShader_HeightMap", 0xFFFFFFFF);
    g_WorkoutHologramShader = GFX_CreateShaderFromFile("WorkoutHologramPortal", 0xFFFFFFFF);
    g_WorkoutHologramPrShader = GFX_CreateShaderFromFile("WorkoutHologramPortalRing", 0xFFFFFFFF);
    g_FinalCopyShader = GFX_CreateShaderFromFile("FinalCopy", 0xFFFFFFFF);
    g_WorldShaderTerrainHandle = GFX_CreateShaderFromFile("defaultWorld_Terrain", 0xFFFFFFFF);
    g_WorldShaderTerrainHeightMapHandle = GFX_CreateShaderFromFile("defaultWorld_TerrainHeightMap", 0xFFFFFFFF);
    g_ShadowShaderTerrainHeightMap = GFX_CreateShaderFromFile("shadowmap_TerrainHeightMap", 0xFFFFFFFF);
    g_WatopiaSpecialTileShaderHeightmap = GFX_CreateShaderFromFile("WatopiaDesert_TerrainHeightMap", 0xFFFFFFFF);
    g_WatopiaSpecialTileShader = GFX_CreateShaderFromFile("WatopiaDesert", 0xFFFFFFFF);
    if (g_ShadowShaderTerrainHeightMap == -1
        || g_WorldShaderTerrainHeightMapHandle == -1
        || g_LondonTerrainHeightMapShader == -1)
    {
        Log("Disabling heightmap shaders", v247, v248, v249);
        g_bUseTextureHeightmaps = 0;
    }
    g_WorldShaderInstancedHandle = GFX_CreateShaderFromFile("defaultWorld_instanced", 0xFFFFFFFF);
    GFX_CreateShaderFromFile("defaultWorld_notShadowed_instanced", 0xFFFFFFFF);// g_WorldShaderNoShadowsInstancedHandle
    g_WorldShaderInstancedTerrainConformingHandle = GFX_CreateShaderFromFile(
        "defaultWorld_instanced_TerrainHeightMap",
        0xFFFFFFFF);
    g_World2LayerShaderHandle = GFX_CreateShaderFromFile("defaultWorld2Layer", 0xFFFFFFFF);
    GFX_CreateShaderFromFile("defaultWorld2Layer_instanced", 0xFFFFFFFF);// g_World2LayerShaderInstancedHandle
    g_VegetationShaderHandle = GFX_CreateShaderFromFile("vegetation", 0xFFFFFFFF);
    g_VegetationShaderInstancedTerrainConformHandle = GFX_CreateShaderFromFile(
        "vegetationTerrainHeightmapInstance",
        0xFFFFFFFF);
    g_VegetationShaderInstancedHandle = GFX_CreateShaderFromFile("vegetationInstance", 0xFFFFFFFF);
    g_VegetationShadowmapShaderHandle = GFX_CreateShaderFromFile("vegetationShadowmap", 0xFFFFFFFF);
    g_VegetationShadowmapInstancedShaderHandle = GFX_CreateShaderFromFile("vegetationShadowmapInstance", 0xFFFFFFFF);
    g_WireShaderHandle = GFX_CreateShaderFromFile("wire", 0xFFFFFFFF);
    g_WireShadowShaderHandle = GFX_CreateShaderFromFile("wireShadow", 0xFFFFFFFF);
    g_BikeShaderInstancedHandle = GFX_CreateShaderFromFile("accessory_instanced", 0xFFFFFFFF);
    g_HairShaderHandle = GFX_CreateShaderFromFile("Hair", 0xFFFFFFFF);
    GFX_CreateShaderFromFile("defaultWorld_notShadowed", 0xFFFFFFFF);// g_WorldShaderNoShadowHandle
    g_SkinShader = GFX_CreateShaderFromFile("skinShader", 0xFFFFFFFF);
    g_ShadowmapSkinShader = GFX_CreateShaderFromFile("shadowmapSkin", 0xFFFFFFFF);
    g_SkinShaderHologram = GFX_CreateShaderFromFile("skinShaderHologram", 0xFFFFFFFF);
    g_grayScaleShader = GFX_CreateShaderFromFile("grayScale", 0xFFFFFFFF);
    p1[0] = 0;
    p1[1] = 1048592;
    p1[2] = 1048592;
    v379 = 0i64;
    if (GFX_GetTier())
    {
        argv_ = "GNdefaultWorld_Terrain";
        argv__8 = 0;
        v318 = GFX_CreateShader(&argv_);
        argv_ = "GNdefaultWorld_Terrain";
        argv__8 = 1;
        *a6 = GFX_CreateShader(&argv_);
        argv_ = "GNJapanTerrain";
        argv__8 = 0;
        v252 = GFX_CreateShader(&argv_);
        argv_ = "GNJapanTerrain";
        argv__8 = 1;
        v253 = GFX_CreateShader(&argv_);
        argv_ = "GNInnsbruckTerrain";
        argv__8 = 0;
        v322 = GFX_CreateShader(&argv_);
        argv_ = "GNInnsbruckTerrain";
        argv__8 = 1;
        v323 = GFX_CreateShader(&argv_);
        argv_ = "GNLondonTerrain";
        argv__8 = 0;
        v324 = GFX_CreateShader(&argv_);
        argv_ = "GNLondonTerrain";
        argv__8 = 1;
        v325 = GFX_CreateShader(&argv_);
        argv_ = "GNBolognaTerrain";
        argv__8 = 0;
        v326 = GFX_CreateShader(&argv_);
        argv_ = "GNBolognaTerrain";
        argv__8 = 1;
        v327 = GFX_CreateShader(&argv_);
        argv_ = "GNRichmondTerrain";
        argv__8 = 0;
        v330 = GFX_CreateShader(&argv_);
        argv_ = "GNRichmondTerrain";
        argv__8 = 1;
        v331 = GFX_CreateShader(&argv_);
        argv_ = "GNYorkshireTerrain";
        argv__8 = 0;
        v328 = GFX_CreateShader(&argv_);
        argv_ = "GNYorkshireTerrain";
        argv__8 = 1;
        v329 = GFX_CreateShader(&argv_);
        argv_ = "GNFranceTerrain";
        argv__8 = 0;
        v332 = GFX_CreateShader(&argv_);
        argv_ = "GNFranceTerrain";
        argv__8 = 1;
        v333 = GFX_CreateShader(&argv_);
        argv_ = "GNWatopiaDesert_Terrain";
        argv__8 = 0;
        v321 = GFX_CreateShader(&argv_);
        argv_ = "GNWatopiaDesert_Terrain";
        argv__8 = 1;
        v320 = GFX_CreateShader(&argv_);
        argv_ = "GNTerrainShadow";
        argv__8 = 0;
        v353 = GFX_CreateShader(&argv_);
        argv_ = "GNTerrainShadow";
        argv__8 = 1;
        v254 = GFX_CreateShader(&argv_);
        argv_ = "GNRoad";
        argv__8 = 0;
        v255 = GFX_CreateShader(&argv_);
        argv_ = "GNRoadAccessory";
        argv__8 = 0;
        v354 = GFX_CreateShader(&argv_);
        argv_ = "GNRoadAccessorySSR";
        argv__8 = 0;
        v355 = GFX_CreateShader(&argv_);
        argv_ = "GNRoadWet";
        argv__8 = 0;
        v356 = GFX_CreateShader(&argv_);
        argv_ = "GNdefaultWorld";
        argv__8 = 0;
        LODWORD(v357) = GFX_CreateShader(&argv_);
        argv_ = "GNdefaultWorldBillboard";
        argv__8 = 0;
        LODWORD(m_bitField) = GFX_CreateShader(&argv_);
        argv_ = "GNdefaultWorld_instanced";
        argv__8 = 0;
        v334 = GFX_CreateShader(&argv_);
        argv_ = "GNdefaultWorld_instanced_TerrainHeightMap";
        argv__8 = 0;
        v335 = GFX_CreateShader(&argv_);
        argv_ = "GNaccessory_instanced";
        argv__8 = 0;
        v336 = GFX_CreateShader(&argv_);
        argv_ = "GNSkin";
        argv__8 = 0;
        v314 = GFX_CreateShader(&argv_);
        argv_ = "GNshadowmapSkin";
        argv__8 = 0;
        v344 = GFX_CreateShader(&argv_);
        argv_ = "GNHair";
        argv__8 = 0;
        v337 = GFX_CreateShader(&argv_);
        argv_ = "GNshadowmapHair";
        argv__8 = 0;
        v343 = GFX_CreateShader(&argv_);
        argv_ = "GNGlossy_shadowmapped";
        argv__8 = 0;
        v338 = GFX_CreateShader(&argv_);
        argv_ = "GNshadowmap";
        argv__8 = 0;
        v339 = GFX_CreateShader(&argv_);
        argv_ = "GNshadowmap_instanced";
        argv__8 = 0;
        v340 = GFX_CreateShader(&argv_);
        v256 = sub_7FF719735590(0, 0);
        v259 = sub_7FF719735590(v258, v257);
        argv_ = "GNvegetation";
        LOWORD(argv__8) = v259;
        HIWORD(argv__8) = v256;
        v341 = GFX_CreateShader(&argv_);
        v260 = sub_7FF719735590(1, 0);
        v262 = sub_7FF719735590(v261 + 1, v261);
        argv_ = "GNvegetation";
        LOWORD(argv__8) = v262;
        HIWORD(argv__8) = v260;
        v342 = GFX_CreateShader(&argv_);
        v263 = sub_7FF719735590(0, 1);
        argv_ = "GNvegetation";
        LOWORD(argv__8) = v259;
        HIWORD(argv__8) = v263;
        v315 = GFX_CreateShader(&argv_);
        v264 = sub_7FF719735590(1, 1);
        argv_ = "GNvegetation";
        LOWORD(argv__8) = v262;
        HIWORD(argv__8) = v264;
        v316 = GFX_CreateShader(&argv_);
        v265 = sub_7FF719735590(1, 1);
        argv_ = "GNvegetation";
        LOWORD(argv__8) = v265;
        HIWORD(argv__8) = v260;
        *try16 = GFX_CreateShader(&argv_);
        argv_ = "GNWater";
        argv__8 = 0;
        LODWORD(crasher) = GFX_CreateShader(&argv_);
        argv_ = "GNWater";
        argv__8 = 0x10000;
        LODWORD(argv_) = GFX_CreateShader(&argv_);
        coa1.vptr = "GNBib";
        LODWORD(coa1.m_countPtr) = 0;
        v266 = GFX_CreateShader(&coa1);
        if (v318 != -1
            && *a6 != -1
            && v321 != -1
            && v320 != -1
            && v252 != -1
            && v253 != -1
            && v322 != -1
            && v323 != -1
            && v324 != -1
            && v325 != -1
            && v326 != -1
            && v327 != -1
            && v330 != -1
            && v331 != -1
            && v328 != -1
            && v329 != -1
            && v332 != -1
            && v333 != -1
            && v353 != -1
            && v254 != -1
            && v255 != -1)
        {
            v267 = v354;
            if (v354 != -1)
            {
                v268 = v355;
                if (v355 != -1)
                {
                    v269 = v356;
                    if (v356 != -1)
                    {
                        v270 = v357;
                        if (v357 != -1)
                        {
                            v271 = m_bitField;
                            if (m_bitField != -1
                                && v334 != -1
                                && v335 != -1
                                && v336 != -1
                                && v314 != -1
                                && v344 != -1
                                && v337 != -1
                                && v343 != -1
                                && v338 != -1
                                && v339 != -1
                                && v340 != -1
                                && v341 != -1
                                && v342 != -1
                                && v315 != -1
                                && v316 != -1
                                && *try16 != -1
                                && crasher != -1
                                && argv_ != -1
                                && v266 != -1
                                && GameShaders::LoadTier())
                            {
                                p1[0] = 1;
                                v380 = xmmword_7FF71A3CE710;
                                v381 = 1132593152;
                                v382 = 1195593728;
                                LOADER_UseHWInstancing(1);
                                g_SkinShader = v314;
                                g_ShadowmapSkinShader = v344;
                                g_WorldShaderHandle = v270;
                                g_WorldShaderTerrainHandle = v318;
                                g_WorldShaderTerrainHeightMapHandle = *a6;
                                g_WatopiaSpecialTileShaderHeightmap = v320;
                                g_WatopiaSpecialTileShader = v321;
                                g_InnsbruckTerrainHandle = v322;
                                g_InnsbruckTerrainHeightMapShader = v323;
                                g_LondonTerrainShader = v324;
                                g_LondonTerrainHeightMapShader = v325;
                                g_BolognaTerrainShader = v326;
                                g_BolognaTerrainHeightMapShader = v327;
                                g_YorkshireTerrainShader = v328;
                                g_YorkshireTerrainHeightMapShader = v329;
                                g_RichmondTerrainShader = v330;
                                g_RichmondTerrainHeightMapShader = v331;
                                g_FranceTerrainShader = v332;
                                g_FranceTerrainHeightMapShader = v333;
                                g_ShadowShaderTerrainHeightMap = v254;
                                g_WorldShaderInstancedHandle = v334;
                                g_WorldShaderInstancedTerrainConformingHandle = v335;
                                g_BikeShaderInstancedHandle = v336;
                                g_HairShaderHandle = v337;
                                g_ShadowmapHairShaderHandle = v343;
                                g_TestShaderHandle = v338;
                                g_ShadowmapShaderHandle = v339;
                                g_ShadowmapInstancedShaderHandle = v340;
                                g_RoadShader = v255;
                                g_RoadAccessoryShader = v267;
                                dword_7FF71A5A9AB8 = v268;
                                g_RoadWetShader = v269;
                                g_WorldShaderBillboardedHandle = v271;
                                g_VegetationShaderHandle = v341;
                                g_VegetationShaderInstancedHandle = v342;
                                g_VegetationShaderInstancedTerrainConformHandle = *try16;
                                g_VegetationShadowmapInstancedShaderHandle = v316;
                                g_VegetationShadowmapShaderHandle = v315;
                            }
                        }
                    }
                }
            }
        }
    }
    Log("[ZWIFT]: GFX_Tier %d", p1[0], v250, v251);
    GNScene::Initialize(g_GNSceneSystem, p1);
    v349 = 240;
    v350 = 135;
    v351 = 256;
    v348 = g_GNSceneSystem;
    GNView::Initialize(g_ctMainView, &v348);
    v349 = (gRT_ReflectionMap.m_dw_width + 7) >> 3;
    v350 = (gRT_ReflectionMap.m_dw_height + 7) >> 3;
    v351 = 128;
    GNView::Initialize(g_ctReflView, &v348);
    v349 = (stru_7FF71A5C5EC0.m_dw_width + 15) >> 4;
    v350 = (stru_7FF71A5C5EC0.m_dw_height + 15) >> 4;
    v351 = 4;
    v348 = &unk_7FF71A943030;
    GNView::Initialize(g_PreviewView, &v348);
    timeGetTime();
    if (g_WorldShaderHandle == -1)
    {
        MessageBoxA(0i64, "Could not find required data files.  Closing application now.", dword_7FF71A229980, 0);
        HardwareInfo::Info_dtr(&v400._Bx._Ptr);
        SuppLogs_dtr(&suppLogs);
        som_else_dtr(&v413);
        some_else_dtr(v359);
    } else
    {
        PostFX_Initialize();
        timeGetTime();
        Sky::Initialize();
        Weather::Initialize();
        timeGetTime();
        ParticulateManager::Create();
        ParticulateManager::Init();
        AccessoryManager::InitAllAccessories();
        ZWIFT_UpdateLoading(nullptr, false);
        timeGetTime();
        v272 = BikeManager::g_BikeManager;
        v273 = ExperimentationInstance();
        BikeManager::Initialize(v272, v273);
        timeGetTime();
        v274 = SteeringModule::Self();
        SteeringModule::Init(v274, BikeManager::g_BikeManager->m_mainBike);
        g_ScreenMeshHandle = LOADER_LoadGdeFile("data/screenbox.gde", 0);
        g_TrainerMeshHandle = LOADER_LoadGdeFile("data/bikes/Trainers/Zwift/Trainer.gde", 0);
        g_HandCycleTrainerMeshHandle = LOADER_LoadGdeFile("data/bikes/Trainers/Zwift/HandcycleTrainer.gde", 0);
        g_TreadmillMeshHandle = LOADER_LoadGdeFile("data/Humans/Treadmill/Treadmill.gde", 0);
        g_PaperMeshHandle = LOADER_LoadGdeFile("data/bikes/Frames/DefaultOrange/Paper.gde", 0);
        ZWIFT_UpdateLoading(nullptr, false);
        timeGetTime();
        CFont2D::Load(g_SmallFont, 0);
        CFont2D::Load(g_MediumFont, 1);
        CFont2D::Load(g_LargeFontW, 2);
        CFont2D::SetHeadAndBaseLines(g_LargeFontW, 14.0, 20.0);
        HUD_UpdateChatFont();
        CFont2D::SetScaleAndKerning(g_LargeFontW, 0.60000002, 0.88700002);
        CFont2D::SetLanguageKerningScalar(g_LargeFontW, 3, 1.3);
        timeGetTime();
        timeGetTime();
        g_ButterflyTexture = GFX_CreateTextureFromTGAFile("blue_butterfly.tga", -1, 1);
        g_RedButterflyTexture = GFX_CreateTextureFromTGAFile("white_butterfly.tga", -1, 1);
        g_MonarchTexture = GFX_CreateTextureFromTGAFile("monarch.tga", -1, 1);
        g_FireflyTexture = GFX_CreateTextureFromTGAFile("firefly.tga", -1, 1);
        GFX_CreateTextureFromTGAFile("grid1meter.tga", -1, 1);// g_GridTexture
        g_CausticTexture = GFX_CreateTextureFromTGAFile("Environment/Shared/Effects/Caustics/Caustics_00.tga", -1, 1);
        GFX_SetAnimatedTextureFramerate(g_CausticTexture, 20.0);
        g_GrassTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/grass.tga", -1, 1);
        GFX_CreateTextureFromTGAFile("Environment/Ground/WindsweptGrassD.tga", -1, 1);// g_JapanGrassTexture
        g_GravelMtnGrassTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/DesertGroundD.tga", -1, 1);
        GFX_CreateTextureFromTGAFile("Environment/Buildings/CentralLondon/Textures/Concrete02.tga", -1, 1);// g_ConcreteTexture
        g_InnsbruckConcreteTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/InnsbruckConcrete.tga", -1, 1);
        GFX_CreateTextureFromTGAFile("Environment/Ground/InnsbruckConcrete_NRM.tga", -1, 1);// g_InnsbruckConcreteNRMTexture
        GFX_CreateTextureFromTGAFile("Environment/Ground/RichmondConcreteD.tga", -1, 1);// g_RichmondConcreteTexture
        g_ParisConcreteTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/FranceConcrete.tga", -1, 1);
        g_DefaultNormalMapNoGloss = GFX_CreateTextureFromTGAFile("defaultNormalMap_nogloss.tga", -1, 1);
        g_RoadDustTexture = GFX_CreateTextureFromTGAFile("FX/Particles/RoadDust_D.tga", -1, 1);
        g_GravelDustTexture = GFX_CreateTextureFromTGAFile("FX/Particles/GravelDust_D.tga", -1, 1);
        g_SandTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/sand.tga", -1, 1);
        g_SandNormalTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/sand_NRM.tga", -1, 1);
        g_RockTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/Rock.tga", -1, 1);
        g_FranceRockTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/VentouxRockD.tga", -1, 1);
        g_FranceRockNTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/VentouxRockN.tga", -1, 1);
        g_RockNormalTexture = GFX_CreateTextureFromTGAFile("Environment/Ground/Rock_NRM.tga", -1, 1);
        GFX_CreateTextureFromTGAFile("shadow.tga", -1, 1);// g_ShadowTexture
        GFX_CreateTextureFromTGAFile("black.tga", -1, 1);// g_BlackTexture
        g_ShowroomFloorTexture = GFX_CreateTextureFromTGAFile("showroomfloor.tga", -1, 1);
        GFX_CreateTextureFromTGAFile("beams_bg.tga", -1, 1);// g_LightBeamsTexture
        g_HeadlightTexture = GFX_CreateTextureFromTGAFile("proj_headlight.tga", -1, 1);
        GFX_CreateTextureFromTGAFile("beam.tga", -1, 1);// g_BeamTexture
        g_VignetteTexture = GFX_CreateTextureFromTGAFile("vignette_evenless.tga", -1, 1);
        GFX_CreateTextureFromTGAFile("particle2.tga", -1, 1);// g_ParticleTexture
        GNScene::SetDefaultEnvironmentReflection(g_GNSceneSystem);
        INSTANCING_CreateDitherTex();
        ArtConfig::LoadArtConfig();
        ZFeatureManager::Construct();
        v275 = ZFeatureManager::Instance();
        ZFeatureManager::AddFeature(v275, &gPlayerHighlight);
        v276 = ZFeatureManager::Instance();
        ZFeatureManager::AddFeature(v276, &gHoloEffect);
        v277 = ZFeatureManager::Instance();
        sub_7FF71971C7E0(v277);
        v278 = ZFeatureManager::Instance();
        ZFeatureManager::LoadConfig(v278);
        LODWORD(v272) = GFX_CreateShaderFromFile("defaultParticles", 0xFFFFFFFF);
        argv_ = operator new(0xD0ui64);
        pDustSystem = ParticleSystem_ctr(argv_, 0x3E8u, DustParticleUpdate, 0i64, 0i64, 0i64, 0i64);
        g_pDustSystem = pDustSystem;
        pDustSystem[4] = g_RoadDustTexture;
        pDustSystem[6] = 0;
        pDustSystem[47] = v272;
        PARTICLESYS_Register(pDustSystem);
        argv_ = operator new(0xD0ui64);
        g_pConfettiSystem = ParticleSystem_ctr(argv_, 0xBB8u, ConfettiParticleUpdate, 0i64, 0i64, 0i64, 0i64);
        TextureFromTGAFile = GFX_CreateTextureFromTGAFile("FX/Particles/confetti03_sheet.tga", -1, 1);
        v281 = g_pConfettiSystem;
        *(g_pConfettiSystem + 16) = TextureFromTGAFile;
        *(v281 + 20) = 1i64;
        *(v281 + 188) = v272;
        PARTICLESYS_Register(v281);
        Bib::InitOnce();
        timeGetTime();
        ZWIFT_UpdateLoading(nullptr, false);
        timeGetTime();
        HUD_Initialize();
        if (!UI_DialogPointer(61))
            UI_CreateDialog(UID_CONNECTION_NOTIFICATIONS, 0i64, 0i64, v2);
        timeGetTime();
        ZWIFT_UpdateLoading(nullptr, false);
        U32 = g_UserConfigDoc.GetU32("ZWIFT\\DEVICES\\LASTTRAINERDEVICE", 0xFFFFFFFF, v282);
        if ((U32 - 43) <= 0xFFFFFFD3 && (v284 = ZwiftPowers::GetInst(), (Power = ZwiftPowers::GetPower(v284, U32)) != 0))
        {
            *(*&BikeManager::g_BikeManager->m_mainBike->gapC65[107] + 552i64) = Power;
        } else
        {
            XMLDoc::ClearPath(&g_UserConfigDoc, "ZWIFT\\DEVICES\\LASTTRAINERDEVICE");
            XMLDoc::ClearPath(&g_UserConfigDoc, "ZWIFT\\DEVICES\\LASTSPEEDDEVICE");
        }
        v286 = EventSystem::GetInst();
        EventSystem::SubscribeFunc(v286, EV_SLIPPING_ON, PopWheelSlippingMessage);
        v287 = EventSystem::GetInst();
        EventSystem::SubscribeFunc(v287, EV_SLIPPING_OFF, ClearWheelSlippingMessage);
        v288 = EventSystem::GetInst();
        EventSystem::SubscribeFunc(v288, EV_BC_PROMPT, BroadcastPrompt);
        glClearColor(0.21176472, 0.24313727, 0.27843139, 0.0);
        sub_7FF719C1B700(g_mainWindow, sub_7FF719765950);// ZwiftAppKeyProcessorManager::Init {
        sub_7FF719C1B7A0(g_mainWindow, sub_7FF719765AA0);
        sub_7FF719C1B7E0(g_mainWindow, sub_7FF71974E010);
        sub_7FF719C1B760(g_mainWindow, sub_7FF71974DC50);
        sub_7FF719C1B820(g_mainWindow, sub_7FF71974DA90);
        sub_7FF719C1C0E0(g_mainWindow, sub_7FF71974CC70);
        sub_7FF719C1C1F0(g_mainWindow, resize);
        sub_7FF719C1C0A0(g_mainWindow, sub_7FF719735860);
        sub_7FF719C1C120(g_mainWindow, sub_7FF719761530);
        v289 = sub_7FF719763B40();
        sub_7FF7197650F0(v289);                     // ZwiftAppKeyProcessorManager::Init }
        SetIcon();
        Time = time64(0i64) - 14400;                // MAP_SCHEDULE_GMT_4_OFFSET
        GAME_GetMapForTime(&Time);
        v291 = g_UserConfigDoc.GetU32("ZWIFT\\WORLD", 0, v290);
        if (!GAME_IsWorldIDAvailableViaPrefsFile(v291))
            v291 = 0;
        GFX_SetLoadedAssetMode(0);
        BillboardInfoDatabase::LoadBillboardInfoDatabase(&g_BillboardInfo);
        ShrubHelperInfoDatabase::LoadShrubHelperInfoDatabase(&g_ShrubHelperInfo);
        RegionsDatabase::LoadRegionsDatabase();
        GAME_LoadLevel(v291, v292, v293, v294);
        AccessoryManager::CreateSegmentJerseys();
        DetermineNoesisFeatureFlags();
        if (v422)
        {
            Log("Got an access token", v295, v296, v297);
            g_lastStartupFlowState = v422;
            g_gameStartupFlowState = ZSF_1;
        } else
        {
            Log("No username specified", v295, v296, v297);
            g_lastStartupFlowState = 0i64;
            g_gameStartupFlowState = ZSF_LOGIN;
        }
        ZSF_SwitchState(g_gameStartupFlowState, g_lastStartupFlowState);
        v300 = UnicodeString_ctr(&coa2, 1, L"Ride On.", -1);
        v301 = UnicodeString_c_str(v300);
        ZWIFT_UpdateLoading(v301, 1);
        UnicodeString_dtr(&coa2);
        ANTRECEIVER_PostConnect();
        g_TotalLoadTimeInSeconds = (timeGetTime() - g_GlobalAppStartTime) * 0.001;
        if (v423)
        {
            GameDictionary::Create();
            ZwiftExit(0);
        }
        VideoCapture::InitVideoCapture();
        HardwareInfo::Info_dtr(&v400._Bx._Ptr);
        SuppLogs_dtr(&suppLogs);
        som_else_dtr(&v413);
        some_else_dtr(v359);
    }
#endif
}

void EndGameSession(bool bShutDown) {
    //TODO
    //UI_CloseDialog(UID_CONNECTION_NOTIFICATIONS);
    ShutdownSingletons();
    //TODO
    Log("ZwiftApp Gracefully Shutdown");
    //TODO
}

void ShutdownSingletons() {
    zassert(g_sCrashReportingUPtr.get() != nullptr);
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down VideoCapture");
    VideoCapture::ShutdownVideoCapture();
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down Powerups");
    Powerups::g_pPowerups.reset(); //DestroyPowerups
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down IoCpp");
    IoCPP::Set(nullptr);
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down NoesisPerfAnalytics");
    if (NoesisPerfAnalytics::IsInitialized())
        NoesisPerfAnalytics::Shutdown();
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down GoalsManager");
    GoalsManager::Shutdown();
   //CrashReporting::AddBreadcrumb(0i64, "Shutting down UnitTypeManager");
    UnitTypeManager::Shutdown();
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down ClientTelemetry");
    ClientTelemetry::Shutdown();
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down SaveActivityService");
    if (SaveActivityService::IsInitialized())
        SaveActivityService::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down ConnectionManager");
    if (ConnectionManager::IsInitialized())
        ConnectionManager::DestroyInstance();
    //CrashReporting::AddBreadcrumb("Shutting down GameAssertHandler");
    GameAssertHandler::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down Localization");
    LOC_Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down GroupEvents");
    zassert("sExperimentationUPtr.get() != nullptr");
    GroupEvents::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down PlayerAchievementService");
    if (PlayerAchievementService::IsInitialized())
        PlayerAchievementService::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down HoloReplayManager");
    if (HoloReplayManager::IsInitialized())
        HoloReplayManager::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down BLEModule");
    if (BLEModule::IsInitialized())
        BLEModule::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down DataRecorder");
    if (DataRecorder::IsInitialized())
        DataRecorder::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down Experimentation");
    if (Experimentation::IsInitialized())
        Experimentation::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down ZFeatureManager");
    if (ZFeatureManager::IsInitialized())
        ZFeatureManager::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down ZNet::NetworkService");
    if (ZNet::NetworkService::IsInitialized())
        ZNet::NetworkService::Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down zwift_network");
    ZNETWORK_Shutdown();
    //CrashReporting::AddBreadcrumb("Shutting down CrashReporting");
    CrashReporting::Shutdown();
    if (EventSystem::IsInitialized())
        EventSystem::Destroy();
}
