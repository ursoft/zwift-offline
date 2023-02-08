#include "ZwiftApp.h"

float g_kwidth, g_kheight, g_view_x, g_view_y, g_view_w, g_view_h;
int g_width, g_height;

std::weak_ptr<NoesisLib::NoesisGUI> g_pNoesisGUI;

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
    {LAUNCHER, 0, "l", "launcher_version", option::Arg::Optional, "  --launcher_version=1.0.8 \tThe version of the Launcher"},
    {TOKEN,    0, "t", "token",            option::Arg::Optional, "  --token=jsonToken \tLogin Token"},
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
        LOC_Initialize(&wadhLoc->m_firstChar, wadhLoc->m_fileLength, 0);
    LOC_LoadStringTable("data/Localization/Workouts/Loc_WOSelection.xml");
    /* not need, I think if (sub_7FF7196E5900(&v315, &crasher, &argv_) && v315 < 0xA)
    {
        Text_0 = GetText_0("LOC_UPDATE_REQUIRED");
        v303 = GetText_0("LOC_UPDATE_REQUIRED_MESSAGE_WINDOWS");
        MessageBoxW(0i64, v303, Text_0, 0);
        ZwiftExit(-1);
    }*/
#if 0
    JM_Initialize();
    ANIM_PostInit();
    AUDIO_Init();
    g_MinimalUI = XMLDoc::GetS32(&g_UserConfigDoc, "ZWIFT\\CONFIG\\MINIMAL_UI", 0, v106) == 0;// GFX_Initialize (no param)
    g_WorkoutDistortion = XMLDoc::GetBool(&g_UserConfigDoc, "ZWIFT\\CONFIG\\WORKOUTDISTORTION", 1, 1);
    g_bFullScreen = XMLDoc::GetS32(&g_UserConfigDoc, "ZWIFT\\CONFIG\\FULLSCREEN", 0, v107) != 0;
    WINWIDTH = XMLDoc::GetS32(&g_UserConfigDoc, "ZWIFT\\CONFIG\\WINWIDTH", 0, v108);
    WINHEIGHT = XMLDoc::GetS32(&g_UserConfigDoc, "ZWIFT\\CONFIG\\WINHEIGHT", 0, v110);
    PREFERRED_MONITOR = XMLDoc::GetU32(&g_UserConfigDoc, "ZWIFT\\CONFIG\\PREFERRED_MONITOR", 0xFFFFFFFF, v112);
    VSYNC = XMLDoc::GetS32(&g_UserConfigDoc, "ZWIFT\\CONFIG\\VSYNC", 1u, v114);
    GPU = XMLDoc::GetU32(&g_UserConfigDoc, "ZWIFT\\CONFIG\\GPU", 0xFFFFFFFF, v116);
    GFX_TIER = XMLDoc::GetS32(&g_UserConfigDoc, "ZWIFT\\CONFIG\\GFX_TIER", 0xFFFFFFFF, v118);
    Log("Initializing graphics window of size %d x %d", WINWIDTH, WINHEIGHT, v120);
    v366 = 0;
    v368 = 0;
    v369 = 0;
    v373 = 0;
    LODWORD(v376._Myend) = 0;
    *&v364 = WINWIDTH;
    v365 = WINHEIGHT;
    v367 = g_bFullScreen;
    v374 = PREFERRED_MONITOR;
    v375 = 0i64;
    v376._Myfirst = 0i64;
    v376._Mylast = qword_7FF71A893CD0;
    v370 = VSYNC;
    v371 = GPU;
    v372 = GFX_TIER;
    WORD2(v376._Myend) = 266;
    if (!GFX_Initialize(&v364))
    {
        Log("Failed initializing graphics", v121, v122, v123);
        v304 = GetText("LOC_ERROR_NO_GRAPHICS");
        MsgBoxAndExit(v304);
    }
    RendererName = GFX_GetRendererName();
    v396._Bx._Ptr = 0i64;
    v396._Mysize = 0i64;
    v396._Myres = 15i64;
    v125 = -1i64;
    do
        ++v125;
    while (RendererName[v125]);
    string_assign(&v396, RendererName, v125);
    VendorName = GFX_GetVendorName();
    path._Bx._Ptr = 0i64;
    path._Mysize = 0i64;
    path._Myres = 15i64;
    v127 = -1i64;
    do
        ++v127;
    while (VendorName[v127]);
    string_assign(&path, VendorName, v127);
    coa1.vptr = 0i64;
    v387 = 0i64;
    v388 = 0i64;
    v128 = &v396;
    v313 = v396._Myres >= 0x10ui64;
    v129 = v396._Bx._Ptr;
    if (v396._Myres >= 0x10ui64)
        v128 = v396._Bx._Ptr;
    Mysize = v396._Mysize;
    if (v396._Mysize >= 0x10ui64)
    {
        v131 = v396._Mysize | 0xF;
        argv_ = (v396._Mysize | 0xF);
        if ((v396._Mysize | 0xFui64) > 0x7FFFFFFFFFFFFFFFi64)
            v131 = 0x7FFFFFFFFFFFFFFFi64;
        coa1.vptr = stl_allocator_new(v131 + 1);
        memmove(coa1.vptr, v128, Mysize + 1);
    } else
    {
        coa1 = v128->_Bx;
        v131 = 15i64;
    }
    v132 = Mysize;
    v388 = v131;
    v387 = Mysize;
    Str[0] = 0i64;
    v398 = 0i64;
    v399 = 0i64;
    p_path = &path;
    Myres = path._Myres;
    v312 = path._Myres >= 0x10ui64;
    v135 = path._Bx._Ptr;
    if (path._Myres >= 0x10ui64)
        p_path = path._Bx._Ptr;
    v136 = path._Mysize;
    if (path._Mysize >= 0x10ui64)
    {
        v137 = path._Mysize | 0xF;
        argv_ = (path._Mysize | 0xF);
        if ((path._Mysize | 0xFui64) > 0x7FFFFFFFFFFFFFFFi64)
            v137 = 0x7FFFFFFFFFFFFFFFi64;
        Str[0] = stl_allocator_new(v137 + 1);
        memmove(Str[0], p_path, v136 + 1);
        v399 = v137;
        v131 = v388;
        v132 = v387;
    } else
    {
        *Str = p_path->_Bx;
        v399 = 15i64;
    }
    v398 = v136;
    p_coa1 = &coa1;
    if (v131 < 0x10)
    {
        v140 = &coa1 + v132;
        v139 = &coa1;
    } else
    {
        v139 = coa1.vptr;
        v140 = coa1.vptr + v132;
        p_coa1 = coa1.vptr;
    }
    std::foreach(&argv_, v139, v140, p_coa1, toupper);
    v141 = Str;
    if (v399 < 0x10)
    {
        v143 = Str + v398;
        v142 = Str;
    } else
    {
        v142 = Str[0];
        v143 = &Str[0][v398];
        v141 = Str[0];
    }
    std::foreach(&argv_, v142, v143, v141, toupper);
    if (Mysize)
    {
        v144 = &coa1;
        if (v388 >= 0x10)
            v144 = coa1.vptr;
        if (strstr(v144, "HD GRAPHICS 2"))
            goto LABEL_220;
        v145 = &coa1;
        if (v388 >= 0x10)
            v145 = coa1.vptr;
        if (strstr(v145, "HD GRAPHICS 3"))
        {
        LABEL_220:
            if (v398)
            {
                v146 = Str;
                if (v399 >= 0x10)
                    v146 = Str[0];
                if (strstr(v146, "INTEL"))
                {
                    v305 = sub_7FF71917CB40(&argv_, &path, word_7FF71A2A1168);
                    sub_7FF71917CBB0(&coa2, v305, &v396);
                    j_vector_dtr(&argv_);
                    v306 = sub_7FF7190E67D0(&coa2);
                    v307 = GetText("LOC_UPDATE_GFX_MESSAGE");
                    sprintf_s(Text, 0x400ui64, v307, v306);
                    v308 = sub_7FF71999156C(Text);
                    v309 = GetText_0("LOC_UPDATE_GFX_TITLE");
                    MessageBoxW(0i64, v308, v309, 0);
                    ZwiftExit(-1);
                }
            }
        }
    }
    if (v399 >= 0x10)
    {
        v147 = Str[0];
        if (v399 + 1 >= 0x1000)
        {
            v147 = *(Str[0] - 1);
            if ((Str[0] - v147 - 8) > 0x1F)
                invalid_parameter_noinfo_noreturn();
        }
        j_j_free(v147);
    }
    v398 = 0i64;
    v399 = 15i64;
    LOBYTE(Str[0]) = 0;
    if (v388 >= 0x10)
    {
        v148 = coa1.vptr;
        if (v388 + 1 >= 0x1000)
        {
            v148 = *(coa1.vptr - 1);
            if ((coa1.vptr - v148 - 8) > 0x1F)
                invalid_parameter_noinfo_noreturn();
        }
        j_j_free(v148);
    }
    v387 = 0i64;
    v388 = 15i64;
    LOBYTE(coa1.vptr) = 0;
    if (v312)
    {
        v149 = v135;
        if ((Myres + 1) >= 0x1000)
        {
            v135 = *(v135 - 1);
            if ((v149 - v135 - 8) > 0x1F)
                invalid_parameter_noinfo_noreturn();
        }
        j_j_free(v135);
    }
    path._Mysize = 0i64;
    path._Myres = 15i64;
    path._Bx._Buf[0] = 0;
    if (v313)
    {
        v150 = v129;
        if ((v396._Myres + 1) >= 0x1000)
        {
            v129 = *(v129 - 1);
            if ((v150 - v129 - 8) > 0x1F)
                invalid_parameter_noinfo_noreturn();
        }
        j_j_free(v129);
    }
    v396._Mysize = 0i64;
    v396._Myres = 15i64;
    v396._Bx._Buf[0] = 0;
    glfwGetWindowSize(g_mainWindow, &WIN32_WindowWidth, &WIN32_WindowHeight);
    g_width = WIN32_WindowWidth;
    v152 = WIN32_WindowHeight;
    g_height = WIN32_WindowHeight;
    if (!WIN32_WindowWidth || !WIN32_WindowHeight)
    {
        if (IsDebuggerPresent_1())
            __debugbreak();
        if (ZwiftAssert::BeforeAbort(
            "WIN32_WindowWidth != 0 && WIN32_WindowHeight != 0",
            "D:\\git\\zwift-game-client\\Projects\\ZwiftApp\\CODE\\ZwiftApp.cpp",
            0x494Au,
            0))
        {
            ZwiftAssert::Abort();
        }
        v152 = g_height;
    }
    v153 = v152;
    v154 = g_width;
    if ((g_width / v152) <= 1.78)
    {
        v155 = 0.0;
    } else
    {
        v153 = v152;
        v154 = VRAM_GetUIAspectRatio() * v152;
        v155 = (g_width - v154) * 0.5;
    }
    g_view_w = v154;
    g_view_h = v153;
    g_view_x = v155;
    g_view_y = 0.0;
    v156 = 0i64;
    do
    {
        v157 = *(&aZwiftConfig + v156);
        *(&coa1.vptr + v156++) = v157;
    } while (v157);                               // end of GFX_Initialize
    strcpy(&coa1.m_countPtr + 5, "PERF");
    perf_flags = XMLDoc::GetU32(&g_UserConfigDoc, &coa1, 0xFFFFFFFF, v151);
    if (perf_flags != (GPF_NO_AUTO_BRIGHT | GFX_PerformanceFlags_2000000 | GFX_PerformanceFlags_1000000 | GFX_PerformanceFlags_800000 | GFX_PerformanceFlags_400000 | GFX_PerformanceFlags_200000 | GFX_PerformanceFlags_100000 | GFX_PerformanceFlags_80000 | GFX_PerformanceFlags_40000 | GFX_PerformanceFlags_20000 | GFX_PerformanceFlags_10000 | GFX_PerformanceFlags_8000 | GFX_PerformanceFlags_4000 | GFX_PerformanceFlags_2000 | GFX_PerformanceFlags_1000 | GFX_PerformanceFlags_800 | GFX_PerformanceFlags_400 | GFX_PerformanceFlags_200 | GFX_PerformanceFlags_100 | GFX_PerformanceFlags_80 | GFX_PerformanceFlags_40 | GFX_PerformanceFlags_20 | GFX_PerformanceFlags_10 | GFX_PerformanceFlags_8 | GFX_PerformanceFlags_4 | GFX_PerformanceFlags_2 | GFX_PerformanceFlags_1 | 0xF8000000))
        GFX_AddPerformanceFlags_0(perf_flags);
    ZNETWORK_Initialize();
    if (g_ConnectionManagerUPtr)                // ConnectionManager::Initialize
    {
        if (IsDebuggerPresent_1())
            __debugbreak();
        if (ZwiftAssert::BeforeAbort(
            "sConnectionManagerUPtr.get() == nullptr",
            "D:\\git\\zwift-game-client\\Projects\\ZwiftApp\\CODE\\Connections\\ConnectionManager.cpp",
            0x1Cu,
            0))
        {
            ZwiftAssert::Abort();
        }
    }
    v159 = operator new(0x138ui64);
    argv_ = v159;
    memset(v159, 0, 0x138ui64);
    v160 = ConnectionManager_ctr(v159);
    v162 = g_ConnectionManagerUPtr;
    g_ConnectionManagerUPtr = v160;
    if (v162)
        ConnectionManager_dtr(v161, v162);
    v163 = sub_7FF71916CB60();
    v164 = EventSystem::GetInst();
    v165 = ExperimentationInstance();
    if (!g_SaveActivityServicePtr)
        goto LABEL_267;
    if (IsDebuggerPresent_1())
        __debugbreak();
    if (ZwiftAssert::BeforeAbort(
        "sSaveActivityServicePtr.get() == nullptr",
        "D:\\git\\zwift-game-client\\Projects\\ZwiftApp\\CODE\\SaveActivity\\SaveActivityService.cpp",
        0x15Cu,
        0))
    {
        ZwiftAssert::Abort();
    }
    if (!g_SaveActivityServicePtr)              // SaveActivityService::Initialize
    {
    LABEL_267:
        argv_ = operator new(0x128ui64);
        v166 = SaveActivityService_ctr(argv_, &g_UserConfigDoc, v165, v164);
        v167 = g_SaveActivityServicePtr;
        g_SaveActivityServicePtr = v166;
        if (v167)
        {
            (**v167)(v167, 1i64);
            v166 = g_SaveActivityServicePtr;
        }
        ConnectionManager::Subscribe(v163, v166);
    }
    v168 = EventSystem::GetInst();
    if (g_HoloReplayManagerPtr)                 // HoloReplayManager::Initialize
    {
        if (IsDebuggerPresent_1())
            __debugbreak();
        if (ZwiftAssert::BeforeAbort(
            "sHoloReplayManagerPtr.get() == nullptr",
            "D:\\git\\zwift-game-client\\Projects\\ZwiftApp\\CODE\\FeatureDrivenLocalAI\\HoloReplay\\HoloReplayManager.cpp",
            0x38u,
            0))
        {
            ZwiftAssert::Abort();
        }
    }
    argv_ = operator new(0x110ui64);
    v169 = sub_7FF71927C0E0(argv_, v168, &g_UserConfigDoc);
    v173 = g_HoloReplayManagerPtr;
    g_HoloReplayManagerPtr = v169;
    if (v173)
        HoloReplayManager::Shutdown(v170, v173);
    if (!g_LanExerciseDeviceManager)            // LanExerciseDeviceManager::Initialize
    {
        v174 = operator new(1ui64);
        argv_ = v174;
        coa2.vptr = &std::_Func_impl_no_alloc<void (*)(zwift_network::LanExerciseDeviceInfo const &, std::vector<unsigned char> const &), void, zwift_network::LanExerciseDeviceInfo const &, std::vector<unsigned char> const &>::`vftable';
            coa2.m_countPtr = sub_7FF7192585A0;
        p_coa2 = &coa2;
        sub_7FF719997E50(&coa2);
        if (p_coa2)
        {
            v175 = &coa2;
            LOBYTE(v175) = p_coa2 != &coa2;
            (*(p_coa2->vptr + 4))(p_coa2, v175);
            p_coa2 = 0i64;
        }
        path._Bx._Ptr = &std::_Func_impl_no_alloc<void (*)(zwift_network::LanExerciseDeviceInfo const &), void, zwift_network::LanExerciseDeviceInfo const &>::`vftable';
            * (&path._Bx._Ptr + 1) = sub_7FF719258790;
        v395 = &path;
        sub_7FF719997E60(&path);
        if (v395)
        {
            v176 = &path;
            LOBYTE(v176) = v395 != &path;
            (*(v395->_Bx._Ptr + 4))(v395, v176);
            v395 = 0i64;
        }
        byte_7FF71A8914D0 = 0;
        sub_7FF71925A970(&qword_7FF71A89BAD8);
        g_LanExerciseDeviceManager = v174;
    }
    Log("Suceeded initializing graphics", v173, v171, v172);
    LOBYTE(v177) = 1;
    VRAM_CreateRenderTarget_0(&stru_7FF71A5C5EC0, 2048, 1024, v177, 0, 1);
    VRAM_EndRenderTo(0);
    MATERIAL_Init();
    GFX_DrawInit();
    WADManager::LoadWADFile(WADManager::g_WADManager, "assets/fonts/font.wad");
    g_ChatFontGW = g_GiantFontW;
    g_ChatFontLW = g_LargeFontW;
    CFont2D::Load(g_GiantFontW, 3);
    CFont2D::SetScaleAndKerning(g_GiantFontW, 0.34108528, 0.93000001);
    ZWIFT_UpdateLoading(0i64, 0);
    PerformanceGroup = GFX_GetPerformanceGroup();
    if (PerformanceGroup)
    {
        v182 = PerformanceGroup - 1;
        if (v182)
        {
            v183 = v182 - 1;
            if (v183)
            {
                if (v183 == 1)
                {
                    Log("Using ultra graphics profile", v179, v180, v181);
                    g_nSkipMipCount = 0;
                    COMMAND_RunCommandsFromFile("ultra");
                }
            } else
            {
                Log("Using high graphics profile", v179, v180, v181);
                g_nSkipMipCount = 0;
                COMMAND_RunCommandsFromFile("high");
            }
        } else
        {
            Log("Using medium graphics profile", v179, v180, v181);
            g_nSkipMipCount = 1;
            COMMAND_RunCommandsFromFile("medium");
        }
    } else
    {
        Log("Using basic graphics profile", v179, v180, v181);
        g_nSkipMipCount = 2;
        COMMAND_RunCommandsFromFile("basic");
    }
    if ((GFX_GetPerformanceFlags() & 1) != 0)
    {
        v184 = 1;
        if (dword_7FF71A5C60C8 < 1)
            v184 = dword_7FF71A5C60C8;
        if (dword_7FF71A5C60C4 > v184)
            v184 = dword_7FF71A5C60C4;
        GfxConfig::gLODBias = v184;
    }
    if ((GFX_GetPerformanceFlags() & 2) != 0)
    {
        v185 = 1;
        if (dword_7FF71A5C60C8 < 1)
            v185 = dword_7FF71A5C60C8;
        if (dword_7FF71A5C60C4 > v185)
            v185 = dword_7FF71A5C60C4;
        GfxConfig::gLODBias = v185;
    }
    v186 = (GFX_GetPerformanceFlags() & 4) == 0;
    v187 = g_nSkipMipCount;
    if (!v186)
        v187 = 1;
    g_nSkipMipCount = v187;
    v186 = (GFX_GetPerformanceFlags() & 8) == 0;
    v188 = g_nSkipMipCount;
    if (!v186)
        v188 = 2;
    g_nSkipMipCount = v188;
    ZWIFT_UpdateLoading(0i64, 0);
    S32 = XMLDoc::GetS32(&g_UserConfigDoc, "ZWIFT\\CONFIG\\BATTPREFS", 0x14u, v189);
    GFX_SetMaxFPSOnBattery(S32);
    CStr = XMLDoc::GetCStr(&g_UserConfigDoc, "ZWIFT\\CONFIG\\USER_RESOLUTION_PREF", 0i64, 1);
    if (CStr)
    {
        sprintf_s(Text, 0x400ui64, "res %s", CStr);
        v192 = Text;
    } else if ((GFX_GetPerformanceFlags() & 0x80u) == 0)
    {
        if ((GFX_GetPerformanceFlags() & 0x100) != 0)
        {
            v192 = "res 1280x720";
        } else if ((GFX_GetPerformanceFlags() & 0x800) != 0)
        {
            v192 = "res 960x720";
        } else if ((GFX_GetPerformanceFlags() & 0x200) != 0)
        {
            v192 = "res 1334x750";
        } else if ((GFX_GetPerformanceFlags() & 0x400) != 0)
        {
            v192 = "res 1536x864";
        } else if ((GFX_GetPerformanceFlags() & 0x1000) != 0)
        {
            v192 = "res 1024x768";
        } else if ((GFX_GetPerformanceFlags() & 0x2000) != 0)
        {
            v192 = "res 1280x960";
        } else if ((GFX_GetPerformanceFlags() & 0x4000) != 0)
        {
            v192 = "res 1920x1080";
        } else if ((GFX_GetPerformanceFlags() & 0x8000) != 0)
        {
            v192 = "res 1536x1152";
        } else if ((GFX_GetPerformanceFlags() & 0x10000) != 0)
        {
            v192 = "res 1792x1344";
        } else if ((GFX_GetPerformanceFlags() & 0x20000) != 0)
        {
            v192 = "res 2048x1536";
        } else
        {
            if ((GFX_GetPerformanceFlags() & 0x40000) == 0)
                goto LABEL_334;
            v192 = "res 3840x2160";
        }
    } else
    {
        v192 = "res 1024x576";
    }
    COMMAND_RunCommand(v192);
