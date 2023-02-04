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

std::unique_ptr<CrashReporting> g_sCrashReportingUPtr;
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
    //TODO v16 = (ZNet::NetworkService *)Cloud::CloudSyncManager::Initialize(Inst, v15);
    DataRecorder::Initialize(exp, ZNet::NetworkService::Instance(), evSysInst);
    Thread::Initialize(exp);
    //TODO option::Parser::workhorse
    GFX_SetLoadedAssetMode(true);
    char userPath[MAX_PATH] = {};
    if (OS_GetUserPath(userPath)) {
        char downloadPath[MAX_PATH] = {};
        sprintf_s(downloadPath, "%s/Zwift/", userPath);
        g_mDownloader.SetLocalPath(downloadPath);
    }
    g_mDownloader.SetServerURLPath("https://cdn.zwift.com/gameassets/");
    g_mDownloader.Download("MapSchedule_v2.xml", 0LL, Downloader::m_noFileTime, -1, GAME_onFinishedDownloadingMapSchedule);
    //TODO: check GFX driver if no "<data>\Zwift\olddriver.ok" exist
    //line 1030
    /*
  g_bUpdateCheckInProgress = 1;
  v32 = (void *)operator new(0x20uLL);
  v33 = Downloader::m_noFileTime;
  v241[2] = v32;
  *(_OWORD *)v241 = xmmword_1DA2B90;
  strcpy((char *)v32, "ZwiftAndroid_ver_cur.xml");
  Downloader::Download(&gDownloader, v241, 0LL, v33, 0xFFFFFFFFLL, OnDownloadVerCur_End);
  if ( ((__int64)v241[0] & 1) != 0 )
    operator delete(v241[2]);*/
    g_pDownloader->Update();
    ZMUTEX_SystemInitialize();
    LogInitialize(); //TODO: move up
    OS_Initialize();
    /*
  InitICUBase();
  g_LauncherVersion[2] = 0u;
  g_LauncherVersion[3] = 0u;
  g_LauncherVersion[0] = 0u;
  g_LauncherVersion[1] = 0u;
  if ( s )
  {
    Log("Launcher Version : %s", s);
    v34 = s;
    v35 = strlen(s);
    if ( v35 >= 0x40 )
      v36 = 64LL;
    else
      v36 = v35;
    __memcpy_chk(g_LauncherVersion, v34, v36, 64LL);
  }
  g_WorldTime = dword_1E2D810[(unsigned int)timeGetTime() % 3];
  SetupGameCameras();
  XMLDoc::UserLoad((XMLDoc *)&g_UserConfigDoc, "prefs.xml");
   
    */
    //TODO
    //evSysInst->Subscribe(EV_SLIPPING_ON, nullptr);
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
