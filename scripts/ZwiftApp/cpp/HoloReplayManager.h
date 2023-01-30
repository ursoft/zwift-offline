#pragma once
class HoloReplayManager {
    static inline std::unique_ptr<HoloReplayManager> g_HoloReplayManagerPtr;
public:
    HoloReplayManager();
    static bool IsInitialized() { return g_HoloReplayManagerPtr.get() != nullptr; }
    static HoloReplayManager *Instance() { zassert(g_HoloReplayManagerPtr.get() != nullptr); return g_HoloReplayManagerPtr.get(); }
    static void Shutdown() { g_HoloReplayManagerPtr.reset(); }
};