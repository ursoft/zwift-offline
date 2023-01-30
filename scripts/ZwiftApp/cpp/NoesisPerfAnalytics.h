#pragma once

class NoesisPerfAnalytics {
	void initFPS();
public:
	static NoesisPerfAnalytics *Instance() { zassert(g_pPerfAnalytics);  return g_pPerfAnalytics; }
	static void Initialize(Experimentation *exp);
	NoesisPerfAnalytics(Experimentation *exp);
	~NoesisPerfAnalytics();
	static inline NoesisPerfAnalytics *g_pPerfAnalytics;
	const Experimentation *m_exp;
	bool m_enabled = false, m_homeScreenEntered, m_beforeStartGameTransitionToLoadingScreen, m_worldSelected, m_beforeHome, m_beforeHomeTwice;
	int m_homeScreenPendingElements = 0;
	enum { MAX_FPS = 144 };
	uint64_t m_fpsDistribution[MAX_FPS + 1] = {}; //last is overall frames count
	char m_field_4F0[0x330] = {}; //zu::EnumArray<zwift_analytics::HomeScreenPerfProperty,zwift_analytics::Metric<zu::ZwiftDispatcher>,17ul>::EnumArray
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
	bool IsInitialized() { return g_pPerfAnalytics != nullptr; }
	void OnFullyLoadedHomeScreen();
	void OnInitialLoadedHomeScreen();
	void OnWorldSelected();
	void Shutdown();
	void UpdateGFXSpecs();
};