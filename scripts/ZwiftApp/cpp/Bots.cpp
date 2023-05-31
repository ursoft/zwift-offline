#include "ZwiftApp.h"
//    std::__hash_table<std::__hash_value_type<long long,BotInfo>,std::__unordered_map_hasher<long long,std::__hash_value_type<long long,BotInfo>,std::hash<long long>,true>,std::__unordered_map_equal<long long,std::__hash_value_type<long long,BotInfo>,std::equal_to<long long>,true>,std::allocator<std::__hash_value_type<long long,BotInfo>>>::__emplace_unique_key_args<long long,std::pair<long long const,BotInfo> const&>(
/* kDefaultMultiplier, kDefaultMultiplierTime, kDefaultSpecialOccasion*/
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
