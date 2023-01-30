#pragma once

enum Achievement { ACH_CNT = 512 };
class PlayerAchievementService : public EventObject {
    uint64_t m_ridemask;
    DWORD m_lastPeriodEnd, m_repeatPeriod;
    bool m_changeFlag;
    void *m_field18;
    enum AchSaveLoadState { SLS_INITIAL = 0, SLS_INPROCESS = 1, SLS_DONE = 2, SLS_FAILED = 3 };
    AchSaveLoadState m_stateLoad, m_stateSave;
    uint8_t m_bitField[ACH_CNT / 8];
public:
    static void Initialize(EventSystem *ev);
    PlayerAchievementService(EventSystem *ev);

    bool DidRideNDaysAgo(int n) { return m_ridemask & (1 << n); }
    void HandleEvent(EVENT_ID, va_list) override;
    void HandleLogout();
    bool HasAchievement(Achievement a) { return m_bitField[a / 8] & (1 << (a & 7)); }
    static PlayerAchievementService *Instance();
    static bool IsInitialized();
    bool IsLoaded() { return m_stateLoad == SLS_DONE; }
    bool IsLoading() { return m_stateLoad == SLS_INPROCESS; }
    bool IsSaving() { return m_stateSave == SLS_INPROCESS; }
    void LoadAchievements();
    void LoadAchievementsFailure(ZNet::Error, void *);
    void LoadAchievementsSuccess(const protobuf::Achievements &a, void *);
    void LoadRideHistory();
    void PersistAchievements();
    void PersistAchievementsFailure(std::vector<int>, ZNet::Error);
    void PersistAchievementsSuccess();
    void SetAchievement(Achievement);
    static void Shutdown();
    void Update(float t);
    std::vector<int> listPlayerAchievements();
    ~PlayerAchievementService();
};
