#pragma once
class DataRecorder {
    inline static std::unique_ptr<DataRecorder> g_DataRecorder;
public:
    DataRecorder(Experimentation *exp);
    static void Initialize(Experimentation *exp);
    static bool IsInitialized() { return g_DataRecorder.get() != nullptr; }
    static DataRecorder *Instance() { zassert(g_DataRecorder.get() != nullptr); return g_DataRecorder.get(); }
    static void Shutdown() { g_DataRecorder.reset(); }
};
