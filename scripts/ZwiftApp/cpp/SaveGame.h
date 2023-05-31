#pragma once
struct GenericTrackingData { //16 bytes
    uint32_t m_hash = 0; //RODE_WITH_ZML -> 0x2ea8df6a
    enum class vt_id : uint32_t { FLOAT = 1, INT = 2, BYTE = 3 } m_vt;
    union vt_val { float f; int i; uint8_t b; } m_val;
    int gap;
    GenericTrackingData(int v = 0, uint32_t hash = 0) {
        m_hash = hash;
        m_vt = vt_id::INT;
        m_val.i = v;
    }
    GenericTrackingData(uint8_t v, uint32_t hash) {
        m_hash = hash;
        m_vt = vt_id::BYTE;
        m_val.b = v;
    }
    GenericTrackingData(float v, uint32_t hash) {
        m_hash = hash;
        m_vt = vt_id::FLOAT;
        m_val.f = v;
    }
};
struct SaveGame {
    std::map<uint32_t, GenericTrackingData> m_trackingMap;
    uint64_t m_loadDT = 0;
    bool m_field_1E1 = false; //change flag???
    //TODO
    SaveGame();
    ~SaveGame();
    void TryPromoCode(const char *, bool);
    //OMIT void TranslatedEntitlementToAssociatedHash(Entitlement);
    void TranslateAssociateHashToEntitlement(uint32_t);
    void SetTrackingData(uint32_t hash, const GenericTrackingData &data);
    void SeenEntitlementByHash(uint32_t);
    void SeenEntitlement(uint32_t, uint32_t);
    void Save(uint32_t *);
    void RevokeEntitlementByAssociated(uint32_t);
    //OMIT void RevokeEntitlement(Entitlement);
    void RevokeDeprecatedEntitlements();
    void ReceiveUnseenEntitlementWhitelist(const std::vector<uint32_t> &);
    void Load(void *);
    void IsEntitlementUnseenByHash(uint32_t);
    void IsEntitlementUnseen(uint32_t, uint32_t);
    void IsEntitledTo(uint32_t);
    void InitTimedPromo(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void InitEntitlements();
    //OMIT void GrantSessionEntitlement(Entitlement, bool);
    void GrantEntitlementByAssociated(uint32_t, bool);
    //OMIT void GrantEntitlement(Entitlement, bool);
    void GetUnseenEntitlementsCount(void);
    GenericTrackingData GetTrackingData(uint32_t hash, const GenericTrackingData &def);
    void GetTempPromo(const char *);
    void GetSystemDateTime(bool);
    void GetDateNDaysFromDate(DateTime, int);
    void FinalizeSave(uint32_t *);
    void CountDaysBetweenDates(DateTime, DateTime);
    void CompareDateAtoB(DateTime, DateTime);
    void AddUnseenEntitlement(uint32_t);
};