LABEL_334:
    if ((GFX_GetPerformanceFlags() & 0x10) != 0)
        COMMAND_RunCommand("sres 512x512");
    if ((GFX_GetPerformanceFlags() & 0x20) != 0)
        COMMAND_RunCommand("sres 1024x1024");
    if ((GFX_GetPerformanceFlags() & 0x40) != 0)
        COMMAND_RunCommand("sres 2048x2048");
    if ((GFX_GetPerformanceFlags() & 0x100000) != 0)
        GFX_SetMaxFPSOnBattery(20.0);
    v193 = (GFX_GetPerformanceFlags() & 0x80000) != 0i64;
    v194 = GfxConfig::gFXAA;
    if (v193)
        v194 = 0;
    LOBYTE(GfxConfig::gFXAA) = v194;
    APIName = GFX_GetAPIName();
    Version = GFX_GetVersion();
    Log("%s %s initialized", Version, APIName, v197);
    v198 = GFX_GetVendorName();
    v201 = "UNKNOWN";
    v202 = "UNKNOWN";
    if (v198)
        LODWORD(v202) = v198;
    Log("Graphics Vendor: %s", v202, v199, v200);
    v203 = GFX_GetRendererName();
    if (v203)
        LODWORD(v201) = v203;
    Log("Graphics Renderer: %s", v201, v204, v205);
    if (GetPhysicallyInstalledSystemMemory(&TotalMemoryInKilobytes))
        Log("RAM: %dGB", TotalMemoryInKilobytes >> 20, v206, v207);
    LODWORD(argv_) = _mm_load_si128(&xmmword_7FF71A3CE4A0).m128i_u32[0];
    _RAX = 0x80000000i64;
    __asm { cpuid }
    v213 = _RAX;
    HIDWORD(argv_) = _RBX;
    argv__8 = _RCX;
    argv__12 = _RDX;
    *g_strCPU = 0i64;
    xmmword_7FF71A893C20 = 0i64;
    xmmword_7FF71A893C30 = 0i64;
    xmmword_7FF71A893C40 = 0i64;
    xmmword_7FF71A893C50 = 0i64;
    xmmword_7FF71A893C60 = 0i64;
    xmmword_7FF71A893C70 = 0i64;
    xmmword_7FF71A893C80 = 0i64;
    for (i = 0x80000000; i <= v213; ++i)
    {
        _RAX = i;
        __asm { cpuid }
        argv_ = __PAIR64__(_RBX, _RAX);
        argv__8 = _RCX;
        argv__12 = _RDX;
        switch (i)
        {
        case 0x80000002:
            *g_strCPU = *&argv_;
            break;
        case 0x80000003:
            xmmword_7FF71A893C20 = *&argv_;
            break;
        case 0x80000004:
            xmmword_7FF71A893C30 = *&argv_;
            break;
        }
    }
    Log("CPU: %s", g_strCPU, i, v213);
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
    ZWIFT_UpdateLoading(0i64, 0);
    v238 = time64(0i64);
    v239 = GameHolidayManager::Instance();
    GameHolidayManager::SetupCurrentHoliday(v239, v238);
    ANTRECEIVER_Initialize();
    ANTRECEIVER_Connect();
    v240 = "ANT USB receiver found";
    if (!ANTRECEIVER_IsConnected)
        v240 = "ANT USB receiver NOT found";
    LogTyped(LOG_ANT_IMPORTANT, v240);
    ZWIFT_UpdateLoading(0i64, 0);
    GAME_Initialize();
    LODWORD(v241) = sub_7FF719288A40(&g_UserConfigDoc, "ZWIFT\\CONFIG\\TRAINER_EFFECT", COERCE_DOUBLE(1056964608i64), 1).m128_u32[0];
    GAME_SetTrainerSlopeModifier(v241);
    *(*&BikeManager::g_BikeManager->m_mainBike->gapC65[107] + 192i64) = XMLDoc::GetU32(// VirtualBikeComputer::SetTireSize
        &g_UserConfigDoc,
        "ZWIFT\\CONFIG\\TIRE_CIRC",
        0x839u,
        v242);
    *&BikeManager::g_BikeManager->m_mainBike->gap498[1916] = SIG_CalcCaseInsensitiveSignature(
        "bikes/Wheels/Campagnolo_Bora_Ultra/Campagnolo_Bora_Ultra_Low_Front.gde");
    v243 = SIG_CalcCaseInsensitiveSignature("bikes/Wheels/Campagnolo_Bora_Ultra/Campagnolo_Bora_Ultra_Low_Rear.gde");
    v244 = BikeManager::g_BikeManager;
    *&BikeManager::g_BikeManager->m_mainBike->gap498[1920] = v243;
    *&v244->m_mainBike->gapC65[1419] = 0i64;
    v244->m_mainBike->gap498[876] = 1;
    v186 = XMLDoc::GetS32(&g_UserConfigDoc, "ZWIFT\\CONFIG\\POWERSMOOTHING", 1u, v245) == 0;
    v246 = *&BikeManager::g_BikeManager->m_mainBike->gapC65[107];
    *(v246 + 2208) = !v186;
    g_GlobalMouseOverSID = "Play_SFX_UI_MOUSEOVER_1";
    g_GlobalToggleOnSID = "Play_SFX_UI_RADIOBUTTON_ON_1";
    g_GlobalToggleOffSID = "Play_SFX_UI_RADIOBUTTON_OFF_1";
    g_GlobalSelectSID = "Play_SFX_UI_Menu_Select_1";
    GUI_Initialize(quick_exit, g_IsOnProductionServer == 0);
    GUI_SetDefaultFont(g_GiantFontW);
    ZWIFT_UpdateLoading(0i64, 0);
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
        ZWIFT_UpdateLoading(0i64, 0);
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
        ZWIFT_UpdateLoading(0i64, 0);
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
        ZWIFT_UpdateLoading(0i64, 0);
        timeGetTime();
        HUD_Initialize();
        if (!UI_DialogPointer(61))
            UI_CreateDialog(UID_CONNECTION_NOTIFICATIONS, 0i64, 0i64, v2);
        timeGetTime();
        ZWIFT_UpdateLoading(0i64, 0);
        U32 = XMLDoc::GetU32(&g_UserConfigDoc, "ZWIFT\\DEVICES\\LASTTRAINERDEVICE", 0xFFFFFFFF, v282);
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
        v291 = XMLDoc::GetU32(&g_UserConfigDoc, "ZWIFT\\WORLD", 0, v290);
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
