#include "ZwiftApp.h"
bool g_NetworkOn;
void ZNETWORK_Shutdown() {
    if (g_NetworkOn) {
        zwift_network::shutdown_zwift_network();
        g_NetworkOn = false;
    }
}
namespace zwift_network {
    void shutdown_zwift_network() {
        //TODO
    }
    void get_goals(int64_t playerId) { 
        //TODO
    }
    void save_goal(const protobuf::Goal &g) {
        //TODO
    }
}
uint64_t g_serverTime;
double g_accumulatedTime;
uint64_t ZNETWORK_GetNetworkSyncedTimeGMT() {
    if (!g_serverTime)
        return 0i64;
    uint64_t ovf_corr = 0i64;
    if (g_accumulatedTime >= 9.223372036854776e18) {
        g_accumulatedTime -= 9.223372036854776e18;
        if (g_accumulatedTime - 9.223372036854776e18 < 9.223372036854776e18)
            ovf_corr = 0x8000000000000000ui64;
    }
    return g_serverTime + ovf_corr + (uint64_t)g_accumulatedTime;
}
bool ZNETWORK_IsLoggedIn() {
    //TODO
    return false;
}