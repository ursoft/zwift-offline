#pragma once
struct BikeEntity;
struct PrivateAttributesHelper;
struct BikeManager {
    inline static std::unique_ptr<BikeManager> g_pBikeManager;
    BikeManager(/*EventSystem *, bool*/);
    //static void Shutdown() { g_pBikeManager.reset(); }
    static BikeManager *Instance() { /*TODO*/g_pBikeManager.reset(new BikeManager()); zassert(g_pBikeManager.get() != nullptr); return g_pBikeManager.get(); }
    //static bool IsInitialized() { return g_pBikeManager.get() != nullptr; }
    void Initialize(Experimentation *exp);
    //~BikeManager();
    BikeEntity *m_mainBike;
    PrivateAttributesHelper *m_pah;
};
