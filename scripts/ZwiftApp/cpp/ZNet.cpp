#include "ZwiftApp.h"
bool g_NetworkOn;
void ZNETWORK_Shutdown() {
    if (g_NetworkOn) {
        zwift_network::shutdown_zwift_network();
        g_NetworkOn = false;
    }
}
struct NetworkClientOptions {
    bool m_skipCertCheck;
    int m_timeoutSec, m_uploadTimeoutSec;
    NetworkLogLevel m_maxLogLevel;
    bool m_disableEncr = g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\DISABLE_ENCRYPTION", false, false), 
        m_bHttpOnly, m_ignoreEncryptionFeatureFlag, m_disableEncryptionWithServer, m_disableEncryptionWithZc;
    std::string m_secretKeyBase64;
    uint64_t m_zcSecurePort;
    std::string m_zcSecretKeyBase64;
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
    std::string m_curlVersion;
    std::string m_zaVersion;
    std::string m_machineId;
    void initialize(const std::string &zaVersion, const std::string &machineId) {
        auto v = curl_version_info(CURLVERSION_NOW);
        m_curlVersion = v->version;
        NetworkingLogDebug("curl/%s ssl/%s zlib/%s", v->version, v->ssl_version, v->libz_version);
        m_zaVersion = zaVersion;
        m_machineId = machineId;
    }
};
const char *g_CNL_VER = "3.27.4";
struct HttpConnection {

};
struct HttpConnectionManager {
    CurlHttpConnectionFactory *m_curlf;
    std::string m_certs;
    std::mutex m_mutex;
    std::condition_variable m_conditionVar;
    int m_ncoTimeoutSec, m_ncoUploadTimeoutSec, m_ctr_a7, m_nThreads;
    bool m_ncoSkipCertCheck;
    HttpConnectionManager(CurlHttpConnectionFactory *curlf, const std::string &certs, bool ncoSkipCertCheck, int ncoTimeoutSec, int ncoUploadTimeoutSec, int a7, int nThreads) :
        m_curlf(curlf), m_certs(certs), m_ncoTimeoutSec(ncoTimeoutSec), m_ncoUploadTimeoutSec(ncoUploadTimeoutSec), m_ctr_a7(a7), m_nThreads(nThreads), m_ncoSkipCertCheck(ncoSkipCertCheck) {}
};
struct GenericHttpConnectionManager : public HttpConnectionManager { //0x128 bytes
    GenericHttpConnectionManager(CurlHttpConnectionFactory *curlf, const std::string &certs, bool ncoSkipCertCheck, int ncoTimeoutSec, int ncoUploadTimeoutSec, int a7) :
        HttpConnectionManager(curlf, certs, ncoSkipCertCheck, ncoTimeoutSec, ncoUploadTimeoutSec, a7, 1) {
        //TODO
        /*v12 = (char **)operator new(0x10ui64);
        v12[1] = 0i64;
        *(_QWORD *)&this->field_100 = v12;
        *v12 = &this->field_100;*/
    }
};
#if 0
struct ZwiftHttpConnectionManager : public HttpConnectionManager { //0x160 bytes
    void StartThreads() {
        for (auto i = 0; i < m_nThreads; ++i) {
            //TODO
            /* *(_QWORD *)&v7 = this;
            DWORD2(v7) = i;
            v3 = *(_QWORD *)&this->m_base.field_58;
            if ( v3 == *(_QWORD *)&this->m_base.field_60 )
            {
              sub_7FF620A5A610((__int64 *)&this->m_base.field_50, *(char **)&this->m_base.field_58, &v7);
            }
            else
            {
              v4 = operator new(0x10ui64);
              v5 = v4;
              if ( v4 )
                *v4 = v7;
              else
                v5 = 0i64;
              v8 = v5;
              v6 = beginthreadex(0i64, 0, sub_7FF620A5A7F0, v5, 0, (unsigned int *)(v3 + 8));
              *(_QWORD *)v3 = v6;
              if ( !v6 )
              {
                *(_DWORD *)(v3 + 8) = 0;
                std::_Throw_Cpp_error(6);
              }
              *(_QWORD *)&this->m_base.field_58 += 16i64;
            }
            */
        }
    }
    ZwiftHttpConnectionManager(CurlHttpConnectionFactory *curlf, const std::string &certs, bool ncoSkipCertCheck, ) :
        m_curlf(curlf), m_certs(certs), m_nco_4(nco_4), m_nco_8(nco_8), m_ctr_a7(a7), m_nThreads(1), m_ncoSkipCertCheck(ncoSkipCertCheck) {
        //TODO
        /*  *(_QWORD *)&this->field_100 = 0i64;
  *(_QWORD *)&this->field_108 = 0i64;
  *(_QWORD *)&this->field_100 = *a5;
  *(_QWORD *)&this->field_108 = a5[1];
  *a5 = 0i64;
  a5[1] = 0i64;
  *(_QWORD *)&this->field_110 = a6;
  *(_QWORD *)&this->field_118 = 0i64;
  *(_QWORD *)&this->field_120 = 0i64;
  *(_QWORD *)&this->field_128 = 0i64;
  *(_QWORD *)&this->field_130 = 0i64;
  *(_QWORD *)&this->field_138 = 0i64;
  v15 = (char **)operator new(0x10ui64);
  v15[1] = 0i64;
  *(_QWORD *)&this->field_118 = v15;
  *v15 = &this->field_118;
  this->field_140 = 0;
  *(_QWORD *)&this->field_148 = 0i64;
  *(_QWORD *)&this->field_150 = 0i64;
  *(_QWORD *)&this->field_158 = 1i64;
*/
        StartThreads();
    }
}
#endif
struct JsonWebToken {
    std::string asString() {
        //TODO
        return std::string();
    }
    std::string getSessionState() {
        //TODO
        return std::string();
    }
    std::string getSubject() {
        //TODO
        return std::string();
    }
};
struct Oauth2Credentials {
    Oauth2Credentials() {
        //TODO
    }
    std::string asString() {
        //TODO
        return std::string();
    }
    JsonWebToken getAccessToken() {
        //TODO
        return JsonWebToken();
    }
    uint64_t getAccessTokenExpiresIn() {
        //TODO
        return 0;
    }
    JsonWebToken getRefreshToken() {
        //TODO
        return JsonWebToken();
    }
    void parse(const std::string &) {
        //TODO
    }
};
struct ZwiftAuthenticationManager { //0x118 bytes, many virtual functions
    std::string m_apiUrl, m_accessToken, m_mail, m_password, m_oauthClient, m_field_F8;
    uint64_t m_accessTokeDeathTime = 0, m_reqId = 0;
    Oauth2Credentials *m_oauth2;
    bool m_field_80 = true, m_loggedIn = false; //all other data also 0
    ZwiftAuthenticationManager(const std::string &server) { m_apiUrl = server + "/api/auth"; }
    ~ZwiftAuthenticationManager() { //vptr[0]
        //TODO: destroy() inlined
        //looks like field_40 is shared ptr too - destructed; also pure base class (AuthenticationManager) dtr called
    }
    bool isAccessTokenInvalidOrExpired() { return m_accessToken.empty() || int64_t(g_steadyClock.now() - m_accessTokeDeathTime) > 0; } //vptr[1]
    const std::string &getAccessTokenHeader() { return m_accessToken; } //vptr[2]
    const std::string &getRefreshTokenStr() { return m_oauth2->getRefreshToken().asString(); } //vptr[3]
    const JsonWebToken &getRefreshToken() { return m_oauth2->getRefreshToken(); } //vptr[4]
    const std::string &getSessionStaleFromToken() { return m_oauth2->getAccessToken().getSessionState(); } //vptr[5]
    const std::string &getSubjectFromToken() { return m_oauth2->getAccessToken().getSubject(); } //vptr[6]
    const std::string &getOauthClient() { return m_oauthClient; } //vptr[7]
    void setLoggedIn(bool val) { m_loggedIn = val; } //vptr[8]
    bool getLoggedIn() { return m_loggedIn; } //vptr[9]
    void attendToAccessToken(HttpConnection *a2) { //vptr[10]
        //TODO
    }
    void setRequestId(uint64_t id) { m_reqId = id; } //vptr[11]
    uint64_t setRequestId() { return m_reqId; } //vptr[12]
    void setAccessTokenAsExpired() { m_accessTokeDeathTime = 0; } //vptr[13]
    bool resetCredentials() { //vptr[14]
        m_accessToken.clear();
        m_mail.clear();
        m_accessTokeDeathTime = 0;
        m_field_80 = true;
        //TODO *(_DWORD *)&this->field_70 = 0; *(_QWORD *)&this->field_78 = 0i64;
        m_mail.clear();
        m_password.clear();
        m_oauthClient.clear();
        bool ret = m_loggedIn;
        m_loggedIn = false;
        return ret;
    }
#if 0
    void setCredentials(many strings) { //vptr[15]
        //TODO
    }
#endif
    void setCredentialsOld(const std::string &mail, const std::string &pwd, const std::string &oauthClient) { //vptr[16]
        m_accessToken.clear();
        m_accessTokeDeathTime = 0;
        m_field_80 = 1;
        //TODO *(_DWORD *)&this->field_70 = 0; *(_QWORD *)&this->field_78 = 0i64;
        setEmailAndPassword(mail, pwd);
        m_oauthClient = oauthClient;
        m_loggedIn = false;
    }
    void setCredentialsMid(const std::string &a2, const std::string &oauth, const std::string &oauthClient) { //vptr[17], last
        //TODO
    }
    void setEmailAndPassword(const std::string &mail, const std::string &pwd) {
        m_mail = mail;
        str_tolower(m_mail);
        m_password = pwd;
    }
};
struct NetworkClientImpl { //0x400 bytes, calloc
    MachineIdProviderFactory m_machine;
    CurlHttpConnectionFactory m_curlf;
    NetworkClientOptions m_nco{ false, 20, 300, /*NL_INFO RTL*/ NL_DEBUG };
    time_t m_netStartTime1, m_netStartTime2;
    GenericHttpConnectionManager *m_httpConnMgr0, *m_httpConnMgr1, *m_httpConnMgr2;
    ZwiftAuthenticationManager *m_authMgr;
    bool m_someFunc0;
    NetworkClientImpl() {}
    void somefunc0_0() {
        //TODO
#if 0
        if (!*(_QWORD *)&a1->field_98[656] && !(unsigned __int8)sub_7FF620845640((__int64)&a1->m_nco)) {
            v2 = (volatile signed __int32 *)operator new(0x10C80ui64);
            v3 = v2;
            if (v2) {
                *(_OWORD *)v2 = 0i64;
                *((_DWORD *)v2 + 2) = 1;
                *((_DWORD *)v2 + 3) = 1;
                *(_QWORD *)v2 = &std::_Ref_count_obj2<TcpClient>::`vftable';
                    sub_7FF620A3A4F0(
                        (__int64)(v2 + 16),
                        &a1->field_50,
                        (__int64 *)&a1->field_98[424],
                        (__int64 *)&a1->field_98[440],
                        (__int64 *)&a1->field_98[488],
                        &a1->field_98[640],
                        &a1->field_98[248],
                        &a1->field_98[280],
                        (__int64)a1);
            } else {
                v3 = 0i64;
            }
            v5 = v3 + 16;
            v6 = v3;
            sub_7FF620A45FF0(&a1->field_98[656], (__int64 *)&v5);
            if (v6 && _InterlockedExchangeAdd(v6 + 2, 0xFFFFFFFF) == 1) {
                v4 = v6;
                (**(void(__fastcall ***)(volatile signed __int32 *))v6)(v6);
                if (_InterlockedExchangeAdd(v4 + 3, 0xFFFFFFFF) == 1)
                    (*(void(__fastcall **)(volatile signed __int32 *))(*(_QWORD *)v6 + 8i64))(v6);
            }
        }
#endif
    }
    void somefunc0_1() {
        //TODO
#if 0
        v2 = *(_QWORD *)&a1->field_98[656];
        if (!v2)
            return result;
        sub_7FF620AFED00(v2);
        v3 = *(volatile signed __int32 **)&a1->field_98[664];
        result = 0i64;
        *(_QWORD *)&a1->field_98[656] = 0i64;
        *(_QWORD *)&a1->field_98[664] = 0i64;
        if (!v3)
            return result;
        result = (unsigned int)_InterlockedExchangeAdd(v3 + 2, 0xFFFFFFFF);
        if ((_DWORD)result != 1)
            return result;
        result = (**(__int64(__fastcall ***)(volatile signed __int32 *))v3)(v3);
        if (_InterlockedExchangeAdd(v3 + 3, 0xFFFFFFFF) == 1)
            return (*(__int64(__fastcall **)(volatile signed __int32 *))(*(_QWORD *)v3 + 8i64))(v3);
#endif
    }
    void somefunc0(bool mode) { //1st vfunc
        if (mode)
            somefunc0_1();
        else
            somefunc0_0();
        m_someFunc0 = mode;
    }
    void somefunc1(bool mode) { //2nd vfunc
#if 0
        v3 = *(_QWORD *)&a1->field_98[656];
        if (v3)
            sub_7FF620AFBAA0(v3, a2, a3);
#endif
    }
    ~NetworkClientImpl() { //3rd vfunc
        //TODO IDA NetworkClientImpl_destroy
    }
    void LogStart() {
        NetworkingLogInfo("CNL %s", g_CNL_VER);
        NetworkingLogInfo("Machine Id: %s", m_machine.m_id.c_str());
    }
    void initialize(const std::string &server, const std::string &certs, const std::string &zaVersion) {
        //SetNetworkMaxLogLevel(maxLogLevel); combined with g_MinLogLevel (why two???)
        m_curlf.initialize(zaVersion, m_machine.m_id);
        _time64(&m_netStartTime1);
        m_netStartTime2 = m_netStartTime1;
        LogStart();
        bool skipCertCheck = m_nco.m_skipCertCheck;
        //here is smart pointer used, but what for???
        m_httpConnMgr0 = new GenericHttpConnectionManager(&m_curlf, certs, skipCertCheck, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, 0);
        m_httpConnMgr1 = new GenericHttpConnectionManager(&m_curlf, certs, skipCertCheck, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, 1);
        m_httpConnMgr2 = new GenericHttpConnectionManager(&m_curlf, certs, skipCertCheck, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, 0);
        m_authMgr = new ZwiftAuthenticationManager(server);
        //TODO
    }
};
NetworkClient::NetworkClient() { m_pImpl = new(calloc(sizeof(NetworkClientImpl), 1)) NetworkClientImpl; }
NetworkClient::~NetworkClient() { m_pImpl->~NetworkClientImpl(); free(m_pImpl); }
void NetworkClient::globalInitialize() { curl_global_init(CURL_GLOBAL_ALL); }
void NetworkClient::globalCleanup() { curl_global_cleanup(); }
void NetworkClient::initialize(const std::string &server, const std::string &certs, const std::string &version) {
    m_pImpl->initialize(server, certs, version);
}
namespace zwift_network {
bool initialize_zwift_network(const std::string &server, const std::string &certs, const std::string &version) {
    NetworkClient::globalInitialize();
    g_networkClient = new NetworkClient();
    g_networkClient->initialize(server, certs, version);
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
    struct tm v51 { 16, 15, 14, 13, 0, 70 };
    g_magicLeaderboardBirthday = _mktime64(&v51);
    //OMIT CrashReporting::SetServer
    Log("Calling initialize_zwift_network()");
    LogDebug("  With serverurl: %s", server);
    g_NetworkOn = zwift_network::initialize_zwift_network(server, GAMEPATH("data/cacert.pem"), "1.0.106405");
    //zwift_network::register_logging_function inlined by URSOFT
    LogDebug("End ZNETWORK_Initilize() - serverURL = %s", server); //URSOFT: should be ZNETWORK_Initialize, but...
    PopulateBotInfo(server);
}