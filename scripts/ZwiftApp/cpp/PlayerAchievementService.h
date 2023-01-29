#pragma once

enum Achievement { ACH_CNT };
enum PlayerAchievementServiceLoadingState { PAS_LOADING = 1, PAS_SAVING = 1, PAS_LOADED = 2 };
class PlayerAchievementService : public EventObject {
	uint64_t m_ridemask;
	DWORD m_start;
	PlayerAchievementServiceLoadingState m_state_load, m_state_save;
	uint8_t m_bitField[16 /*TODO*/];
public:
	static void Initialize(EventSystem *ev);
	PlayerAchievementService(EventSystem *ev);

	bool DidRideNDaysAgo(int n) { return m_ridemask & (1 << n); }
	void HandleEvent(EVENT_ID, va_list) override;
	void HandleLogout();
	bool HasAchievement(Achievement a) { return m_bitField[a / 8] & (1 << (a & 7)); }
	static PlayerAchievementService *Instance();
	static bool IsInitialized();
	bool IsLoaded() { return m_state_load == PAS_LOADED; }
	bool IsLoading() { return m_state_load == PAS_LOADING; }
	bool IsSaving() { return m_state_save == PAS_SAVING; }
	void LoadAchievements();
	void LoadAchievementsFailure(ZNet::Error, uint32_t);
	void LoadAchievementsSuccess(const protobuf::Achievements &a, uint32_t);
	void LoadRideHistory();
	void PersistAchievements();
	void PersistAchievementsFailure(std::vector<int>, ZNet::Error);
	void PersistAchievementsSuccess();
	void SetAchievement(Achievement);
	void Shutdown();
	void Update(float dt);
	void listPlayerAchievements();
	~PlayerAchievementService();
};
