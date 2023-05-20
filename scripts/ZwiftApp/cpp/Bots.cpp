#include "ZwiftApp.h"
//    std::__hash_table<std::__hash_value_type<long long,BotInfo>,std::__unordered_map_hasher<long long,std::__hash_value_type<long long,BotInfo>,std::hash<long long>,true>,std::__unordered_map_equal<long long,std::__hash_value_type<long long,BotInfo>,std::equal_to<long long>,true>,std::allocator<std::__hash_value_type<long long,BotInfo>>>::__emplace_unique_key_args<long long,std::pair<long long const,BotInfo> const&>(
/* kDefaultMultiplier, kDefaultMultiplierTime, kDefaultSpecialOccasion*/
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
std::unordered_map<int, BotInfo> g_botInfo; //key is player id
void PopulateBotInfo(const char *server) {
    g_botInfo = { 
        {0x25A225, BotInfo(0xDCA1752F, 'A', "5-4"s,      0xFF1942FC, {1.00f, 0.00f, 0.00f, 0.75f}, "LOC_PACE_PARTNER_DESCRIPTION_A"s, 1.5f, 7.0f, true, true, true, "300,300,300,300,300,300"s, "1.0,1.2,1.4,1.6,1.8,2.0"s, "None"s, -1)},
        {0x25A21B, BotInfo(0x24055B74, 'B', "3.2-4"s,    0xFF4EC359, {0.00f, 1.00f, 0.00f, 1.00f}, "LOC_PACE_PARTNER_DESCRIPTION_B"s, 1.5f, 7.0f, true, true, true, "300,300,300,300,300,300"s, "1.0,1.2,1.4,1.6,1.8,2.0"s, "None"s, -1)},
        {0x25A217, BotInfo(0x24055B74, 'C', "2.5-3.1"s,  0xFFE9C13F, {0.20f, 0.75f, 1.20f, 1.00f}, "LOC_PACE_PARTNER_DESCRIPTION_C"s, 1.5f, 7.0f, true, true, true, "300,300,300,300,300,300"s, "1.0,1.2,1.4,1.6,1.8,2.0"s, "None"s, -1)},
        {0x259235, BotInfo(0xCA663E5C, 'D', "1-2.4"s,    0xFF0CCFFC, {0.95f, 0.77f, 0.06f, 1.00f}, "LOC_PACE_PARTNER_DESCRIPTION_D"s, 1.5f, 7.0f, true, true, true, "300,300,300,300,300,300"s, "1.0,1.2,1.4,1.6,1.8,2.0"s, "None"s, -1)},
        {0x2B3E22, BotInfo(0x0829CE80, 'D', "8-9.5"s,    0xFF0CCFFC, {0.95f, 0.77f, 0.06f, 1.00f}, "LOC_RUN_PARTNER_DESCRIPTION_D"s,  1.5f, 7.0f, true, true, true, "300,300,300,300,300,300"s, "1.0,1.2,1.4,1.6,1.8,2.0"s, "None"s, -1)},
        {0x2B3E33, BotInfo(0x24055B74, 'C', "9.5-10.5"s, 0xFFE9C13F, {0.20f, 0.75f, 1.20f, 1.00f}, "LOC_RUN_PARTNER_DESCRIPTION_C"s,  1.5f, 7.0f, true, true, true, "300,300,300,300,300,300"s, "1.0,1.2,1.4,1.6,1.8,2.0"s, "None"s, -1)},
        {0x2B3E3C, BotInfo(0xAFA95889, 'B', "10.5-12"s,  0xFF4EC359, {0.00f, 1.00f, 0.00f, 1.00f}, "LOC_RUN_PARTNER_DESCRIPTION_B"s,  1.5f, 7.0f, true, true, true, "300,300,300,300,300,300"s, "1.0,1.2,1.4,1.6,1.8,2.0"s, "None"s, -1)},
        {0x2B3E43, BotInfo(0x9F723B2C, 'A', "12-14"s,    0xFF1942FC, {1.00f, 0.00f, 0.00f, 0.75f}, "LOC_RUN_PARTNER_DESCRIPTION_A"s,  1.5f, 7.0f, true, true, true, "300,300,300,300,300,300"s, "1.0,1.2,1.4,1.6,1.8,2.0"s, "None"s, -1)},
        {0x2CEB71, BotInfo(0xBFDF2BDC, 'C', "2.5-3.1"s,  0xFF3D6CF3, {0.95f, 0.42f, 0.24f, 1.00f}, "LOC_PACE_PARTNER_GUEST_DESCRIPTION"s, 1.5f, 7.0f, false, true, true, "900,900,900,900,300,300,300"s, "1.0,1.2,1.4,1.6,1.8,2.0,2.5"s, "Halloween2020"s, 0)},
        {0x2CEB55, BotInfo(0x9F723B2C, 'D', "1-2.4"s,    0xFF3D6CF3, {0.95f, 0.42f, 0.24f, 1.00f}, "LOC_PACE_PARTNER_GUEST_DESCRIPTION"s, 1.5f, 7.0f, false, true, true, "900,900,900,900,300,300,300"s, "1.0,1.2,1.4,1.6,1.8,2.0,2.5"s, "Halloween2020"s, 1)},
        {0x2CEB7A, BotInfo(0x2C546CB9, 'D', "1-2.4"s,    0xFF3D6CF3, {0.95f, 0.42f, 0.24f, 1.00f}, "LOC_PACE_PARTNER_GUEST_DESCRIPTION"s, 1.5f, 7.0f, false, true, true, "900,900,900,900,300,300,300"s, "1.0,1.2,1.4,1.6,1.8,2.0,2.5"s, "Halloween2020"s, 2)},
    };
}
struct PacerBotSweatOMeter {
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
    /* TODO:
ActivityRange(float)
AddFollower(long long)
AdjustPacePartnerRanges(ZNETWORK_PacePartnerInfo)
Broadcast_Update(void)
CalculatePacePartnerRanges(void)
CheckForFollowingDistance(void)
DetectAndResetIfDifferentDirection(BikeEntity const*,BikeEntity const*)
EngageRange(float)
FlushStaleFollowers(void)
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
PacerBotSweatOMeter(void)
ReadData(PacerBotSweatOMeter::HalloweenPacerBot,bool,long long)
RemoveFollower(long long)
SendPacePartnerPeriodBroadcast(float)
SendStatusToPacePartner(PacerBotSweatOMeter::BroadcastState)
SetBotInfo(long long)
UpdateDistanceRanges(float,long long,PacerBotSweatOMeter::BroadcastState)
UpdateDropsInfoList(std::string const&,std::string const&)
UpdateDropsMultiplier(float)
UpdateFollowerCount(ulong)
~PacerBotSweatOMeter()
_detail::SingletonBase<PacerBotSweatOMeter>::Instance(void)    */
};