#pragma once //READY for testing
inline int g_nGoalsAdded;
struct GoalsManager : public EventObject { //176 bytes
    inline static std::unique_ptr<GoalsManager> s_pSelf;
    std::future<NetworkResponse<protobuf::Goals>> m_get_future;
    std::future<NetworkResponse<protobuf::Goal>> m_save_future;
    //protobuf::Goals m_goals;
    std::list<protobuf::Goal> m_goals;
    float m_timeAcc = 0.0f;
    bool m_saving = false, m_oldConverted = false, m_loadedOk = false, m_loadingNow = false;
    const protobuf::Goal *m_activeDistanceGoal = nullptr, *m_activeDurationGoal = nullptr;
    GoalsManager(EventSystem *ev);
    static void Shutdown() { s_pSelf.reset(); }
    static void Initialize(EventSystem *ev) { s_pSelf.reset(new GoalsManager(ev)); }
    static GoalsManager *Instance() { zassert(s_pSelf.get() != nullptr); return s_pSelf.get(); }
    void HandleEvent(EVENT_ID, va_list) override;
    void AddPersonalGoal(const protobuf::Goal &);
    void ConvertOldGoals();
    void FinishLoad(const NetworkResponse<protobuf::Goals> &);
    float GetActiveGoalCompletion(protobuf::GoalType);
    const protobuf::Goal *GetActivePersonalGoalOfType(protobuf::GoalType);
    std::vector<const protobuf::Goal *> GetActivePersonalGoals();
    const protobuf::Goal *GetNewestInactivePersonalGoalOfType(protobuf::GoalType);
    void Load();
    bool ProcessActivities() { return true; }
    void Reset();
    void Update(float dt);
    void UpdateGoals();
    ~GoalsManager();
};