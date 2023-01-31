#pragma once
class BikeEntity;
class BikeManager {
    inline static std::unique_ptr<BikeManager> g_pBikeManager;
public:
    //BikeManager(EventSystem *, bool);
    //static void Shutdown() { g_pBikeManager.reset(); }
    static BikeManager *Instance() { zassert(g_pBikeManager.get() != nullptr); return g_pBikeManager.get(); }
    //static bool IsInitialized() { return g_pBikeManager.get() != nullptr; }
    //static void Initialize(EventSystem *ev, bool b);
    //~BikeManager();
    BikeEntity *m_mainBike;
};
