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
    assert(g_sCrashReportingUPtr.get() == nullptr);
    CrashReporting::Initialize(evSysInst);
    Experimentation::Initialize(evSysInst);
    assert(g_sExperimentationUPtr.get() != nullptr);
    BLEModule::Initialize(g_sExperimentationUPtr.get());
    /*
    v9 = (EventSystem *)GameAssertHandler::Initialize(v8);
    v10 = (PlayerAchievementService *)EventSystem::GetInst(v9);
    PlayerAchievementService::Initialize(v10, v11);
    NoesisPerfAnalytics::Initialize(v7);
    ClientTelemetry::Initialize(v7, Inst);
    GroupEvents::Initialize(v7);
    zwift::localization::UnitTypeManager::Initialize(Inst, 0LL, v12);
    PlayerProfileCache::Initialize(Inst, v13);
    GoalsManager::Initialize(Inst, v14);
    v16 = (ZNet::NetworkService *)Cloud::CloudSyncManager::Initialize(Inst, v15);
    ZNet::NetworkService::Initialize(v16);
    v17 = (ZNet::NetworkService *)Experiment::Impl::Experimentation<Experiment::Feature, Experiment::Impl::ZNetAdapter, zu::ZwiftDispatcher>::Instance();
    v18 = (EventSystem *)ZNet::NetworkService::Instance(v17);
    v19 = EventSystem::GetInst(v18);
    DataRecorder::Initialize(v17, v18, v19);
    Experiment::Feature::Feature(v291, 83LL);*/
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
    assert(g_sCrashReportingUPtr.get() != nullptr);
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down VideoCapture");
    //...TODO
    //CrashReporting::AddBreadcrumb(0i64, "Shutting down Experimentation");
    g_sExperimentationUPtr.reset();
    //TODO
}