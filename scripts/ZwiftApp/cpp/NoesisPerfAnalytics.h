#pragma once
struct HomeScreenPerfProperty { //40 bytes
    char data[40] = {};
};
struct NoesisPerfAnalytics { //2016 bytes
    void initFPS();
    static NoesisPerfAnalytics *Instance() { zassert(g_pPerfAnalytics);  return g_pPerfAnalytics; }
    static void Initialize(Experimentation *exp);
    NoesisPerfAnalytics(Experimentation *exp);
    ~NoesisPerfAnalytics();
    static inline NoesisPerfAnalytics *g_pPerfAnalytics;
    const Experimentation *m_exp;
    bool m_enabled = false, m_homeScreenEntered, m_beforeStartGameTransitionToLoadingScreen, m_worldSelected, m_beforeHome, m_beforeHomeTwice;
    int m_homeScreenPendingElements = 0;
    enum { MAX_FPS = 144 };
    uint64_t m_fpsDistribution[MAX_FPS] = {}, m_framesCounter = 0;
    char HomeScreenPerfProperty[17]; //zu::EnumArray<zwift_analytics::HomeScreenPerfProperty,zwift_analytics::Metric<zu::ZwiftDispatcher>,17ul>::EnumArray
    double m_minFrameTime, m_maxFrameTime;
    std::chrono::time_point<std::chrono::steady_clock> m_lastEventTime, m_beforeStartGameTransitionToLoadingScreenTime, m_worldSelectedTime, m_beforeHomeT1, m_beforeHomeT2;
    void AfterEnterHomeScreen();
    void BeforeStartGameTransitionToLoadingScreen();
    void AfterExitHomeScreen();
    void AfterOnscreenRender();
    void AfterStartGameDoneLoading();
    void AfterUpdateRouteSelectionModel();
    void BeforeEnterHomeScreen();
    void HomeScreenElementAdded();
    void HomeScreenElementClear() { if (m_enabled) m_homeScreenPendingElements = 0; }
    void HomeScreenElementLoaded();
    static bool IsInitialized() { return g_pPerfAnalytics != nullptr; }
    void OnFullyLoadedHomeScreen();
    void OnInitialLoadedHomeScreen();
    void OnWorldSelected();
    static void Shutdown();
    void UpdateGFXSpecs();
};