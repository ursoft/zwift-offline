#pragma once
class HoloReplayManager { //0x110 bytes
    static inline std::unique_ptr<HoloReplayManager> g_HoloReplayManagerPtr;
public:
    HoloReplayManager(EventSystem *ev, XMLDoc *doc);
    static bool IsInitialized() { return g_HoloReplayManagerPtr.get() != nullptr; }
    static HoloReplayManager *Instance() { zassert(g_HoloReplayManagerPtr.get() != nullptr); return g_HoloReplayManagerPtr.get(); }
    static void Shutdown() { g_HoloReplayManagerPtr.reset(); }
    static void Initialize(EventSystem *ev, XMLDoc *doc) { g_HoloReplayManagerPtr.reset(new HoloReplayManager(ev, doc)); }
};