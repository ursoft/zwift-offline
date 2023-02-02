#pragma once
class DataRecorder : EventObject {
    inline static std::unique_ptr<DataRecorder> g_DataRecorder;
public:
    static void Initialize(Experimentation *exp, ZNet::NetworkService *net, EventSystem *ev) { g_DataRecorder.reset(new DataRecorder(exp, net, ev)); }
    static bool IsInitialized() { return g_DataRecorder.get() != nullptr; }
    static DataRecorder *Instance() { zassert(g_DataRecorder.get() != nullptr); return g_DataRecorder.get(); }
    static void Shutdown() { g_DataRecorder.reset(); }
    DataRecorder(Experimentation *exp, ZNet::NetworkService *net, EventSystem *ev);
    void HandleEvent(EVENT_ID, va_list) override;
    void ExpensiveCalculateTSS();
    void ExpensiveCalculateTSS_EstimatedPower(const std::vector<float> &);
    //void GetComponent(RecorderComponent::TYPE);
    void Init();
    void ResetSessionData();
    void Update(float);
    ~DataRecorder();
};
