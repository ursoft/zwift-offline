#include "ZwiftApp.h"
void BikeEntity::AdjustRandomXZ() {
    //TODO
}
BikeEntity::BikeEntity() : m_pah(this) {
    //TODO
    m_bc = new VirtualBikeComputer(); //delme
}
void BikeEntity::SaveProfile(bool, bool) {
    //TODO
}
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
void BikeEntity::ClearPowerups() {
    //TODO
}
void BikeEntity::Respawn(int segment, double a2, bool a3, bool a4) {
    //TODO
}
void BikeEntity::GiveRideOn(int64_t fromPlayerId) {
    static_assert(sizeof(RideOnAnim) == 0x30);
    ++m_rxRideons;
    m_rxRideonsList.emplace_back(RideOnAnim{.m_fromPlayerId = fromPlayerId });
    m_field_AA8 += 15.0f;
    m_field_AAC = 0x4000'0000;
}
void BikeEntity::PerformAction(protobuf::UserBikeAction act) {
    auto v4 = m_field_13C;
    const char *rideon[2][3] = { { "Play_VOX_Biker_M_RIDEON_01", "Play_VOX_Biker_M_RIDEON_02", "Play_VOX_Biker_M_RIDEON_03" }, { "Play_VOX_Biker_F_RIDEON_01", "Play_VOX_Biker_F_RIDEON_02", "Play_VOX_Biker_F_RIDEON_03" } };
    const char *hammer[2][3] = { { "Play_VOX_Biker_M_HAMMERTIME_01", "Play_VOX_Biker_M_HAMMERTIME_02", "Play_VOX_Biker_M_HAMMERTIME_03" }, { "Play_VOX_Biker_F_HAMMERTIME_01", "Play_VOX_Biker_F_HAMMERTIME_02", "Play_VOX_Biker_F_HAMMERTIME_03" } };
    const char *nice[2][3] = { { "Play_VOX_Biker_M_NICE_01", "Play_VOX_Biker_M_NICE_02", "Play_VOX_Biker_M_NICE_03" }, { "Play_VOX_Biker_F_NICE_01", "Play_VOX_Biker_F_NICE_02", "Play_VOX_Biker_F_NICE_03" } };
    const char *bring[2][3] = { { "Play_VOX_Biker_M_BRINGIT_01", "Play_VOX_Biker_M_BRINGIT_02", "Play_VOX_Biker_M_BRINGIT_03" }, { "Play_VOX_Biker_F_BRINGIT_01", "Play_VOX_Biker_F_BRINGIT_02", "Play_VOX_Biker_F_BRINGIT_03" } };
    const char *toast[2][3] = { { "Play_VOX_Biker_M_IMTOAST_01", "Play_VOX_Biker_M_IMTOAST_02", nullptr }, { "Play_VOX_Biker_F_IMTOAST_01", "Play_VOX_Biker_F_IMTOAST_02", "Play_VOX_Biker_F_IMTOAST_03" } };
    switch (act) {
    case protobuf::UBA_ELBOW:
        m_field_3D4 = 3;
        m_field_3D8 = m_field_3D9 = false;
        return;
    case protobuf::UBA_WAVE:
        m_field_3D4 = 1;
        m_field_3D8 = m_field_3D9 = false;
        return;
    case protobuf::UBA_02:
        m_field_3D4 = 2;
        m_field_3D8 = m_field_3D9 = false;
        return;
    case protobuf::UBA_RIDEON:
        AUDIO_Event(rideon[m_profile.body_type() & 1][rand() % 3], v4, false);
        break;
    case protobuf::UBA_HAMMER:
        AUDIO_Event(hammer[m_profile.body_type() & 1][rand() % 3], v4, false);
        break;
    case protobuf::UBA_NICE:
        AUDIO_Event(nice[m_profile.body_type() & 1][rand() % 3], v4, false);
        break;
    case protobuf::UBA_BRING_IT:
        AUDIO_Event(bring[m_profile.body_type() & 1][rand() % 3], v4, false);
        break;
    case protobuf::UBA_TOAST:
        AUDIO_Event(toast[m_profile.body_type() & 1][rand() % (2 + m_profile.body_type() & 1)], v4, false);
        break;
    case protobuf::UBA_BELL:
        AUDIO_Event("Play_SFX_BikeBell", v4, false);
        break;
    case protobuf::UBA_HOLIDAY_WAVE:
        switch (m_field_3CC) {
        case 0: case 1: case 2: case 3: case 4: case 6: case 0xF:
            m_field_3D4 = 15;
            m_field_3D8 = m_field_3D9 = false;
            break;
        case 5: case 9: case 0xE:
            m_field_3D4 = 14;
            m_field_3D8 = m_field_3D9 = false;
            break;
        case 7: case 8: case 0x10:
            m_field_3D4 = 16;
            m_field_3D8 = m_field_3D9 = false;
            break;
        default:
            return;
        }
        break;
    default:
        return;
    }
}
