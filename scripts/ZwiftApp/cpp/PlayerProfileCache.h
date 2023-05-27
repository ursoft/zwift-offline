#pragma once
struct CachedProfile { //72 bytes(A), 88 (Win)
    uint32_t m_trp_id = 0; //training plan hash
    std::string m_str0, m_str20;
    uint64_t m_trp_start = 0, m_trp_end = 0; // training plan start/end
    void Clear();
    void Parse(std::stringstream &);
    void Serialize(std::stringstream &) const;
    //void Set(const CachedProfile &);
    //void Set(const std::string &, const std::string &, uint32_t, uint64_t, uint64_t);
};
struct PlayerProfileCache : public EventObject { //72 bytes
    inline static std::unique_ptr<PlayerProfileCache> g_pPlayerProfileCache;
    std::list<CachedProfile *> m_list;
    bool m_socialActual = false;
    uint64_t m_socialUpdTime = 0;
    float m_secsUntilSocialUpdate;
    static void DeallocatePlayerCacheList(std::list<CachedProfile *> *);
    PlayerProfileCache(EventSystem *);
    static void Shutdown() { g_pPlayerProfileCache.reset(); }
    static PlayerProfileCache *Instance() { zassert(g_pPlayerProfileCache.get() != nullptr); return g_pPlayerProfileCache.get(); }
    static bool IsInitialized() { return g_pPlayerProfileCache.get() != nullptr; }
    static void Initialize(EventSystem *ev);
    void HandleEvent(EVENT_ID, va_list) override;
    ~PlayerProfileCache();
    void ClearCache();
    void ClearSavedCache();
    void AllocateEnrolledFriendProfiles(std::list<CachedProfile *> *, uint32_t);
    void LoadCache();
    void SaveCache();
    void TestCache();
    void UpdateCache(float utim);
};
struct BikeEntity;
struct PrivateAttributesHelper {
    BikeEntity *m_be;
    PrivateAttributesHelper(BikeEntity *be) : m_be(be) {}
    bool ClearValue(const char *name);
    bool ClearValue(uint32_t nameCrc);
    float GetFloatValue(const char *name, float);
    float GetFloatValue(uint32_t nameCrc, float);
    int64_t GetIntValue(const char *name, int64_t);
    int64_t GetIntValue(uint32_t, int64_t);
    std::string GetStringValue(const char *, const std::string &);
    std::string GetStringValue(uint32_t, const std::string &);
    bool IsValuePresent(const char *);
    bool IsValuePresent(const char *, float *);
    bool IsValuePresent(const char *, int64_t *);
    bool IsValuePresent(const char *, std::string *);
    static void Parse(std::stringstream &, std::string *);
    static void Parse(std::stringstream &, uint8_t *);
    static void Parse(std::stringstream &, uint32_t *);
    static void Parse(std::stringstream &, uint64_t *);
    static void Parse(std::stringstream &, uint16_t *);
    static void Serialize(std::stringstream &, const std::string &);
    static void Serialize(std::stringstream &, uint8_t);
    static void Serialize(std::stringstream &, uint32_t);
    static void Serialize(std::stringstream &, uint64_t);
    static void Serialize(std::stringstream &, uint16_t);
    void StoreValue(const char *name, const char *val);
    void StoreValue(const char *, float);
    void StoreValue(const char *, int64_t);
    void StoreValue(uint32_t, const char *);
    void StoreValue(uint32_t, float);
    void StoreValue(uint32_t, int64_t);
};