#pragma once
struct DataPoint {
    float m_val = 0.0f, m_time = 0.0f;
};
struct DataPointVEC3 {
    VEC3 m_val{};
    float m_time = 0.0f;
};
struct RecorderComponent { //0x98 bytes
    float m_valMax = 0.0f, m_valMin = 1e15f, m_startTime = 0.0f, m_field_84 = 0.0f, m_period = 60.0f, m_timeBetweenPoints = 1.0f, m_curTime = 0.0f, m_valMax_init, m_valMin_init, m_nextPointTime = 0.0f, m_maxTime = 0.0f;
    VEC3 m_v3max, m_v3min, m_v3max_init, m_v3min_init;
    enum TYPE { T_0 = 0, T_1 = 1, T_RIDE_SPEED = 2, T_3 = 3, T_4 = 4, T_DISTANCE = 5, T_6 = 6, T_CPC = 7, T_8 = 8, T_RUN_SPEED = 9 } m_type;
    enum TIME_PERIOD { TP_NOT_SET = -1, TP_5S, TP_MINUTE, TP_5MIN, TP_HOUR, TP_CNT };
    std::vector<DataPoint> m_dataPoints;
    std::vector<DataPointVEC3> m_dataPointsVec3;
    bool m_b1 = true, m_res0 = true;
    RecorderComponent(TYPE ty) : m_type(ty) {}
    void SampleSensor(float, float);
    size_t SampleCount() { return m_dataPoints.size(); }
    void PreConfigure(float, float, float);
    void ImportXML(const tinyxml2::XMLDocument &src);
    void GetTimeRange(float *from, float *to);
    const DataPoint &GetDataPointAtTime(float);
    float GetBestTimeAverage(float pastSec);
    float GetAverageFromPastNSeconds_Weighted(float secs);
    float GetAverageFromPastNSeconds(float secs);
    tinyxml2::XMLDocument *ExportXML();
    virtual ~RecorderComponent() {} //vptr[0]
    void Init(float timeBetweenPoints, uint32_t reserve); //vptr[1]
    virtual void Update(float); //vptr[2]
    virtual bool InitializeDrawData(RecorderComponent::TIME_PERIOD) { return true; } //vptr[3]
    virtual void GetBestTimeAverage(std::vector<DataPoint> *, bool, uint64_t *, uint64_t *) {} //vptr[4]
    virtual void Reset(); //vptr[5]
    virtual const std::vector<DataPoint> &GetDataPoints() { return m_dataPoints; } //vptr[6]
    const std::vector<DataPointVEC3> &GetDataPointsVEC3() { return m_dataPointsVec3; } //vptr[7]
    virtual void AddDataPoint(float val, float time); //vptr[8]
    void AddDataPointVEC3(const VEC3 &val, float time); //vptr[9]
};
struct Grapher { //0x60 bytes
    enum RenderStyle { RS_0 };
    enum PointReductionMethod { PRM_0 };
    Grapher(RecorderComponent *rc);
    void Reset();
    void RenderYLabels(float, float, float, float, const char *, uint32_t);
    void RenderXLabels(float, float, float, char const *, uint32_t);
    void PleaseMatchThisGraph(Grapher *);
    void GetVertexPosition(float *, float *, float, float, float, float, bool);
    void GetTimeForXPosition(float, float);
    void GetMinValue(float *);
    void GetMaxValue(float *);
    void GetInterpolatedXPosition(float, float);
    void FormatTimeString(uint32_t, char *);
    void DrawSpecificTimes(float, float, float, float, uint32_t, Grapher::RenderStyle, uint32_t, float, float);
    void DrawBG(float, float, float, float, uint32_t, Grapher::RenderStyle);
    void Draw(float, float, float, float, uint32_t, Grapher::RenderStyle, uint32_t, bool, const char *, const char *, uint32_t, bool, bool, int32_t, Grapher::PointReductionMethod);
    void CalculateXTimeSteps(bool, bool);
    inline static int s_reducedDataPointsCache = -1;
};
struct AntiSandbaggingParams {
    //TODO - no functions
};
struct RideCPC { //32 bytes
    uint64_t m_netTime = 0;
    std::vector<DataPoint> m_data;
};
struct JobData;
inline float g_CPCCheatTimeScale = 1.0f;
struct CriticalPowerCurve : public RecorderComponent, EventObject { //0x188 bytes
    ZNet::NetworkService *m_net;
    Experimentation *m_exp;
    std::vector<DataPoint> m_cpDataPoints;
    std::vector<RideCPC> m_rideCPC;
    std::vector<float> m_normMPO;
    std::unordered_map<uint16_t, uint16_t> m_maxPowerAtInterval;
    uint64_t m_playerId = 0;
    uint32_t m_field_60 = 0, m_field_5C = 0, m_field_68 = 0, m_cpIdMin = 0, m_cpIdMax = 0, m_cpIdOffset = 0, m_maxWattsKey = 0;
    int m_field_64 = 0, m_flierBits = 0, m_sandBits = 0;
    TIME_PERIOD m_timePeriod = TP_NOT_SET;
    float m_sandbaggerCoeff = 1.0;
    ZNet::RequestId m_requestId = 0;
    float m_lastSearchTime = 0.0;
    bool m_loaded = false, m_threadCalcInProgress = false, m_immunToCheatFlag = false, m_isMale = true;
    CriticalPowerCurve(ZNet::NetworkService *net, Experimentation *exp, EventSystem *ev);
    ~CriticalPowerCurve(); //vptr[0]
    static int StartThreadedCalculation(JobData *);
    //void SearchCriticalPowerInCurrentRide(); inlined
    void SearchCriticalPower(uint16_t, uint16_t, uint16_t, uint32_t);
    bool Save();
    void SandbaggerWarning(const float *, float, float);
    void LoggedIn(const protobuf::PlayerProfile &, int32_t);
    void Load();
    uint16_t GetServerMeanMaximalPowerAtInterval(uint16_t interval);
    void GetServerMeanMaximalPower(std::vector<DataPoint> *, bool);
    float GetMaxPowerOutputTiers(uint32_t sel);
    static void DoneThreadedCalculation(int32_t, JobData *);
    //void AntiSandbaggingCheck(AntiSandbaggingParams *ptr); inlined
    //void AntiCheatingCheck(); inlined
    void PrepareForNextRide();
    void Reset();
    void UpdateServerBestPowerCurve();
    void HandleEvent(EVENT_ID, va_list) override;
    float GetBestTimeAverage(float period, float resultTimeSec);
    void Update(float) override;
    bool InitializeDrawData(RecorderComponent::TIME_PERIOD) override;
    void GetBestTimeAverage(std::vector<DataPoint> *, bool, uint64_t *, uint64_t *) override;
    const std::vector<DataPoint> &GetDataPoints() { return m_cpDataPoints; }
    void AddDataPoint(float val, float time) override;
};
struct DataRecorder {
    inline static std::unique_ptr<DataRecorder> g_DataRecorder;
    std::vector<RecorderComponent *> m_components;
    Grapher *m_graphers[6];
    Experimentation *m_exp;
    ZNet::NetworkService *m_net;
    EventSystem *m_ev;
    static void Initialize(Experimentation *exp, ZNet::NetworkService *net, EventSystem *ev) { g_DataRecorder.reset(new DataRecorder(exp, net, ev)); }
    static bool IsInitialized() { return g_DataRecorder.get() != nullptr; }
    static DataRecorder *Instance() { zassert(g_DataRecorder.get() != nullptr); return g_DataRecorder.get(); }
    static void Shutdown() { g_DataRecorder.reset(); }
    DataRecorder(Experimentation *exp, ZNet::NetworkService *net, EventSystem *ev);
    static float ExpensiveCalculateTSS();
    float ExpensiveCalculateTSS_EstimatedPower(const std::vector<float> &);
    RecorderComponent *GetComponent(RecorderComponent::TYPE);
    void Init();
    void ResetSessionData();
    void Update(float);
    ~DataRecorder();
};
