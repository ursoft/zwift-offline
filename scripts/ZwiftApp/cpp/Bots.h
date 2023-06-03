#pragma once
void PopulateBotInfo(const char *server);
inline bool g_ShowPacePartnerInfoPanel;
struct BotInfo {
    std::string m_pwrRange, m_descrLoc, m_dropsMulTime, m_dropsMul, m_specOccasion;
    VEC4 m_botColors;
    uint32_t m_route, m_grpColor;
    int32_t m_specId;
    float m_engageRange, m_activityRange;
    char m_group;
    bool m_isHolo, m_showOnMinimap, m_showNameplate;
    //char m_b_D4 = 0;
    BotInfo(uint32_t route, char group, std::string &&pwrRange, uint32_t grpColor, VEC4 botColors, std::string &&descrLoc, float engageRange, float activityRange,
        bool isHolo, bool showOnMinimap, bool showNameplate, std::string &&dropsMulTime, std::string &&dropsMul, std::string &&specOccasion, int32_t specId) :
        m_route(route), m_group(group), m_pwrRange(pwrRange), m_grpColor(grpColor), m_botColors(botColors), m_descrLoc(descrLoc), m_engageRange(engageRange), m_activityRange(activityRange), m_isHolo(isHolo),
        m_showOnMinimap(showOnMinimap), m_showNameplate(showNameplate), m_dropsMulTime(dropsMulTime), m_dropsMul(dropsMul), m_specOccasion(specOccasion), m_specId(specId) {}
};
struct PacerBotSweatOMeter { //0x140 bytes
    static bool IsAlienPacerbot(const BotInfo *pBi) {
        return pBi && pBi->m_specId == 0 && pBi->m_specOccasion == "Halloween2020"s;
    }
    static bool IsAstronautPacerbot(const BotInfo *pBi) {
        return pBi && pBi->m_specId == 1 && pBi->m_specOccasion == "Halloween2020"s;
    }
    static bool IsDinosaurPacerbot(const BotInfo *pBi) {
        return pBi && pBi->m_specId == 2 && pBi->m_specOccasion == "Halloween2020"s;
    }
    static bool IsHalloweenPacerbot(const BotInfo *pBi) {
        return pBi && pBi->m_specOccasion == "Halloween2020"s;
    }
    PacerBotSweatOMeter() {
        //TODO
    }
    static PacerBotSweatOMeter *Instance() { //simplified: no need for PacerBotSweatOMeter::ReplaceInstance
        static PacerBotSweatOMeter *g_PacerBotSweatOMeter = new PacerBotSweatOMeter();
        return g_PacerBotSweatOMeter;
    }
    std::unordered_map<int64_t, int64_t/*TODO*/> m_followersMap;
    int m_field_FC = 0;
    void RemoveFollower(int64_t fid) {
        auto mainBike = BikeManager::Instance()->m_mainBike;
        if (mainBike && mainBike->IsPacerBot()) {
            auto v6 = m_followersMap.find(fid);
            if (v6 != m_followersMap.end()) {
                m_followersMap.erase(v6);
                CalculatePacePartnerRanges();
                /* TODO m_ptr = mainBike->m_ptr;
                *(m_ptr + 7) |= 0x80u;            // protobuf::PlayerState m_ps_f40 is followers count?
                *(m_ptr + 0x34) = m_followerList.size();*/
                if (g_ShowPacePartnerInfoPanel)
                    ++m_field_FC;
            }
        }
    }
    void CalculatePacePartnerRanges() {
        //TODO
    }
    void UpdateDistanceRanges(float, int64_t, ZNETWORK_PacePartnerInfo::BroadcastState) {
        //TODO
    }
    void AdjustPacePartnerRanges(const ZNETWORK_PacePartnerInfo &) {
        //TODO
    }
    /* TODO:
ActivityRange(float)
AddFollower(long long)
Broadcast_Update(void)
CheckForFollowingDistance(void)
DetectAndResetIfDifferentDirection(BikeEntity const*,BikeEntity const*)
EngageRange(float)
FlushStaleFollowers(void) inlined to SendPacePartnerPeriodBroadcast
GetBit(PacerBotSweatOMeter::HalloweenPacerBot,long long,long long)
GetCostumeProgress(PacerBotSweatOMeter::HalloweenPacerBot,long long)
HUD_UpdateSquishyState(float)
IncrementCostume(PacerBotSweatOMeter::HalloweenPacerBot)
IncrementCurrentMultiplier(void)
InitializeDropsInfoList(void)
IsInPacerBotActivityVicinity(bool)
LoadConfigValues_PP_Ranges(void)
OutOfActivity(float)
OutOfRange(void)
ReadData(PacerBotSweatOMeter::HalloweenPacerBot,bool,long long)
SendPacePartnerPeriodBroadcast(float)
SendStatusToPacePartner(ZNETWORK_PacePartnerInfo::BroadcastState)
SetBotInfo(long long)
UpdateDropsInfoList(std::string const&,std::string const&)
UpdateDropsMultiplier(float)
UpdateFollowerCount(ulong)
~PacerBotSweatOMeter() */
};