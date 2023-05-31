#include "ZwiftApp.h"
BikeEntity::BikeEntity() : m_pah(this) {
    //TODO
    m_bc = new VirtualBikeComputer(); //delme
}
void BikeEntity::SaveProfile(bool, bool) {
    //TODO
}
int64_t g_GroupEventsActive_CurrentEventId;
int64_t BikeEntity::GetEventID() {
    if (m_field_C98) {
        if (g_currentPrivateEvent)
            return g_currentPrivateEvent->m_eventId;
        return g_GroupEventsActive_CurrentEventId;
    }
    return m_curEventId;
}
float BikeEntity::GetFTP(bool *bEstimated /*= nullptr*/) {
    if (m_profile.has_ftp()) {
        if (m_profile.ftp()) {
            if (bEstimated)
                *bEstimated = false;
            return (float)m_profile.ftp();
        }
    }
    if (bEstimated)
        *bEstimated = true;
    float v28 = (m_profile.has_age()) ? (float)m_profile.age() : 35.0f;
    float v27 = (m_field_C98 && GetShouldUseSkillLevel()) ? GetSkillLevelWKG() * (1.0f - (fabs(v28 - 25.0f) * 0.005f)) : 2.5f;
    return GetRiderWeightKG(true) * v27;
}
bool BikeEntity::GetShouldUseSkillLevel() {
    auto s = GAME_GetCurrentPlayerSport();
    if (s != protobuf::CYCLING)
        return m_pah.GetIntValue("HasSetRunningPaces", 0) != 0;
    else
        return GetSaveGame()->GetTrackingData(SIG_CalcCaseSensitiveSignature("UseSkillLevelCycling"), GenericTrackingData()).m_val.i != 0;
}
float BikeEntity::GetSkillLevelWKG() {
    if (m_skillWKG == -1) {
        auto lev = std::clamp(GetSaveGame()->GetTrackingData(SIG_CalcCaseSensitiveSignature("CyclingSkillLevel"), 2).m_val.i, 0, 2);
        if (lev > 2)
            lev = 2;
        m_skillWKG = lev;
    }
    static const float level2wkg[] = { 1.6f, 2.4f, 2.8f, 1.8f, 2.8f, 3.2f };
    return level2wkg[3 * m_profile.is_male() + m_skillWKG];
}
float BikeEntity::GetRiderWeightKG(bool a2) {
    float ret;
    bool limitLow = true;
    if (m_playerIdTx < 0 && !m_field_806) {
        ret = fmaxf(fabsf(m_playerIdTx % 50) + 50.0f, 36.0f);
    } else {
        ret = 73.0f;
        if (m_profile.has_weight_in_grams()) {
            ret = m_profile.weight_in_grams() * 0.001f;
            if (m_profile.has_age()) {
                if (m_profile.age() && m_profile.age() <= 17) {
                    limitLow = false;
                    if (m_profile.age() > 9)
                        ret = fmaxf(m_profile.age() * 2.0555556f, ret);
                    else
                        ret = fmaxf(m_profile.age() * 2.3333335f + 3.0f, ret);
                }
            }
        }
        if (limitLow)
            ret = fmaxf(ret, 36.0f);
    }
    ret = fminf(ret, 200.0f);
    if (a2)
        return (ret + m_field_59C) * m_field_5A0;
    return ret;
}
void BikeEntity::RequestProfileFromServer() {
    //TODO
}
void BikeEntity::GiveRideOn(int64_t fromPlayerId) {
    static_assert(sizeof(RideOnAnim) == 0x30);
    ++m_rxRideons;
    m_rxRideonsList.emplace_back(RideOnAnim{.m_fromPlayerId = fromPlayerId });
    m_field_AA8 += 15.0f;
    m_field_AAC = 0x4000'0000;
}
