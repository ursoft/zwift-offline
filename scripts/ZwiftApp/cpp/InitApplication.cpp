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
