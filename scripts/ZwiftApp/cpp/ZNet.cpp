#include "ZwiftApp.h"
bool g_NetworkOn;
void ZNETWORK_Shutdown() {
    if (g_NetworkOn) {
        zwift_network::shutdown_zwift_network();
        g_NetworkOn = false;
    }
}
struct NetworkClientOptions {
    bool m_bField_0;
    int m_field_4, m_field_8;
    NetworkLogLevel m_maxLogLevel;
    bool m_field_10, m_disableEncr, m_field_12, m_field_13, m_field_14;
    std::string m_field_18;
    uint64_t m_field_38;
    std::string m_field_40;
};
struct MachineIdProviderFactory {
    std::string m_id;
    bool CheckMachineID(int ver) {
        //Will not check quality: almost any OK
        return (m_id.length() == 39 && ver + '0' == m_id[0] && m_id[1] == '-');
    }
    MachineIdProviderFactory() { //WindowsMachineIdProvider::acquireMachineId
        auto lpSubKey = "Software\\Zwift", lpValueName = "Machine Id";
        HKEY hKey{};
        DWORD disp = 0;
        auto status = RegCreateKeyExA(HKEY_CURRENT_USER, lpSubKey, 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &hKey, &disp);
        if (status) {
            NetworkingLogError("Failed to get or create registry key \"%s\". [status: %d]", lpSubKey, status);
        } else {
            if (disp != REG_CREATED_NEW_KEY) {
                DWORD len = 0;
                status = RegGetValueA(hKey, "", lpValueName, RRF_RT_REG_SZ, nullptr, nullptr, &len);
                if (status) {
                    NetworkingLogError("Failed to find registry value \"%s\". [status: %d]", lpValueName, status);
                } else {
                    m_id.resize(len - 1);
                    status = RegGetValueA(hKey, "", lpValueName, RRF_RT_REG_SZ, nullptr, m_id.data(), &len);
                    if (status) {
                        NetworkingLogError("Failed to get registry value \"%s\". [status: %d]", lpValueName, status);
                    } else {
                        if (CheckMachineID(1)) {
                            RegCloseKey(hKey);
                            return;
                        }
                        NetworkingLogError("Invalid machine id recovered: %s", m_id.c_str());
                    }
                }
            }
            m_id = "1-" + uuid::generate_uuid_v4();
            status = RegSetValueExA(hKey, lpValueName, 0, REG_SZ, (const BYTE *)m_id.c_str(), m_id.length() + 1);
            if (status) {
                NetworkingLogError("Failed to set registry value \"%s\". [status: %d]", lpValueName, status);
            } else {
                NetworkingLogInfo("New machine id created: %s", m_id.c_str());
                RegCloseKey(hKey);
                return;
            }
        }
        NetworkingLogError("Failed to acquire machine id.");
        m_id = "1-00000000-0000-0000-0000-000000000000";
        if (hKey)
            RegCloseKey(hKey);
    }
};
struct HttpStatistics { //0x320 - 16 bytes
    HttpStatistics() {
        //TODO
    }
};
struct CurlHttpConnectionFactory { //0x90 - 16 bytes
    HttpStatistics m_httpStat;
    bool m_field_18;
    std::string m_field_20;
    std::string m_field_40;
    std::string m_field_60;
};
struct NetworkClientImpl { //0x400 bytes, calloc
    MachineIdProviderFactory m_machine;
    CurlHttpConnectionFactory m_curlf;
    NetworkClientImpl() {}
    ~NetworkClientImpl() {
        //TODO
    }
    void initialize(const std::string &server, const std::string &certs, const std::function<void(char *)> &empty, const std::string &version, const NetworkClientOptions &nco) {
        //TODO
    }
};
NetworkClient::NetworkClient() { m_pImpl = new(calloc(sizeof(NetworkClientImpl), 1)) NetworkClientImpl; }
NetworkClient::~NetworkClient() { m_pImpl->~NetworkClientImpl(); free(m_pImpl); }
void NetworkClient::globalInitialize() { curl_global_init(CURL_GLOBAL_ALL); }
void NetworkClient::globalCleanup() { curl_global_cleanup(); }
void NetworkClient::initialize(const std::string &server, const std::string &certs, const std::function<void(char *)> &empty, const std::string &version, const NetworkClientOptions &nco) {
    m_pImpl->initialize(server, certs, empty, version, nco);
}
namespace zwift_network {
bool initialize_zwift_network(const std::string &server, const std::string &certs, const std::function<void(char *)> &empty, const std::string &version, const NetworkClientOptions &nco) {
    NetworkClient::globalInitialize();
    g_networkClient = new NetworkClient();
    g_networkClient->initialize(server, certs, empty, version, nco);
    return true;
}
void shutdown_zwift_network() {
    auto v0 = g_networkClient;
    g_networkClient = nullptr;
    if (v0)
        delete v0;
    NetworkClient::globalCleanup();
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
int g_ZNETWORK_Stats[8];
time_t g_lastPlayerStateTime, g_magicLeaderboardBirthday;
void PopulateBotInfo(const char *server) {
    //TODO
}
void ZNETWORK_Initialize() {
    auto server = g_UserConfigDoc.GetCStr("ZWIFT\\CONFIG\\SERVER_URL", "https://us-or-rly101.zwift.com", false);
    g_IsOnProductionServer = (strstr(server, "us-or-rly101") != nullptr);
    struct tm v51 {};
    v51.tm_year = 70;
    v51.tm_mday = 13;
    v51.tm_hour = 14;
    v51.tm_min = 15;
    v51.tm_sec = 16;
    g_magicLeaderboardBirthday = _mktime64(&v51);
    //OMIT CrashReporting::SetServer
    Log("Calling initialize_zwift_network()");
    LogDebug("  With serverurl: %s", server);
    bool disEncr = g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\DISABLE_ENCRYPTION", false, false);
    NetworkClientOptions v42{false, 20, 300, /*NL_INFO RTL*/ NL_DEBUG, false, disEncr, false, false, false}; //v42.setDisableEncryption(v23);
    std::string v38 = GAMEPATH("data/cacert.pem");
    std::function<void(char *)> v54;
    g_NetworkOn = zwift_network::initialize_zwift_network(server, v38, v54, "1.0.106405", v42);
    //zwift_network::register_logging_function inlined by URSOFT
    LogDebug("End ZNETWORK_Initilize() - serverURL = %s", server); //URSOFT: should be ZNETWORK_Initialize, but...
    PopulateBotInfo(server);
}