#pragma once
using GenericTrackingData = int32_t;
struct SaveGame {
    std::map<uint32_t, GenericTrackingData> m_trackingMap;
    //TODO
    SaveGame();
    ~SaveGame();
    void TryPromoCode(const char *, bool);
    //OMIT void TranslatedEntitlementToAssociatedHash(Entitlement);
    void TranslateAssociateHashToEntitlement(uint32_t);
    void SetTrackingData(uint32_t, GenericTrackingData);
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
    GenericTrackingData GetTrackingData(uint32_t hash, GenericTrackingData def);
    void GetTempPromo(const char *);
    void GetSystemDateTime(bool);
    void GetDateNDaysFromDate(DateTime, int);
    void FinalizeSave(uint32_t *);
    void CountDaysBetweenDates(DateTime, DateTime);
    void CompareDateAtoB(DateTime, DateTime);
    void AddUnseenEntitlement(uint32_t);
};