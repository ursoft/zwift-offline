#include "ZwiftApp.h"
bool g_NetworkOn;
void ZNETWORK_Shutdown() {
    if (g_NetworkOn) {
        zwift_network::shutdown_zwift_network();
        g_NetworkOn = false;
    }
}
void zwift_network::shutdown_zwift_network() {
    //TODO
}