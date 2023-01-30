#include "ZwiftApp.h"

void NoesisPerfAnalytics::Initialize(Experimentation *exp) {
    zassert(!g_pPerfAnalytics);
    g_pPerfAnalytics = new NoesisPerfAnalytics(exp);
}

void NoesisPerfAnalytics::initFPS() {
    m_minFrameTime = std::numeric_limits<double>::max();
    m_maxFrameTime = std::numeric_limits<double>::min();
}
NoesisPerfAnalytics::NoesisPerfAnalytics(Experimentation *exp) : m_exp(exp) {
    //TODO
    initFPS();
    exp->IsEnabled(FID_PERFANA, [this](ExpVariant val) {
        m_enabled = (val == EXP_ENABLED);
        if (m_enabled) {
            m_homeScreenEntered = false;
            m_beforeHome = false;
            m_beforeHomeTwice = false;
            m_beforeStartGameTransitionToLoadingScreen = false;
            m_worldSelected = false;
            UpdateGFXSpecs();
        }
        });
}
NoesisPerfAnalytics::~NoesisPerfAnalytics() {
    //TODO
}
void NoesisPerfAnalytics::BeforeEnterHomeScreen() {
    if (this->m_enabled) {
        m_beforeHomeT1 = m_beforeHomeT2 = std::chrono::steady_clock::now();
        if (m_beforeHome)
            m_beforeHomeTwice = true;
        m_beforeHome = true;
    }
}
void NoesisPerfAnalytics::AfterEnterHomeScreen() {
    if (this->m_enabled) {
        m_lastEventTime = std::chrono::steady_clock::now();
        initFPS();
        memset(m_fpsDistribution, 0, sizeof(m_fpsDistribution));
        m_homeScreenEntered = true;
    }
}
void NoesisPerfAnalytics::AfterOnscreenRender() {
    if (m_enabled && m_homeScreenEntered) {
        auto now = std::chrono::steady_clock::now();
        auto frameTimeSec = (now - m_lastEventTime).count() / 1000000000.0;
        m_lastEventTime = now;
        if (frameTimeSec > 0.0) {
            uint64_t FPS = 1.0 / frameTimeSec;
            if (FPS < MAX_FPS) {
                ++m_fpsDistribution[FPS];
                ++m_framesCounter;
            }
            if (frameTimeSec > m_maxFrameTime) m_maxFrameTime = frameTimeSec;
            if (frameTimeSec < m_minFrameTime) m_minFrameTime = frameTimeSec;
        }
    }
}
void NoesisPerfAnalytics::AfterExitHomeScreen() {
    if (m_enabled && m_homeScreenEntered) {
        //TODO
        m_homeScreenEntered = false;
    }
}
void NoesisPerfAnalytics::AfterStartGameDoneLoading() {
    if (m_enabled) {
        //TODO zwift_analytics::HVCAnalyticsSender::Send<zwift_analytics::HomeScreenPerfProperty>
    }
}
void NoesisPerfAnalytics::AfterUpdateRouteSelectionModel() {
    if (m_enabled && m_worldSelected) {
        //TODO m_pps
    }
}
void NoesisPerfAnalytics::BeforeStartGameTransitionToLoadingScreen() {
    if (m_enabled) {
        m_beforeStartGameTransitionToLoadingScreen = true;
        m_beforeStartGameTransitionToLoadingScreenTime = std::chrono::steady_clock::now();
    }
}
void NoesisPerfAnalytics::HomeScreenElementAdded() {
    if (m_homeScreenPendingElements < std::numeric_limits<int>::max())
        m_homeScreenPendingElements++;
}
void NoesisPerfAnalytics::HomeScreenElementLoaded() {
    if (m_enabled && m_homeScreenPendingElements > 0) {
        m_homeScreenPendingElements--;
        if (m_homeScreenPendingElements == 0) {
            //TODO m_pps
        }
    }
}
void NoesisPerfAnalytics::OnFullyLoadedHomeScreen() {
    if (m_enabled) {
        //TODO m_pps
    }
}
void NoesisPerfAnalytics::OnInitialLoadedHomeScreen() {
    if (m_enabled) {
        //TODO m_pps
    }
}
void NoesisPerfAnalytics::OnWorldSelected() {
    if (m_enabled) {
        m_worldSelectedTime = std::chrono::steady_clock::now();
        m_worldSelected = true;
    }
}
void NoesisPerfAnalytics::Shutdown() {
    auto del = g_pPerfAnalytics;
    g_pPerfAnalytics = nullptr;
    delete del;
}
void NoesisPerfAnalytics::UpdateGFXSpecs() {
    //TODO some zwift_analytics::AnalyticsEvent etc, not vital
}
