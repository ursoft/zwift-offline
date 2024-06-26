﻿//UT Coverage: 38%, 2503/6638
#include "ZwiftApp.h"
#include "readerwriterqueue/readerwriterqueue.h"
#include "concurrentqueue/concurrentqueue.h"
#include "openssl/md5.h"
#include "xxhash.h"
#include "unicode/ustdio.h"
#include <regex>
std::vector<uint8_t> fix_google(const protobuf_bytes &src) { return std::vector<uint8_t>((uint8_t *)src.data(), (uint8_t *)src.data() + src.length()); }
bool g_NetworkOn;
void ZNETWORK_Shutdown() {
    if (g_NetworkOn) {
        shutdown_zwift_network();
        g_NetworkOn = false;
    }
}
std::string_view NetworkRequestOutcomeToString(NetworkRequestOutcome code) {
    switch (code) {
    default:
        return "Unknown";
    case NRO_OK:
        return "OK";
    case NRO_INITIALIZATION_FAILED:
        return "INITIALIZATION_FAILED";
    case NRO_NOT_INITIALIZED:
        return "NOT_INITIALIZED";
    case NRO_NOT_LOGGED_IN:
        return "NOT_LOGGED_IN";
    case NRO_NO_LOG_IN_ATTEMPTED:
        return "NO_LOG_IN_ATTEMPTED";
    case NRO_NO_PLAYER_ID_YET:
        return "NO_PLAYER_ID_YET";
    case NRO_NO_WORLD_SELECTED:
        return "NO_WORLD_SELECTED";
    case NRO_INVALID_WORLD_ID:
        return "INVALID_WORLD_ID";
    case NRO_UDP_CLIENT_STOPPED:
        return "UDP_CLIENT_STOPPED";
    case NRO_DISCONNECTED_DUE_TO_SIMULTANEOUS_LOGINS:
        return "DISCONNECTED_DUE_TO_SIMULTANEOUS_LOGINS";
    case NRO_CLIENT_TO_SERVER_IS_TOO_BIG:
        return "CLIENT_TO_SERVER_IS_TOO_BIG";
    case NRO_REQUEST_ABORTED:
        return "REQUEST_ABORTED";
    case NRO_INVALID_ARGUMENT:
        return "INVALID_ARGUMENT";
    case NRO_REQUEST_TIMED_OUT:
        return "REQUEST_TIMED_OUT";
    case NRO_UNEXPECTED_HTTP_RESPONSE:
        return "UNEXPECTED_HTTP_RESPONSE";
    case NRO_TOO_MANY_SEGMENT_RESULTS_SUBSCRIPTIONS:
        return "TOO_MANY_SEGMENT_RESULTS_SUBSCRIPTIONS";
    case NRO_HTTP_STATUS_BAD_REQUEST:
        return "HTTP_STATUS_BAD_REQUEST";
    case NRO_HTTP_STATUS_UNAUTHORIZED:
        return "HTTP_STATUS_UNAUTHORIZED";
    case NRO_HTTP_STATUS_FORBIDDEN:
        return "HTTP_STATUS_FORBIDDEN";
    case NRO_HTTP_STATUS_NOT_FOUND:
        return "HTTP_STATUS_NOT_FOUND";
    case NRO_HTTP_STATUS_CONFLICT:
        return "HTTP_STATUS_CONFLICT";
    case NRO_HTTP_STATUS_GONE:
        return "HTTP_STATUS_GONE";
    case NRO_HTTP_STATUS_TOO_MANY_REQUESTS:
        return "HTTP_STATUS_TOO_MANY_REQUESTS";
    case NRO_HTTP_STATUS_SERVICE_UNAVAILABLE:
        return "HTTP_STATUS_SERVICE_UNAVAILABLE";
    case NRO_HTTP_STATUS_BANDWIDTH_LIMIT_EXCEEDED:
        return "HTTP_STATUS_BANDWIDTH_LIMIT_EXCEEDED";
    }
}
struct QueryStringBuilder : std::multimap<std::string, std::string> {
    void add(const std::string &name, bool val) { emplace(name, val ? "true"s : "false"s); }
    void add(const std::string &name, const char *val) { emplace(name, val); }
    void add(const std::string &name, int32_t val) { emplace(name, std::to_string(val)); }
    void add(const std::string &name, int64_t val) { emplace(name, std::to_string(val)); }
    void add(const std::string &name, const std::string &val) { emplace(name, val); }
    void add(const std::string &name, uint32_t val) { emplace(name, std::to_string(val)); }
    void add(const std::string &name, uint64_t val) { emplace(name, std::to_string(val)); }
    void addIfNotEmpty(const std::string &name, const char *val) { if (val && *val) emplace(name, val); }
    void addIfNotEmpty(const std::string &name, const std::string &val) { if (!val.empty()) emplace(name, val); }
    void addIfNotFalse(const std::string &name, bool val) { if (val) emplace(name, "true"s); }
    void addIfNotZero(const std::string &name, int32_t val) { if(val) emplace(name, std::to_string(val)); }
    void addIfNotZero(const std::string &name, int64_t val) { if (val) emplace(name, std::to_string(val)); }
    void addIfNotZero(const std::string &name, uint64_t val) { if (val) emplace(name, std::to_string(val)); }
    void addIfNotZero(const std::string &name, uint32_t val) { if (val) emplace(name, std::to_string(val)); }
    template<class T>
    void addOptional(const std::string &name, const Optional<T> &val) {
        if (val.m_hasValue)
            add(name, val.m_T);
    }
    std::string getString(bool needQuest) {
        int total = 0;
        for (const auto &i : *this)
            total += int(i.first.length() + i.second.length() + 3);
        std::string ret;
        ret.reserve(total);
        if (needQuest)
            ret += '?';
        for(const auto &i : *this) {
            if (ret.length() > 1)
                ret += '&';
            ret += i.first;
            ret += '=';
            ret += i.second;
        }
        return ret;
    }
};
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
            status = RegSetValueExA(hKey, lpValueName, 0, REG_SZ, (const BYTE *)m_id.c_str(), DWORD(m_id.length() + 1));
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
struct ElapsedTimeStatistics { //32 bytes
    int64_t m_elapsedTime = 0;
    int64_t m_minElapsedTime = std::numeric_limits<int64_t>::max();
    int64_t m_maxElapsedTime = 0;
    int m_counter = 0;
    void addElapsedTime(int64_t a2) {
        m_counter++;
        m_elapsedTime += a2;
        if (m_maxElapsedTime < a2) m_maxElapsedTime = a2;
        if (m_minElapsedTime > a2) m_minElapsedTime = a2;
    }
    int64_t getAverageTime() { return (m_counter) ? m_elapsedTime / m_counter : 0; }
    int64_t getSampleCount() { return m_counter; }
    void toJson() { /*OMIT - telemetry? */ }
    int64_t getMinTime() { return m_counter ? m_minElapsedTime : 0; }
    int64_t getMaxTime() { return m_maxElapsedTime; }
};
struct HttpStatistics { //0x320 - 16 bytes
    struct EndpointEvent {};
    bool m_field_0 = true;
    ElapsedTimeStatistics m_field_8, m_field_28;
    moodycamel::ConcurrentQueue<HttpStatistics::EndpointEvent> m_field_80;
    HttpStatistics() : m_field_80(100) {
        /*OMIT TELEMETRY (_OWORD *)((char *)this + 104) = 0u;
        *(_OWORD *)((char *)this + 88) = 0u;
        *(_OWORD *)((char *)this + 72) = 0u;*/
    }
};
std::string g_CNL_VER = "3.27.4"s;
enum HttpRequestMode { HRM_SEQUENTIAL, HRM_CONCURRENT };
enum AcceptType { ATH_PB, ATH_JSON, ATH_OCTET };
struct AcceptHeader {
    AcceptHeader() {}
    AcceptHeader(AcceptType src) {
        switch (src) {
        case ATH_PB:
            m_hdr = "Accept: application/x-protobuf-lite"s;
            break;
        case ATH_JSON:
            m_hdr = "Accept: application/json"s;
            break;
        case ATH_OCTET:
            m_hdr = "Accept: application/octet-stream"s;
            break;
        default:
            break;
        }
    }
    std::string m_hdr; 
};
enum ContentType { CTH_UNK, CTH_JSON, CTH_PB, CTH_PBv2, CTH_URLENC, CTH_OCTET };
struct ContentTypeHeader { 
    ContentTypeHeader() {}
    ContentTypeHeader(ContentType src) {
        switch (src) {
        case CTH_JSON:
            m_hdr = "Content-Type: application/json"s;
            break;
        case CTH_PB:
            m_hdr = "Content-Type: application/x-protobuf-lite"s;
            break;
        case CTH_PBv2:
            m_hdr = "Content-Type: application/x-protobuf-lite; version=2.0"s;
            break;
        case CTH_URLENC:
            m_hdr = "Content-Type: application/x-www-form-urlencoded"s;
            break;
        case CTH_OCTET:
            m_hdr = "Content-Type: application/octet-stream"s;
            break;
        default:
            break;
        }
    }
    std::string m_hdr; 
};
std::string GetWinVer() { return "Windows"s; /* OMIT exact version */ }
using QueryResult = NetworkResponse<std::vector<char>>;
struct CurlHttpConnection {
    static inline std::mutex g_libcryptoMutex;
    CURL *m_curl;
    curl_slist *m_headers = nullptr;
    void *m_cbData;
    std::string m_authHeader, m_sessionState, m_subject, m_sidHeader, m_requestIdHdr, m_cainfo, m_platformHeader, m_uaHeader, m_machHeader;
    std::string m_hrmHeader;
    uint64_t m_requestId = 0, m_headersSize = 0;
    int m_timeout, m_uplTimeout;
    HttpRequestMode m_hrm;
    bool m_http2, m_sslNoVerify;
    CurlHttpConnection(const std::string &certs, bool ncoSkipCertCheck, bool http2, int ncoTimeoutSec, int ncoUploadTimeoutSec,
        const std::string &curlVersion, const std::string &zaVersion, const std::string &machineId, bool *pKilled, HttpRequestMode hrm) {
        m_cainfo = certs;
        m_sslNoVerify = ncoSkipCertCheck;
        m_http2 = http2;
        m_timeout = ncoTimeoutSec;
        m_uplTimeout = ncoUploadTimeoutSec;
        m_cbData = pKilled;
        m_hrm = hrm;
        m_curl = curl_easy_init();
        if (!m_curl)
            NetworkingLogError("Fail to acquire curl handle.");
        m_platformHeader = "PLATFORM: PC"s;
        m_uaHeader = "User-Agent: CNL/"s + g_CNL_VER + " ("s + GetWinVer() + ") zwift/"s + zaVersion + " curl/"s + curlVersion;
        m_machHeader = "X-Machine-Id: "s + machineId;
        if (hrm == HRM_CONCURRENT)
            m_hrmHeader = "X-Requested-With: ConcurrentHttpRequest"s;
    }
    void appendHeader(const std::string &h) {
        if (h.size()) {
            m_headers = curl_slist_append(m_headers, h.data());
            m_headersSize += h.size();
        }
    }
    ~CurlHttpConnection() /*vptr[0]*/ {
        if (m_curl)
            curl_easy_cleanup(m_curl);
        curl_slist_free_all(m_headers);
    }
    void setAuthorizationHeader(const std::string &ah) /*vptr[1]*/ { m_authHeader = ah; }
    void clearAuthorizationHeader() /*vptr[2]*/ { m_authHeader.clear(); }
    void setTokenInfo(const std::string &sessionState, const std::string &subject) /*vptr[3]*/ {
        m_sessionState = sessionState; m_subject = subject;
    }
    void clearTokenInfo() /*vptr[4]*/ { m_sessionState.clear(); m_subject.clear(); }
    void setSessionIdHeader(const std::string &val) /*vptr[5]*/ { m_sidHeader = "X-Zwift-Session-Id: " + val; }
    void setRequestId(uint64_t id) /*vptr[6]*/ { m_requestId = id; m_requestIdHdr = "X-Request-Id: " + std::to_string(id); }
    void setTimeout(int to) /*vptr[7]*/ { m_timeout = to; }
    int getTimeout() /*vptr[8]*/ { return m_timeout; }
    void setUploadTimeout(int to) /*vptr[9]*/ { m_uplTimeout = to; }
    const static inline std::string GET = "GET"s;
    QueryResult performGet(const std::string &url, const AcceptHeader &ach, const std::string &descr) /*vptr[10]*/ {
        reset(false, 0, true);
        curl_easy_setopt(m_curl, CURLOPT_HTTPGET, 1);
        appendHeader(ach.m_hdr);
        return performRequest(GET, url, descr, 0);
    }
    const static inline std::string sDELETE = "DELETE"s;
    QueryResult performDelete(const std::string &url, const AcceptHeader &ach, const std::string &descr) /*vptr[11]*/ {
        reset(false, 0, true);
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, sDELETE.c_str());
        appendHeader(ach.m_hdr);
        return performRequest(sDELETE, url, descr, 0);
    }
    const static inline std::string POST = "POST"s;
    QueryResult performPost(const std::string &url, const ContentTypeHeader &cth, const std::vector<char> &payload, const AcceptHeader &ach, const std::string &descr, bool upload) /*vptr[12,13], performPostVec*/ {
        reset(upload, 0, true);
        curl_easy_setopt(m_curl, CURLOPT_POST, 1);
        appendHeader(cth.m_hdr);
        appendHeader(ach.m_hdr);
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, payload.size());
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, payload.data());
        return performRequest(POST, url, descr, payload.size());
    }
    QueryResult performPost(const std::string &url, const ContentTypeHeader &cth, const std::string &payload, const AcceptHeader &ach, const std::string &descr, bool upload) { /*vptr[14], performPostStr*/
        reset(upload, 0, true);
        curl_easy_setopt(m_curl, CURLOPT_POST, 1);
        appendHeader(cth.m_hdr);
        appendHeader(ach.m_hdr);
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, payload.size());
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, payload.data());
        return performRequest(POST, url, descr, payload.size());
    }
    QueryResult performPost(const std::string &url, const AcceptHeader &ach, const std::string &descr) { /*vptr[15]*/
        reset(false, 0, true);
        curl_easy_setopt(m_curl, CURLOPT_POST, 1);
        appendHeader(ach.m_hdr);
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, 0);
        return performRequest(POST, url, descr, 0);
    }
    QueryResult performPostWithTimeout(const std::string &url, const ContentTypeHeader &cth, const std::string &payload, const AcceptHeader &ach, const std::string &descr, int to) { /*vptr[16]*/
        reset(false, to, true);
        curl_easy_setopt(m_curl, CURLOPT_POST, 1);
        appendHeader(cth.m_hdr);
        appendHeader(ach.m_hdr);
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, payload.size());
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, payload.data());
        return performRequest(POST, url, descr, payload.size());
    }
    QueryResult performPostWithHash(const std::string &url, const ContentTypeHeader &cth, const std::vector<char> &payload, const AcceptHeader &ach, const std::string &descr, const std::string &hash) { /*vptr[17]*/
        reset(false, 0, true);
        curl_easy_setopt(m_curl, CURLOPT_POST, 1);
        appendHeader(cth.m_hdr);
        appendHeader(ach.m_hdr);
        appendHashHeader(payload, hash);
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, payload.size());
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, payload.data());
        return performRequest(POST, url, descr, payload.size());
    }
    const static inline std::string PUT = "PUT"s;
    QueryResult performPut(const std::string &url, const ContentTypeHeader &cth, const std::vector<char> &payload, const AcceptHeader &ach, const std::string &descr, bool upload) { /*vptr[18] performPutVec*/
        reset(upload, 0, true);
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, PUT.data());
        appendHeader(cth.m_hdr);
        appendHeader(ach.m_hdr);
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, payload.size());
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, payload.data());
        return performRequest(PUT, url, descr, payload.size());
    }
    QueryResult performPut(const std::string &url, const ContentTypeHeader &cth, const std::string &payload, const AcceptHeader &ach, const std::string &descr, bool upload) { /*vptr[19] performPutStr*/
        reset(upload, 0, true);
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, PUT.data());
        appendHeader(cth.m_hdr);
        appendHeader(ach.m_hdr);
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, payload.size());
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, payload.data());
        return performRequest(PUT, url, descr, payload.size());
    }
    QueryResult performPut(const std::string &url, const AcceptHeader &ach, const std::string &descr, bool upload) { /*vptr[20]*/
        reset(false, 0, true);
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, PUT.data());
        appendHeader(ach.m_hdr);
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, 0);
        return performRequest(PUT, url, descr, 0);
    }
    QueryResult performPutOrPost(bool put, const std::string &url, const ContentTypeHeader &cth, const std::vector<char> &payload, const AcceptHeader &ach, const std::string &putDescr, const std::string &postDescr, bool upload) { /*vptr[21] performPutOrPostVec*/
        return put ? performPut(url, cth, payload, ach, putDescr, upload) : performPost(url, cth, payload, ach, postDescr, upload);
    }
    QueryResult performPutOrPost(bool put, const std::string &url, const ContentTypeHeader &cth, const std::string &payload, const AcceptHeader &ach, const std::string &putDescr, const std::string &postDescr, bool upload) { /*vptr[22]*/
        return put ? performPut(url, cth, payload, ach, putDescr, upload) : performPost(url, cth, payload, ach, postDescr, upload);
    }
    std::string escapeUrl(const std::string &src) { /*vptr[23]*/ auto ptr = curl_easy_escape(m_curl, src.c_str(), int(src.size())); auto ret = std::string(ptr); curl_free(ptr); return ret; }

    static int progressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
        return *((bool *)clientp);
    }
    static size_t writeCallback(char *ptr, size_t size, size_t nmemb, std::vector<char> *userdata) {
        auto s = size * nmemb;
        userdata->insert(userdata->end(), ptr, ptr + s);
        return s;
    }
    void reset(bool upload, int uplTimeout, bool useAuth) {
        curl_easy_reset(m_curl);
        curl_slist_free_all(m_headers);
        m_headersSize = 0;
        m_headers = nullptr;
        curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
        curl_easy_setopt(m_curl, CURLOPT_XFERINFODATA, m_cbData);
        curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS);
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(m_curl, CURLOPT_CAINFO, m_cainfo.c_str());
        curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);
        if (m_http2)
            curl_easy_setopt(m_curl, CURLOPT_HTTP_VERSION, 2);
        if (!uplTimeout) {
            if (upload)
                uplTimeout = m_uplTimeout;
            else
                uplTimeout = m_timeout;
        }
        curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, uplTimeout);
        curl_easy_setopt(m_curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);
        if (m_sslNoVerify) {
            curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0);
        }
        appendHeader(m_platformHeader);
        appendHeader("SOURCE: Game Client"s);
        appendHeader(m_uaHeader);
        appendHeader(m_machHeader);
        if (useAuth)
            appendHeader(m_authHeader);
    }
    void appendHashHeader(const std::vector<char> &payload, const std::string &hash) {
        std::vector<char> v35;
        v35.push_back(-61);
        v35.push_back(-72);
        v35.insert(v35.end(), payload.begin(), payload.end());
        v35.push_back(-61);
        v35.push_back(-122);
        v35.insert(v35.end(), m_sessionState.begin(), m_sessionState.end());
        v35.push_back(-61);
        v35.push_back(-98);
        v35.insert(v35.end(), m_subject.begin(), m_subject.end());
        v35.push_back(-61);
        v35.push_back(-105);
        v35.insert(v35.end(), hash.begin(), hash.end());
        uint8_t md5h[16];
        MD5((const uint8_t *)v35.data(), v35.size(), md5h);
        char xhash[33], *pxhash = xhash;
        for (auto c : md5h) {
            sprintf(pxhash, "%02x", c);
            *pxhash += 2;
        }
        const std::string h("X-Zwift-Hash: ");
        appendHeader(h + xhash);
    }
    QueryResult performRequest(const std::string &op, const std::string &url, const std::string &descr, size_t payloadSize) {
        QueryResult ret;
        appendHeader(m_sidHeader);
        appendHeader(m_requestIdHdr);
        curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_headers);
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &ret.m_T);
        CURLcode v47;
        uint64_t v51;
        if (m_hrm == HRM_CONCURRENT) { // CurlHttpConnection::performConcurrentRequest
            appendHeader(m_hrmHeader);
            NetworkingLogDebug("Performing concurrent request: %s %s (payload: %li bytes)", op.c_str(), url.c_str(), payloadSize);
            Stopwatch v62;
            v47 = curl_easy_perform(m_curl);
            v62.stop();
            v51 = v62.elapsedInMilliseconds();
        } else { // CurlHttpConnection::performSequentialRequest
            //OMIT Stopwatch v66;
            std::lock_guard l(g_libcryptoMutex);
            //OMIT v66.stop();
            NetworkingLogDebug("Performing request: %s %s (payload: %li bytes)", op.c_str(), url.c_str(), payloadSize);
            Stopwatch v62;
            v47 = curl_easy_perform(m_curl);
            v62.stop();
            //OMIT v22 = Stopwatch::elapsed(v66, &v56);
            //OMIT HttpStatistics::addRequestLockTime(m_stat->field_0, *v22);
            v51 = v62.elapsedInMilliseconds();
        }
        if (v47) {
            //OMIT HttpStatistics::enqueueEndpointEvent(m_stat, descr, this->m_requestId, 0, v47, v51);
            if (v47 == CURLE_OPERATION_TIMEDOUT) {
                NetworkingLogWarn("Request timed out for: %s %s", op.c_str(), url.c_str());
                ret.storeError(NRO_REQUEST_TIMED_OUT, "Request timed out"s);
                //QUEST: why was empty ret here
                return ret;
            }
            auto v60 = curl_easy_strerror(v47);
            NetworkingLogError("Curl error: [%d] '%s' for: %s %s", v47, v60, op.c_str(), url.c_str());
            ret.storeError(NRO_CURL_ERROR, v60);
            //QUEST: why was empty ret here
            return ret;
        } else {
            //OMIT v63 = 0i64;
            //OMIT curl_easy_getinfo(m_curl, CURLINFO_REQUEST_SIZE, &v63);
            //OMIT HttpStatistics::addRequestSize(m_stat->field_0, v63);
            //OMIT HttpStatistics::addResponseSize(m_stat->field_0, *((_QWORD *)v15 + 3) - (_QWORD)*v16);
            long v59 = 0;
            curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &v59);
            //OMIT HttpStatistics::enqueueEndpointEvent(m_stat->field_0, descr, this->m_requestId, (int)v59, 0, v51);
            NetworkRequestOutcome v31 = NRO_HTTP_STATUS_SERVICE_UNAVAILABLE;
            switch (v59) {
            case 200: case 201: case 202: case 203: case 204: case 205: case 206:
                v31 = NRO_OK;
                break;
            case NRO_HTTP_STATUS_BAD_REQUEST: case NRO_HTTP_STATUS_UNAUTHORIZED: case NRO_HTTP_STATUS_FORBIDDEN:
            case NRO_HTTP_STATUS_NOT_FOUND: case NRO_HTTP_STATUS_CONFLICT: case NRO_HTTP_STATUS_GONE:
            case NRO_HTTP_STATUS_TOO_MANY_REQUESTS /*added by URSOFT */ :
            case NRO_HTTP_STATUS_SERVICE_UNAVAILABLE: case NRO_HTTP_STATUS_BANDWIDTH_LIMIT_EXCEEDED:
                v31 = (NetworkRequestOutcome)v59;
                break;
            default:
                if (v59 > 503) {
                    std::string v61;
                    if (ret.m_T.size())
                        v61.assign(ret.m_T.data(), ret.m_T.size());
                    NetworkingLogWarn("Unexpected HTTP response: [%d] '%s' for: %s %s", v59, v61.c_str(), op.c_str(), url.c_str());
                    ret.storeError(NRO_UNEXPECTED_HTTP_RESPONSE, v61);
                    //QUEST: why was empty ret here
                    return ret;
                }
            }
            NetworkingLogDebug("Completed request: %s %s (status: %d, elapsed: %lims)", op.c_str(), url.c_str(), v59, v51);
            if (v31 != NRO_OK) {
                std::string v61;
                if (ret.m_T.size())
                    v61.assign(ret.m_T.data(), ret.m_T.size());
                ret.storeError(v31, v61);
                //QUEST: why was empty ret here
            }
        }
        return ret;
    }
};
struct CurlHttpConnectionFactory { //0x90 - 16 bytes
    //OMIT TELEMETRY HttpStatistics m_httpStat;
    bool m_killed = false;
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
    ~CurlHttpConnectionFactory() {}
    void shutdown() { m_killed = true; }
    std::unique_ptr<CurlHttpConnection> instance(const std::string &certs, bool ncoSkipCertCheck, bool a5, int ncoTimeoutSec, int ncoUploadTimeoutSec, HttpRequestMode hrm) {
        return std::make_unique<CurlHttpConnection>(certs, ncoSkipCertCheck, a5, ncoTimeoutSec, ncoUploadTimeoutSec, m_curlVersion, m_zaVersion,
            m_machineId, &m_killed, hrm);
    }
    //global downloader will init/deinit CURL CURLcode globalInitialize() { return curl_global_init(CURL_GLOBAL_DEFAULT); } globalCleanup()
    //OMITE TELEMETRY getHttpStatistics()
    CurlHttpConnectionFactory() {}
};
struct HttpConnectionManager {
    CurlHttpConnectionFactory *m_curlf;
    std::string m_certs;
    std::mutex m_mutex;
    std::condition_variable m_conditionVar;
    std::vector<std::jthread> m_pool;
    uint32_t m_ncoTimeoutSec, m_ncoUploadTimeoutSec, m_nThreads;
    HttpRequestMode m_hrm;
    bool m_ncoSkipCertCheck;
    HttpConnectionManager(CurlHttpConnectionFactory *curlf, const std::string &certs, bool ncoSkipCertCheck, uint32_t ncoTimeoutSec, uint32_t ncoUploadTimeoutSec, HttpRequestMode hrm, uint32_t nThreads) :
        m_curlf(curlf), m_certs(certs), m_ncoTimeoutSec(ncoTimeoutSec), m_ncoUploadTimeoutSec(ncoUploadTimeoutSec), m_hrm(hrm), m_nThreads(nThreads), m_ncoSkipCertCheck(ncoSkipCertCheck) {}
    virtual ~HttpConnectionManager() {}
    virtual void worker(uint32_t nr) = 0;// { /*empty*/ }
    void startWorkers() {
        m_pool.reserve(m_nThreads);
        for (uint32_t i = 0; i < m_nThreads; ++i)
            m_pool.emplace_back([i, this]() { worker(i); });
    }
    void shutdown() {
        setThreadPoolSize(0);
    }
    void setUploadTimeout(uint64_t to) { std::lock_guard l(m_mutex); m_ncoUploadTimeoutSec = to; }
    void setTimeout(uint64_t to) { std::lock_guard l(m_mutex); m_ncoTimeoutSec = to; }
    void setThreadPoolSize(int val) {
        auto oldVal = m_nThreads;
        { std::lock_guard l(m_mutex); m_nThreads = val; }
        if (oldVal) {
            m_conditionVar.notify_all();
            m_pool.clear(); //should call join automatically
        } else while (oldVal < m_nThreads) {
            m_pool.emplace_back([this, oldVal]() { worker(oldVal); });
            ++oldVal;
        }
    }
};
struct GenericHttpConnectionManager : public HttpConnectionManager { //0x128 bytes
    using task_type = std::packaged_task<NetworkResponseBase(CurlHttpConnection *)>;
    std::queue<task_type> m_ptq;
    GenericHttpConnectionManager(CurlHttpConnectionFactory *curlf, const std::string &certs, bool ncoSkipCertCheck, uint32_t ncoTimeoutSec, uint32_t ncoUploadTimeoutSec, HttpRequestMode hrm) :
        HttpConnectionManager(curlf, certs, ncoSkipCertCheck, ncoTimeoutSec, ncoUploadTimeoutSec, hrm, 1) {}
    ~GenericHttpConnectionManager() { shutdown(); }
    void worker(uint32_t myThreadNo) override {
        auto conn = m_curlf->instance(m_certs, m_ncoSkipCertCheck, true, m_ncoTimeoutSec, m_ncoUploadTimeoutSec, HRM_SEQUENTIAL);
        while (myThreadNo < m_nThreads) {
            task_type task;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_conditionVar.wait(lock, [this, myThreadNo] { return myThreadNo >= this->m_nThreads || !this->m_ptq.empty(); });
                if (myThreadNo >= m_nThreads)
                    break;
                task = std::move(m_ptq.front());
                m_ptq.pop();
            }
            task(conn.get());
        }
        conn.reset();
        OPENSSL_thread_stop();
    }
    template<class T>
    std::future<NetworkResponse<T>> pushRequestTask(const std::function<NetworkResponse<T>(CurlHttpConnection *)> &f) {
        if (m_nThreads == 0)
            startWorkers();
        auto specific_task = std::make_shared<std::promise<NetworkResponse<T>(CurlHttpConnection *)>>();
        auto res = specific_task->get_future();
        task_type stored_task([specific_task, f](CurlHttpConnection *conn) {
            auto ret = f(conn);
            specific_task->set_value(ret);
            return NetworkResponseBase(ret);
        });
        { std::lock_guard l(m_mutex); m_ptq.push(std::move(stored_task)); }
        m_conditionVar.notify_one();
        return res;
    }
/* TODO or OMIT:
pushComposableRequestTask<std::vector<zwift_network::model::WorkoutsFromPartner>,std::multiset<zwift_network::model::Workout>>(RequestTaskComposer<std::vector<zwift_network::model::WorkoutsFromPartner>,std::multiset<zwift_network::model::Workout>>::Composable,std::function<std::shared_ptr<zwift_network::NetworkResponse<std::multiset<zwift_network::model::Workout>> const> ()(HttpConnection &)> const&)*/
};
struct UdpClient;
struct EventLoop;
struct EncryptionInfo {
    std::string m_sk;
    uint32_t m_relaySessionId = 0, m_expiration = 0;
};
template<class T>
struct ServiceListeners {
    moodycamel::ReaderWriterQueue<T *> m_rwqAdded, m_rwqRemoved;
    ServiceListeners() : m_rwqAdded(1), m_rwqRemoved(1) {}
    void flushPendingQueues() {
        //не стал разгребать, что там у них - сделал по-своему
        T *added = nullptr, *removed = nullptr;
        while (m_rwqAdded.try_dequeue(added));
        while (m_rwqRemoved.try_dequeue(removed))
            if (removed == added)
                added = nullptr;
        if (added)
            m_rwqAdded.enqueue(added);
    }
    void notify(std::function<void(T &)> func) {
        flushPendingQueues();
        T **lic = m_rwqAdded.peek();
        if (lic && *lic)
            func(**lic);
    }
    ServiceListeners &operator +=(T *obj) { m_rwqAdded.enqueue(obj); return *this; }
    ServiceListeners &operator -=(T *obj) { m_rwqRemoved.enqueue(obj); return *this; }
};
struct UdpConfigListener {
    virtual void handleUdpConfigChange(const protobuf::UdpConfigVOD &uc, uint64_t a3) = 0;
};
struct EncryptionListener {
    virtual void handleEncryptionChange(const EncryptionInfo &ei) = 0;
};
struct WorldIdListener {
    virtual void handleWorldIdChange(int64_t worldId) = 0;
};
struct WorldAttributeServiceListener {
    virtual void handleWorldAttribute(const protobuf::WorldAttribute &wa) = 0;
};
struct GlobalState { //0x530 bytes
    ServiceListeners<EncryptionListener> m_encLis;
    ServiceListeners<WorldIdListener> m_widLis;
    ServiceListeners<UdpConfigListener> m_ucLis;
    EventLoop *m_evloop;
    protobuf::PerSessionInfo m_psi;
    std::string m_sessionInfo;
    EncryptionInfo m_ei;
    uint64_t m_worldId = 0, m_playerId = 0, m_time;
    std::mutex m_mutex;
    bool m_shouldUseEncryption;
    GlobalState(EventLoop *, const protobuf::PerSessionInfo &, const std::string &, const EncryptionInfo &);
    bool shouldUseEncryption() const { return m_shouldUseEncryption; }
    void registerUdpConfigListener(UdpConfigListener *lis);
    void registerEncryptionListener(EncryptionListener *lis);
    void registerWorldIdListener(WorldIdListener *lis);
    std::string getSessionInfo() { return m_sessionInfo; }
    const protobuf::PerSessionInfo &getPerSessionInfo() { return m_psi; }
    uint64_t getWorldId() { return m_worldId; }
    void setWorldId(int64_t worldId);
    bool isInWorld() { return m_worldId != 0; }
    uint64_t getPlayerId() { return m_playerId; }
    void setPlayerId(uint64_t newVal) { m_playerId = newVal; }
    EncryptionInfo getEncryptionInfo() { std::lock_guard l(m_mutex); return m_ei; }
    void setEncryptionInfo(const EncryptionInfo &ei);
    void setUdpConfig(const protobuf::UdpConfigVOD &a2, uint64_t a3);
};
static const int B64index[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6,
    7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,
    0, 0, 0, 63, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };
static const char indexB64[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3',
'4', '5', '6', '7', '8', '9', '+', '/' };
static const int modB64[] = { 0, 2, 1 };
namespace base64 { //IDA: Base64Url, base64::
    std::vector<uint8_t> toBin(const std::string &src, int offset = 0, int len = -1) {
        std::vector<uint8_t> ret;
        const uint8_t *p = (uint8_t *)src.c_str() + offset;
        if (len < 0)
            len = int(src.length());
        int pad = len > 0 && (len % 4 || p[len - 1] == '=');
        const size_t L = ((len + 3) / 4 - pad) * 4;
        ret.resize(0);
        ret.reserve(L / 4 * 3 + pad);
        for (size_t i = 0; i < L; i += 4) {
            int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
            ret.push_back(n >> 16);
            ret.push_back(n >> 8 & 0xFF);
            ret.push_back(n & 0xFF);
        }
        if (pad) {
            int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
            ret.push_back(n >> 16);
            if (len > L + 2 && p[L + 2] != '=') {
                n |= B64index[p[L + 2]] << 6;
                ret.push_back(n >> 8 & 0xFF);
            }
        }
        return ret;
    }
    protobuf_bytes toProtobufBytes(const std::string &src, int offset = 0, int len = -1) {
        protobuf_bytes ret;
        const uint8_t *p = (uint8_t *)src.c_str() + offset;
        if (len < 0)
            len = int(src.length());
        int pad = len > 0 && (len % 4 || p[len - 1] == '=');
        const size_t L = ((len + 3) / 4 - pad) * 4;
        ret.resize(0);
        ret.reserve(L / 4 * 3 + pad);
        for (size_t i = 0; i < L; i += 4) {
            int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
            ret += char(n >> 16);
            ret += char(n >> 8 & 0xFF);
            ret += char(n & 0xFF);
        }
        if (pad) {
            int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
            ret += char(n >> 16);
            if (len > L + 2 && p[L + 2] != '=') {
                n |= B64index[p[L + 2]] << 6;
                ret += char(n >> 8 & 0xFF);
            }
        }
        return ret;
    }
    //void encode(const std::string &src) { encode((const uint8_t *)src.c_str(), src.length()); }
    std::string toString(const uint8_t *src, size_t len) {
        std::string ret;
        auto output_length = 4 * ((len + 2) / 3);
        ret.resize(output_length);
        auto dest = ret.data();
        for (int i = 0, j = 0; i < len;) {
            uint32_t octet_a = i < len ? src[i++] : 0;
            uint32_t octet_b = i < len ? src[i++] : 0;
            uint32_t octet_c = i < len ? src[i++] : 0;
            uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
            dest[j++] = indexB64[(triple >> 3 * 6) & 0x3F];
            dest[j++] = indexB64[(triple >> 2 * 6) & 0x3F];
            dest[j++] = indexB64[(triple >> 1 * 6) & 0x3F];
            dest[j++] = indexB64[(triple >> 0 * 6) & 0x3F];
        }
        for (int i = 0; i < modB64[len % 3]; i++)
            dest[output_length - 1 - i] = '=';
        return ret;
    }
    template<class T>
    std::string toString(const std::vector<T> &src) {
        return toString((const uint8_t *)src.data(), src.size());
    }
    template<typename T, size_t N>
    std::string toString(const T (&src)[N]) {
        return toString((const uint8_t *)&src[0], N);
    }
    std::string toString(const protobuf_bytes &src) {
        return toString((const uint8_t *)src.data(), src.length());
    }
};
namespace HttpHelper {
    static NetworkResponse<Json::Value> parseJsonStr(const std::string &src) {
        NetworkResponse<Json::Value> ret;
        Json::Reader r;
        if (!r.parse(src, ret, false)) {
            NetworkingLogError("Error parsing JSON: %s\nJSON: %s", r.getFormattedErrorMessages().c_str(), src.c_str());
            ret.storeError(NRO_JSON_PARSING_ERROR, "Error parsing json"s);
        }
        return ret;
    }
    template<class T>
    static NetworkResponse<Json::Value> parseJson(const std::vector<T> &src) {
        NetworkResponse<Json::Value> ret;
        Json::Reader r;
        if (src.size() == 0 || !r.parse((const char *)&src.front(), (const char *)&src.back() + 1, ret, false)) {
            NetworkingLogError("Error parsing JSON: %s\nJSON: %s", r.getFormattedErrorMessages().c_str(), src.size() ? (const char *)&src.front() : "empty");
            ret.storeError(NRO_JSON_PARSING_ERROR, "Error parsing json"s);
        }
        return ret;
    }
    static std::string sanitizeUrl(const std::string &url) {
        std::string ret;
        if (url.size()) {
            if (url.back() == '/') {
                ret.assign(url, 0, url.size() - 1);
            } else {
                ret = url;
            }
        }
        return ret;
    }
    void protobufToCharVector(std::vector<char> *dest, const google::protobuf::MessageLite &src) {
        auto size = src.ByteSizeLong();
        dest->resize(size);
        if (!src.SerializeToArray(dest->data(), int(size)))
            LogDebug("Failed to encode protobuf.");
    }
    template <class RET>
    static NetworkResponse<RET> convertToResultResponse(const QueryResult &queryResult) {
        NetworkResponse<RET> ret;
        if (queryResult.m_errCode) {
            protobuf::ZErrorMessageProtobuf v40;
            ret.m_errCode = queryResult.m_errCode;
            if (v40.ParseFromString(queryResult.m_msg) && v40.message().length())
                ret.m_msg = v40.message();
            else
                ret.m_msg = queryResult.m_msg;
        } else {
            if (!ret.m_T.ParseFromArray(queryResult.m_T.data(), int(queryResult.m_T.size())))
                ret.storeError(NRO_PROTOBUF_FAILURE_TO_DECODE, "Failed to decode protobuf"s);
        }
        return ret;
    }
    template <class RET>
    static NetworkResponse<RET> convertToResultResponse(const QueryResult &queryResult, const std::function<RET(const Json::Value &)> &f) {
        NetworkResponse<RET> ret;
        if (queryResult.m_errCode) {
            protobuf::ZErrorMessageProtobuf v40;
            ret.m_errCode = queryResult.m_errCode;
            if (v40.ParseFromString(queryResult.m_msg) && v40.message().length())
                ret.m_msg = v40.message();
            else
                ret.m_msg = queryResult.m_msg;
        } else {
            auto rx_json = parseJson(queryResult.m_T);
            if (rx_json.ok(&ret))
                ret.m_T = f(rx_json.m_T);
        }
        return ret;
    }
    static NetworkResponse<Json::Value> convertToJsonResponse(const QueryResult &src) {
        NetworkResponse<Json::Value> ret;
        return src.ok(&ret) ? parseJson(src.m_T) : ret;
    }
    static NetworkResponse<std::string> convertToStringResponse(const NetworkResponse<std::vector<char>> &src) {
        NetworkResponse<std::string> ret;
        if (src.ok(&ret))
            ret.m_T.assign(src.m_T.begin(), src.m_T.end());
        return ret;
    }
    static NetworkResponse<int64_t> convertToLongResponse(const QueryResult &queryResult, int64_t lng) {
        NetworkResponse<int64_t> ret;
        if (queryResult.ok(&ret))
            ret.m_T = lng;
        return ret;
    }
    template<class T>
    static NetworkResponse<void> convertToVoidResponse(const NetworkResponse<T> &src) {
        return NetworkResponse<void>{src.m_msg, src.m_errCode};
    }
    static std::string jsonToString(const Json::Value &json) {
        return Json::FastWriter().write(json);
    }
};
struct JsonWebToken : public NetworkResponseBase { //0x68 bytes
    std::string m_sub, m_sessionState, m_base64;
    int64_t m_exp = 0;
    const std::string &asString() const { return m_base64; }
    const std::string &getSessionState() const { return m_sessionState; }
    const std::string &getSubject() const { return m_sub; }
    bool parsePayload(const std::vector<uint8_t> &payload) {
        auto pr = HttpHelper::parseJson(payload);
        if (pr.ok(this)) {
            m_exp = 10'000'000ull * pr.m_T["exp"].asInt();
            m_sub = pr.m_T["sub"].asString();
            m_sessionState = pr.m_T["session_state"].asString();
            return true;
        }
        return false;
    }
    bool parse(const std::string &jwt) {
        auto firstSep = jwt.find('.');
        if (firstSep == -1) {
            storeError(NRO_JSON_WEB_TOKEN_PARSING_ERROR, "First separator not found"s);
            return false;
        }
        auto secondSep = jwt.find('.', firstSep + 1);
        if (secondSep == -1) {
            storeError(NRO_JSON_WEB_TOKEN_PARSING_ERROR, "Second separator not found"s);
            return false;
        }
        if (parsePayload(base64::toBin(jwt, int(firstSep + 1), int(secondSep - firstSep - 1)))) {
            m_base64 = jwt;
            return true;
        }
        return false;
    }
    bool parseOk(const std::string &jwt, NetworkResponseBase *dest) { parse(jwt); return ok(dest); }
};
struct Oauth2Credentials : public NetworkResponseBase {
    const std::string &asString() const { return m_base64; }
    JsonWebToken m_acToken, m_rfToken;
    std::string m_json, m_base64;
    time_t m_exp = 0;
    const JsonWebToken &getAccessToken() const { return m_acToken; }
    uint64_t getAccessTokenExpiresIn() const { return m_exp; }
    const JsonWebToken &getRefreshToken() const { return m_rfToken; }
    bool parse(const std::string &src) {
        auto pr = HttpHelper::parseJsonStr(src);
        bool ret = false;
        if (pr.ok(this)) {
            auto at = pr.m_T["access_token"].asString();
            if (m_acToken.parseOk(at, this)) {
                auto rt = pr.m_T["refresh_token"].asString();
                if (m_rfToken.parseOk(rt, this)) {
                    m_exp = pr.m_T["expires_in"].asInt();
                    m_json = src;
                    m_base64 = src;
                    ret = true;
                }
            }
        }
        return ret;
    }
    bool parseOk(const std::string &jwt, NetworkResponseBase *dest) { parse(jwt); return ok(dest); }
};
struct ZwiftAuthenticationManager : public NetworkResponseBase { //0x118 bytes, many virtual functions
    std::string m_apiUrl, m_accessToken, m_mail, m_password, m_oauthClient, m_authUrl;
    uint64_t m_accessTokenDeathTime = 0, m_reqId = 0, m_ratNextTime = 0;
    Oauth2Credentials m_oauth2;
    int m_throttlingLevel = 0;
    bool m_accessTokenInvalid = true, m_loggedIn = false; //all other data also 0
    ZwiftAuthenticationManager(const std::string &server) { m_apiUrl = server + "/api/auth"; }
    ~ZwiftAuthenticationManager() { //vptr[0]
        //destroy() inlined
        //looks like field_40 is shared ptr too - destructed; also pure base class (AuthenticationManager) dtr called
    }
    bool isAccessTokenInvalidOrExpired() const { return isAccessTokenInvalid() || isAccessTokenExpired(); } //vptr[1]
    const std::string &getAccessTokenHeader() const { return m_accessToken; } //vptr[2]
    const std::string &getRefreshTokenStr() const { return m_oauth2.getRefreshToken().asString(); } //vptr[3]
    const JsonWebToken &getRefreshToken() const { return m_oauth2.getRefreshToken(); } //vptr[4]
    const std::string &getSessionStateFromToken() const { return m_oauth2.getAccessToken().getSessionState(); } //vptr[5]
    const std::string &getSubjectFromToken() const { return m_oauth2.getAccessToken().getSubject(); } //vptr[6]
    const std::string &getOauthClient() const { return m_oauthClient; } //vptr[7]
    void setLoggedIn(bool val) { m_loggedIn = val; } //vptr[8]
    bool getLoggedIn() const { return m_loggedIn; } //vptr[9]
    const NetworkResponseBase &attendToAccessToken(CurlHttpConnection *conn) { //vptr[10]
        auto n = g_steadyClock.now();
        if (n >= m_ratNextTime) {
            m_ratNextTime = n;
            auto v8 = 60'000'000'000i64;
            if (m_throttlingLevel < 32) {
                auto v7 = (uint32_t)(1u << m_throttlingLevel);
                if (v7 > 60)
                    v7 = 60;
                v8 = 1'000'000'000 * v7;
            }
            m_ratNextTime += v8;
            m_throttlingLevel++;
            if (!m_accessToken.empty()) {
                if (n < m_accessTokenDeathTime) {
                    m_errCode = NRO_OK;
                    m_msg.clear();
                } else {
                    refreshAccessToken(conn);
                    return (m_errCode) ? acquireAccessToken(conn) : *this;
                }
            } else {
                return acquireAccessToken(conn);
            }
        } else {
            m_errCode = NRO_HTTP_STATUS_TOO_MANY_REQUESTS;
            m_msg = "Could not acquire access token due to throttling"s;
        }
        return *this;
    }
    void setRequestId(uint64_t id) { m_reqId = id; } //vptr[11]
    uint64_t getRequestId() { return m_reqId; } //vptr[12]
    void setAccessTokenAsExpired() { m_accessTokenDeathTime = 0; } //vptr[13]
    bool resetCredentials() { //vptr[14]
        m_accessToken.clear();
        m_accessTokenDeathTime = 0;
        m_accessTokenInvalid = true;
        m_throttlingLevel = 0;
        m_ratNextTime = 0;
        m_mail.clear();
        m_password.clear();
        m_oauthClient.clear();
        bool ret = m_loggedIn;
        m_loggedIn = false;
        m_errCode = NRO_OK;
        return ret;
    }
    const NetworkResponseBase &setCredentials(const std::string &sOauth, const std::string &mail, const std::string &pwd, const std::string &oauthClient) { //vptr[15]
        if (m_oauth2.parseOk(sOauth, this)) {
            setTokens(false);
            setEmailAndPassword(mail, pwd);
            m_oauthClient = oauthClient;
            m_loggedIn = false;
        }
        return m_oauth2;
    }
    void setCredentialsOld(const std::string &mail, const std::string &pwd, const std::string &oauthClient) { //vptr[16]
        m_accessToken.clear();
        m_accessTokenDeathTime = 0;
        m_accessTokenInvalid = true;
        m_throttlingLevel = 0;
        m_ratNextTime = 0;
        setEmailAndPassword(mail, pwd);
        m_oauthClient = oauthClient;
        m_loggedIn = false;
    }
    const NetworkResponseBase &setCredentialsMid(const std::string &sOauth, const std::string &oauthClient) { //vptr[17], last
        if (m_oauth2.parseOk(sOauth, this)) {
            setTokens(false);
            m_mail.clear();
            m_password.clear();
            m_oauthClient = oauthClient;
            m_loggedIn = false;
        }
        return m_oauth2;
    }
    void setEmailAndPassword(const std::string &mail, const std::string &pwd) {
        m_mail = mail;
        str_tolower(m_mail);
        m_password = pwd;
    }
    void setTokens(bool refresh) {
        m_accessToken = "Authorization: Bearer " + m_oauth2.m_acToken.asString();
        if (refresh) {
            m_accessTokenDeathTime = g_steadyClock.now() + 1'000'000'000ull * m_oauth2.m_exp - 30'000'000'000ull;
        } else {
            m_accessTokenDeathTime = 0;
        }
        m_accessTokenInvalid = false;
        m_throttlingLevel = 0;
        m_ratNextTime = 0;
    }
    const NetworkResponseBase &acquireAccessToken(CurlHttpConnection *conn) {
        QueryStringBuilder qsb;
        if (!m_mail.empty()) {
            if (m_authUrl.empty() && !findAuthenticationServer(conn))
                return m_oauth2;
            qsb.add("grant_type"s, "password"s);
            qsb.add("client_id"s, conn->escapeUrl(m_oauthClient));
            qsb.add("username"s, conn->escapeUrl(m_mail));
            qsb.add("password"s, conn->escapeUrl(m_password));
            conn->setRequestId(++m_reqId);
            auto resp = conn->performPost(m_authUrl, ContentTypeHeader(CTH_URLENC), qsb.getString(false), AcceptHeader(ATH_JSON),
                "Acquire Access Token"s, false);
            parseOauth2Credentials(HttpHelper::convertToStringResponse(resp));
            if (resp.m_errCode != 401 && resp.m_errCode != 403) {
                if (!m_oauth2.m_errCode)
                    setTokens(true);
                else
                    NetworkingLogError("Error acquiring access token: [%d] %s", m_oauth2.m_errCode, m_oauth2.m_msg.c_str());
            } else {
                NetworkingLogError("Error acquiring access token: [%d] %s", m_oauth2.m_errCode, m_oauth2.m_msg.c_str());
                m_accessToken.clear();
                m_accessTokenDeathTime = 0;
                m_accessTokenInvalid = true;
                m_throttlingLevel = 0;
                m_ratNextTime = 0;
                m_mail.clear();
                m_password.clear();
                m_loggedIn = false;
                m_oauth2.m_errCode = NRO_HTTP_STATUS_UNAUTHORIZED;
            }
            return m_oauth2;
        }
        return m_oauth2.storeError(NRO_NO_CREDENTIALS_TO_REQUEST_ACCESS_TOKEN_WITH, 
            "Could not acquire access token because credentials are missing"s);
    }
    const NetworkResponseBase &refreshAccessToken(CurlHttpConnection *conn) {
        if (m_accessTokenInvalid || _Xtime_get_ticks() >= getRefreshToken().m_exp)
            return m_oauth2.storeError(NRO_REFRESH_TOKEN_EXPIRED, "Refresh token expired"s);
        if (m_authUrl.empty() && !findAuthenticationServer(conn))
            return m_oauth2;
        QueryStringBuilder qsb;
        qsb.add("grant_type"s, "refresh_token"s);
        qsb.add("refresh_token"s, conn->escapeUrl(getRefreshToken().asString()));
        qsb.add("client_id"s, conn->escapeUrl(m_oauthClient));
        conn->setRequestId(++m_reqId);
        auto resp = conn->performPost(m_authUrl, ContentTypeHeader(CTH_URLENC), qsb.getString(false), AcceptHeader(ATH_JSON),
            "Refresh Access Token"s, false);
        parseOauth2Credentials(HttpHelper::convertToStringResponse(resp));
        if (resp.m_errCode == NRO_HTTP_STATUS_BAD_REQUEST || resp.m_errCode == NRO_HTTP_STATUS_UNAUTHORIZED || resp.m_errCode == NRO_HTTP_STATUS_FORBIDDEN) {
            NetworkingLogError("Error refreshing access token: [%d] %s", m_oauth2.m_errCode, m_oauth2.m_msg.c_str());
            m_accessToken.clear();
            m_accessTokenDeathTime = 0;
            m_accessTokenInvalid = true;
            m_throttlingLevel = 0;
            m_ratNextTime = 0;
            m_oauth2.m_errCode = NRO_HTTP_STATUS_UNAUTHORIZED;
        } else if (resp.m_errCode) {
            NetworkingLogError("Error refreshing access token: [%d] %s", m_oauth2.m_errCode, m_oauth2.m_msg.c_str());
        } else {
            setTokens(true);
        }
        return m_oauth2;
    }
    bool findAuthenticationServer(CurlHttpConnection *conn) {
        conn->setRequestId(++m_reqId);
        m_oauth2.storeError(NRO_OK, nullptr);
        auto v6 = conn->performGet(m_apiUrl, AcceptHeader(ATH_JSON), "Find Authentication Server"s);
        auto json = HttpHelper::convertToJsonResponse(v6);
        if (json.m_errCode) {
            NetworkingLogError("Error finding authorization server: [%d] %s", json.m_errCode, json.m_msg.c_str());
            m_oauth2.storeError(json);
            return false;
        } else {
            auto v15 = json.m_T["realm"s];
            auto v17 = json.m_T["url"s];
            m_authUrl = HttpHelper::sanitizeUrl(v17.asString()) + "/realms/"s + v15.asString() + "/protocol/openid-connect/token"s;
            return true;
        }
    }
    std::string getOauth2CredentialsString() const { return m_oauth2.asString(); }
    bool isAccessTokenExpired() const { return int64_t(g_steadyClock.now() - m_accessTokenDeathTime) > 0; }
    bool isAccessTokenInvalid() const { return getRefreshTokenStr().empty(); }
    bool isLoggedIn() const { return m_loggedIn; }
    void parseOauth2Credentials(const NetworkResponse<std::string> &src) {
        if (src.m_errCode)
            m_oauth2.storeError(src);
        else
            m_oauth2.parse(src.m_T);
    }
};
struct ZwiftHttpConnectionManager : public HttpConnectionManager { //0x160 bytes
    using task_t = std::packaged_task<NetworkResponseBase(CurlHttpConnection *, bool)>;
    struct RequestTaskContext : public task_t {
        using task_t::task_t;
        bool m_secured = false;
    };
    std::queue<RequestTaskContext> m_rtq;
    GlobalState *m_gs = nullptr;
    ZwiftAuthenticationManager *m_authMgr = nullptr;
    volatile int64_t m_requestId = 1;
    bool m_needNewAcToken = false, *m_tcpDisconnected;
    void setGlobalState(GlobalState *gs) { m_gs = gs; }
    ZwiftHttpConnectionManager(CurlHttpConnectionFactory *curlf, const std::string &certs, bool ncoSkipCertCheck, ZwiftAuthenticationManager *am, bool *tcpDisconnected, int ncoTimeoutSec, int ncoUploadTimeoutSec, HttpRequestMode rm, int nThreads) :
        HttpConnectionManager(curlf, certs, ncoSkipCertCheck, ncoTimeoutSec, ncoUploadTimeoutSec, rm, nThreads), m_authMgr(am), m_tcpDisconnected(tcpDisconnected) {
        startWorkers();
    }
    const NetworkResponseBase &attendToAccessToken(CurlHttpConnection *conn) {
        conn->clearAuthorizationHeader();
        conn->clearTokenInfo();
        m_authMgr->setRequestId(m_requestId);
        auto &ret = m_authMgr->attendToAccessToken(conn);
        InterlockedExchange64(&m_requestId, m_authMgr->getRequestId());
        m_needNewAcToken = false;
        m_conditionVar.notify_all();
        return ret;
    }
    void worker(uint32_t myThreadNo) override {
        auto conn = m_curlf->instance(m_certs, m_ncoSkipCertCheck, false, m_ncoTimeoutSec, m_ncoUploadTimeoutSec, m_hrm);
        while (myThreadNo < m_nThreads) {
            RequestTaskContext task;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_conditionVar.wait(lock, [this, myThreadNo] { return myThreadNo >= this->m_nThreads || (!this->m_needNewAcToken && !this->m_rtq.empty()); });
                if (myThreadNo >= this->m_nThreads)
                    break;
                task = std::move(m_rtq.front());
                m_rtq.pop();
            }
            conn->clearAuthorizationHeader();
            conn->clearTokenInfo();
            bool needNewAcToken = false;
            if (task.m_secured) {
                if (m_authMgr->isAccessTokenInvalidOrExpired()) {
                    m_needNewAcToken = true;
                    needNewAcToken = true;
                } else {
                    conn->setAuthorizationHeader(m_authMgr->getAccessTokenHeader());
                    conn->setTokenInfo(m_authMgr->getSessionStateFromToken(), m_authMgr->getSubjectFromToken());
                }
            }
#if 0 //OMIT
            v17 = Stopwatch::elapsed(&a1->m_rtq.Map[a1->m_rtq.Myoff & (a1->m_rtq.Mapsize - 1)][1].field_0[7], &v49);
            HttpStatistics::addWaitingTime(v6, *v17);
            std::numpunct<unsigned short>::_Init(p_Ptr);
#endif
            conn->setTimeout(m_ncoTimeoutSec);
            conn->setUploadTimeout(m_ncoUploadTimeoutSec);
            if (m_gs)
                conn->setSessionIdHeader(m_gs->getSessionInfo());
            else
                conn->setSessionIdHeader("");
            task(conn.get(), needNewAcToken);
        }
        conn.reset();
        OPENSSL_thread_stop();
    }
    ~ZwiftHttpConnectionManager() { shutdown(); }
    template<class T>
    std::future<NetworkResponse<T>> pushRequestTask(const std::function<NetworkResponse<T>(CurlHttpConnection *)> &f, bool secured, bool simultOk) {
        if (simultOk || !*m_tcpDisconnected) {
            auto specific_task = std::make_shared<std::promise<NetworkResponse<T>>>();
            auto res = specific_task->get_future();
            RequestTaskContext stored_task([specific_task, this, f](CurlHttpConnection *conn, bool needNewAcToken) {
                if (needNewAcToken) {
                    auto rett = this->attendToAccessToken(conn);
                    if (rett.m_errCode)
                        return NetworkResponseBase(std::move(rett));
                    conn->setAuthorizationHeader(this->m_authMgr->getAccessTokenHeader());
                    conn->setTokenInfo(this->m_authMgr->getSessionStateFromToken(), this->m_authMgr->getSubjectFromToken());
                }
                conn->setRequestId(InterlockedExchangeAdd64(&this->m_requestId, 1));
                auto res = f(conn);
                specific_task->set_value(res);
                if (res.m_errCode == 401) {
                    NetworkingLogWarn("Request status unauthorized, token invalidated.");
                    this->m_authMgr->setAccessTokenAsExpired();
                }
                return NetworkResponseBase(res);
            });
            stored_task.m_secured = secured; //TODO: check (not sure)
            { std::lock_guard l(m_mutex); m_rtq.push(std::move(stored_task)); }
            m_conditionVar.notify_one();
            return res;
        } else {
            return makeNetworkResponseFuture<T>(NRO_DISCONNECTED_DUE_TO_SIMULTANEOUS_LOGINS, "Disconnected due to simultaneous log ins"s);
        }
    }
    //TODO or OMIT template<class T1, T2> //OMIT T1=model::WorkoutsFromPartner, T2=model::Workout
    //std::future<?> pushComposableRequestTask<std::vector<T1>, std::multiset<T2>>(
    //    RequestTaskComposer<std::vector<T1>, std::multiset<T2>>::Composable,
    //    std::function<NetworkResponse<std::multiset<T1>>(HttpConnection &)> &, bool, bool)
};
struct EncryptionOptions {
    bool m_disableEncr, m_disableEncrWithServer, m_ignoreEncrFeatureFlag;
    const std::string &m_secretKeyBase64;
};
struct ExperimentsRestInvoker;
namespace protocol_encryption {
struct iv {
    enum class DeviceType : uint16_t { None = 0, Relay = 1, Zc = 2 };
    enum class ChannelType : uint16_t { None = 0, UdpClient = 1, UdpServer = 2, TcpClient = 3, TcpServer = 4 };
    uint16_t m_res0 = 0, m_deviceType = 0, m_channelType = 0;
    uint16_t m_CI = 0;
    uint32_t m_netSEQ = 0;
};
struct Codec {
    uint8_t m_secretRaw[16] = {}, m_headerBuf[16] = {};
    std::string m_secret;
    enum Direction { ENC, DEC, DCNT };
    EVP_CIPHER_CTX *m_ciphers[DCNT] = {};
    std::mutex m_mutex;
    std::vector<uint8_t> m_tmp[DCNT];
    iv m_ivs[DCNT];
    uint32_t m_netSeqNo = 0, m_hostRelayId = 0, m_hostSeqNo[DCNT] = {}, m_tag32[DCNT] = {};
    iv::DeviceType m_dt;
    iv::ChannelType m_cts[DCNT];
    volatile int16_t m_netConnId = 0;
    bool m_initOK = false, m_generateKey = true, m_initHasRelayId = false, m_hasRelayId = false, m_reset[DCNT] = {}, m_hasConnId[DCNT] = {}, m_hasSeqNo = false,
        m_vitalCI = false;
    Codec(iv::DeviceType dt, iv::ChannelType cts0, iv::ChannelType cts1) : m_dt(dt), m_cts{ cts0, cts1 } {}
    void freeCiphers() { for (auto c : m_ciphers) if (c) EVP_CIPHER_CTX_free(c); }
    ~Codec() { freeCiphers(); }
    bool fail(std::string &&errSrc, std::string *err) { err->assign(std::move(errSrc)); freeCiphers(); return false; }
    int failM1(std::string &&errSrc, std::string *err) { err->assign(std::move(errSrc)); freeCiphers(); return -1; }
    void secretRawToString() { m_secret = base64::toString(m_secretRaw); }
    bool initialize(std::string *err) {
        std::lock_guard l(m_mutex);
        if (!m_initOK) {
            for (auto &c : m_ciphers) {
                c = EVP_CIPHER_CTX_new();
                if (!c)
                    return fail("EVP_CIPHER_CTX_new"s, err);
            }
            auto cipher = EVP_aes_128_gcm();
            if (!cipher)
                return fail("cipher not found"s, err);
            if (!EVP_EncryptInit_ex(m_ciphers[ENC], cipher, nullptr, nullptr, nullptr))
                return fail("EVP_EncryptInit_ex cipher"s, err);
            if (!EVP_DecryptInit_ex(m_ciphers[DEC], cipher, nullptr, nullptr, nullptr))
                return fail("EVP_DecryptInit_ex cipher"s, err);
            for (auto c : m_ciphers) {
                if (!EVP_CIPHER_CTX_set_padding(c, 0))
                    return fail("EVP_CIPHER_CTX_set_padding"s, err);
                if (!EVP_CIPHER_CTX_ctrl(c, 0, 0, nullptr))
                    return fail("EVP_CIPHER_CTX_ctrl"s, err);
                if (!EVP_CIPHER_CTX_set_key_length(c, 16))
                    return fail("EVP_CIPHER_CTX_set_key_length"s, err);
            }
            if (m_generateKey) {
                EVP_CIPHER_CTX_rand_key(m_ciphers[ENC], m_secretRaw);
                secretRawToString();
            } else {
                if (m_secret.length()) {
                    auto bin = base64::toBin(m_secret);
                    if (bin.size() == sizeof(m_secretRaw))
                        memmove(m_secretRaw, bin.data(), sizeof(m_secretRaw));
                    else
                        return fail("decode key is greater than expected"s, err);
                } else {
                    secretRawToString();
                }
            }
            m_initHasRelayId = m_hasRelayId;
            m_ivs[ENC].m_deviceType = ntohs((uint16_t)m_dt);
            m_ivs[ENC].m_channelType = ntohs((uint16_t)m_cts[ENC]);
            m_ivs[ENC].m_CI = ntohs(m_netConnId);
            m_ivs[ENC].m_netSEQ = 0;
            m_ivs[DEC].m_deviceType = ntohs((uint16_t)m_dt);
            m_ivs[DEC].m_channelType = ntohs((uint16_t)m_cts[DEC]);
            m_ivs[DEC].m_CI = ntohs(m_netConnId);
            m_ivs[DEC].m_netSEQ = 0;
            m_initOK = true;
        }
        return true;
    }
    bool decode(const uint8_t *src, uint32_t len, std::vector<uint8_t> *dest, std::string *err) { /*vptr[2]*/
        if (!m_initOK && !initialize(err))
            return false;
        if (m_reset[DEC]) {
            m_ivs[DEC].m_CI = ntohs(m_netConnId);
            m_hostSeqNo[DEC]  = 0;
            m_reset[DEC] = false;
        }
        int hlen = decodeHeader(src, len, err);
        if (hlen < 0)
            return false;
        bool result = decrypt(&src[hlen], len - hlen, src, hlen, m_ivs[DEC], dest, err);
        if (result)
            ++m_hostSeqNo[DEC];
        return result;
    }
    int decodeHeader(const uint8_t *h, uint32_t len, std::string *err) {
        if (!len)
            return failM1("message too short"s, err);
        auto flags = *h;
        uint32_t retHlen = 1;
        if ((flags & 0xF0) == 0) {
            if (flags & 4) {
                retHlen = 5;
                if (ntohl(*(uint32_t *)(h + 1)) != m_hostRelayId)
                    return failM1("invalid relay id"s, err);
            }
            if (flags & 2) {
                if (len < retHlen)
                    return failM1("message too short"s, err);
                m_netConnId = ntohs(*(uint16_t *)&h[retHlen]);
                retHlen += 2;
                m_hasConnId[DEC] = true;
                m_reset[ENC] = true;
                m_hostSeqNo[DEC] = 0;
                m_ivs[DEC].m_CI = ntohs(m_netConnId);
            } else if (m_vitalCI && !m_hasConnId[DEC]) {
                return failM1("connection id not received yet"s, err);
            }
            if (flags & 1) {
                if (len >= retHlen) {
                    m_hostSeqNo[DEC] = ntohl(*(uint32_t *)&h[retHlen]);
                    retHlen += 4;
                } else {
                    return failM1("message too short"s, err);
                }
            }
            m_ivs[DEC].m_netSEQ = ntohl(m_hostSeqNo[DEC]);
            return retHlen;
        }
        return failM1("invalid protocol version"s, err);
    }
    bool encrypt(const uint8_t *src, int len, const uint8_t *header, int hlen, const iv &aiv, std::vector<uint8_t> *dest, std::string *err) {
        static_assert(sizeof(iv) == 12);
        int outl = EVP_CIPHER_CTX_block_size(this->m_ciphers[ENC]) + len, outlf = 0;
        m_tmp[ENC].resize(outl);
        if (EVP_EncryptInit_ex(m_ciphers[ENC], nullptr, nullptr, m_secretRaw, (const uint8_t *)&aiv))
            if (header && hlen && !EVP_EncryptUpdate(this->m_ciphers[ENC], nullptr, &outl, header, hlen))
                return fail("EVP_EncryptUpdate AAD"s, err);
            else if (EVP_EncryptUpdate(this->m_ciphers[ENC], &m_tmp[ENC][0], &outl, src, len))
                if (EVP_EncryptFinal_ex(this->m_ciphers[ENC], &m_tmp[ENC][0] + outl, &outlf))
                    if (!EVP_CIPHER_CTX_ctrl(this->m_ciphers[ENC], 0, 0, nullptr))
                        return fail("EVP_CIPHER_CTX_ctrl"s, err);
                    else
                        outlf += outl;
                else
                    return fail("EVP_EncryptFinal_ex"s, err);
            else
                return fail("EVP_EncryptUpdate"s, err);
        else
            return fail("EVP_EncryptInit_ex"s, err);
        dest->insert(dest->end(), m_tmp[ENC].begin(), m_tmp[ENC].begin() + outlf);
        dest->insert(dest->end(), (uint8_t *)&(m_tag32[ENC]), (uint8_t *)&(m_tag32[ENC]) + 4);
        return true;
    }
    bool decrypt(const uint8_t *src, int len, const uint8_t *header, int hlen, const iv &aiv, std::vector<uint8_t> *dest, std::string *err) {
        int outl = EVP_CIPHER_CTX_block_size(m_ciphers[DEC]) + len, outlf = 0;
        m_tmp[DEC].resize(outl);
        m_tag32[DEC] = *(uint32_t *)&src[len - 4];
        if (EVP_DecryptInit_ex(m_ciphers[DEC], nullptr, nullptr, m_secretRaw, (const uint8_t *)&aiv))
            if (header && hlen && !EVP_DecryptUpdate(m_ciphers[DEC], nullptr, &outl, header, hlen))
                return fail("EVP_DecryptUpdate AAD"s, err);
            else if (EVP_DecryptUpdate(m_ciphers[DEC], &m_tmp[DEC][0], &outl, src, len - 4))
                if (EVP_CIPHER_CTX_ctrl(m_ciphers[DEC], EVP_CTRL_AEAD_SET_TAG, 4, &m_tag32[DEC]))
                    if (!EVP_DecryptFinal_ex(m_ciphers[DEC], &m_tmp[DEC][0] + outl, &outlf))
                        return fail("EVP_DecryptFinal_ex"s, err);
                    else
                        outlf += outl;
                else
                    return fail("EVP_CIPHER_CTX_ctrl"s, err);
            else
                return fail("EVP_DecryptUpdate"s, err);
        else
            return fail("EVP_DecryptInit_ex"s, err);
        dest->insert(dest->end(), &m_tmp[DEC][0], &m_tmp[DEC][0] + outlf);
        return true;
    }
    bool encode(const uint8_t *src, int len, std::vector<uint8_t> *dest, std::string *err) { /*vptr[1]*/
        if (!m_initOK && !initialize(err))
            return false;
        if (m_reset[ENC]) {
            m_ivs[ENC].m_CI = ntohs(m_netConnId);
            m_hostSeqNo[ENC] = 0;
            m_reset[ENC] = false;
        }
        if (m_vitalCI && !m_hasConnId[DEC])
            return fail("connection id not defined"s, err);
        int hdrIdx = 1;
        m_headerBuf[ENC] = (m_hasRelayId ? 4 : 0) | (m_hasConnId ? 2 : 0) | (m_hasSeqNo ? 1 : 0);
        if (m_hasRelayId) {
            *(uint32_t *)(m_headerBuf + 1) = htonl(m_hostRelayId);
            hdrIdx = 5;
        }
        if (m_hasConnId) {
            *(uint16_t *)(m_headerBuf + hdrIdx) = htons(m_netConnId); //QUEST - why?
            hdrIdx += 2;
        }
        if (m_hasSeqNo) {
            *(uint32_t *)(m_headerBuf + hdrIdx) = htonl(m_hostSeqNo[ENC]);
            hdrIdx += 4;
        }
        m_ivs[ENC].m_netSEQ = ntohl(m_hostSeqNo[ENC]); //QUEST - why?
        dest->insert(dest->end(), m_headerBuf, m_headerBuf + hdrIdx);
        auto result = encrypt(src, len, m_headerBuf, hdrIdx, m_ivs[ENC], dest, err);
        if (result)
            ++m_hostSeqNo[ENC];
        return result;
    }
    bool newConnection(std::string *err) { /*vptr[3]*/
        if (m_initOK || initialize(err)) {
            _InterlockedIncrement16(&m_netConnId);
            m_hasConnId[ENC] = true;
            m_hasRelayId = m_initHasRelayId;
            m_reset[ENC] = true;
            m_reset[DEC] = true;
            return true;
        }
        return false;
    }
};
struct TcpRelayClientCodec : public Codec {
    TcpRelayClientCodec() : Codec(iv::DeviceType::Relay, iv::ChannelType::TcpClient, iv::ChannelType::TcpServer) {
        m_hasRelayId = true;
        m_hasConnId[ENC] = true; 
    }
    bool encode(const uint8_t *src, uint32_t len, std::vector<uint8_t> *dest, std::string *err) {
        if (!Codec::encode(src, len, dest, err))
            return false;
        m_hasRelayId = false;
        m_hasConnId[ENC] = false;
        return true;
    }
};
struct UdpRelayClientCodec : public Codec {
    UdpRelayClientCodec() : Codec(iv::DeviceType::Relay, iv::ChannelType::UdpClient, iv::ChannelType::UdpServer) {
        m_hasRelayId = true;
        m_hasSeqNo = true;
        m_hasConnId[ENC] = true;
    }
    bool decode(const uint8_t *src, uint32_t len, std::vector<uint8_t> *dest, std::string *err) {
        if (!m_initOK && !initialize(err))
            return false;
        if (m_reset[DEC]) {
            m_ivs[DEC].m_CI = ntohs(m_netConnId);
            m_hostSeqNo[DEC] = 0;
            m_reset[DEC] = false;
        }
        auto hlen = decodeHeader(src, len, err);
        if (hlen >= 0 && decrypt(&src[hlen], len - hlen, src, hlen, m_ivs[DEC], dest, err)) {
            ++m_hostSeqNo[DEC];
            m_hasRelayId = false;
            m_hasConnId[ENC] = false;
            return true;
        }
        return false;
    }
};
struct ZcClientCodec : public Codec {
    ZcClientCodec() : Codec(iv::DeviceType::Zc, iv::ChannelType::TcpClient, iv::ChannelType::TcpServer) {
        m_hasConnId[ENC] = true;
    }
    bool encode(const uint8_t *src, uint32_t len, std::vector<uint8_t> *dest, std::string *err) {
        bool result = Codec::encode(src, len, dest, err);
        if (result)
            m_hasConnId[ENC] = false;
        return result;
    }
};
}
struct ExperimentsRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_expUrl;
    ExperimentsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &expUrl) : m_mgr(mgr), m_expUrl(expUrl) {}
    std::future<NetworkResponse<protobuf::FeatureResponse>> getFeatureResponse(const protobuf::FeatureRequest &rq) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::FeatureResponse>(CurlHttpConnection *)>([this, rq](CurlHttpConnection *conn) {
            NetworkResponse<protobuf::FeatureResponse> ret;
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, rq);
            auto v9 = conn->performPost(this->m_expUrl + "/experimentation/v1/variant"s, ContentTypeHeader(CTH_PB), payload,
                AcceptHeader(ATH_PB), "Get Feature Response"s, false);
            return HttpHelper::convertToResultResponse<protobuf::FeatureResponse>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::FeatureResponse>> getFeatureResponseByMachineId(const protobuf::FeatureRequest &rq) { 
        /*TODO: used in PC>106405 and Android>=106405, need to fill one-byter FeatureRequest.f2 with some data */
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::FeatureResponse>(CurlHttpConnection *)>([this, rq](CurlHttpConnection *conn) {
            NetworkResponse<protobuf::FeatureResponse> ret;
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, rq);
            auto v9 = conn->performPost(this->m_expUrl + "/experimentation/v1/machine-id-variant"s, ContentTypeHeader(CTH_PB), payload,
                AcceptHeader(ATH_PB), "Get Feature Response By Machine ID"s, false);
            return HttpHelper::convertToResultResponse<protobuf::FeatureResponse>(v9);
        }), true, false);
    }
};
struct AuthServerRestInvoker { //0x60 bytes
    const std::string &m_machineId, &m_server;
    ZwiftAuthenticationManager *m_authMgr;
    ExperimentsRestInvoker *m_expRi;
    ZwiftHttpConnectionManager *m_conn;
    std::future<NetworkResponse<std::string>> logIn(const EncryptionOptions &encr, const std::vector<std::string> &anEventProps,
        const std::function<void(const protobuf::PerSessionInfo &, const std::string &, const EncryptionInfo &)> &func) {
        return m_conn->pushRequestTask(std::function<NetworkResponse<std::string>(CurlHttpConnection *)>([this, encr, anEventProps, func](CurlHttpConnection *conn) {
            auto sk = this->getSecretKey(encr);
            auto sid = uuid::generate_uuid_v4();
            conn->setSessionIdHeader(sid);
            auto ret = this->doLogIn(sk, anEventProps, conn);
            if (!ret.m_errCode) {
                if (ret.m_T.has_relay_session_id()) {
                    NetworkingLogDebug("Server has encryption enabled (relay id %u)", ret.m_T.relay_session_id());
                } else {
                    NetworkingLogDebug("Server has encryption disabled (no relay_session_id in login response)");
                    sk.clear();
                }
                m_authMgr->setLoggedIn(true);
                func(ret.m_T.info(), sid, {sk, ret.m_T.relay_session_id(), ret.m_T.expiration()});
                ret.m_msg = m_authMgr->getRefreshTokenStr();
            } else {
                this->m_authMgr->resetCredentials();
                NetworkingLogError("Couldn't obtain a session id from the server.");
            }
            return NetworkResponse<std::string>(ret);
        }), true, false);
    }
    AuthServerRestInvoker(const std::string &machineId, ZwiftAuthenticationManager *authMgr, ZwiftHttpConnectionManager *httpConnMgr3, ExperimentsRestInvoker *expRi, const std::string &server) : m_machineId(machineId), m_server(server), m_authMgr(authMgr), m_expRi(expRi), m_conn(httpConnMgr3) {}
    NetworkResponse<protobuf::LoginResponse> doLogIn(const protobuf_bytes &sk, const std::vector<std::string> &anEventProps, CurlHttpConnection *conn) {
        std::string LogInV2;
        auto url = m_server + "/api/users/login"s;
        protobuf::LoginRequest lr;
        lr.set_key(sk);
        for (int i = 0; i < anEventProps.size(); i += 2) {
            auto p = lr.mutable_properties()->add_props();
            p->set_name(anEventProps[i]);
            p->set_value(anEventProps[i + 1]);
        }
        auto pm = lr.mutable_properties()->add_props();
        pm->set_name("Machine Id"s);
        pm->set_value(m_machineId);
        ContentType ct;
        if (sk.empty()) {
            LogInV2 = "Log In"s;
            ct = CTH_PB;
        } else {
            LogInV2 = "Log In V2"s;
            ct = CTH_PBv2;
            lr.set_key(sk);
        }
        std::vector<char> payload;
        HttpHelper::protobufToCharVector(&payload, lr);
        auto v42 = conn->performPost(url, ContentTypeHeader(ct), payload, 
            AcceptHeader(ATH_PB), LogInV2, false);
        return HttpHelper::convertToResultResponse<protobuf::LoginResponse>(v42);
    }
    protobuf_bytes getSecretKey(const EncryptionOptions &eo) {
        if (!eo.m_disableEncrWithServer && shouldEnableEncryptionBasedOnFeatureFlag(eo)) {
            if (!eo.m_secretKeyBase64.empty()) {
                return base64::toProtobufBytes(eo.m_secretKeyBase64);
            } else {
                protocol_encryption::TcpRelayClientCodec codec;
                std::string err;
                if (codec.m_initOK || codec.initialize(&err))
                    return protobuf_bytes((char *)codec.m_secretRaw, _countof(codec.m_secretRaw));
                else
                    NetworkingLogError("Failed to get secret key: %s", err.c_str());
            }
        }
        return protobuf_bytes();
    }
    std::future<NetworkResponse<std::string>> logOut(const std::function<void()> &func) {
        return m_authMgr->getLoggedIn() ?
            m_conn->pushRequestTask(std::function<NetworkResponse<std::string>(CurlHttpConnection *)>([this, func](CurlHttpConnection *conn) {
                QueryStringBuilder qsb;
                std::string url(this->m_server + "/api/users/logout"s);
                qsb.add("grant_type"s, "refresh_token"s);
                qsb.add("refresh_token"s, conn->escapeUrl(this->m_authMgr->getRefreshTokenStr()));
                qsb.add("client_id"s, conn->escapeUrl(this->m_authMgr->getOauthClient()));
                auto ret = HttpHelper::convertToStringResponse(conn->performPost(url, ContentTypeHeader(CTH_URLENC), qsb.getString(false), AcceptHeader(), "Log Out"s, false));
                this->m_authMgr->resetCredentials();
                func();
                return ret;
            }), true, false) :
            makeNetworkResponseFuture<std::string>(NRO_NOT_LOGGED_IN, "Not logged in"s);
    }
    std::future<NetworkResponse<void>> resetPassword(const std::string &newPwd) { //not implemented in ZA for Windows, but we can try to implement
        return m_conn->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([this](CurlHttpConnection *conn) {
            QueryStringBuilder qsb;
            std::string url(this->m_server + "/api/users/reset-password-email"s);
            qsb.add("client_id"s, conn->escapeUrl(this->m_authMgr->getOauthClient()));
            auto ret = HttpHelper::convertToVoidResponse(conn->performPost(url, ContentTypeHeader(CTH_URLENC), qsb.getString(false), AcceptHeader(), "Reset Password"s, false));
            this->m_authMgr->resetCredentials();
            return ret;
        }), false, true);
    }
    bool shouldEnableEncryptionBasedOnFeatureFlag(const EncryptionOptions &eo) {
        if (!m_expRi)
            return false;
        if (eo.m_ignoreEncrFeatureFlag) {
            NetworkingLogDebug("Ignore Encryption feature flag and enable encryption");
            return true;
        }
        bool ret = false, disable_encryption_bypass = true;
        protobuf::FeatureRequest v49;
        v49.add_params()->set_value("game_1_26_2_data_encryption"s);
        v49.add_params()->set_value("game_1_27_0_disable_encryption_bypass"s);
        auto respFuture = m_expRi->getFeatureResponse(v49);
        auto resp = respFuture.get();
        if (resp.m_errCode != NRO_OK) {
            NetworkingLogDebug("Failed to check encryption feature flags: %d [%s]", resp.m_errCode, resp.m_msg.c_str());
        } else {
            for (auto &i : resp.m_T.variants()) {
                if (i.name() == "game_1_26_2_data_encryption"s) {
                    ret = i.value();
                    NetworkingLogDebug("game_1_26_2_data_encryption: %s", ret ? "true" : "false");
                } else if (i.name() == "game_1_27_0_disable_encryption_bypass"s) {
                    disable_encryption_bypass = i.value();
                    NetworkingLogDebug("game_1_27_0_disable_encryption_bypass: %s", disable_encryption_bypass ? "true" : "false");
                }
            }
        }
        if (eo.m_disableEncr && !disable_encryption_bypass) {
            NetworkingLogDebug("Encryption disabled by option");
            ret = false;
        }
        return ret;
    }
    ~AuthServerRestInvoker() {}
};
struct EventLoop { //0x30 bytes
    boost::asio::io_context m_asioCtx;
    std::thread m_thrd;
    static void Execute(EventLoop *t) { 
        auto w = boost::asio::make_work_guard(t->m_asioCtx);
        t->m_asioCtx.run(); 
    }
    EventLoop() : m_asioCtx(), m_thrd(Execute, this) {} //OMIT: get m_asioCtx->impl pointer and increment references count to it
    ~EventLoop() { shutdown(); }
    void shutdown() { 
        m_asioCtx.stop(); 
        if (m_thrd.joinable()) 
            m_thrd.join(); 
    }
    void enqueueShutdown() {
        boost::asio::post(m_asioCtx, [this]() { m_asioCtx.stop(); });
        if (m_thrd.joinable()) 
            m_thrd.join();
    }
    void post(std::function<void()> &&f) { 
        boost::asio::post(m_asioCtx, f);
    }
};
struct NetworkClockService;
struct WorldClockService { //0x2120 bytes
    NetworkClockService *m_ncs;
    int64_t m_twoLegsLatency = 0, m_field_40 = 0, m_calcWorldClockOffset[1024] = {};
    uint64_t m_idx = 0, m_stab = 2;
    volatile time_t m_worldTime = 0, m_worldClockOffset = 0;
    std::map<uint32_t, uint64_t> m_map38; //seq to send time
    bool m_stc_useF5 = true, m_bInit = false;
    WorldClockService(EventLoop *, NetworkClockService *ncs) : m_ncs(ncs) {
        //OMIT NumericStatisticsPeriodicLogger<int64_t> ctr
    }
    bool isInitialized() /*vptr[1]*/ { return m_bInit; }
    uint64_t getWorldTime() /*vptr[2]*/ {
        if (!this->m_bInit)
            return 0;
        auto steadyDelta = int64_t(g_steadyClock.nowInMilliseconds() - this->m_worldClockOffset);
        auto worldTime = int64_t(this->m_worldTime);
        auto stable = (steadyDelta == worldTime);
        if (steadyDelta > worldTime) {
            while (true) {
                auto exch = _InterlockedCompareExchange64(&m_worldTime, steadyDelta, worldTime);
                stable = worldTime == exch;
                worldTime = exch;
                if (stable)
                    return steadyDelta;
                if (steadyDelta <= worldTime) {
                    stable = (steadyDelta == worldTime);
                    break;
                }
            }
        }
        if (!stable)
            NetworkingLogDebug("Time tried to go back from %ldms to %ldms", worldTime, steadyDelta);
        return m_worldTime;
    }
    bool shouldSetClock() {
        if ((1ui64 << m_stab) & m_idx) {
            if (m_stab <= 9)
                m_stab++;
            return true;
        }
        return false;
    }
    void adjustClock(int64_t offset, int64_t oll) {
        m_twoLegsLatency = 2 * oll;
        if (m_bInit && oll >= 500) {
            /* OMIT stat *((_QWORD *)&arg + 1) = oll;
            m_stat = this->m_stat;
            *(_QWORD *)&arg = &this->m_stat;
            ret0();
            v6 = asio_mem(80i64);
            *((_QWORD *)v6 + 5) = sub_7FF76A532D00;
            *((_QWORD *)v6 + 4) = 0i64;
            *(_QWORD *)v6 = 0i64;
            *((_QWORD *)v6 + 1) = 0i64;
            *((_QWORD *)v6 + 2) = 0i64;
            *((_QWORD *)v6 + 3) = 0i64;
            *((_DWORD *)v6 + 12) = 0;
            *(_OWORD *)(v6 + 56) = arg;
            asio::detail::scheduler::post_immediate_completion(m_stat[2], (struct _OVERLAPPED *)v6);*/
        } else {
            if (m_idx >= _countof(m_calcWorldClockOffset))
                m_idx = 0;
            m_calcWorldClockOffset[m_idx++] = offset;
            if (shouldSetClock()) {
                auto v10 = m_idx >> 1;
                //QUEST: not sure this method is fastest
                std::nth_element(m_calcWorldClockOffset, m_calcWorldClockOffset + v10, m_calcWorldClockOffset + m_idx);
                auto worldClockOffsetNew = m_calcWorldClockOffset[v10];
                if (worldClockOffsetNew) {
                    auto worldClockOffsetOld = this->m_worldClockOffset;
                    _InterlockedExchange64(&m_worldClockOffset, worldClockOffsetNew);
                    m_bInit = true;
                    if (worldClockOffsetOld) {
                        NetworkingLogInfo("World clock offset updated from %+ldms to %+ldms (%+ldms)", worldClockOffsetOld, worldClockOffsetNew,
                            worldClockOffsetNew - worldClockOffsetOld);
                        //OMIT WorldClockStatistics::registerClockDrift((_Mtx_t)this->m_ncs, worldClockOffsetNew - worldClockOffsetOld);
                    }
                }
            }
        }
    }
    bool calculateOneLegLatency(const protobuf::ServerToClient &stc, int64_t *pOffset, int64_t *pOll) {
        bool ret = false;
        *pOll = *pOffset = 0;
        auto serverTime = stc.world_time();
        if (stc.has_cts_latency()) {
            m_stc_useF5 = false;
            *pOll = (stc.cts_latency() + getWorldTime() - serverTime) / 2;
            *pOffset = g_steadyClock.nowInMilliseconds() - serverTime - *pOll;
            ret = true;
        } else if (m_stc_useF5 && stc.has_stc_f5()) {
            auto mySendTimeIterator = m_map38.find(stc.stc_f5());
            if (mySendTimeIterator != m_map38.end()) {
                auto mySendTime = mySendTimeIterator->second;
                *pOll = (g_steadyClock.nowInMilliseconds() - mySendTime) / 2;
                *pOffset = *pOll + mySendTimeIterator->second - serverTime;
                ret = true;
            }
        }
        NetworkingLogDebug("shouldSetClock: %s, oneLegLatency: %ldms, worldClockOffset: %+ldms", ret ? "true" : "false", *pOll, *pOffset);
        return ret;
    }
    void handleServerToClient(const protobuf::ServerToClient &stc) {
        int64_t offset, oll;
        if (calculateOneLegLatency(stc, &offset, &oll)) {
            //TODO or OMIT m_ncs->WorldClockStatistics::registerOneLegLatency(oll);
            adjustClock(offset, oll);
        }
    }
    uint64_t getRoundTripLatencyInMilliseconds() { return m_twoLegsLatency; }
    void storeSequenceNumberSendTime(uint32_t seq) { //сам написал, требуется проверка
        if (m_map38.size() > 100) {
            std::erase_if(m_map38, [seq](const auto &item) {
               return int(seq - item.first) >= 100;
            });
        }
        m_map38[seq] = g_steadyClock.nowInMilliseconds();
    }
};
struct RelayServerRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_relayUrl;
    RelayServerRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &relayUrl) : m_mgr(mgr), m_relayUrl(relayUrl) {}
    std::future<NetworkResponse<protobuf::TcpConfig>> fetchTcpConfig() {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::TcpConfig>(CurlHttpConnection *)>([this](CurlHttpConnection *conn) {
            NetworkResponse<protobuf::TcpConfig> ret;
            auto v9 = conn->performGet(this->m_relayUrl + "/tcp-config"s, AcceptHeader(ATH_PB), "TCP Config"s);
            return HttpHelper::convertToResultResponse<protobuf::TcpConfig>(v9);
            }), true, false);
    }
    std::future<NetworkResponse<void>> leaveWorld(int64_t worldId) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([this, worldId](CurlHttpConnection *conn) {
            std::string url(m_relayUrl);
            url += "/worlds/"s + std::to_string(worldId) + "/leave"s;
            auto v9 = conn->performPost(url, ContentTypeHeader(CTH_JSON), ""s,
                AcceptHeader(ATH_JSON), "Leave World"s, false);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::PlayerState>> latestPlayerState(int64_t worldId, int64_t playerId) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PlayerState>(CurlHttpConnection *)>([this, worldId, playerId](CurlHttpConnection *conn) {
            NetworkResponse<protobuf::PlayerState> ret;
            std::string url(m_relayUrl);
            url += "/worlds/"s + std::to_string(worldId) + "/players/"s + std::to_string(playerId);
            auto v9 = conn->performGet(url, AcceptHeader(ATH_PB), "Get Latest Player State"s);
            return HttpHelper::convertToResultResponse<protobuf::PlayerState>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<void>> setPhoneAddress(const std::string &phoneAddress, int port, protobuf::IPProtocol ipp, int securePort, const std::string &sk) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            QueryStringBuilder qsb;
            qsb.add("phone-address"s, phoneAddress);
            qsb.add("port"s, port);
            qsb.add("protocol"s, (int32_t)ipp);
            qsb.add("secure-port"s, (int32_t)securePort);
            qsb.add("secret"s, sk);
            std::string url(m_relayUrl + "/profiles/me/set-phone-address"s);
            auto v9 = conn->performPut(url, ContentTypeHeader(CTH_URLENC), qsb.getString(false),
                AcceptHeader(), "Set Phone Address"s, false);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::HashSeeds>> requestHashSeeds(bool isAi) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::HashSeeds>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            NetworkingLogInfo("Requesting seeds...");
            std::string url(m_relayUrl + "/worlds/hash-seeds"s);
            if (isAi)
                url += "?isAi=true"s;
            auto v9 = conn->performGet(url, AcceptHeader(ATH_PB), "Get Hash Seeds"s);
            return HttpHelper::convertToResultResponse<protobuf::HashSeeds>(v9);
        }), true, true);
    }
    std::future<NetworkResponse<protobuf::DropInWorldList>> fetchDropInWorldList(bool unlim) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::DropInWorldList>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_relayUrl + "/dropin"s);
            if (unlim)
                url += "?maxPlayersPerWorld=0"s;
            auto v9 = conn->performGet(url, AcceptHeader(ATH_PB), "Fetch Drop-in World List"s);
            return HttpHelper::convertToResultResponse<protobuf::DropInWorldList>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::LateJoinInformation>> getLateJoinInformation(int64_t meetup_id) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::LateJoinInformation>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_relayUrl + "/events/subgroups/"s);
            url += std::to_string(meetup_id);
            url += "/late-join"s;
            auto v9 = conn->performGet(url, AcceptHeader(ATH_PB), "Get Late Join Information"s);
            return HttpHelper::convertToResultResponse<protobuf::LateJoinInformation>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::RelaySessionRefreshResponse>> refreshRelaySession(int64_t id) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::RelaySessionRefreshResponse>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_relayUrl + "/session/refresh"s);
            std::vector<char> payload;
            protobuf::RelaySessionRefreshRequest rq;
            rq.set_id(id);
            HttpHelper::protobufToCharVector(&payload, rq);
            auto v9 = conn->performPost(url, ContentTypeHeader(CTH_PB), payload, AcceptHeader(ATH_PB),
                "Refresh Relay Session"s, false);
            return HttpHelper::convertToResultResponse<protobuf::RelaySessionRefreshResponse>(v9);
        }), true, true);
    }
    std::future<NetworkResponse<int64_t>> saveWorldAttribute(int64_t id, const protobuf::WorldAttribute &wa) {
        protobuf::WorldAttribute waCopy(wa);
        waCopy.set_server_realm(id); //not sure
        std::vector<char> payload;
        HttpHelper::protobufToCharVector(&payload, waCopy);
        return m_mgr->pushRequestTask(std::function<NetworkResponse<int64_t>(CurlHttpConnection *)>([this, payload](CurlHttpConnection *conn) {
            std::string url(m_relayUrl + "/worlds/attributes"s);
            auto v9 = conn->performPost(url, ContentTypeHeader(CTH_PB), payload, AcceptHeader(),
                "Save World Attribute"s, false);
            return HttpHelper::convertToLongResponse(v9, 0);
        }), true, false);
    }
    std::future<NetworkResponse<void>> saveTimeCrossingStartLine(int64_t event_id, const protobuf::CrossingStartingLineProto &csl) {
        protobuf::CrossingStartingLineProto cslCopy(csl);
        std::vector<char> payload;
        HttpHelper::protobufToCharVector(&payload, cslCopy);
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([this, payload, event_id](CurlHttpConnection *conn) {
            std::string url(m_relayUrl);
            url += "/race/event_starting_line/"s + std::to_string(event_id);
            auto v9 = conn->performPost(url, ContentTypeHeader(CTH_PB), payload, AcceptHeader(ATH_JSON),
                "Save Time Crossing Start Line"s, false);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
};
template<class T>
struct FutureWaiter : public boost::asio::steady_timer {
    using boost::asio::steady_timer::steady_timer;
    uint64_t m_timeout = 0;
    std::future<T> m_obj;
    std::function<void(const T &)> m_func;
    bool m_inWait = false;
    void poll(const boost::system::error_code &ec) {
        if (!ec) {
            if (!is_ready(m_obj))
                return waitAgain();
            m_inWait = false;
            m_func(m_obj.get());
        }
    }
    void waitAsync(std::future<T> &&obj, uint64_t toMs, const std::function<void(const T &)> &f) {
        m_inWait = true;
        m_obj = std::move(obj);
        m_timeout = toMs;
        m_func = f;
        waitAgain();
    }
    void waitAgain() {
        expires_after(std::chrono::milliseconds(m_timeout));
        async_wait([this](const boost::system::error_code &ec) { this->poll(ec); });
    }
};
struct HashSeedService { //0x370 bytes
    std::mutex m_mutex;
    EventLoop *m_eventLoop;
    GlobalState *m_gs;
    RelayServerRestInvoker *m_relay;
    WorldClockService *m_worldClock;
    boost::asio::steady_timer m_asioTimer3, m_asioTimer4;
    FutureWaiter<NetworkResponse<protobuf::RelaySessionRefreshResponse>> m_fwRelayRefresh;
    FutureWaiter<NetworkResponse<protobuf::HashSeeds>> m_fwHashSeeds;
    EncryptionInfo m_ei;
    std::map<uint64_t, uint64_t> m_map;
    int m_a7 = 300, m_a8 = 10;
    bool m_isAi, m_started = false;
    void start() {
        if (!m_started) {
            m_started = true;
            if (!m_isAi) {
                fetchHashSeeds();
                if (m_gs->shouldUseEncryption()) {
                    m_ei = m_gs->getEncryptionInfo();
                    auto v7 = calculateRefreshRelaySessionInAdvance(m_ei.m_expiration);
                    scheduleRefreshRelaySession(1000 * v7);
                }
            }
        }
    }
    HashSeedService(EventLoop *el, GlobalState *gs, RelayServerRestInvoker *ri, WorldClockService *wc, bool isAi) :
        m_eventLoop(el), m_gs(gs), m_relay(ri), m_worldClock(wc), m_asioTimer3(m_eventLoop->m_asioCtx), m_asioTimer4(m_eventLoop->m_asioCtx),
        m_fwRelayRefresh(m_eventLoop->m_asioCtx), m_fwHashSeeds(m_eventLoop->m_asioCtx), m_isAi(isAi) {}
    ~HashSeedService() {}
    uint64_t calculateRefreshRelaySessionInAdvance(uint32_t expiration) { //vptr[1]
        int v4 = m_a7 / 60;
        auto v5 = (int)((double)expiration * 0.1);
        if (v4 >= v5) {
            if (v5 < 1)
                v5 = 1;
            v4 = v5;
        }
        return 60 * (expiration - v4);
    }
    void onHashSeedsReceived(const NetworkResponse<protobuf::HashSeeds> &src) { //vptr[2]
        if (src.m_errCode) {
            NetworkingLogError("Failed to fetch seeds: [%d] %s", src.m_errCode, src.m_msg.c_str());
            scheduleFetchHashSeeds(1000 * m_a8);
        } else {
            auto &v7 = src.m_T.seeds().Get(src.m_T.seeds_size() - 1);
            auto v9 = m_worldClock->getWorldTime();
            auto v11 = 3000000i64;
            if (v9)
                v11 = v7.expiry_date() - v9 - 1000 * m_a7;
            NetworkingLogInfo("Next seeds fetch will happen in %d minutes", v11 / 60000);
            scheduleFetchHashSeeds(v11);
            std::lock_guard l(m_mutex); //not sure, but looks like:
            std::erase_if(m_map, [v9](const auto &item) {
                return int(v9 - item.first) >= 1800000;
            });
            m_map[v9] = MAKELONG(v7.seed1(), v7.seed2());
        }
    }
    void refreshRelaySession() {
        if (m_relay) {
            m_fwRelayRefresh.waitAsync(m_relay->refreshRelaySession(m_ei.m_relaySessionId), 100,
                [this](const NetworkResponse<protobuf::RelaySessionRefreshResponse> &r) {
                    onRefreshRelaySession(r);
                }
            );
        } else {
            scheduleRefreshRelaySession(1000 * this->m_a8);
        }
    }
    void fetchHashSeeds() {
        m_fwHashSeeds.waitAsync(m_relay->requestHashSeeds(m_isAi), 100,
            [this](const NetworkResponse<protobuf::HashSeeds> &r) {
                onHashSeedsReceived(r);
            }
        );
    }
    void scheduleRefreshRelaySession(int64_t val) {
        m_asioTimer4.expires_after(std::chrono::milliseconds(val));
        m_asioTimer4.async_wait([this](const boost::system::error_code &ec) {
            if (!ec)
                this->refreshRelaySession();
        });
    }
    uint64_t getHashSeed(uint64_t worldTime) {
        if (worldTime) {
            std::lock_guard l(m_mutex);
            if (!m_isAi || !m_map.empty() || m_fwHashSeeds.m_inWait /*not sure*/) {
                auto f = m_map.find(worldTime);
                if (f != m_map.end())
                    return f->second;
            } else {
                fetchHashSeeds();
            }
        }
        return 0x162E000004D2LL; // HashSeedService::DEFAULT_HASH_SEED;
    }
    void onRefreshRelaySession(const NetworkResponse<protobuf::RelaySessionRefreshResponse> &src) { /*vptr[3]*/
        int64_t val;
        if (src.m_errCode) {
            NetworkingLogError("Failed to refresh relay session: [%d] %s", src.m_errCode, src.m_msg.c_str());
            val = 1000 * m_a8;
        } else {
            m_ei.m_expiration = src.m_T.expiration();
            val = 1000 * calculateRefreshRelaySessionInAdvance(m_ei.m_expiration);
            m_gs->setEncryptionInfo(m_ei);
        }
        scheduleRefreshRelaySession(val);
    }
    void scheduleFetchHashSeeds(int64_t val) {
        m_asioTimer3.expires_after(std::chrono::milliseconds(val));
        m_asioTimer3.async_wait([this](const boost::system::error_code &ec) {
            if (!ec)
                this->fetchHashSeeds();
        });
    }
    NetworkRequestOutcome signMessage(uint8_t *buf, uint32_t len, uint64_t worldTime) {
        auto hashSeed = getHashSeed(worldTime);
        if (hashSeed) {
            *(DWORD *)&buf[len] = htonl((DWORD)hashSeed);
            DWORD v9 = XXH32(buf, len + 4, hashSeed >> 32);
            *(DWORD *)&buf[len] = htonl(v9);
            return NRO_OK;
        } else {
            NetworkingLogWarn("No hash seed available");
            return NRO_NO_HASH_CODES_YET;
        }
    }
};
struct UdpStatistics;
struct RestServerRestInvoker { //0x70 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url, m_machId, m_version;
    RestServerRestInvoker(const std::string &machId, ZwiftHttpConnectionManager *mgr, const std::string &server, const std::string &version) 
        : m_mgr(mgr), m_url(server), m_machId(machId), m_version(version) {}
    std::future<NetworkResponse<protobuf::SocialNetworkStatus>> addFollowee(int64_t playerId, int64_t followeeId, bool a5, protobuf::ProfileFollowStatus pfs) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::SocialNetworkStatus>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url);
            url += "/api/profiles/"s + std::to_string(playerId) + "/following/"s + std::to_string(followeeId);
            protobuf::SocialNetworkStatus pb;
            pb.set_sns_f2(a5);
            pb.set_pfs(pfs);
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, pb);
            auto v9 = conn->performPost(url, ContentTypeHeader(CTH_PB), payload, AcceptHeader(ATH_PB),
                "Follow Player"s, false);
            return HttpHelper::convertToResultResponse<protobuf::SocialNetworkStatus>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<void>> removeFollowee(int64_t playerId, int64_t followeeId) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url);
            url += "/api/profiles/"s + std::to_string(playerId) + "/following/"s + std::to_string(followeeId);
            auto v9 = conn->performDelete(url, AcceptHeader(), "Stop Following Player"s);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
    std::future<NetworkResponse<int64_t>> createActivityRideOn(int64_t playerIdSender, int64_t playerIdTarget) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<int64_t>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url);
            url += "/api/profiles/"s + std::to_string(playerIdTarget) + "/activities/0/rideon"s;
            Json::Value json;
            json["profileId"] = playerIdSender;
            std::string payload = HttpHelper::jsonToString(json);
            auto v9 = conn->performPost(url, ContentTypeHeader(CTH_JSON), payload, AcceptHeader(ATH_JSON), "Create Activity Ride On"s, false);
            return HttpHelper::convertToResultResponse(v9, std::function<int64_t(const Json::Value &)>([](const Json::Value &rx_json) {
                return rx_json["id"].asInt64();
            }));
        }), true, false);
    }
    std::future<NetworkResponse<void>> createUser(const std::string &email, const std::string &pwd, const std::string &firstN, const std::string &lastN) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(this->m_url);
            url += "/api/users?"s + conn->escapeUrl("Machine Id"s) + "="s + conn->escapeUrl(this->m_machId);
            Json::Value json, p;
            json["email"] = email;
            json["password"] = pwd;
            p["firstName"] = firstN;
            p["lastName"] = lastN;
            json["profile"] = p;
            std::string payload = HttpHelper::jsonToString(json);
            auto v9 = conn->performPost(url, ContentTypeHeader(CTH_JSON), payload, AcceptHeader(), "Create User"s, false);
            return HttpHelper::convertToVoidResponse(v9);
        }), false, false);
    }
    std::future<NetworkResponse<void>> deleteActivity(int64_t playerId, int64_t actId) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(this->m_url);
            url += "/api/profiles/"s + std::to_string(playerId) + "/activities/"s + std::to_string(actId);
            auto v9 = conn->performDelete(url, AcceptHeader(), "Delete Activity"s);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::ActivityList>> getActivities(int64_t profileId, const Optional<int64_t> &startsAfter, const Optional<int64_t> &startsBefore, bool fetchSnapshots) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::ActivityList>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            QueryStringBuilder qsb;
            qsb.addOptional("startsAfter"s, startsAfter);
            qsb.addOptional("startsBefore"s, startsBefore);
            qsb.addIfNotFalse("fetchSnapshots"s, fetchSnapshots);
            auto url = this->m_url + "/api/profiles/"s + std::to_string(profileId) + "/activities"s + qsb.getString(true);
            auto v9 = conn->performGet(url, AcceptHeader(ATH_PB), "List Activities"s);
            return HttpHelper::convertToResultResponse<protobuf::ActivityList>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::ProfileEntitlements>> myProfileEntitlements() {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::ProfileEntitlements>(CurlHttpConnection *)>([this](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(this->m_url + "/api/profiles/me/entitlements"s, AcceptHeader(ATH_PB), "Get My Profile Entitlements"s);
            return HttpHelper::convertToResultResponse<protobuf::ProfileEntitlements>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::EventProtobuf>> getEvent(int64_t eventId) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::EventProtobuf>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(this->m_url + "/api/events/"s + std::to_string(eventId), AcceptHeader(ATH_PB), "Get Group Ride"s);
            return HttpHelper::convertToResultResponse<protobuf::EventProtobuf>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::Goals>> getGoals(int64_t profileId) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::Goals>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(this->m_url + "/api/profiles/"s + std::to_string(profileId) + "/goals"s, AcceptHeader(ATH_PB), "List Goals"s);
            return HttpHelper::convertToResultResponse<protobuf::Goals>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<bool>> registerForEventSubgroup(int64_t eventId) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<bool>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performPost(this->m_url + "/api/events/subgroups/register/"s + std::to_string(eventId) + "/goals"s, 
                ContentTypeHeader(CTH_JSON), "{}"s, AcceptHeader(ATH_JSON), "Register for Group Ride Subgroup"s, false);
            return HttpHelper::convertToResultResponse<bool>(v9, std::function<bool(const Json::Value &)>([](const Json::Value &rx_json) {
                return rx_json["registered"].asBool();
            }));
        }), true, false);
    }
    std::future<NetworkResponse<bool>> removeSignupForEvent(int64_t eventId) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<bool>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performDelete(this->m_url + "/api/events/signup/"s + std::to_string(eventId), AcceptHeader(ATH_JSON), "Remove Sign Up for Group Ride"s);
            return HttpHelper::convertToResultResponse<bool>(v9, std::function<bool(const Json::Value &)>([](const Json::Value &rx_json) {
                return rx_json["signedUp"].asBool();
            }));
        }), true, false);
    }
    std::future<NetworkResponse<model::EventSignupResponse>> signupForEventSubgroup(int64_t eventId) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<model::EventSignupResponse>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performPost(this->m_url + "/api/events/subgroups/signup/"s + std::to_string(eventId), 
                ContentTypeHeader(CTH_JSON), "{}"s, AcceptHeader(ATH_JSON), "Sign Up for Group Ride Subgroup"s, false);
            return HttpHelper::convertToResultResponse<model::EventSignupResponse>(v9, std::function<model::EventSignupResponse(const Json::Value &)>([](const Json::Value &rx_json) {
                auto timeTrialData = rx_json["timeTrialData"];
                return model::EventSignupResponse{
                    timeTrialData.isNull() ? 0 : timeTrialData["riderStartTime"].asString(),
                    rx_json["signUpStatus"].asInt(),
                    timeTrialData.isNull() ? 0 : timeTrialData["riderSlot"].asInt(),
                    rx_json["signedUp"].asBool()
                };
            }));
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::PlayerSocialNetwork>> getFollowees(int64_t profileId, bool followRequests) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PlayerSocialNetwork>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(this->m_url + "/api/profiles/"s + std::to_string(profileId) + "/followees?include-follow-requests="s +
                (followRequests ? "true"s : "false"s), AcceptHeader(ATH_PB), "List Followees"s);
            return HttpHelper::convertToResultResponse<protobuf::PlayerSocialNetwork>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::PlayerProfiles>> getEventSubgroupEntrants(protobuf::EventParticipation ep, int64_t eventId, uint32_t limit) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PlayerProfiles>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            QueryStringBuilder qsb;
            qsb.add("type"s, "all"s);
            qsb.add("participation"s, (ep == protobuf::EVENT_PARTICIPATION_REGISTERED) ? "registered"s : "signed_up"s);
            qsb.add("start"s, 0);
            qsb.add("limit"s, limit);
            auto url = this->m_url + "/api/events/subgroups/entrants/"s + std::to_string(eventId) + qsb.getString(true);
            auto v9 = conn->performGet(url, AcceptHeader(ATH_PB), "List Group Ride Subgroup Entrants"s);
            return HttpHelper::convertToResultResponse<protobuf::PlayerProfiles>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::EventsProtobuf>> getEventsInInterval(const std::string &start, const std::string &end, int limit) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::EventsProtobuf>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            Json::Value json;
            json["dateRangeStartISOString"] = start;
            json["dateRangeEndISOString"] = end;
            std::string payload = HttpHelper::jsonToString(json);
            auto v9 = conn->performPost(this->m_url + "/api/events/search?limit="s + std::to_string(limit), ContentTypeHeader(CTH_JSON), payload, AcceptHeader(ATH_PB), "Query Group Rides"s, false);
            return HttpHelper::convertToResultResponse<protobuf::EventsProtobuf>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::PlayerProfile>> myProfile(const std::function<void(const NetworkResponse<protobuf::PlayerProfile> &)> &f) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PlayerProfile>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto url = this->m_url + "/api/profiles/me"s;
            auto v9 = conn->performGet(url, AcceptHeader(ATH_PB), "Get My Profile"s);
            auto ret = HttpHelper::convertToResultResponse<protobuf::PlayerProfile>(v9);
            f(ret);
            return ret;
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::PlayerProfile>> profileByPlayerId(int64_t playerId, bool bSocial) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PlayerProfile>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto url = this->m_url + "/api/profiles/"s + std::to_string(playerId);
            if (bSocial)
                url += "?include-social-facts=true"s;
            auto v9 = conn->performGet(url, AcceptHeader(ATH_PB), "Get Profile by Player Id"s);
            return HttpHelper::convertToResultResponse<protobuf::PlayerProfile>(v9);
        }), true, false);
    }
    NetworkResponse<protobuf::PlayerProfiles> requestProfilesByPlayerIds(CurlHttpConnection *conn, const std::unordered_set<int64_t> &ids) {
        if (ids.size()) {
            QueryStringBuilder qsb;
            for (auto i : ids)
                qsb.add("id"s, i);
            auto url = this->m_url + "/api/profiles"s + qsb.getString(true);
            auto v9 = conn->performGet(url, AcceptHeader(ATH_PB), "Get Profiles by Player Ids"s);
            return HttpHelper::convertToResultResponse<protobuf::PlayerProfiles>(v9);
        } else {
            return NetworkResponse<protobuf::PlayerProfiles>();
        }
    }
    std::future<NetworkResponse<protobuf::PlayerProfiles>> profilesByPlayerIds(const std::unordered_set<int64_t> &ids) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PlayerProfiles>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            return requestProfilesByPlayerIds(conn, ids);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::PlayerProfiles>> profilesByPlayerIds(const ProfileRequestLazyContext &ctx) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PlayerProfiles>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            return requestProfilesByPlayerIds(conn, ctx.getPlayerIds());
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::SegmentResults>> querySegmentResults(int64_t serverRealm, int64_t segmentId, const std::string &from, const std::string &to, bool full) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::SegmentResults>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            QueryStringBuilder qsb;
            qsb.add("world_id"s, serverRealm);
            qsb.add("full"s, full);
            qsb.add("from"s, from);
            qsb.add("to"s, to);
            qsb.add("segmentId"s, segmentId);
            auto url = this->m_url + "/api/personal-records/my-records"s + qsb.getString(true);
            auto v9 = conn->performGet(url, AcceptHeader(ATH_PB), "Query Segment Results"s);
            return HttpHelper::convertToResultResponse<protobuf::SegmentResults>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::SegmentResults>> querySegmentResults(int64_t serverRealm, const std::unordered_set<int64_t> &segmentIds, int64_t eventId, bool full, int64_t playerId) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::SegmentResults>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            QueryStringBuilder qsb;
            qsb.add("world_id"s, serverRealm);
            qsb.add("event_subgroup_id"s, eventId);
            qsb.add("full"s, full);
            qsb.add("player_id"s, playerId);
            for (auto i : segmentIds)
                qsb.add("segment_id"s, i);
            auto url = this->m_url + "/api/segment-results"s + qsb.getString(true);
            auto v9 = conn->performGet(url, AcceptHeader(ATH_PB), "Query Segment Results"s);
            return HttpHelper::convertToResultResponse<protobuf::SegmentResults>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<void>> removeGoal(int64_t playerId, int64_t goalId) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(this->m_url);
            url += "/api/profiles/"s + std::to_string(playerId) + "/goals/"s + std::to_string(goalId);
            auto v9 = conn->performDelete(url, AcceptHeader(), "Delete Goal"s);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
    std::future<NetworkResponse<int64_t>> saveActivity(const protobuf::Activity &act, bool bFinalSave, const std::string &fitPath, const std::function<void(const NetworkResponse<int64_t> &)> &func) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<int64_t>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(this->m_url);
            url += "/api/profiles/"s + std::to_string(act.player_id()) + "/activities"s;
            auto pAct = &act;
            std::unique_ptr<protobuf::Activity> actMutable;
            if (act.has_id())
                url += "/"s + std::to_string(act.id());
            if (bFinalSave)
                url += "?upload-to-strava=true"s;
            if (!act.has_fit() && fitPath.size()) {
                NetworkingLogInfo("Activity Upload: load fit file from path %s", fitPath.c_str());
                std::ifstream fit(fitPath, std::ios::binary | std::ios::ate);
                auto fitLen = fit.tellg();
                if (fit.fail() || fitLen == 0) {
                    NetworkingLogError("Activity Upload: failed to open fit file");
                    return NetworkResponse<int64_t>{"Failed to open fit file"s, NRO_ERROR_READING_FILE};
                }
                actMutable.reset(new protobuf::Activity(act));
                auto mfit = actMutable->mutable_fit();
                mfit->resize(fitLen);
                fit.seekg(0);
                fit.read(mfit->data(), fitLen);
                if (fit.fail()) {
                    NetworkingLogError("Activity Upload: failed to read fit file");
                    return NetworkResponse<int64_t>{"Failed to read fit file"s, NRO_ERROR_READING_FILE};
                }
                pAct = actMutable.get();
            }
            if (pAct->has_fit())
                NetworkingLogInfo("Activity Upload: fit file is length %d", pAct->fit().length());
            else
                NetworkingLogInfo("Activity Upload: no fit file");
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, *pAct);
            auto v9 = conn->performPutOrPost(act.has_id(), url, ContentTypeHeader(CTH_PB),
                payload, AcceptHeader(ATH_JSON), "Update Activity"s, "Save Activity"s, true);
            auto ret = HttpHelper::convertToResultResponse<int64_t>(v9, [](const Json::Value &json) {
                return json["id"].asInt64();
            });
            func(ret);
            return ret;
        }), true, true);
    }
    std::future<NetworkResponse<int64_t>> saveActivityImage(int64_t profileId, const protobuf::ActivityImage &img, const std::string &imgPath) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<int64_t>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(this->m_url);
            url += "/api/profiles/"s + std::to_string(profileId) + "/activities/"s + std::to_string(img.act_id()) + "/images"s;
            auto pImg = &img;
            std::unique_ptr<protobuf::ActivityImage> imgMutable;
            if (img.has_img_id()) {
                url += "/"s + std::to_string(img.img_id());
            } else if (!img.has_bits() && imgPath.size()) {
                NetworkingLogInfo("Activity Image Upload: load image file from path %s", imgPath.c_str());
                std::ifstream stream(imgPath, std::ios::binary | std::ios::ate);
                auto imgLen = stream.tellg();
                if (stream.fail() || imgLen == 0) {
                    NetworkingLogError("Activity Image Upload: failed to open image file");
                    return NetworkResponse<int64_t>{"Failed to open image file"s, NRO_ERROR_READING_FILE};
                }
                imgMutable.reset(new protobuf::ActivityImage(img));
                auto mimg = imgMutable->mutable_bits();
                mimg->resize(imgLen);
                stream.seekg(0);
                stream.read(mimg->data(), imgLen);
                if (stream.fail()) {
                    NetworkingLogError("Activity Image Upload: failed to read image file");
                    return NetworkResponse<int64_t>{"Failed to read image file"s, NRO_ERROR_READING_FILE};
                }
                pImg = imgMutable.get();
            }
            if (pImg->has_bits())
                NetworkingLogInfo("Activity Upload: image file is length %d", pImg->bits().length());
            else
                NetworkingLogInfo("Activity Upload: no image file");
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, *pImg);
            auto v9 = conn->performPutOrPost(img.has_img_id(), url, ContentTypeHeader(CTH_PB),
                payload, AcceptHeader(ATH_JSON), "Update Activity Image"s, "Save Activity Image"s, true);
            return HttpHelper::convertToResultResponse<int64_t>(v9, [](const Json::Value &json) {
                return json["id"].asInt64();
            });
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::Goal>> saveGoal(const protobuf::Goal &goal) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::Goal>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(this->m_url);
            url += "/api/profiles/"s + std::to_string(goal.player_id()) + "/goals"s;
            if (goal.has_id())
                url += "/"s + std::to_string(goal.id());
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, goal);
            auto v9 = conn->performPutOrPost(goal.has_id(), url, ContentTypeHeader(CTH_PB),
                payload, AcceptHeader(ATH_PB), "Update Goal"s, "Save Goal"s, false);
            return HttpHelper::convertToResultResponse<protobuf::Goal>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<int64_t>> saveSegmentResult(const protobuf::SegmentResult &sr) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<int64_t>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, sr);
            auto v9 = conn->performPost(this->m_url + "/api/segment-results"s, ContentTypeHeader(CTH_PB), payload, AcceptHeader(ATH_JSON), "Save Segment Result"s, false);
            return HttpHelper::convertToResultResponse<int64_t>(v9, [](const Json::Value &json) {
                return json["id"].asInt64();
            });
        }), true, false);
    }
    std::future<NetworkResponse<void>> updateProfile(const protobuf::PlayerProfile &prof, bool inGameFields, const std::function<void()> &f) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url = m_url + "/api/profiles/"s + std::to_string(prof.id());
            if (inGameFields)
                url += "/in-game-fields"s;
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, prof);
            auto v9 = conn->performPut(url, ContentTypeHeader(CTH_PB), payload, AcceptHeader(), inGameFields ? "Update Profile Game Fields"s : "Update Profile"s, false);
            auto ret = HttpHelper::convertToVoidResponse(v9);
            f();
            return ret;
        }), true, false);
    }
    std::future<NetworkResponse<bool>> createRegistration(int64_t id) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<bool>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url + "/events/subgroups/register/"s + std::to_string(id));
        auto v9 = conn->performPost(url, ContentTypeHeader(CTH_JSON), "{}"s, AcceptHeader(ATH_JSON), "Create Registration"s, false);
        return HttpHelper::convertToResultResponse<bool>(v9, [](const Json::Value &json) {
            return json["registered"].asBool();
            });
            }), true, false);
    }
    /* OMIT:
RestServerRestInvoker::partnerAccessToken(const std::string &) //OMIT
RestServerRestInvoker::sendMixpanelEvent(const std::string &,std::vector<std::string> const&) //OMIT
//not found in PC version yet:
RestServerRestInvoker::deleteActivityImage(int64_t,int64_t,int64_t) "Delete Activity Image" "/api/profiles/" "/activities/" "/images/" pushRequestTask<void>(true, false) 
RestServerRestInvoker::getActivity(int64_t,int64_t,bool)
RestServerRestInvoker::getActivityImage(int64_t,int64_t,int64_t)
RestServerRestInvoker::getEventsInInterval
RestServerRestInvoker::getFollowers(int64_t,bool)
RestServerRestInvoker::getFollowers(int64_t,int64_t,bool)
RestServerRestInvoker::getNotifications()
RestServerRestInvoker::getSegmentResult(int64_t)
RestServerRestInvoker::getTimeSeries(int64_t,protobuf::TimeSeriesType)
RestServerRestInvoker::getTimeSeries(int64_t,protobuf::TimeSeriesType,const std::string &)
RestServerRestInvoker::getTimeSeries(int64_t,protobuf::TimeSeriesType,const std::string &,const std::string &)
RestServerRestInvoker::listPlayerTypes()
RestServerRestInvoker::profileByEmail(const std::string &)
RestServerRestInvoker::redeemCoupon(const std::string &)
RestServerRestInvoker::removeFollower(int64_t,int64_t)
RestServerRestInvoker::removeRegistrationForEvent(int64_t)
RestServerRestInvoker::resumeSubscription()
RestServerRestInvoker::saveProfileReminders(protobuf::PlayerProfile const&)
RestServerRestInvoker::signUrls(const std::string &,std::vector<std::string> const&)
RestServerRestInvoker::updateFollower(int64_t,int64_t,bool,protobuf::ProfileFollowStatus)
RestServerRestInvoker::updateNotificationReadStatus(int64_t,int64_t,bool)
RestServerRestInvoker::uploadReceipt(int64_t,protobuf::InAppPurchaseReceipt &)
*/
};
struct NetworkClientImpl;
struct HostAndPorts {
    uint16_t m_ports[4] = {}; //encr, not encr and last 2 are not used
    std::string m_host;
};
struct RelayAddressService {
    protobuf::RelayAddressesVOD m_vod;
    std::map<uint64_t, protobuf::RelayAddressesVOD> m_field_48;
    int m_world = 0, m_map = 0;
    uint16_t m_defaultPort = 0, m_defaultPortEncr = 0;
    HostAndPorts getRandomAddress(uint64_t world, uint32_t map, uint64_t playerId, bool *a6) {
        m_world = world;
        m_map = map;
        auto v18 = m_field_48.find(MAKELONG(m_world, m_map));
        auto &v8 = (v18 == m_field_48.end()) ? m_vod : v18->second;
        auto curSize = v8.relay_addresses_size();
        if (curSize) { 
            *a6 = (int)curSize > 1 || (curSize == 1 && v8.relay_addresses(0).has_x());
            const auto &v12 = v8.relay_addresses(playerId % curSize);
            HostAndPorts ret;
            ret.m_ports[0] = v12.has_cport() ? v12.cport() : m_defaultPortEncr;
            ret.m_ports[1] = v12.has_port() ? v12.port() : m_defaultPort;
            ret.m_host = v12.ip();
            return ret;
        }
        return HostAndPorts();
    }
    HostAndPorts getAddress(uint64_t world, uint32_t map, float x, float z, bool *a7) {
        m_map = map;
        m_world = world;
        auto v15 = m_field_48.find(MAKELONG(m_world, m_map));
        protobuf::RelayAddressesVOD *ret;
        if (v15 == m_field_48.end()) {
            if (0 == m_vod.relay_addresses_size())
                NetworkingLogWarn("No cluster available for world %lld and map revision %d", world, map);
            ret = &m_vod;
        } else {
            ret = &v15->second;
        }
        return getAddress(ret, x, z, a7);
    }
    HostAndPorts getAddress(protobuf::RelayAddressesVOD *rav, float x, float z, bool *a6) {
        HostAndPorts ret;
        auto curSize = rav->relay_addresses_size();
        *a6 = (int)curSize > 1 || (curSize == 1 && rav->relay_addresses(0).has_x());
        if (curSize) {
            const protobuf::RelayAddress *v10 = nullptr;
            if (0 == rav->rav_f4()) {
                double v16 = 1.7e308;
                for (const auto &i : rav->relay_addresses()) {
                    auto v23 = (float)(x - i.x());
                    auto v24 = (float)(z - i.z());
                    v24 = v24 * v24 + v23 * v23;
                    if (v16 > v24) {
                        v16 = v24;
                        v10 = &i;
                    }
                }
            } else {
                for (const auto &i : rav->relay_addresses()) {
                    v10 = &i;
                    if (i.x() >= x && i.z() >= z)
                        break;
                }
            }
            if (v10) {
                ret.m_ports[0] = v10->has_cport() ? v10->cport() : m_defaultPortEncr;
                ret.m_ports[1] = v10->has_port() ? v10->port() : m_defaultPort;
                ret.m_host = v10->ip();
                return ret;
            }
        }
        return HostAndPorts();
    }
    void updateAddresses(const protobuf::UdpConfigVOD &src) {
        if (src.relay_addresses_vod_size()) {
            m_defaultPort = src.port();
            m_defaultPortEncr = src.cport();
            m_vod.Clear();
            m_field_48.clear();
            for (const auto &srci : src.relay_addresses_vod()) {
                if (srci.lb_realm() || srci.lb_course()) {
                    m_world = srci.lb_realm();
                    m_map = srci.lb_course();
                    m_field_48[MAKELONG(m_world, m_map)] = srci;
                } else {
                    m_vod.CopyFrom(srci);
                }
            }
        }
    }
};
struct PeriodicLogger {
    boost::asio::io_context &m_ctx;
    uint64_t m_period;
    std::function<void(uint32_t, uint64_t)> m_func;
    boost::asio::steady_timer m_timer;
    int m_counter = 0;
    PeriodicLogger(boost::asio::io_context &ctx, uint64_t period, const std::function<void(uint32_t, uint64_t)> &f) : m_ctx(ctx), m_period(period), m_timer(ctx) { }
    void cancel() { m_timer.cancel(); }
    void log() { m_ctx.post([this]() { resetLogTimer(); }); }
    void resetLogTimer() {
        ++m_counter;
        if (m_counter != 1)
            return;
        m_func(1, 0);
        m_timer.expires_after(std::chrono::milliseconds(m_period));
        m_timer.async_wait([this](const boost::system::error_code &ec) {
            if (!ec)
                m_func(m_counter - 1, m_period);
        });
    }
};
template<typename KEY, typename VAL> class LruCache {
    std::vector<std::pair<KEY, VAL>> m_arr;
    std::unordered_map<KEY, uint16_t> m_map;
    uint16_t m_nextIdx = 0;
public:
    LruCache(uint16_t size) : m_arr(size) {}
    VAL *add(const KEY &k) {
        uint16_t idx;
        auto f = m_map.find(k);
        if (f != m_map.end()) {
            idx = f->second;
        } else {
            idx = m_nextIdx++ % m_arr.size();
            if (m_map.size() == m_arr.size()) {
                auto deferredKey = m_arr[idx].first;
                m_map.erase(deferredKey);
            }
            m_map[k] = idx;
        }
        return &m_arr[idx].second;
    }
    VAL *find(const KEY &k) {
        auto f = m_map.find(k);
        if (f != m_map.end()) {
            auto idx = f->second;
            return &m_arr[idx].second;
        }
        return nullptr;
    }
};
#define ZWIFT_FAST_STAT //OMIT all stat except vital
struct UdpStatistics { //0x450 bytes
    struct PlayerStateInfo {
        int64_t m_id = 0, m_born = 0;
        float m_x = 0.0f, m_z = 0.0f;
        bool isAged(uint64_t nt) { return nt - m_born >= 2000; }
        /* inlined bool isStarved(uint64_t a2) {
            return false;
        }*/
    };
    std::string m_curHost, m_txPosition, m_field_360;
    std::mutex m_mutex;
    std::vector<int64_t> m_field_390;
    std::vector<uint32_t> m_txSpeeds, m_txPowers, m_txCads, m_txHRs;
    uint64_t m_latestFanViewedPlayerId = 0, m_ctsTraffic = 0, m_stcTraffic = 0, m_field_408 = 0, m_field_400 = (uint64_t)-1,
        m_field_330 = 0, m_ctsLastTime = 0, m_field_1F0 = 0, m_field_1F8 = 0, m_field_208 = 0, m_field_328 = 0, m_field_280 = 0,
        m_group_id = 0, m_field_2C0 = 0, m_field_3F8 = 0;
    int64_t m_field_3E8 = 0, m_field_3E0 = 0x7FFFFFFFFFFFFFFFi64, m_field_3D8 = 0;
    LruCache<uint64_t, PlayerStateInfo> m_field_100{2500};
    std::set<uint32_t> m_field_78;
    std::set<int64_t> m_field_158, m_field_198; //or google::protobuf::Descriptor const * ?
    double m_innermostDistance = 0.0;
    int m_parseErrorCount = 0, m_sendErrorCount = 0, m_encrEncodeErrs = 0, m_stcStatesCnt = 0, m_encrWrongServer = 0, m_encrNewConnectionErr = 0, m_encrWrongServer6 = 0, m_connErrCnt1 = 0, m_connErrCnt2 = 0,
        m_connToCnt1 = 0, m_connToCnt2 = 0, m_expungesCnt = 0, m_interpToCnt1 = 0, m_interpToCnt2 = 0, m_encrDecodeErrs = 0, m_field_3F0 = 0, m_field_410 = 0, m_field_C8 = 0, m_field_C0 = 0, m_field_C4 = 0,
        m_field_CC = 0, m_field_88 = 0, m_field_E0 = 0, m_field_430 = 0, m_field_448 = 0, m_field_44C = 0, m_field_E4 = 0, m_field_BC = 0, m_field_B8 = 0, m_speed = 0, m_power = 0, m_cadence_uhz = 0, m_zwifters = 0,
        m_heartrate = 0, m_climbing = 0, m_field_1E0 = 0, m_field_1E4 = 0, m_field_1E8 = 0, m_field_25C = 0, m_field_288 = 0, m_txClimbing = 0;
    uint32_t m_mapRevisionId = 0;
    protobuf::Sport m_sport = protobuf::CYCLING;
    float m_x = 0.0f, m_z = 0.0f, m_wx = 0.0f, m_wz = 0.0f;
    bool m_encrEnabled = false, m_hasSpeed = false, m_hasPower = false, m_hasCadence = false, m_hasZwifters = false, m_hasCoords = false, m_hasHeartrate = false, m_hasClimbing = false, m_field_1ED = false,
        m_hasJustWatching = false, m_just_watching = false, m_hasGroupId = false, m_hasSport = false, m_hasMapRevisionId = false, m_hasWorkoutMode = false, m_presentHasPhoneConnected = false, m_phoneConnected = false,
        m_hasTxPosition = false, m_hasTxClimbing = false;
    char m_workoutMode = 0;
    UdpStatistics() {
        /*TODO
  v2 = operator new(0x18ui64);
  *v2 = v2;
  v2[1] = v2;
  *(_QWORD *)&this->field_80 = v2;
  *(_QWORD *)&this->field_90 = 0i64;
  *(_QWORD *)&this->field_98 = 0i64;
  *(_QWORD *)&this->field_A0 = 0i64;
  *(_QWORD *)&this->field_A8 = 7i64;
  *(_QWORD *)&this->field_B0 = 8i64;
  *(_DWORD *)&this->m_field_78 = 0x3F800000;
  v3 = *(_QWORD *)&this->field_80;
  v4 = operator new(0x80ui64);
  *(_QWORD *)&this->field_90 = v4;
  v5 = v4 + 16;
  *(_QWORD *)&this->field_98 = v4 + 16;
  for ( *(_QWORD *)&this->field_A0 = v4 + 16; v4 != v5; ++v4 )
    *v4 = v3;
  std::unordered_set<google::protobuf::Descriptor const *>::unordered_set<google::protobuf::Descriptor const *>((__int64)&this->m_field_158);
  std::unordered_set<google::protobuf::Descriptor const *>::unordered_set<google::protobuf::Descriptor const *>((__int64)&this->m_field_198);
  this->field_344 = 1;
  this->field_34C = 1;
  this->field_354 = 1;
  this->field_35C = 1;
          */
    }
    void inspectServerToClient(const std::shared_ptr<protobuf::ServerToClient> &stc, uint64_t time, const std::string &host) {
        std::lock_guard l(m_mutex);
#ifdef ZWIFT_FAST_STAT
        for (const auto &v12 : stc->states1())
            registerPlayerStateInfo(v12);
#else
        registerReceivedDatagram(stc->seqno(), host);
        for (const auto &v12 : stc->states1()) {
            if (m_field_328 && time - m_field_330 < 2000) {
                auto v16 = m_x - v12.x();
                auto v17 = m_z - v12.z();
                if (m_innermostDistance > v17 * v17 + v16 * v16) {
                    m_field_390.push_back(time - v12.world_time());
                    m_field_158.emplace(v12.id());
                    m_field_198.emplace(v12.id());
                }
            }
            registerPlayerStateInfo(v12);
            if (v12.watching_rider_id() == m_latestFanViewedPlayerId) { //UdpStatistics::registerFanViewLatestPlayerStateInfo
                m_hasSpeed = v12.has_speed();
                m_speed = m_hasSpeed ? v12.speed() : 0;
                m_hasPower = v12.has_power();
                m_power = m_hasPower ? v12.power() : 0;
                m_hasCadence = v12.has_cadence_uhz();
                m_cadence_uhz = m_hasCadence ? v12.cadence_uhz() : 0;
                m_hasHeartrate = v12.has_heartrate();
                m_heartrate = m_hasHeartrate ? v12.heartrate() : 0;
                m_hasClimbing = v12.has_climbing();
                m_climbing = m_hasClimbing ? v12.climbing() : 0;
                if (v12.has_x() && v12.has_z()) {
                    m_wz = v12.z();
                    m_wx = v12.x();
                    m_hasCoords = true;
                } else {
                    m_hasCoords = false;
                }
            }
        }
        if (stc->has_zwifters()) {
            m_zwifters = stc->zwifters();
            m_hasZwifters = true;
        }
        m_stcStatesCnt += stc->states1_size();
        m_stcTraffic += stc->ByteSizeLong();
#endif
    }
    void inspectClientToServer(const protobuf::ClientToServer &cts, uint64_t time) {
        std::lock_guard l(m_mutex);
#ifdef ZWIFT_FAST_STAT
        if (cts.has_state()) {
            auto &state = cts.state();
            auto watching_rider_id = state.watching_rider_id();
            if (watching_rider_id == state.id())
                watching_rider_id = 0;
            if (watching_rider_id != m_latestFanViewedPlayerId)
                m_latestFanViewedPlayerId = watching_rider_id;
        }
#else
        auto &state = cts.has_state() ? cts.state() : protobuf::PlayerState::default_instance();
        auto watching_rider_id = state.watching_rider_id();// inlined UdpStatistics::registerFanView
        if (watching_rider_id == state.id()) {
            watching_rider_id = 0;
            m_field_1ED = false;
        }
        m_ctsLastTime = time;
        if (watching_rider_id != m_latestFanViewedPlayerId) {
            if (m_field_1E0) {
                m_field_1F0 += time - m_field_1F8;
                m_field_1F8 = time;
            }
            if (m_latestFanViewedPlayerId) {
                auto v13 = m_field_100.find(m_latestFanViewedPlayerId);
                if (v13) {
                    if (int64_t(time - v13->m_born) >= 5000)
                        ++m_field_1E4;
                }
            }
            m_latestFanViewedPlayerId = watching_rider_id;
            if (watching_rider_id) {
                m_field_1E0++;
                m_field_1ED = true;
                m_field_208 = watching_rider_id;
                m_field_1F8 = time;
                auto v14 = m_field_100.find(watching_rider_id);
                if (v14 == nullptr || int64_t(time - v14->m_born) >= 2000)
                    ++m_field_1E8;
            }
        }
        if (cts.has_state()) {
            ++m_field_25C;
            m_x = state.x();
            m_z = state.z();
            m_field_330 = state.world_time();
            if (!m_field_328)
                m_field_328 = state.id();
        }
        if (state.group_id() != m_field_280) {
            ++m_field_288;
            m_field_280 = state.group_id();
        }
        m_hasJustWatching = state.has_just_watching();
        m_just_watching = m_hasJustWatching ? state.just_watching() : false;
        m_hasGroupId = state.has_group_id();
        m_group_id = m_hasGroupId ? state.group_id() : 0;
        m_hasSport = state.has_sport();
        m_sport = m_hasSport ? state.sport() : protobuf::CYCLING;
        auto map = PlayerStateHelper::getMapRevisionId(state, &m_hasMapRevisionId); //URSOFT FIX
        m_mapRevisionId = m_hasMapRevisionId ? map : 0;
        m_hasWorkoutMode = state.has_progress();
        m_workoutMode = m_hasWorkoutMode ? PlayerStateHelper::getWorkoutMode(state) : 0;
        m_presentHasPhoneConnected = state.has_f19();
        m_phoneConnected = m_presentHasPhoneConnected ? PlayerStateHelper::getHasPhoneConnected(state) : false;
        if (PlayerStateHelper::hasPosition(state)) {
            m_txPosition = PlayerStateHelper::getPosition(state);
            m_hasTxPosition = true;
        } else {
            m_txPosition.clear();
            m_hasTxPosition = false;
        }
        m_hasTxClimbing = state.has_climbing();
        m_txClimbing = m_hasTxClimbing ? state.climbing() : 0;
        if (time >= m_field_2C0) {
            m_field_2C0 = time + 1000;
            if (state.has_speed())
                m_txSpeeds.push_back(state.speed());
            if (state.has_power())
                m_txPowers.push_back(state.power());
            if (state.has_cadence_uhz())
                m_txCads.push_back(state.cadence_uhz());
            if (state.has_heartrate())
                m_txHRs.push_back(state.heartrate());
        }
        m_ctsTraffic += cts.ByteSizeLong();
#endif
    }
    uint64_t getLatestFanViewedPlayerId() { return m_latestFanViewedPlayerId; }
    PlayerStateInfo *getLatestInfoFromFanViewedPlayer() {
        if (m_latestFanViewedPlayerId)
            return m_field_100.find(m_latestFanViewedPlayerId);
        return nullptr;
    }
    void increaseEncryptionDecodeError() {
#ifndef ZWIFT_FAST_STAT
        std::lock_guard l(m_mutex);
        m_encrDecodeErrs++;
#endif
    }
    void increaseEncryptionEncodeError() {
#ifndef ZWIFT_FAST_STAT
        std::lock_guard l(m_mutex);
        m_encrEncodeErrs++;
#endif
    }
    void increaseEncryptionNewConnectionError() {
#ifndef ZWIFT_FAST_STAT
        std::lock_guard l(m_mutex);
        m_encrNewConnectionErr++;
#endif
    }
    void increaseEncryptionWrongServer() {
#ifndef ZWIFT_FAST_STAT
        std::lock_guard l(m_mutex);
        m_encrWrongServer++;
#endif
    }
    void increaseEncryptionWrongServerIpV6() {
#ifndef ZWIFT_FAST_STAT
        std::lock_guard l(m_mutex);
        m_encrWrongServer6++;
#endif
    }
    void increaseParseErrorCount() {
#ifndef ZWIFT_FAST_STAT
        std::lock_guard l(m_mutex);
        m_parseErrorCount++;
#endif
    }
    void increaseSendErrorCount() {
#ifndef ZWIFT_FAST_STAT
        std::lock_guard l(m_mutex);
        m_sendErrorCount++;
#endif
    }
    void incrementConnectionErrorCount() {
#ifndef ZWIFT_FAST_STAT
        std::lock_guard l(m_mutex);
        m_connErrCnt1++;
        m_connErrCnt2++;
#endif
    }
    void incrementConnectionTimeoutCount() {
#ifndef ZWIFT_FAST_STAT
        std::lock_guard l(m_mutex);
        m_connToCnt1++;
        m_connToCnt2++;
#endif
    }
    void incrementExpungeReasonReceived() {
#ifndef ZWIFT_FAST_STAT
        std::lock_guard l(m_mutex);
        m_expungesCnt++;
#endif
    }
    void incrementGameInterpolationTimeoutCount() {
#ifndef ZWIFT_FAST_STAT
        std::lock_guard l(m_mutex);
        m_interpToCnt1++;
        m_interpToCnt2++;
#endif
    }
    void setEncryptionEnabled(bool en) {
#ifndef ZWIFT_FAST_STAT
        std::lock_guard l(m_mutex);
        m_encrEnabled = en;
#endif
    }
    void registerPlayerStateInfo(const protobuf::PlayerState &pst) {
        auto id = pst.id();
        auto f = m_field_100.add(id);
        if (id != f->m_id || pst.world_time() > f->m_born) {
            f->m_id = id;
            f->m_born = pst.world_time();
            if (pst.has_x() && pst.has_z()) {
                f->m_x = pst.x();
                f->m_z = pst.z();
            }
        }
    }
    void registerServerToClientQueueStatistics(int64_t waitInQueueMs, uint64_t qsize) {
#ifndef ZWIFT_FAST_STAT
        std::lock_guard l(m_mutex);
        m_field_3D8 += waitInQueueMs;
        ++m_field_3F0;
        if (waitInQueueMs > m_field_3E8)
            m_field_3E8 = waitInQueueMs;
        if (waitInQueueMs < m_field_3E0)
            m_field_3E0 = waitInQueueMs;
        m_field_3F8 += qsize;
        ++m_field_410;
        if (qsize > m_field_408)
            m_field_408 = qsize;
        if (qsize < m_field_400)
            m_field_400 = qsize;
#endif
    }
    uint64_t m_packets = 0; //Ursoft simplification
    void registerReceivedDatagram(uint32_t seqno, const std::string &host) {
#ifndef ZWIFT_FAST_STAT
        if (host != m_curHost) { // inlined UdpStatistics::resetServerToClientSequenceNumber
            m_field_C8 += m_field_C0 - m_field_C4;
            m_field_CC += m_field_88;
            m_field_78.clear();
            m_field_C0 = m_field_C4 = 0;
            auto v12 = m_field_E0 - m_field_430;
            if (v12) {
                m_field_430 = m_field_E0;
                m_field_E4 += m_field_448 - m_field_44C - v12 + 1;
            }
            m_field_448 = m_field_44C = 0;
            m_curHost = host;
        }
        uint32_t v14;
        if (m_field_C0) {
            v14 = m_field_C4;
        } else {
            v14 = seqno - 1;
            m_field_C0 = seqno - 1;
            m_field_C4 = seqno - 1;
        }
        if (seqno > v14 && m_field_78.insert(seqno).second) {
            if (seqno >= m_field_C0)
                m_field_C0 = seqno;
            else
                ++m_field_BC;
            if (seqno > m_field_448) {
                m_field_448 = seqno;
                if (m_field_44C == 0)
                    m_field_44C = seqno;
            }
            ++m_field_E0;
        } else {
            ++m_field_B8;
        }
#endif
    }
        /*TODO UdpStatistics::FanViewStats::toJson(), Sample::encryptionToJson(), Sample::toJson() - human readable field names
OMIT getConnectionMetrics(zwift_network::ConnectivityInfo &), getSample(), setSampleInterval(std::chrono::duration<long long,std::ratio<1l,1l>>), setInnermostDistance(uint)
reset(void) - not used
inlined:
resetFields(void)
resetFanViewFields(void)
registerTripDuration(protobuf::PlayerState const&,ulong)
registerSeenPlayers(protobuf::PlayerState const&)
registerFanViewLatestPlayerStateInfo(protobuf::PlayerState const&)
registerDatagramWindowSize(void)
isInsideInnermostRing(protobuf::PlayerState const&,ulong)
getParseErrorCount(void)
getInnermostSquaredDistance(void)
computeFanView(void)
computeDatagramLostPercentage(void)
computeDatagramOutOfOrderPercentage(void)
*/
};
struct UdpClient : public WorldAttributeServiceListener, UdpConfigListener, EncryptionListener { //0x1400-16 bytes
    GlobalState *m_gs;
    WorldClockService *m_wcs;
    HashSeedService *m_hs1, *m_hs2;
    UdpStatistics *m_us;
    RelayServerRestInvoker *m_relay;
    NetworkClientImpl *m_netImpl;
    EventLoop m_eventLoop;
    RelayAddressService m_ras;
    EncryptionInfo m_ei;
    protocol_encryption::UdpRelayClientCodec m_codec;
    boost::asio::steady_timer m_connectionTimeoutTimer, m_gameInterpolationTimeoutTimer, m_metricsLogTimer;
    boost::asio::ip::udp::socket m_udpSocket;
    std::string m_host, m_chkHost, m_strError;
    uint8_t m_rxBuf[1492] = {}, m_txBuf[1492] = {};
    boost::asio::ip::udp::endpoint m_senderEndpoint, m_connEndpoint;
    moodycamel::ReaderWriterQueue<std::pair<std::shared_ptr<protobuf::ServerToClient>, uint64_t>> m_stcs;
    moodycamel::ReaderWriterQueue<uint64_t> m_updatedIds;
    std::map<uint64_t, uint64_t> m_field_FC0;
    std::vector<uint8_t> m_decVector, m_encVector;
    PeriodicLogger m_periodicLogger;
    std::future<NetworkResponse<protobuf::PlayerState>> m_field_1210;
    uint64_t m_playerProfileUpdated = 0, m_field_F30 = 0;
    int64_t m_world = 0, m_player_id = 0, m_VOD_playerId = 0, m_field_11F8 = 0, m_field_1200 = 0;
    int m_field_1208 = 0; // enum?
    int m_connTimeout, m_to2, m_bigConnTo = 30, m_maxConnSmallTo = 3, m_stcRx = 0, m_rxError = 0, m_ctsTx = 0, m_txError = 0, m_reconnectionAttempts = 0, m_connectionTimeouts = 0, m_map = 0, m_ctsSeqno = 1;
    float m_VOD_x = 0.0f, m_VOD_z = 0.0f, m_field_11E0 = 0.0f, m_field_11E4 = 0.0f, m_x = 0.0f, m_z = 0.0f;
    protobuf::ExpungeReason m_expungeReason = protobuf::ExpungeReason::NOT_EXPUNGED;
    uint16_t m_port = 0, m_chkPort = 0;
    bool m_field_15C = false, m_useEncr, m_udpReceiving = false, m_shutDown = false, m_field_F20 = false;
    UdpClient(GlobalState *gs, WorldClockService *wcs, HashSeedService *hs1, HashSeedService *hs2, UdpStatistics *us, RelayServerRestInvoker *relay, NetworkClientImpl /*UdpClient::Listener*/ *netImpl, int connTimeout = 10, int to2 = 2000) :
        m_gs(gs), m_wcs(wcs), m_hs1(hs1), m_hs2(hs2), m_us(us), m_relay(relay), m_netImpl(netImpl), m_connectionTimeoutTimer(m_eventLoop.m_asioCtx), m_gameInterpolationTimeoutTimer(m_eventLoop.m_asioCtx), m_metricsLogTimer(m_eventLoop.m_asioCtx), 
        m_udpSocket(m_eventLoop.m_asioCtx), m_periodicLogger(m_eventLoop.m_asioCtx, 10000, [](uint32_t a2, uint64_t dur) {
            if (dur)
                NetworkingLogWarn("Received %d datagrams about another world in the last %ds", a2, dur / 1000);
            else
                NetworkingLogWarn("Received datagram about another world");
        }), m_connTimeout(connTimeout), m_to2(to2) {
        resetMetricsLogTimer();
        m_useEncr = m_gs->shouldUseEncryption();
        if (m_useEncr) {
            m_ei = m_gs->getEncryptionInfo();
            m_codec.m_hostRelayId = m_ei.m_relaySessionId;
            m_codec.m_generateKey = false;
            assert(m_ei.m_sk.length() == 16);
            memmove(m_codec.m_secretRaw, m_ei.m_sk.c_str(), m_ei.m_sk.length());
            m_codec.secretRawToString();
        }
    }
    void resetMetricsLogTimer() {
        m_metricsLogTimer.expires_after(std::chrono::seconds(60));
        m_metricsLogTimer.async_wait([this](const boost::system::error_code &ec) {
            if (!ec) {
                NetworkingLogInfo("UDP metrics {StC Rx: %d, Rx error: %d, CtS Tx: %d, Tx error: %d}", m_stcRx, m_rxError, m_ctsTx, m_txError);
                m_stcRx = m_rxError = m_ctsTx = m_txError = 0;
                resetMetricsLogTimer();
            }
        });
    }
    void handleEncryptionChange(const EncryptionInfo &ei) override {
        //TODO - looks like empty in PC build
    }
    void handleUdpConfigChange(const protobuf::UdpConfigVOD &uc, uint64_t time) override {
        m_eventLoop.post([this, uc, time]() {
            m_VOD_playerId = uc.player_id();
            m_VOD_x = uc.x();
            m_VOD_z = uc.z();
            m_ras.updateAddresses(uc);
            if (m_field_1208 == 2)
                m_field_1208 = 0;
            handleRelayAddress(time);
        });
    }
    void handleWorldAttribute(const protobuf::WorldAttribute &wa) override {
        if (wa.wa_type() == protobuf::WAT_RQ_PROF)
            m_playerProfileUpdated = wa.world_time_born();
    }
    void shutdown() {
        m_eventLoop.post([this]() {
            m_shutDown = true;
            m_connectionTimeoutTimer.cancel();
            m_gameInterpolationTimeoutTimer.cancel();
            m_metricsLogTimer.cancel();
            disconnect();
            clearEndpoint();
        });
        m_eventLoop.enqueueShutdown();
    }
    void clearEndpoint() {
        m_host.clear();
        m_port = 0;
        //OMIT m_connEndpoint = 0;
    }
    void connect() {
        if (!m_codec.initialize(&m_strError)) {
            m_us->increaseEncryptionNewConnectionError();
            NetworkingLogError("UDP failed to get new connection id [%s].", m_strError.c_str());
        }
        m_chkHost = m_host;
        m_chkPort = m_port;
        NetworkingLogInfo("Connecting to UDP server...");
        boost::system::error_code ec;
        m_udpSocket.connect(m_connEndpoint, ec);
        if (ec) {
            NetworkingLogError("Error connecting to UDP host %s:%d", m_host.c_str(), m_port);
        } else {
            resetConnectionTimeoutTimer(m_connTimeout);
            resetGameInterpolationTimeoutTimer();
            receive();
        }
    }
    void disconnect() {
        if (m_udpSocket.is_open()) {
            boost::system::error_code ec;
            m_udpSocket.shutdown(m_udpSocket.shutdown_both, ec);
            if (ec)
                NetworkingLogWarn("Error shutting down UDP socket [%d] %s", ec.value(), ec.message().c_str());
            m_udpSocket.close(ec);
            if (ec)
                NetworkingLogWarn("Error closing UDP socket [%d] %s", ec.value(), ec.message().c_str());
        }
    }
    bool resolveAndSetEndpoint(const std::string &host, uint16_t port) {
        boost::asio::ip::udp::resolver resolver(m_eventLoop.m_asioCtx);
        boost::asio::ip::udp::resolver::query query(host, std::to_string(port));
        boost::system::error_code ec;
        boost::asio::ip::udp::resolver::iterator iter = resolver.resolve(query, ec);
        if (ec) {
            NetworkingLogError("Error resolving UDP host %s:%d [%d] %s", host.c_str(), port, ec.value(), ec.message().c_str());
            clearEndpoint();
            return false;
        }
        m_host = host;
        m_port = port;
        m_us->setEncryptionEnabled(m_useEncr);
        NetworkingLogInfo("UDP host %s:%d%s", m_host.c_str(), m_port, m_useEncr ? " (secure)" : "");
        m_connEndpoint = *iter;
        return true;
    }
    HostAndPorts selectHostAndPorts(uint64_t time) {
        if (time || m_field_F20) {
            auto fid = m_us->getLatestFanViewedPlayerId();
            auto info = m_us->getLatestInfoFromFanViewedPlayer();
            if (m_field_15C && m_field_1208 != 2 && fid && (!info || info->isAged(time))) {
                if (!m_field_1208) {
                    m_field_1210 = m_relay->latestPlayerState(m_world, fid);
                    m_field_1208 = 1;
                }
                m_field_11F8 = 0;
                HostAndPorts ret;
                ret.m_ports[0] = m_port;
                ret.m_ports[1] = m_port;
                ret.m_host = m_host;
                return ret;
            } else {
                m_field_11F8 = fid;
                float x, z;
                if (!info)
                    fid = m_player_id;
                if (m_VOD_playerId == fid) {
                    x = m_VOD_x;
                    z = m_VOD_z;
                } else if (info) {
                    x = info->m_x;
                    z = info->m_z;
                } else {
                    x = m_field_11E0;
                    z = m_field_11E4;
                }
                return m_ras.getAddress(m_world, m_map, x, z, &m_field_15C);
            }
        }
        return m_ras.getRandomAddress(m_world, m_map, m_player_id, &m_field_15C);
    }
    void resetConnectionTimeoutTimer(int to) {
        m_connectionTimeoutTimer.expires_after(std::chrono::seconds(to));
        m_connectionTimeoutTimer.async_wait([this](const boost::system::error_code &ec) {
            if (!ec) //onConnectionTimeout inlined
                reconnect(true);
        });
    }
    void resetGameInterpolationTimeoutTimer() {
        m_gameInterpolationTimeoutTimer.expires_after(std::chrono::seconds(5));
        m_gameInterpolationTimeoutTimer.async_wait([this](const boost::system::error_code &ec) {
            if (!ec && m_world > 0 && !m_field_F20)
                m_us->incrementGameInterpolationTimeoutCount(); 
        });
    }
    void reconnect(bool isTimedOut) {
        if (m_world > 0 && !m_field_F20) {
            ++m_reconnectionAttempts;
            if (isTimedOut) {
                m_us->incrementConnectionTimeoutCount();
                ++m_connectionTimeouts;
                NetworkingLogWarn("UDP connection timeout (%d so far), reconnection attempt %d", m_connectionTimeouts, m_reconnectionAttempts);
            } else {
                m_us->incrementConnectionErrorCount();
                NetworkingLogWarn("UDP communication error, reconnection attempt %d", m_reconnectionAttempts);
            }
            disconnect();
            connect();
        }
        if (m_reconnectionAttempts >= m_maxConnSmallTo)
            resetConnectionTimeoutTimer(m_bigConnTo);
        else
            resetConnectionTimeoutTimer(m_connTimeout);
    }
    void handleWorldAndMapRevisionChanges(uint64_t time, int64_t world, uint32_t map);
    void handleRelayAddress(uint64_t time) {
        auto v10 = selectHostAndPorts(time);
        auto port = m_useEncr ? v10.m_ports[0] : v10.m_ports[1];
        if (v10.m_host != m_host || port != m_port) {
            if (resolveAndSetEndpoint(v10.m_host, port)) {
                disconnect();
                connect();
            } else {
                disconnect();
            }
        }
    }
    void readCallback(const boost::system::error_code &err, std::size_t count) {
        if (!err) {
            resetConnectionTimeoutTimer(m_connTimeout);
            resetGameInterpolationTimeoutTimer();
            auto v13 = decodeMessage(m_rxBuf, &count);
            if (v13) { // UdpClient::processDatagram inlined
                auto stc = std::make_shared<protobuf::ServerToClient>();
                if (count && stc->ParseFromArray(v13, int(count))) {
                    ++m_stcRx;
                    handleServerToClient(stc);
                } else {
                    NetworkingLogError("Failed to parse UDP StC (%d bytes)", count);
                    m_us->increaseParseErrorCount();
                }
            }
            m_udpReceiving = false;
            receive();
        } else {
            if (err.category() == boost::asio::error::system_category && err.value() == ERROR_OPERATION_ABORTED) {
                m_udpReceiving = false;
                receive();
            } else {
                NetworkingLogError("Error receiving UDP datagram [%d] %s", err.value(), err.message().c_str());
                ++m_rxError;
                m_udpReceiving = false;
                reconnect(false);
            }
        }
    }
    void receive() {
        if (!m_udpReceiving && m_shutDown) {
            m_udpReceiving = true;
            m_udpSocket.async_receive_from(boost::asio::buffer(m_rxBuf, sizeof(m_rxBuf)), m_senderEndpoint, 0, std::bind(&UdpClient::readCallback, this, std::placeholders::_1, std::placeholders::_2));
        }
    }
    void handleServerToClient(const std::shared_ptr<protobuf::ServerToClient> &src);
    const uint8_t *decodeMessage(uint8_t *buf, uint64_t *pCount) {
        if (!m_useEncr)
            return buf;
        m_decVector.clear();
        if (m_codec.decode(buf, *pCount, &m_decVector, &m_strError)) {
            *pCount = m_decVector.size();
            return m_decVector.data();
        } else {
            auto sip = m_senderEndpoint.address().to_string();
            if (m_senderEndpoint.port() == m_chkPort && sip == m_chkHost) {
                m_us->increaseEncryptionDecodeError();
                NetworkingLogError("Failed to decode UDP StC [%s].", m_strError.c_str());
                return buf;
            } else {
                if (m_senderEndpoint.address().is_v6()) {
                    NetworkingLogDebug(
                        "Ignore UDP StC from wrong IPv6 %s:%d (expect %s:%d)",
                        sip.c_str(),
                        m_senderEndpoint.port(),
                        m_chkHost.c_str(),
                        m_chkPort);
                    m_us->increaseEncryptionWrongServerIpV6();
                    return nullptr;
                } else {
                    NetworkingLogDebug(
                        "Ignore UDP StC from wrong %s:%d (expect %s:%d)",
                        sip.c_str(), m_senderEndpoint.port(), m_chkHost.c_str(), m_chkPort);
                    m_us->increaseEncryptionWrongServer();
                    return nullptr;
                }
            }
        }
    }
    void handleFanViewedPlayerChanges(uint64_t time, const protobuf::PlayerState &ps) {
        if (this->m_field_15C) {
            auto LatestFanViewedPlayerId = m_us->getLatestFanViewedPlayerId();
            if (LatestFanViewedPlayerId != m_field_11F8) {
                if (LatestFanViewedPlayerId != m_field_1200) {
                    m_field_1200 = LatestFanViewedPlayerId;
                    m_field_1208 = 0;
                    /* OMIT telemetry if (ps.id() == ps.watching_rider_id())
                        (*(void(__fastcall **)(_DWORD *, _QWORD, __int64, const char *, const char *))(*(_QWORD *)m_ts + 8i64))(
                            m_ts,
                            0i64,
                            3i64,
                            "fanview",
                            "Back to me");
                    else
                        TelemetryService::remoteLogF(
                            m_ts,
                            0,
                            3u,
                            (__int64)"fanview",
                            "Watching Player ID %lli",
                            ps->m_watching_rider_id);*/
                }
                if (m_field_1208 == 1 && m_field_1210.valid() && is_ready(m_field_1210)) {
                    auto result = m_field_1210.get();
                    if (result.m_errCode) {
                        NetworkingLogError("Error requesting fan viewed player state: [%d] %s", result.m_errCode, result.m_msg.c_str());
                    } else {
                        m_us->registerPlayerStateInfo(result.m_T);
                        auto sh_stc = std::make_shared<protobuf::ServerToClient>();
                        auto states = sh_stc->mutable_states1();
                        states->Add(std::move(result.m_T));
                        m_stcs.emplace(sh_stc, g_steadyClock.now());
                    }
                }
                handleRelayAddress(time);
            }
        }
    }
    bool popServerToClient(std::shared_ptr<protobuf::ServerToClient> &dest) {
        bool ret = false;
        std::pair <std::shared_ptr<protobuf::ServerToClient>, uint64_t> pop;
        if (m_stcs.try_dequeue(pop)) {
            dest = pop.first;
#ifndef ZWIFT_FAST_STAT
            auto v8 = g_steadyClock.now();
            m_us->registerServerToClientQueueStatistics((v8 - pop.second) / 1000000, m_stcs.size_approx());
#endif
            ret = true;
        }
        return ret;
    }
    void sendClientToServer(const protobuf::ClientToServer &cts, uint32_t map) {
        m_eventLoop.post([this, cts, map]() {
            auto worldTime = this->m_wcs->getWorldTime();
            this->m_player_id = cts.player_id();
            auto &state = cts.has_state() ? cts.state() : protobuf::PlayerState::default_instance();
            auto watching_rider_id = state.watching_rider_id();
            if (watching_rider_id || watching_rider_id == state.id()) { //UdpClient::handlePositionChanges
                if (PlayerStateHelper::hasPosition(state)) {
                    this->m_x = state.x();
                    this->m_z = state.z();
                }
            }
            this->handleWorldAndMapRevisionChanges(worldTime, cts.server_realm(), map);
            uint32_t ctsLen = 0;
            if (NRO_OK == this->serializeToUdpMessage(cts, &ctsLen)) {
                this->m_us->inspectClientToServer(cts, worldTime);
                this->handleFanViewedPlayerChanges(worldTime, state);
                if (this->m_useEncr) { // UdpClient::encodeMessage
                    this->m_encVector.clear();
                    if (!this->m_codec.encode(this->m_txBuf, ctsLen, &this->m_encVector, &this->m_strError)) { // protocol_encryption::Codec::encode
                        this->m_us->increaseEncryptionEncodeError();
                        NetworkingLogError("Failed to encode UDP CtS [%s].", this->m_strError.c_str());
                        return;
                    }
                    ctsLen = uint32_t(this->m_encVector.size());
                    if (ctsLen > sizeof(m_txBuf))
                        return;
                    memmove(this->m_txBuf, &this->m_encVector[0], ctsLen); //TODO: single buffer is faster
                }
                if (cts.server_realm() > 0)
                    this->m_wcs->storeSequenceNumberSendTime(cts.seqno());
                boost::system::error_code ec;
                m_udpSocket.send(boost::asio::buffer(this->m_txBuf, ctsLen), boost::asio::socket_base::message_flags(), ec);
                if (ec) {
                    this->m_us->increaseSendErrorCount();
                    ++this->m_txError;
                    NetworkingLogError("Failed to send UDP CtS: %s [%d]", ec.message().c_str(), ec.value());
                } else {
                    ++this->m_ctsTx;
                }
            }
        });
    }
    NetworkRequestOutcome serializeToUdpMessage(const protobuf::ClientToServer &cts, uint32_t *len) {
        auto payloadLen = uint32_t(cts.ByteSizeLong());
        *len = payloadLen + 4;
        bool shouldPrependDontForwardByte = false;
        uint8_t *ptxBuf = m_txBuf;
        if (0 == cts.server_realm()) {
            shouldPrependDontForwardByte = true;
        } else {
            shouldPrependDontForwardByte = cts.has_state() && cts.state().just_watching();
        }
        if (shouldPrependDontForwardByte) {
            *ptxBuf++ = 0xDF;
            ++(*len);
        } else if (this->m_field_15C && cts.player_id() > 0) { //shouldPrependVoronoiOrDieByte
            m_txBuf[0] = 6;
            memmove(m_txBuf + 1, &m_player_id, 4);
            ptxBuf = &m_txBuf[5];
            (*len) += 5;
        }
        if (*len <= sizeof(m_txBuf)) {
            if (cts.SerializeToArray(ptxBuf, payloadLen)) {
                auto v14 = (cts.player_id() >> 63) & 0x10;
                return (v14 ? m_hs2 : m_hs1)->signMessage(ptxBuf, payloadLen, cts.world_time());
            } else {
                NetworkingLogError("Failed to serialize UDP CtS");
                return NRO_PROTOBUF_FAILURE_TO_ENCODE;
            }
        } else {
            NetworkingLogError("UDP message is too big");
            return NRO_CLIENT_TO_SERVER_IS_TOO_BIG;
        }
    }
    void setPlayerProfileUpdated(uint64_t time) { m_playerProfileUpdated = time; }
    NetworkRequestOutcome sendRegularClientToServer(int64_t realm, uint32_t map, const protobuf::PlayerState &pst) {
        NetworkRequestOutcome ret;
        protobuf::ClientToServer cts; // [rsp+30h] [rbp-69h] BYREF
        cts.set_player_id(pst.id());
        cts.set_server_realm(realm);
        cts.set_seqno(m_ctsSeqno++);
        if (m_wcs->isInitialized()) {
            auto destState = cts.mutable_state();
            destState->CopyFrom(pst);
            auto wt = m_wcs->getWorldTime();
            cts.set_world_time(wt);
            if (!destState->has_world_time())
                destState->set_world_time(wt);
            if (m_playerProfileUpdated)
                destState->set_customization_id(m_playerProfileUpdated);
            sendClientToServer(cts, map);
            ret = NRO_OK;
        } else {
            cts.set_world_time(0);
            sendClientToServer(cts, map);
            ret = NRO_WORLD_CLOCK_NOT_INITIALIZED_YET;
        }
        return ret;
    }
    NetworkRequestOutcome sendPlayerState(int64_t realm, const protobuf::PlayerState &pst) {
        if (realm < 0)
            return NRO_INVALID_WORLD_ID;
        if (!realm)
            return NRO_NO_WORLD_SELECTED;
        bool succ = false;
        auto MapRevisionId = PlayerStateHelper::getMapRevisionId(pst, &succ);
        if (!succ || !MapRevisionId)
            return NRO_NO_MAP_REVISION_ID_SELECTED;
        if (!m_udpSocket.is_open())
            return NRO_UDP_CLIENT_STOPPED;
        if (m_field_F20) {
            auto now = g_steadyClock.now();// UdpClient::sendDisconnectedClientToServer inlined
            if (now < m_field_F30)
                return NRO_DISCONNECTED_DUE_TO_SIMULTANEOUS_LOGINS;
            m_field_F30 = now + 1000000 * m_to2;
            protobuf::ClientToServer cts;
            cts.set_player_id(pst.id());
            cts.set_world_time(0);
            cts.set_server_realm(-1);
            sendClientToServer(cts, m_map);
            return NRO_DISCONNECTED_DUE_TO_SIMULTANEOUS_LOGINS;
        } else {
            switch (m_expungeReason) {
            default: //case protobuf::NOT_EXPUNGED:
                m_expungeReason = protobuf::NOT_EXPUNGED;
                return sendRegularClientToServer(realm, MapRevisionId, pst);
            case protobuf::WORLD_FULL:
                m_expungeReason = protobuf::NOT_EXPUNGED;
                return NRO_WORLD_FULL;
            case protobuf::ROADS_CLOSED:
                m_expungeReason = protobuf::NOT_EXPUNGED;
                return NRO_ROADS_CLOSED;
            }
        }
    }
    ~UdpClient() { shutdown(); }
};
struct WorldAttributeService { //0x270 bytes
    moodycamel::ReaderWriterQueue<protobuf::WorldAttribute> m_rwq;
    ServiceListeners<WorldAttributeServiceListener> m_lis;
    int64_t m_largestTime;
    WorldAttributeService() : m_rwq(100), m_largestTime(0) {
        /*TODO
          *(_QWORD *)&this->field_80[72] = 0i64;
          v4 = operator new(0x20ui64);
          *v4 = v4;
          v4[1] = v4;
          *(_QWORD *)&this->field_80[64] = v4;*/
    }
    void registerListener(WorldAttributeServiceListener *lis) { m_lis += lis; }
    void removeListener(WorldAttributeServiceListener *lis) { m_lis -= lis; }
    int64_t getLargestWorldAttributeTimestamp() { return m_largestTime; }
    void logWorldAttribute(const protobuf::WorldAttribute &wa, bool bDiscarded) {
        NetworkingLogDebug("%s [type=%d, ts=%llu, world=%lld, mapRev=%u, player=%lld xyz=(%d,%d,%d)]", 
            bDiscarded ? "Discarded world attribute" : "Received world attribute",
            (int)wa.wa_type(), wa.timestamp(), wa.server_realm(), wa.map_rev(),
            wa.rel_id(), wa.x(), wa.y_altitude(), wa.z());
        //OMIT if (bDiscarded)
        //    WorldAttributeStatistics::increaseDiscardedWa(m_stat, *((_DWORD *)wa + 12));
    }
    void handleServerToClient(const protobuf::ServerToClient &stc) {
        for (auto &wat : stc.updates()) {
            if (wat.has_timestamp() && wat.timestamp() > m_largestTime) {
                logWorldAttribute(wat, false);
                m_rwq.enqueue(wat);
                m_largestTime = wat.timestamp();
                m_lis.notify([this, wat](WorldAttributeServiceListener &wasl) {
                    wasl.handleWorldAttribute(wat);
                });
            } else {
                logWorldAttribute(wat, true);
            }
        }
    }
    bool popWorldAttribute(protobuf::WorldAttribute &dest) { return m_rwq.try_dequeue(dest); }
};
struct ProfileRequestDebouncer : public ProfileRequestLazyContext::PlayerIdProvider { //0x1E0 bytes
    std::mutex m_mapPromisesMtx, m_cpromMtx;
    EventLoop *m_eventLoop;
    RestServerRestInvoker *m_rest;
    boost::asio::steady_timer m_timer;
    using NR = NetworkResponse<protobuf::PlayerProfile>;
    using NRS = NetworkResponse<protobuf::PlayerProfiles>;
    using PP = std::promise<NR>;
    using PF = std::future<NR>;
    using PPS = std::unordered_multimap<int64_t, PP>;
    PPS m_mapPromises;
    struct RequestContext {
        FutureWaiter<NRS> m_waiter;
        PPS m_multimap;
        RequestContext(boost::asio::io_context &ctx) : m_waiter(ctx) {}
    };
    using CPS = std::unordered_map<uint32_t, RequestContext>;
    CPS m_contextProms;
    int m_rqCounter = 0;
    uint32_t m_cntLimit;
    bool m_dpStarted = false;
    ProfileRequestDebouncer(EventLoop *el, RestServerRestInvoker *rest, uint32_t cntLimit) : m_eventLoop(el), m_rest(rest), m_timer(el->m_asioCtx), m_cntLimit(cntLimit) { m_contextProms.reserve(16); }
    ~ProfileRequestDebouncer() /* vptr[0] */ {
        m_timer.cancel();
        NetworkResponse<protobuf::PlayerProfile> abortValue{ "Request aborted"s, NRO_REQUEST_ABORTED };
        for (auto &i : m_mapPromises)
            i.second.set_value(abortValue);
        for (auto &i : m_contextProms)
            for (auto &j : i.second.m_multimap)
                j.second.set_value(abortValue);
    }
    PF addRequest(int64_t playerId) {
        PP prom;
        PF ret = prom.get_future();
        {
            std::lock_guard l(m_mapPromisesMtx);
            m_mapPromises.emplace(playerId, std::move(prom));
            if (m_mapPromises.size() < 26) {
                if (!m_dpStarted)
                    startDebouncePeriod();
            } else {
                m_timer.cancel();
                requestProfiles();
            }
        }
        return ret;
    }
    std::unordered_set<int64_t> getPlayerIds(uint32_t key) override /* vptr[1] */ {
        std::unordered_set<int64_t> ret;
        auto contextPromises = getContextPromises(key);
        if (contextPromises) {
            std::lock_guard l(m_cpromMtx);
            {
                m_dpStarted = false;
                contextPromises->swap(m_mapPromises);
            }
            uint32_t cnt = 0;
            for (auto &i : *contextPromises) {
                ret.insert(i.first);
                if (++cnt >= m_cntLimit)
                    break;
            }
        }
        return ret;
    }
//protected:
    PPS *getContextPromises(uint32_t key) {
        std::lock_guard l(m_cpromMtx);
        auto it = m_contextProms.find(key);
        if (it == m_contextProms.end())
            return nullptr;
        return &it->second.m_multimap;
    }
    void onProfilesReceived(uint32_t key, const NRS &nr) {
        auto contextPromises = getContextPromises(key);
        if (!contextPromises)
            return;
        if (nr.m_errCode == NRO_HTTP_STATUS_NOT_FOUND) {
            NetworkResponse<protobuf::PlayerProfile> nf{"Profile not found"s, NRO_HTTP_STATUS_NOT_FOUND};
            for (auto &i : *contextPromises)
                i.second.set_value(nf);
            contextPromises->clear();
        } else if (nr.m_errCode == NRO_OK) {
            for (auto &i : nr.m_T.profiles()) {
                auto id = i.id();
                auto f = contextPromises->equal_range(id);
                for (auto &j = f.first; j != f.second; ++j) {
                    NetworkResponse<protobuf::PlayerProfile> ret;
                    ret.m_T.CopyFrom(i);
                    j->second.set_value(std::move(ret));
                }
            }
        }
        returnProfilePromises(contextPromises);
        m_contextProms.erase(key);
    }
    void startDebouncePeriod() {
        m_dpStarted = true;
        m_timer.expires_after(std::chrono::milliseconds(200));
        m_timer.async_wait([this](const boost::system::error_code &ec) {
            if (!ec)
                this->requestProfiles();
        });
    }
    void requestProfiles() {
        uint32_t key;
        RequestContext *ctx;
        {
            std::lock_guard l(m_cpromMtx);
            key = m_rqCounter++;
            auto inserted = m_contextProms.emplace(key, m_eventLoop->m_asioCtx).first;
            ctx = &inserted->second;
        }
        ProfileRequestLazyContext lctx(key, this);
        ctx->m_waiter.waitAsync(m_rest->profilesByPlayerIds(lctx), 50, [this, key](const NetworkResponse<protobuf::PlayerProfiles> &p) {
            this->onProfilesReceived(key, p);
        });
    }
    void returnProfilePromises(PPS *pSrc) {
        std::lock_guard l(m_mapPromisesMtx);
        m_mapPromises.swap(*pSrc);
        if (!m_dpStarted)
            startDebouncePeriod();
    }
};
struct NetworkClockService { //0x18 bytes
    uint64_t m_localCreTime, m_netCreTime;
    NetworkClockService(uint64_t netTime) : m_localCreTime(g_steadyClock.now()), m_netCreTime(netTime) {}
    uint64_t getNetworkTimeInSeconds() {
        return (g_steadyClock.now() - m_localCreTime) / 1000000000 + m_netCreTime;
    }
};
struct AuxiliaryControllerAddress { //80 bytes
    std::string m_localIp;
    std::vector<uint8_t> m_key;
    int m_localPort = 0, m_localCPort = 0;
    protobuf::IPProtocol m_proto = protobuf::TCP;
    AuxiliaryControllerAddress(const std::string &localIp, uint32_t localPort, protobuf::IPProtocol protocol, uint32_t localCPort, const std::vector<uint8_t> &key) :
        m_localIp(localIp), m_key(key), m_localPort(localPort), m_localCPort(localCPort), m_proto(protocol) {}
    AuxiliaryControllerAddress() {}
};
struct ActivityRecommendationRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    ActivityRecommendationRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server + "/api/recommendations"s) {}
    std::future<NetworkResponse<Json::Value>> getActivityRecommendations(const std::string &aGoal) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<Json::Value>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            QueryStringBuilder qsb;
            qsb.add("goal"s, aGoal);
            std::string url(m_url + "/recommendation"s + qsb.getString(true));
            auto v9 = conn->performGet(url, AcceptHeader(ATH_JSON), "Get Activity Recommendations"s);
            return HttpHelper::convertToJsonResponse(v9);
        }), true, false);
    }
};
struct AchievementsRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    AchievementsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server) {}
    std::future<NetworkResponse<protobuf::Achievements>> load() {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::Achievements>(CurlHttpConnection *)>([this](CurlHttpConnection *conn) {
            std::string url(m_url + "/api/achievement/loadPlayerAchievements"s);
            auto v9 = conn->performGet(url, AcceptHeader(ATH_PB), "Load Achievements"s);
            return HttpHelper::convertToResultResponse<protobuf::Achievements>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<void>> unlock(const  protobuf::AchievementUnlockRequest &rq) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string hash("/api/achievement/unlock"s), url(m_url + hash);
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, rq);
            auto v9 = conn->performPostWithHash(url, ContentTypeHeader(CTH_PB), payload, AcceptHeader(), "Unlock Achievements"s, hash);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
};
struct CampaignRestInvoker { //0x70 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url1, m_url2, m_url3;
    CampaignRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url1(server + "/api/profiles"s), 
        m_url2(server + "/api/campaign/proto/campaigns"s), m_url3(server + "/api/campaign/public/proto/campaigns"s) {}
    std::future<NetworkResponse<protobuf::CampaignRegistrationResponse>> enrollPlayer(const std::string &sn) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::CampaignRegistrationResponse>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performPost(m_url2 + "/shortName/"s + sn + "/enroll"s, AcceptHeader(ATH_PB), "Campaign service enroll player"s);
            return HttpHelper::convertToResultResponse<protobuf::CampaignRegistrationResponse>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::ListPublicActiveCampaignResponse>> getActiveCampaigns() {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::ListPublicActiveCampaignResponse>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(m_url3 + "/active"s, AcceptHeader(ATH_PB), "Campaign service get active campaigns"s);
            return HttpHelper::convertToResultResponse<protobuf::ListPublicActiveCampaignResponse>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::ListCampaignRegistrationSummaryResponse>> getCampaigns() {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::ListCampaignRegistrationSummaryResponse>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(m_url2, AcceptHeader(ATH_PB), "Campaign service get campaigns"s);
            return HttpHelper::convertToResultResponse<protobuf::ListCampaignRegistrationSummaryResponse>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::CampaignRegistrationResponse>> getRegistration(const std::string &sn) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::CampaignRegistrationResponse>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(m_url2 + "/shortName/"s + sn + "/registration"s, AcceptHeader(ATH_PB), "Campaign service get campaigns"s);
            return HttpHelper::convertToResultResponse<protobuf::CampaignRegistrationResponse>(v9);
        }), true, false);
    }
    /*PC:absent:
enrollPlayer(int64_t,const std::string &)
getCompletedCampaigns()
getProgress(const std::string &)
playerSummary(int64_t,const std::string &)
registerPlayer(int64_t,const std::string &)
registerPlayer(const std::string &)
withdrawPlayer(int64_t,const std::string &)
withdrawPlayer(const std::string &)*/
};
struct ClubsRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    ClubsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server + "/api/clubs"s) {}
    std::future<NetworkResponse<void>> resetMyActiveClub() {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url + "/club/reset-my-active-club.proto"s);
            auto v9 = conn->performPost(url, ContentTypeHeader(CTH_PB), ""s, AcceptHeader(), "Reset My Active Club"s, false);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
    std::future<NetworkResponse<void>> setMyActiveClub(const protobuf::UUID &id) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url + "/club/my-active-club.proto"s);
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, id);
            auto v9 = conn->performPost(url, ContentTypeHeader(CTH_PB), payload, AcceptHeader(), "Set My Active Club"s, false);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::Clubs>> listMyClubs(Optional<protobuf::Membership_Status> status, Optional<int> start, Optional<int> limit) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::Clubs>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            QueryStringBuilder qsb;
            qsb.addOptional("status"s, status);
            qsb.addOptional("start"s, start);
            qsb.addOptional("limit"s, limit);
            auto v9 = conn->performGet(m_url + "/club/list/my-clubs.proto"s + qsb.getString(true), AcceptHeader(ATH_PB), "List My Clubs"s);
            return HttpHelper::convertToResultResponse<protobuf::Clubs>(v9);
        }), true, false);
    }
};
struct EventCoreRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    EventCoreRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server + "/api/events-core"s) {}
    std::future<NetworkResponse<bool>> createRegistration(int64_t id) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<bool>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url + "/events/subgroups/register/"s + std::to_string(id));
            auto v9 = conn->performPost(url, ContentTypeHeader(CTH_JSON), "{}"s, AcceptHeader(ATH_JSON), "Create Registration"s, false);
            return HttpHelper::convertToResultResponse<bool>(v9, [](const Json::Value &json) {
                return json["registered"].asBool();
            });
        }), true, false);
    }
    std::future<NetworkResponse<bool>> deleteRegistration(int64_t id) { //absent in PC version
        return m_mgr->pushRequestTask(std::function<NetworkResponse<bool>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url + "/events/register/"s + std::to_string(id));
            auto v9 = conn->performDelete(url, AcceptHeader(ATH_JSON), "Delete Registration"s);
            return HttpHelper::convertToResultResponse<bool>(v9, [](const Json::Value &json) {
                return !json["registered"].asBool();
            });
        }), true, false);
    }
    std::future<NetworkResponse<bool>> deleteSignup(int64_t id) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<bool>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url + "/events/signup/"s + std::to_string(id));
            auto v9 = conn->performDelete(url, AcceptHeader(ATH_JSON), "Delete Signup"s);
            return HttpHelper::convertToResultResponse<bool>(v9, [](const Json::Value &json) {
                return !json["signedUp"].asBool();
            });
        }), true, false);
    }
    std::future<NetworkResponse<model::EventSignupResponse>> createSignup(int64_t id) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<model::EventSignupResponse>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url + "/events/subgroups/signup/"s + std::to_string(id));
            auto v9 = conn->performPost(url, ContentTypeHeader(CTH_JSON), "{}"s, AcceptHeader(ATH_JSON), "Create Registration"s, false);
            return HttpHelper::convertToResultResponse<model::EventSignupResponse>(v9, [](const Json::Value &rx_json) {
                auto timeTrialData = rx_json["timeTrialData"];
                return model::EventSignupResponse{
                    timeTrialData.isNull() ? 0 : timeTrialData["riderStartTime"].asString(),
                    rx_json["signUpStatus"].asInt(),
                    timeTrialData.isNull() ? 0 : timeTrialData["riderSlot"].asInt(),
                    rx_json["signedUp"].asBool()
                };
            });
        }), true, false);
    }
};
struct EventFeedRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    EventFeedRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server + "/api/eventfeed"s) {}
    std::future<NetworkResponse<protobuf::EventsProtobuf>> getEvents(const model::EventsSearch &es) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::EventsProtobuf>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performPost(m_url + "/proto/search"s, ContentTypeHeader(CTH_JSON), HttpHelper::jsonToString(es), AcceptHeader(ATH_PB), "Get Events"s, false);
            return HttpHelper::convertToResultResponse<protobuf::EventsProtobuf>(v9);
        }), true, false);
    }
    /*absent in PC getMyEvents(zwift_network::model::BaseEventsSearch const&) "Get My Events" "/proto/my-events" */
};
struct FirmwareUpdateRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    FirmwareUpdateRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server + "/api/dfu"s) {}
    /*OMIT
FirmwareUpdateRestInvoker::getAllFirmwareReleases(const std::string &)
FirmwareUpdateRestInvoker::getFirmwareRelease(const std::string &,const std::string &)
FirmwareUpdateRestInvoker::getFirmwareUpdates(std::vector<zwift_network::model::FirmwareRequest> const&)
FirmwareUpdateRestInvoker::parseFirmwareRelease(Json::Value const&)
FirmwareUpdateRestInvoker::parseFirmwareReleaseInfoList(Json::Value const&)
FirmwareUpdateRestInvoker::parseFirmwareReleaseList(Json::Value const&)
FirmwareUpdateRestInvoker::sendDeviceDiagnostics(const std::string &,const std::string &,const std::vector<uchar> &)*/
};
struct GenericRestInvoker { //0x10 bytes
    GenericHttpConnectionManager *m_mgr;
    GenericRestInvoker(GenericHttpConnectionManager *mgr) : m_mgr(mgr) {}
    /*absent in PC GenericRestInvoker::get(const std::string &) GenericHttpConnectionManager::pushRequestTask<std::string> "Generic Get Request" */
};
struct PrivateEventsRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    PrivateEventsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server + "/api/private_event"s) {}
    std::future<NetworkResponse<void>> accept(int64_t id) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url + std::to_string(id) + "/accept"s);
            auto v9 = conn->performPost(url, AcceptHeader(), "Accept Private Event"s);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
    std::future<NetworkResponse<void>> reject(int64_t id) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url + std::to_string(id) + "/reject"s);
            auto v9 = conn->performPut(url, AcceptHeader(), "Reject Private Event"s, false);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::PrivateEventFeedListProto>> feed(int64_t start_date, int64_t end_date, Optional<protobuf::EventInviteStatus> status, bool organizer_only_past_events) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PrivateEventFeedListProto>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            QueryStringBuilder qsb;
            qsb.add("start_date", start_date);
            qsb.add("end_date", end_date);
            qsb.add("organizer_only_past_events", organizer_only_past_events);
            if (status.m_hasValue) {
                switch (status.m_T) {
                case protobuf::PENDING:
                    qsb.add("status", "PENDING");
                    break;
                case protobuf::ACCEPTED:
                    qsb.add("status", "ACCEPTED");
                    break;
                case protobuf::REJECTED:
                    qsb.add("status", "REJECTED");
                    break;
                default:
                    assert(false);
                }
            }
            auto v9 = conn->performGet(m_url + "/feed"s + qsb.getString(true), AcceptHeader(ATH_PB), "Get Private Event Feed"s);
            return HttpHelper::convertToResultResponse<protobuf::PrivateEventFeedListProto>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::PrivateEventProto>> get(int64_t id) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PrivateEventProto>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(m_url + "/"s + std::to_string(id), AcceptHeader(ATH_PB), "Get Private Event"s);
            return HttpHelper::convertToResultResponse<protobuf::PrivateEventProto>(v9);
        }), true, false);
    }
};
struct RaceResultRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    RaceResultRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server + "/api/race-results"s) {}
    std::future<NetworkResponse<void>> createRaceResultEntry(const protobuf::RaceResultEntrySaveRequest &rq) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, rq);
            auto v9 = conn->performPost(m_url, ContentTypeHeader(CTH_PB), payload, AcceptHeader(), "Create Race Result Entry"s, false);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::RaceResultSummary>> getSubgroupRaceResultSummary(int64_t sid) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::RaceResultSummary>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            QueryStringBuilder qsb;
            qsb.add("event_subgroup_id", sid);
            auto v9 = conn->performGet(m_url + "/summary"s + qsb.getString(true), AcceptHeader(ATH_PB), "Get Subgroup Race Result Summary"s);
            return HttpHelper::convertToResultResponse<protobuf::RaceResultSummary>(v9);
        }), true, false);
    }
    /* absent in PC:
getEventRaceResult(int64_t,zwift_network::Optional<int>,zwift_network::Optional<int>) RaceResult "Get Event Race Result"
getEventRaceResultSummary(int64_t) RaceResultSummary "Get Event Race Result Summary"
getSubgroupRaceResult(int64_t,zwift_network::Optional<int>,zwift_network::Optional<int>) RaceResult "Get Subgroup Race Result"*/
};
struct RouteResultsRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    RouteResultsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server) {}
    std::future<NetworkResponse<void>> save(const protobuf::RouteResultSaveRequest &rq) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, rq);
            auto v9 = conn->performPost(m_url + "/route-results"s, ContentTypeHeader(CTH_PB), payload, AcceptHeader(ATH_PB), "Save Route Result"s, false);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
};
struct PlayerPlaybackRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    PlayerPlaybackRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server + "/api/player-playbacks/player"s) {}
    std::future<NetworkResponse<std::string>> savePlayback(const protobuf::PlaybackData &data) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<std::string>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, data);
            auto v9 = conn->performPost(m_url + "/playback"s, ContentTypeHeader(CTH_PB), payload, AcceptHeader(), "Save Playback"s, false);
            return HttpHelper::convertToStringResponse(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::PlaybackMetadata>> getMyPlaybackLatest(int64_t a2, uint64_t after, uint64_t before) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PlaybackMetadata>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            QueryStringBuilder qsb;
            qsb.add("after", after);
            qsb.add("before", before);
            auto v9 = conn->performGet(m_url + "/me/playbacks/"s + std::to_string(a2) + "/latest"s + qsb.getString(true), AcceptHeader(ATH_PB), "Get My Playback Latest"s);
            return HttpHelper::convertToResultResponse<protobuf::PlaybackMetadata>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::PlaybackMetadata>> getMyPlaybackPr(int64_t a2, uint64_t after, uint64_t before) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PlaybackMetadata>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            QueryStringBuilder qsb;
            qsb.add("after", after);
            qsb.add("before", before);
            auto v9 = conn->performGet(m_url + "/me/playbacks/"s + std::to_string(a2) + "/pr"s + qsb.getString(true), AcceptHeader(ATH_PB), "Get My Playback PR"s);
            return HttpHelper::convertToResultResponse<protobuf::PlaybackMetadata>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::PlaybackMetadataList>> getMyPlaybacks(int64_t a2) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PlaybackMetadataList>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(m_url + "/me/playbacks/"s + std::to_string(a2), AcceptHeader(ATH_PB), "Get My Playback"s);
            return HttpHelper::convertToResultResponse<protobuf::PlaybackMetadataList>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::PlaybackData>> getPlaybackData(const protobuf::PlaybackMetadata &md) {
        std::string url = md.url();
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PlaybackData>(CurlHttpConnection *)>([url](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(url, AcceptHeader(ATH_PB), "Get Playback Data"s);
            return HttpHelper::convertToResultResponse<protobuf::PlaybackData>(v9);
        }), false, false);
    }
    /*PC absent: deletePlayback(const std::string &) "Delete Playback" */
};
struct SegmentResultsRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    SegmentResultsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server + "/live-segment-results-service"s) {}
    std::future<NetworkResponse<protobuf::SegmentResults>> getLeaderboard(int64_t sid) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::SegmentResults>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(m_url + "/leaderboard/"s + std::to_string(sid), AcceptHeader(ATH_PB), "Segment Results Leaderboard"s);
            return HttpHelper::convertToResultResponse<protobuf::SegmentResults>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::SegmentResults>> getSegmentJerseyLeaders() {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::SegmentResults>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(m_url + "/leaders"s, AcceptHeader(ATH_PB), "Segment Jersey Leaders"s);
            return HttpHelper::convertToResultResponse<protobuf::SegmentResults>(v9);
        }), true, false);
    }
};
struct PowerCurveRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    PowerCurveRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server + "/api/power-curve"s) {}
    std::future<NetworkResponse<protobuf::PowerCurveAggregationMsg>> getBestEffortsPowerCurveFromAllTime() {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::PowerCurveAggregationMsg>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(m_url + "/best/all-time"s, AcceptHeader(ATH_PB), "Power Curve Best From All Time"s);
            return HttpHelper::convertToResultResponse<protobuf::PowerCurveAggregationMsg>(v9);
        }), true, false);
    }
    /*absent in PC
getAvailablePowerCurveYears() AvailableYearsMsg "Power Curve List Years"
getBestEffortsPowerCurveByDays(int) PowerCurveAggregationMsg "Power Curve Best By Days"
getBestEffortsPowerCurveByYear(int) PowerCurveAggregationMsg "Power Curve Best By Year"*/
};
struct ZFileRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    ZFileRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server + "/api/zfiles"s) {}
    std::future<NetworkResponse<void>> erase(int64_t id) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<void>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url + '/' + std::to_string(id));
            auto v9 = conn->performDelete(url, AcceptHeader(), "Erase ZFile"s);
            return HttpHelper::convertToVoidResponse(v9);
        }), true, false);
    }
    std::future<NetworkResponse<std::string>> download(int64_t id) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<std::string>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            auto v9 = conn->performGet(m_url + '/' + std::to_string(id) + "/download"s, AcceptHeader(ATH_OCTET), "Download ZFile"s);
            return HttpHelper::convertToStringResponse(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::ZFileProto>> create(const protobuf::ZFileProto &data) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::ZFileProto>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::vector<char> payload;
            HttpHelper::protobufToCharVector(&payload, data);
            auto v9 = conn->performPost(m_url, ContentTypeHeader(CTH_PB), payload, AcceptHeader(ATH_PB), "Create ZFile"s, true);
            return HttpHelper::convertToResultResponse<protobuf::ZFileProto>(v9);
        }), true, false);
    }
    std::future<NetworkResponse<protobuf::ZFilesProto>> list(const std::string &folder) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<protobuf::ZFilesProto>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            QueryStringBuilder qsb;
            qsb.add("folder", folder);
            auto v9 = conn->performGet(m_url + "/list"s + qsb.getString(true), AcceptHeader(ATH_PB), "List ZFiles"s);
            return HttpHelper::convertToResultResponse<protobuf::ZFilesProto>(v9);
        }), true, false);
    }
};
struct ZwiftWorkoutsRestInvoker { //0x38 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    ZwiftWorkoutsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server + "/api/zfiles"s /*hmm*/) {}
    std::future<NetworkResponse<std::string>> fetchWorkout(const std::string &id) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<std::string>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url + '/' + id + "/download");
            auto v9 = conn->performGet(url, AcceptHeader(ATH_OCTET), "Get Workout"s);
            return HttpHelper::convertToStringResponse(v9);
        }), true, false);
    }
    /* absent in PC
ZwiftWorkoutsRestInvoker::createWorkout(const std::string &,protobuf::Sport,const std::string &) ZFileProto "Create Workout"
ZwiftWorkoutsRestInvoker::createWorkoutProto(const std::string &,protobuf::Sport,const std::string &)
ZwiftWorkoutsRestInvoker::deleteWorkout(const std::string &) void "Delete Workout"
ZwiftWorkoutsRestInvoker::editWorkout(const std::string &,const std::string &,protobuf::Sport,const std::string &) void "Update Workout"
//OMIT ZwiftWorkoutsRestInvoker::fetchUpcomingWorkouts(RequestTaskComposer<std::vector<zwift_network::model::WorkoutsFromPartner>,std::multiset<zwift_network::model::Workout>>::Composable)*/
};
struct WorkoutServiceRestInvoker { //0x30 bytes
    ZwiftHttpConnectionManager *m_mgr;
    std::string m_url;
    WorkoutServiceRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) : m_mgr(mgr), m_url(server + "/api/workout"s) {}
    /*absent in PC
WorkoutServiceRestInvoker::fetchAssetSummary(const std::string &) AssetSummary "Get Asset Summary"
WorkoutServiceRestInvoker::fetchCustomWorkouts(zwift_network::Optional<std::string>) WorkoutSummaries "Get Custom Workouts" */
};
struct TcpStatistics { //0xC8 bytes
    uint64_t m_packets = 0;
};
struct WorldClockStatistics { //0xB0 bytes
    WorldClockStatistics() {
        //OMIT
    }
    //OMIT
};
struct LanExerciseDeviceStatistics { //0x110 bytes
    LanExerciseDeviceStatistics() {
        //OMIT
    }
    //OMIT
};
struct AuxiliaryControllerStatistics { //0xB8 bytes
    AuxiliaryControllerStatistics() {
        //OMIT
    }
    //OMIT
};
struct WorldAttributeStatistics { //0x90 bytes
    WorldAttributeStatistics() {
        //OMIT
    }
    //OMIT
};
struct LanExerciseDeviceService { //0x3B0 bytes LATER
    LanExerciseDeviceService(const std::string &zaVersion, uint16_t, int, int, int, int) {
        //TODO
    }
    /*TODO LanExerciseDeviceService::DiscoverySocket::DiscoverySocket(asio::basic_datagram_socket<asio::ip::udp,asio::any_io_executor>)
LanExerciseDeviceService::LanExerciseDevice::LanExerciseDevice(asio::basic_stream_socket<asio::ip::tcp,asio::any_io_executor>,std::shared_ptr<EventLoop> const&)
LanExerciseDeviceService::LanExerciseDevice::~LanExerciseDevice()
LanExerciseDeviceService::LanExerciseDeviceService(std::shared_ptr<LanExerciseDeviceStatistics>,ushort,std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>,std::chrono::duration<int64_t int64_t,std::ratio<1l,1000l>>,std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>,std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>)
LanExerciseDeviceService::addAttribute(std::shared_ptr<LanExerciseDeviceService::LanExerciseDevice> const&,std::string &,std::string &)
LanExerciseDeviceService::addDevice(const std::string &,const std::string &)
LanExerciseDeviceService::closeDiscoverySockets()
LanExerciseDeviceService::configureDiscoverySockets()
LanExerciseDeviceService::connect(uint32_t,std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>,std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>)
LanExerciseDeviceService::connectToDevice(std::shared_ptr<LanExerciseDeviceService::LanExerciseDevice> const&)
LanExerciseDeviceService::disconnect(uint32_t)
LanExerciseDeviceService::disconnectFromDevice(std::shared_ptr<LanExerciseDeviceService::LanExerciseDevice> const&,zwift_network::NetworkRequestOutcome)
LanExerciseDeviceService::discoveryDevices()
LanExerciseDeviceService::handleCommunicationError(std::error_code,std::shared_ptr<LanExerciseDeviceService::LanExerciseDevice> const&)
LanExerciseDeviceService::listenDevice(LanExerciseDeviceService::DiscoverySocket &)
LanExerciseDeviceService::listenDevices()
LanExerciseDeviceService::parseAnswerMessage(uint64_t,LanExerciseDeviceService::DiscoverySocket &)
LanExerciseDeviceService::parseAnswerMessageCallback(int,sockaddr const*,uint64_t,mdns_entry_type,ushort,ushort,ushort,uint32_t,void const*,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,void *)
LanExerciseDeviceService::processMessagesReceivedFromDevice(std::shared_ptr<LanExerciseDeviceService::LanExerciseDevice> const&,uint32_t)
LanExerciseDeviceService::processMessagesToSend()
LanExerciseDeviceService::receiveFromDevice(std::shared_ptr<LanExerciseDeviceService::LanExerciseDevice> const&)
LanExerciseDeviceService::registerMessageReceivedCallback(std::function<void ()(zwift_network::LanExerciseDeviceInfo const&,const std::vector<uchar> &)> const&)
LanExerciseDeviceService::registerStatusCallback(std::function<void ()(zwift_network::LanExerciseDeviceInfo const&)> const&)
LanExerciseDeviceService::resetDiscoveryDevicesTimer()
LanExerciseDeviceService::resetInactivityTimeoutTimer(std::shared_ptr<LanExerciseDeviceService::LanExerciseDevice> const&)
LanExerciseDeviceService::resetListenDevicesTimer()
LanExerciseDeviceService::resetSendMessagesTimer()
LanExerciseDeviceService::resolve(std::shared_ptr<LanExerciseDeviceService::LanExerciseDevice> const&)
LanExerciseDeviceService::resolveDevice(std::shared_ptr<LanExerciseDeviceService::LanExerciseDevice> const&)
LanExerciseDeviceService::sendMessage(uint32_t,const std::vector<uchar> &)
LanExerciseDeviceService::sendQuery(mdns_record_type,char const**,uint64_t)
LanExerciseDeviceService::setDevicePort(uint32_t,ushort,const std::string &)
LanExerciseDeviceService::setServiceName(zwift_network::LanExerciseDeviceType,const std::string &)
LanExerciseDeviceService::shutdown()
LanExerciseDeviceService::shutdownDevices()
LanExerciseDeviceService::startScan()
LanExerciseDeviceService::stopScan()
LanExerciseDeviceService::updateDeviceAddress(std::shared_ptr<LanExerciseDeviceService::LanExerciseDevice> const&,const std::string &,const std::string &)
LanExerciseDeviceService::updateDevicePort(std::shared_ptr<LanExerciseDeviceService::LanExerciseDevice> const&,ushort)
LanExerciseDeviceService::~LanExerciseDeviceService()*/
};
struct TcpAddressService {
    struct TcpAddressCur {
        std::vector<protobuf::TcpAddress> m_vec;
        int m_cur = 0;
    };
    std::map<LONG, TcpAddressCur> m_addrMap;
    bool m_isLastAddress = false;
    const protobuf::TcpAddress *getAddress(int64_t worldId, uint32_t mapRevision) {
        m_isLastAddress = false;
        auto f = m_addrMap.find(MAKELONG(worldId, mapRevision));
        if (f != m_addrMap.end()) {
            auto v10 = f->second.m_cur++;
            if (f->second.m_cur >= f->second.m_vec.size()) {
                m_isLastAddress = true;
                f->second.m_cur = 0;
            }
            return &f->second.m_vec[v10];
        }
        if (worldId || mapRevision)
            return getAddress(0, 0);
        NetworkingLogError("No generic TCP cluster available!");
        return nullptr;
    }
    bool isValidAddress(int64_t worldId, uint32_t mapRevision, const std::string &ip, uint16_t port) {
        LONG key = MAKELONG(worldId, mapRevision);
        auto f = m_addrMap.find(key);
        if (f == m_addrMap.end()) {
            if (!worldId || !mapRevision)
                return false;
            return isValidAddress(0, 0, ip, port);
        }
        for (const auto &i : f->second.m_vec)
            if (i.port() == port && i.ip() == ip)
                return true;
        return false;
    }
    bool updateAddresses(const protobuf::TcpConfig &cfg, int64_t worldId, uint32_t mapRevision, const std::string &ip, uint16_t port) {
        if (cfg.nodes_size() == 0) {
            NetworkingLogDebug("TcpConfig empty");
            return false;
        }
        m_addrMap.clear();
        bool argSpecificServer = worldId && mapRevision, v57 = false, v58 = false;
        bool hasSpecificCluster = false, foundCurrentNodeInSpecificCluster = false, foundCurrentNodeInGenericCluster = false,
            foundAnyNodeInGenericCluster = false, foundAnyNodeInSpecificCluster = false, foundCurrentNodeAsFirstNode = false;
        for (const auto &n : cfg.nodes()) {
            auto worldId_n = n.lb_realm();
            auto mapRevision_n = n.lb_course();
            LONG key_n = MAKELONG(worldId_n, n.lb_course());
            bool genericServer_n = !worldId_n && !mapRevision_n;
            bool n_eqArg = (worldId_n == worldId) && (mapRevision_n == mapRevision);
            m_addrMap[key_n].m_vec.push_back(n);
            if (argSpecificServer && n_eqArg)
                hasSpecificCluster = true;
            const auto &ip_n = n.ip();
            if ((n.cport() != port && n.port() != port) || ip_n != ip) {
                if (genericServer_n) {
                    foundAnyNodeInGenericCluster = true;
                    v58 = true;
                } else if (!n_eqArg) {
                    foundAnyNodeInGenericCluster = v58;
                }
            } else if (genericServer_n) {
                foundCurrentNodeInGenericCluster = true;
                foundAnyNodeInGenericCluster = true;
                if (!v58)
                    foundCurrentNodeAsFirstNode = true;
                v58 = true;
            } else {
                foundAnyNodeInGenericCluster = v58;
                if (n_eqArg) {
                    foundCurrentNodeInSpecificCluster = true;
                    if (!v57)
                        foundCurrentNodeAsFirstNode = true;
                    v57 = true;
                }
            }
            foundAnyNodeInSpecificCluster = v57;
            NetworkingLogDebug(
                "TCP (%d,%d) %s:%d - (%d,%d) %s:%d,%d - hasSpecificCluster: %d foundCurrentNodeInSpecificCluster: %d "
                "foundCurrentNodeInGenericCluster: %d foundAnyNodeInGenericCluster: %d foundAnyNodeInSpecificCluster: %d foundCurrentNodeAsFirstNode: %d",
                worldId, mapRevision, ip.c_str(), port, worldId_n, mapRevision_n,
                ip_n.c_str(), n.port(), n.cport(),
                hasSpecificCluster, foundCurrentNodeInSpecificCluster, foundCurrentNodeInGenericCluster,
                foundAnyNodeInGenericCluster, foundAnyNodeInSpecificCluster, foundCurrentNodeAsFirstNode);
        }
        auto currentHostIsStillGood = foundCurrentNodeInSpecificCluster || (!hasSpecificCluster && foundCurrentNodeInGenericCluster);
        NetworkingLogDebug("TCP currentHostIsStillGood: %d ", currentHostIsStillGood);
        return (cfg.f2() && !foundCurrentNodeAsFirstNode) || !currentHostIsStillGood;
    }
};
struct TcpClient {
    moodycamel::ReaderWriterQueue<std::pair<uint64_t, std::shared_ptr<protobuf::ServerToClient>>> m_rwq;
    struct SegmentSubscription {
        std::promise<NetworkResponse<protobuf::SegmentResults>> *m_promise = nullptr;
        boost::asio::steady_timer m_timer1;
        FutureWaiter<NetworkResponse<protobuf::SegmentResults>> m_fwaiter;
        int m_retryCnt = 0;
        bool m_hasValue = false, m_waitingAck = false, m_pendingRequest = false, m_subsOK = false;
        SegmentSubscription(boost::asio::io_context *ctx, std::promise<NetworkResponse<protobuf::SegmentResults>> *promise) : m_promise(promise), m_timer1(*ctx), m_fwaiter(*ctx) {}
        template<class T> //by ref and by rvalue ref
        void setPromiseValue(uint64_t segment, T val) {
            if (m_promise) {
                if (m_hasValue) {
                    NetworkingLogDebug("SEGMENT RESULTS: Attempted to set value on a promise that already had a value for segment %ld", segment);
                } else {
                    NetworkingLogDebug("SEGMENT RESULTS: Set value on a promise for segment %ld", segment);
                    m_promise->set_value(val);
                    m_hasValue = true;
                }
            } else {
                NetworkingLogDebug("SEGMENT RESULTS: Attempted to set value on an invalid promise to segment %ld", segment);
            }
        }
    };
    GlobalState *m_gs;
    WorldClockService *m_wcs;
    HashSeedService *m_hss;
    WorldAttributeService *m_wat;
    RelayServerRestInvoker *m_relay;
    SegmentResultsRestInvoker *m_segRes;
    NetworkClientImpl *m_ncli;
    std::string m_ip;
    EventLoop m_eventLoop;
    int64_t m_worldId = 0, m_port = 0;
    boost::asio::ip::tcp::socket m_tcpSocket;
    boost::asio::ip::tcp::endpoint m_endpoint;
    uint32_t m_mapRevision = 0;
    TcpAddressService m_tcpAddressService;
    FutureWaiter<NetworkResponse<protobuf::TcpConfig>> m_tcpConfigWaiter;
    boost::asio::steady_timer m_asioTimer2, m_asioTimer3, m_asioTimer4;
    protocol_encryption::TcpRelayClientCodec m_codec;
    std::string m_decodeError;
    std::vector<uint8_t> m_decodedMessage, m_encodedMessage;
    std::unordered_map<int64_t, SegmentSubscription> m_subscrSegments;
    EncryptionInfo m_ei;
    uint8_t m_buf64k[0x10000] = {}, m_txBuf1492[1492] = {};
    uint32_t m_timeout1, m_timeout2, m_max_segm_subscrs = 3, m_ctsSeqNo = 1;
    bool m_shouldUseEncryption = false;
    TcpClient(GlobalState *gs, WorldClockService *wcs, HashSeedService *hss, WorldAttributeService *wat, RelayServerRestInvoker *relay, SegmentResultsRestInvoker *segRes, NetworkClientImpl *ncli, int t1 = 35000, int t2 = 5000) : 
        m_rwq(100), m_gs(gs), m_wcs(wcs), m_hss(hss), m_wat(wat), m_relay(relay), m_segRes(segRes), m_ncli(ncli), 
        m_tcpSocket(m_eventLoop.m_asioCtx, boost::asio::ip::tcp::v4()), 
        m_tcpConfigWaiter(m_eventLoop.m_asioCtx), m_asioTimer2(m_eventLoop.m_asioCtx),
        m_asioTimer3(m_eventLoop.m_asioCtx), m_asioTimer4(m_eventLoop.m_asioCtx), m_timeout1(t1), m_timeout2(t2) {
        m_subscrSegments.reserve(16);
        m_eventLoop.post([this]() {
            auto &psi = this->m_gs->getPerSessionInfo();
            this->handleTcpConfig(psi.nodes());
            if (psi.has_max_segm_subscrs()) {
                this->m_max_segm_subscrs = psi.max_segm_subscrs();
                NetworkingLogDebug("Received max allowed segment subscriptions from session: %d", this->m_max_segm_subscrs);
            }
            this->m_shouldUseEncryption = this->m_gs->shouldUseEncryption();
            if (this->m_shouldUseEncryption) {
                this->m_ei = this->m_gs->getEncryptionInfo();
                this->m_codec.m_hostRelayId = this->m_ei.m_relaySessionId;
                this->m_codec.m_generateKey = false;
                memmove(this->m_codec.m_secretRaw, this->m_ei.m_sk.c_str(), std::max(sizeof(this->m_codec.m_secretRaw), this->m_ei.m_sk.size()));
                this->m_codec.m_secret = base64::toString(this->m_codec.m_secretRaw);
            }
        });
    }
    void segmentSubscriptionsShutdown() {
        for(auto &ss : m_subscrSegments) {
            ss.second.m_timer1.cancel();
            ss.second.m_fwaiter.cancel();
            NetworkingLogDebug("SEGMENT RESULTS: shutdown segment %ld (waitingAck %s, pendingRequest %s)", ss.first, ss.second.m_waitingAck ? "true" : "false", ss.second.m_pendingRequest ? "true" : "false");
            ss.second.setPromiseValue(ss.first, NetworkResponse<protobuf::SegmentResults>{ "Shutdown TCP connection"s, NRO_REQUEST_ABORTED });
        }
        m_subscrSegments.clear();
    }
    void clearEndpoint() {
        m_ip.clear();
        m_port = 0;
        m_endpoint = boost::asio::ip::tcp::endpoint();
    }
    void shutdown() {
        m_eventLoop.post([this]() {
            m_tcpConfigWaiter.cancel();
            m_asioTimer2.cancel();
            m_asioTimer3.cancel();
            m_asioTimer4.cancel();
            this->segmentSubscriptionsShutdown();
            this->disconnect();
            this->clearEndpoint();
        });
        m_eventLoop.enqueueShutdown();
    }
    void handleWorldAndMapRevisionChanged(int64_t worldId, uint32_t mapRevision) {
        m_eventLoop.post([this, worldId, mapRevision]() {
            this->m_worldId = worldId;
            this->m_mapRevision = mapRevision;
            NetworkingLogDebug("TCP handleWorldAndMapRevisionChanged (%d,%d) ", worldId, mapRevision);
            if (m_tcpAddressService.isValidAddress(worldId, mapRevision, this->m_ip, this->m_port)) {
                NetworkingLogDebug("TCP handleWorldAndMapRevisionChanged isValidAddress() true");
            } else {
                NetworkingLogDebug("TCP handleWorldAndMapRevisionChanged isValidAddress() false");
                this->reconnect(false);
            }
        });
    }
    void waitDisconnection(bool refreshTcpConfigIfNeeded) {
        m_asioTimer4.expires_after(std::chrono::milliseconds(100));
        m_asioTimer4.async_wait([this, refreshTcpConfigIfNeeded](const boost::system::error_code &ec) {
            if (ec) {
                NetworkingLogError("Error waiting disconnection [%d] %s", ec.value(), ec.to_string().c_str());
            } else {
                NetworkingLogDebug("TCP waitDisconnection refreshTcpConfigIfNeeded: %d", refreshTcpConfigIfNeeded);
                if (!this->m_tcpSocket.is_open()) {
                    NetworkingLogInfo("TCP disconnected");
                    this->onDisconnected(refreshTcpConfigIfNeeded);
                } else {
                    this->waitDisconnection(refreshTcpConfigIfNeeded);
                }
            }
        });
    }
    void onDisconnected(bool refreshTcpConfigIfNeeded) {
        clearEndpoint();
        resolveEndpointAndConnect();
        NetworkingLogDebug("TCP onDisconnected refreshTcpConfigIfNeeded: %d isLastAddress: %d", refreshTcpConfigIfNeeded, m_tcpAddressService.m_isLastAddress);
        if (refreshTcpConfigIfNeeded && m_tcpAddressService.m_isLastAddress)
            refreshTcpConfig();
    }
    void refreshTcpConfig() {
        m_tcpConfigWaiter.waitAsync(m_relay->fetchTcpConfig(), 100, [this](const NetworkResponse<protobuf::TcpConfig> &resp) {
            if (resp.m_errCode) {
                NetworkingLogError("Failed to fetch tcp config: [%d] %s", resp.m_errCode, resp.m_msg.c_str());
            } else {
                NetworkingLogDebug("TCP TcpConfig from endpoint");
                this->handleTcpConfig(resp.m_T);
            }
        });
    }
    void resolveEndpointAndConnect() {
        boost::asio::ip::tcp::resolver resolver(m_eventLoop.m_asioCtx);
        auto addr = m_tcpAddressService.getAddress(m_worldId, m_mapRevision);
        auto usePort = addr->cport(), normalPort = addr->port();
        auto &sip = addr->ip();
        NetworkingLogDebug("TCP resolveEndpointAndConnect %s:%d ", sip.c_str(), normalPort);
        if (!m_shouldUseEncryption)
            usePort = normalPort;
        auto strPort = std::to_string(usePort);
        boost::asio::ip::tcp::resolver::query query(sip, strPort);
        boost::system::error_code ec;
        boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query, ec);
        if (ec) {
            handleCommunicationError(ec, "Error resolving TCP host "s + sip + ":"s + strPort);
        } else {
            m_ip = sip;
            m_port = usePort;
            if (!m_codec.newConnection(&m_decodeError)) {
                //OMIT TcpStatistics::increaseEncryptionNewConnectionError((_Mtx_t)this->m_stat);
                NetworkingLogError("TCP failed to get new connection id [%s].", m_decodeError.c_str());
            }
            //OMIT TcpStatistics::setEncryptionEnabled((_Mtx_t)this->m_stat, this->m_shouldUseEncryption);
            NetworkingLogInfo("TCP host %s:%d%s", m_ip.c_str(), m_port, m_shouldUseEncryption ? " (secure)" : "");
            NetworkingLogInfo("Connecting to TCP server...");
            m_endpoint = *iter;
            m_tcpSocket.async_connect(*iter, [this](const std::error_code &ec) {
                if (ec) {
                    this->handleCommunicationError(ec, "Error connecting to TCP host "s + this->m_ip + ':' + std::to_string(this->m_port));
                } else {
                    this->m_asioTimer3.cancel();
                    this->sayHello();
                }
            });
        }
    }
    void disconnect() {
        if (!m_tcpSocket.is_open()) {
            NetworkingLogDebug("TCP disconnect socket is not open");
        } else {
            NetworkingLogDebug("TCP disconnect socket is open");
            NetworkingLogDebug("TCP disconnect socket shutdown");
            boost::system::error_code ec;
            m_tcpSocket.shutdown(m_tcpSocket.shutdown_both, ec);
            if (ec && ec.value() != 10057 /*ignore WSAENOTCONN*/)
                NetworkingLogWarn("Error shutting down TCP socket [%d] %s", ec.value(), ec.to_string().c_str());
            NetworkingLogDebug("TCP disconnect socket close"); 
            m_tcpSocket.close(ec);
            if (ec)
                NetworkingLogWarn("Error closing TCP socket [%d] %s", ec.value(), ec.to_string().c_str());
        }
    }
    void handleTcpConfig(const protobuf::TcpConfig &cfg) {
        bool updated = m_tcpAddressService.updateAddresses(cfg, m_worldId, m_mapRevision, m_ip, m_port);
        if (updated) {
            NetworkingLogDebug("TCP hostAndPortUpdated: 1 ");
            reconnect(false);
        }
    }
    void onInactivityTimeout(const std::error_code &ec) {
        if (!ec) {
            NetworkingLogWarn("TCP connection timed out owing to inactivity");
            //OMIT m_stat->TcpStatistics::increaseConnectionTimeoutCount();
            reconnect(true);
        }
    }
    void reconnect(bool refreshTcpConfigIfNeeded) {
        NetworkingLogDebug("TCP reconnect refreshTcpConfigIfNeeded: %d ", refreshTcpConfigIfNeeded);
        disconnect();
        waitDisconnection(refreshTcpConfigIfNeeded);
    }
    void handleCommunicationError(const std::error_code &ec, const std::string &str) {
        NetworkingLogDebug("%s [%d] %s", str.c_str(), ec.value(), ec.message().c_str());
        if (ec.category() == boost::asio::error::system_category && ec.value() == ERROR_OPERATION_ABORTED)
            return;
        //OMIT TcpStatistics::increaseCommunicationErrorCount((_Mtx_t)this->m_stat);
        m_asioTimer2.cancel();
        m_asioTimer3.expires_after(std::chrono::milliseconds(m_timeout2));
        m_asioTimer3.async_wait([this](boost::system::error_code const &lam_ec) {
            if (lam_ec) {
                NetworkingLogDebug("TCP handleCommunicationError waiting time canceled; do not reconnect()");
            } else {
                NetworkingLogDebug("TCP handleCommunicationError finished the waiting time (%dms); now reconnect()", this->m_timeout2);
                reconnect(true);
            }
        });
    }
    uint8_t *decodeMessage(uint8_t *encr, uint64_t *pInOutLen) {
        auto decodedPtr = encr;
        if (m_shouldUseEncryption) {
            m_decodedMessage.clear();
            if (!m_codec.decode(encr, *pInOutLen, &m_decodedMessage, &m_decodeError)) {
                //OMIT TcpStatistics::increaseEncryptionDecodeError((_Mtx_t)this->m_stat);
                NetworkingLogError("Failed to decode TCP CtS [%s]." /*URSOFT: looks like bug here, must be StC*/, m_decodeError.c_str());
                decodedPtr = nullptr;
            } else {
                *pInOutLen = m_decodedMessage.size();
                decodedPtr = m_decodedMessage.data();
            }
        }
        return decodedPtr;
    }
    void encodeMessage(uint8_t *pSignedMsg, uint64_t sizeLimit, uint32_t *pInOutLen) {
        if (m_shouldUseEncryption && *pInOutLen >= 2) {
            m_encodedMessage.resize(2);
            if (m_codec.encode(pSignedMsg + 2, *pInOutLen - 2, &m_encodedMessage, &m_decodeError)) {
                if (m_encodedMessage.size() > sizeLimit) {
                    //OMIT TcpStatistics::increaseEncryptionEncodeBufferTruncate(m_stat);
                    m_encodedMessage.resize(sizeLimit);
                }
                *(uint16_t *)pSignedMsg = htons((uint16_t)m_encodedMessage.size() - 2);
                memmove(pSignedMsg + 2, m_encodedMessage.data() + 2, m_encodedMessage.size() - 2);
                *pInOutLen = uint32_t(m_encodedMessage.size());
            } else {
                //TcpStatistics::increaseEncryptionEncodeError(m_stat);
                NetworkingLogError("Failed to encode TCP CtS [%s].", m_decodeError.c_str());
            }
        }
    }
    void processPayload(uint64_t len);
    void processSubscribedSegment(const protobuf::ServerToClient &stc) {
        for(auto id : stc.ack_subs_segm()) {
            NetworkingLogDebug("SEGMENT RESULTS: ack received for segment %ld", id);
            auto fnd = m_subscrSegments.find(id);
            if (fnd == m_subscrSegments.end()) {
                NetworkingLogDebug("SEGMENT RESULTS: ack received but we are not subscribing for segment %ld", id);
            } else {
                if (fnd->second.m_pendingRequest) {
                    NetworkingLogDebug("SEGMENT RESULTS: ack received but we already have a pending for segment %ld", id);
                } else if (fnd->second.m_waitingAck) {
                    fnd->second.m_waitingAck = false;
                    fnd->second.m_timer1.cancel();
                    NetworkingLogDebug("SEGMENT RESULTS: ack timer canceled for segment %ld", id);
                    if (fnd->second.m_subsOK)
                    {
                        NetworkingLogDebug("SEGMENT RESULTS: resubscribing to segment %ld", id);
                    } else {
                        NetworkingLogDebug("SEGMENT RESULTS: request leaderboard for segment %ld", id);
                        auto ptr = &fnd->second;
                        ptr->m_pendingRequest = true;
                        ptr->m_fwaiter.waitAsync(m_segRes->getLeaderboard(id), 100, [ptr, id](const NetworkResponse<protobuf::SegmentResults> &nr) {
                            NetworkingLogDebug("SEGMENT RESULTS: received leaderboard for segment %ld", id);
                            ptr->setPromiseValue(id, nr);
                            ptr->m_pendingRequest = false;
                        });
                    }
                } else {
                    NetworkingLogDebug("SEGMENT RESULTS: ack received but we are not waiting ack for segment %ld", id);
                }
            }
        }
    }
    enum TcpCommand { TCMD_HELLO_MESSAGE, TCMD_SEGMENT_RESULTS };
    void sendClientToServer(TcpCommand cmd, protobuf::ClientToServer *pCts, const std::function<void()> &onSuccess, const std::function<void()> &onError) {
        pCts->set_seqno(m_ctsSeqNo++);
        auto ptx = m_txBuf1492;
        auto ctsLen = uint32_t(pCts->ByteSizeLong());
        *(uint16_t *)ptx = htons(uint16_t(ctsLen + 6));
        ptx[2] = 1;
        ptx[3] = cmd;
        uint32_t msglen = ctsLen + 8;
        assert(msglen < sizeof(m_txBuf1492));
        pCts->SerializeToArray(ptx + 4, ctsLen);
        m_hss->signMessage(ptx + 2, ctsLen + 2, pCts->world_time());
        encodeMessage(ptx, sizeof(m_txBuf1492), &msglen);
        m_tcpSocket.async_send(boost::asio::buffer(ptx, msglen), 0, [=](const boost::system::error_code &error, std::size_t bytes_transferred) {
            if (error) {
                if (onError)
                    onError();
                std::string msg = "Error sending TCP "s;
                switch (cmd) {
                case TCMD_HELLO_MESSAGE: msg += "HELLO_MESSAGE"s; break;
                case TCMD_SEGMENT_RESULTS: msg += "SEGMENT_RESULTS"s; break;
                default: assert(false); msg += "???"s; break;
                }
                msg += " message"s;
                this->handleCommunicationError(error, msg);
            } else {
                if (onSuccess)
                    onSuccess();
            }
        });
    }
    void sayHello() {
        auto PlayerId = m_gs->getPlayerId();
        auto largestWaTime = m_wat->getLargestWorldAttributeTimestamp();
        protobuf::ClientToServer v24;
        v24.set_world_time(0);
        v24.set_larg_wa_time(largestWaTime);
        v24.set_player_id(PlayerId);
        NetworkingLogInfo("Saying hello to TCP server (largest wa is %llu)", largestWaTime);
        for (auto &ss : m_subscrSegments) {
            v24.add_subssegments(ss.first);
            if (ss.second.m_waitingAck) {
                ss.second.m_timer1.cancel();
            }
            ss.second.m_waitingAck = true;
            ss.second.m_subsOK = true;
        }
        sendClientToServer(TCMD_HELLO_MESSAGE, &v24, [this]() /*succ*/ {
            resetTimeoutTimer();
            readHeader();
        }, [this]() /*error*/ {
            for (auto &ss : this->m_subscrSegments)
                ss.second.m_waitingAck = false;
        });
    }
    void resetTimeoutTimer() {
        m_asioTimer2.expires_after(std::chrono::milliseconds(m_timeout1));
        m_asioTimer2.async_wait([this](const std::error_code &ec) { this->onInactivityTimeout(ec); });
    }
    void tcp_async_receive(uint8_t *pDest, int nBytes, bool header) { m_tcpSocket.async_receive(boost::asio::buffer(pDest, nBytes), 0,
        [this, pDest, nBytes, header] (const boost::system::error_code &error, std::size_t bytes_transferred) {
            int64_t remained = nBytes - bytes_transferred;
            if ((!error && !bytes_transferred) || error || remained == 0) {
                if (error) {
                    this->handleCommunicationError(error, "Error receiving TCP "s + (header ? "header"s : "payload"s));
                } else {
                    this->resetTimeoutTimer();
                    if (header) {
                        auto payLen = ntohs(*(uint16_t *)this->m_buf64k);
                        if (payLen) {
                            this->readPayload(payLen);
                        } else {
                            //OMIT TcpStatistics::registerNetUseIn((_Mtx_t)m_this->m_stat, 2i64);
                            this->readHeader();
                        }
                    } else {
                        this->resetTimeoutTimer();
                        this->processPayload(pDest + bytes_transferred - this->m_buf64k - 2);
                        this->readHeader();
                    }
                }
                return;
            }
            uint8_t *pNewDest = pDest + bytes_transferred;
            if (pNewDest + remained <= this->m_buf64k + sizeof(m_buf64k))
                this->tcp_async_receive(pNewDest, remained, header);
            else
                assert(!"overflow in tcp_async_receive");
        });
    }
    void readPayload(uint32_t size) { tcp_async_receive(m_buf64k + 2, std::min((int)size, (int)sizeof(m_buf64k) - 2), false); }
    void readHeader() { tcp_async_receive(m_buf64k, 2, true); }
    void processSegmentUnsubscription(int64_t id) {
        protobuf::ClientToServer cts;
        cts.set_player_id(m_gs->getPlayerId());
        cts.set_server_realm(m_worldId);
        cts.set_world_time(m_wcs->getWorldTime());
        cts.add_unssegments(id);
        NetworkingLogDebug("SEGMENT RESULTS: unsubscribe from segment %ld", id);
        sendClientToServer(TCMD_SEGMENT_RESULTS, &cts, std::function<void()>(), std::function<void()>());
        auto fnd = m_subscrSegments.find(id);
        if (fnd == m_subscrSegments.end()) {
            NetworkingLogDebug("SEGMENT RESULTS: unsubscribe segment %ld not found", id);
        } else {
            fnd->second.m_timer1.cancel();
            fnd->second.m_fwaiter.cancel();
            NetworkingLogDebug("SEGMENT RESULTS: unsubscribe from segment %ld (waitingAck %s, pendingRequest %s)", id, fnd->second.m_waitingAck ? "true" : "false", fnd->second.m_pendingRequest ? "true" : "false");
            fnd->second.setPromiseValue(id, NetworkResponse<protobuf::SegmentResults>{ "Unsubscribe from segment"s, NRO_REQUEST_ABORTED });
            NetworkingLogDebug("SEGMENT RESULTS: erase unsubscribed segment %ld", id);
            m_subscrSegments.erase(fnd);
        }
    }
    void sendSubscribeToSegment(int64_t id, SegmentSubscription *pSS) {
        protobuf::ClientToServer cts;
        cts.set_player_id(m_gs->getPlayerId());
        cts.set_server_realm(m_worldId);
        cts.set_world_time(m_wcs->getWorldTime());
        cts.add_subssegments(id);
        if (pSS->m_retryCnt)
            NetworkingLogDebug("SEGMENT RESULTS: subscribe to segment %ld (retry %d)", id, pSS->m_retryCnt);
        else
            NetworkingLogDebug("SEGMENT RESULTS: subscribe to segment %ld", id);
        pSS->m_waitingAck = true;
        sendClientToServer(TCMD_SEGMENT_RESULTS, &cts, [this, pSS, id]() {
            pSS->m_timer1.expires_after(std::chrono::milliseconds(5000));
            pSS->m_timer1.async_wait([this, id](const std::error_code &ec) {
                if (!ec) {
                    NetworkingLogDebug("SEGMENT RESULTS: ack not received after 5 seconds for segment %ld; retry ", id);
                    this->processSegmentSubscription(id, nullptr);
                }
            });
        }, [pSS]() {
            pSS->m_waitingAck = false;
        });
    }
    void processSegmentSubscription(int64_t sid, const std::shared_ptr<std::promise<NetworkResponse<protobuf::SegmentResults>>> &ptr) {
        if (ptr && m_subscrSegments.size() >= m_max_segm_subscrs) {
            NetworkingLogDebug("SEGMENT RESULTS: detected too many segment subscriptions (max count: %d)", m_max_segm_subscrs);
            NetworkResponse<protobuf::SegmentResults> err{ "Too many segment subscriptions"s, NRO_TOO_MANY_SEGMENT_RESULTS_SUBSCRIPTIONS };
            ptr->set_value(std::move(err));
            return;
        }
        auto fnd = m_subscrSegments.find(sid);
        SegmentSubscription *pSS = nullptr;
        if (fnd == m_subscrSegments.end()) {
            NetworkingLogDebug("SEGMENT RESULTS: new subscription to segment %ld", sid);
            auto e = m_subscrSegments.emplace(sid, TcpClient::SegmentSubscription(&m_eventLoop.m_asioCtx, ptr.get()));
            pSS = &e.first->second;
        } else {
            pSS = &fnd->second;
            if (pSS->m_waitingAck) {
                NetworkingLogDebug("SEGMENT RESULTS: cancel waiting ack for segment %ld", sid);
                pSS->m_timer1.cancel();
                pSS->m_waitingAck = false;
            } else if (pSS->m_pendingRequest) {
                NetworkingLogDebug("SEGMENT RESULTS: cancel pending request for segment %ld", sid);
                pSS->m_fwaiter.cancel();
                pSS->m_pendingRequest = false;
            }
            if (ptr) {
                pSS->m_retryCnt = 0;
                pSS->m_hasValue = false;
                pSS->m_promise->swap(*ptr.get());
            } else {
                if (pSS->m_retryCnt) {
                    pSS->setPromiseValue(sid, NetworkResponse<protobuf::SegmentResults>{ "Ack not received"s, NRO_REQUEST_TIMED_OUT });
                    NetworkingLogDebug("SEGMENT RESULTS: erase segment %ld ack not received", sid);
                    m_subscrSegments.erase(fnd);
                    return;
                }
                pSS->m_retryCnt = 1;
            }
            NetworkingLogDebug("SEGMENT RESULTS: already %s to segment %ld", pSS->m_retryCnt ? "subscribing" : "subscribed", sid);
        }
        sendSubscribeToSegment(sid, pSS);
    }
    bool popServerToClient(std::shared_ptr<protobuf::ServerToClient> &dest) {
        std::pair<uint64_t, std::shared_ptr<protobuf::ServerToClient>> res;
        bool ret = m_rwq.try_dequeue(res);
        dest = res.second;
        return ret;
    }
    std::future<NetworkResponse<protobuf::SegmentResults>> subscribeToSegmentAndGetLeaderboard(int64_t id) {
        auto promise = std::make_shared<std::promise<NetworkResponse<protobuf::SegmentResults>>>();
        auto ret = promise->get_future();
        m_eventLoop.post([this, id, promise]() {
            processSegmentSubscription(id, promise);
        });
        return ret;
    }
    void unsubscribeFromSegment(int64_t id) { m_eventLoop.post([this, id]() { this->processSegmentUnsubscription(id); }); }
/*inlined:
connect()
getClientToServerForHelloMessage(uint64_t,int64_t)
getMaximumHelloMessageSize()
getTcpMessageSize(uint32_t)
onTcpConfigReceived(std::shared_ptr<zwift_network::NetworkResponse<protobuf::TcpConfig> const> const&)
serializeToTcpMessage(TcpCommand,protobuf::ClientToServer const&,std::array<char,1492ul> &,uint32_t &)
absent in PC:
void handleTcpConfigChanged(const protobuf::TcpConfig &cfg)
Listener::~Listener()
~TcpClient()*/
};
struct AuxiliaryController : public WorldIdListener { //0x105B8-16 bytes
    GlobalState *m_gs;
    WorldClockService *m_wcs;
    NetworkClockService *m_ncs;
    std::mutex m_mutex1, m_mutex2, m_mutex3, m_mutex4;
    protocol_encryption::ZcClientCodec m_codec;
    std::string m_hardKey, m_errString;
    zwift_network::Motion m_motion1, m_motion2;
    boost::asio::io_context m_asioCtx;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::steady_timer m_tmrConnect, m_tmrKeepAlive, m_tmrKeepAliveIOS;
    std::function<void()> m_func;
    std::vector<protobuf::GameToPhoneCommand> m_commands;
    std::queue<protobuf::PhoneToGameCommand> m_ptgc_queue;
    uint8_t m_buf[65536] = {};
    std::vector<uint8_t> m_decrVector;
    double m_max_ptg_f9 = 0.0;
    uint64_t m_playerId = 0;
    int64_t m_worldId = 0;
    uint32_t m_starts = 0, m_seqNoGtp = 0, m_seqNoCmd = 0, m_maxPtgSeqno = 0, m_last_command_seq_num_phone_reports = 0;
    protobuf::IPProtocol m_proto = protobuf::TCP;
    uint16_t m_hardPort = 0;
    bool m_stopped = true, m_encryption = false, m_writeToMot2 = false, m_hasMotionData = false, m_connectedOK = false, 
        m_lastStatus = false, m_teleportAllowed = false, m_use_metric = false, m_connInProgress = false, m_ci2telemetry_sent = false, m_socketWriteInProgress = false;
    AuxiliaryController(int64_t playerId, GlobalState *gs, WorldClockService *wcs, /*OMIT AuxiliaryControllerStatistics*/std::function<void()> f) : 
        m_gs(gs), m_wcs(wcs), m_ncs(wcs->m_ncs), m_socket(m_asioCtx), m_tmrConnect(m_asioCtx), m_tmrKeepAlive(m_asioCtx), m_tmrKeepAliveIOS(m_asioCtx), m_func(f) {}
    void send_game_to_phone(protobuf::GameToPhone *gtp) {
        if (m_connectedOK) {
            m_buf[0] = 1;
            gtp->set_seqno(_InterlockedExchangeAdd(&m_seqNoGtp, 1));
            gtp->set_ack_seqno(m_maxPtgSeqno);
            gtp->set_teleport_allowed(m_teleportAllowed);
            gtp->set_use_metric(m_use_metric);
            gtp->set_time(m_wcs->getWorldTime());
            {
                std::lock_guard l(m_mutex3);
                for (auto &i : m_commands) {
                    if (i.seqno() <= m_last_command_seq_num_phone_reports)
                        NetworkingLogWarn("Auxiliary Controller discarding command %d: i->sequence_number() > last_command_seq_num_phone_reports_ (%d > %d)", (int)i.type(), i.seqno(), m_last_command_seq_num_phone_reports);
                    else if (m_proto == protobuf::TCP || i.type() != protobuf::GAME_TO_PHONE_SOCIAL_PLAYER_ACTION)
                        i.Swap(gtp->add_game_to_phone_cmds());
                }
                m_commands.clear();
            }
            auto len = uint32_t(gtp->protobuf::GameToPhone::ByteSizeLong());
            uint8_t *buf = (uint8_t *)malloc(len + 4);
            *(uint32_t *)buf = htonl(len);
            if (gtp->SerializeToArray(buf + 4, len)) {
                m_asioCtx.post([this, buf, len]() {
                    this->attempt_write_to_tcp_socket(buf, len + 4);
                });
            } else {
                NetworkingLogError("Auxiliary Controller failed to encode GameToPhone protobuf.");
            }
        }
    }
    void attempt_write_to_tcp_socket(uint8_t *buf, uint32_t len) {
        if (!m_socketWriteInProgress) {
            m_socketWriteInProgress = true;
            if (m_encryption) {
                auto payloadLen = len - 4;
                std::vector<uint8_t> encrypted;
                encrypted.resize(4);
                if (!this->m_codec.encode(buf + 4, payloadLen, &encrypted, &this->m_errString)) {
                    free(buf);
                    //OMIT AuxiliaryControllerStatistics::increaseEncryptionEncodeError((_Mtx_t)this->m_stat);
                    NetworkingLogError("Auxiliary Controller failed to encode message: %s", this->m_errString.c_str());
                    return;
                }
                free(buf);
                len = uint32_t(encrypted.size());
                *(uint32_t *)encrypted.data() = len - 4;
                buf = (uint8_t *)malloc(len);
                memmove(buf, encrypted.data(), len);
            }
            m_socket.async_send(boost::asio::buffer(buf, len), [this, buf, len](const boost::system::error_code &error, std::size_t bytes_transferred) {
                //OMITAuxiliaryControllerStatistics::registerNetUseOut(this->m_stat, bytes_transferred);
                bool disconnect = true;
                if (error) {
                    NetworkingLogError("Auxiliary Controller failed to send data: '%s' (%d)", error.message().c_str(), error.value());
                } else {
                    if (bytes_transferred == len )
                        disconnect = false;
                    else
                        NetworkingLogError("Auxiliary Controller failed to send the correct number of bytes (%d/%d)", bytes_transferred, len);
                }
                if (disconnect)
                    this->disconnect();
                this->m_socketWriteInProgress = false;
                if (buf)
                    free(buf);
            });
        }
        m_asioCtx.post([this, buf, len]() {
            this->attempt_write_to_tcp_socket(buf, len);
        });
    }
    uint16_t init_encryption(const AuxiliaryControllerAddress &addr) {
        if (!m_encryption)
            return addr.m_localPort;
        if (!m_hardKey.empty()) {
            NetworkingLogDebug("Auxiliary Controller using a hardcoded secret key for encryption (prefs.xml) [%s]", m_hardKey.c_str());
            auto hardKey = base64::toBin(m_hardKey);
            m_codec.m_generateKey = false;
            if (hardKey.size() != 16) {
                m_errString = "decode key is greater than expected"s;
                NetworkingLogError("Auxiliary Controller failed to set secret key: %s", m_errString.c_str());
                return m_hardPort;
            }
            memmove(m_codec.m_secretRaw, hardKey.data(), 16);
            return m_hardPort;
        }
        if (addr.m_key.empty()) {
            m_encryption = false;
            return addr.m_localPort;
        }
        m_codec.m_secret = base64::toString(addr.m_key);
        NetworkingLogDebug("Auxiliary Controller using a generated secret key for encryption (/profiles/me/phone) [%s]", m_codec.m_secret.c_str());
        m_codec.m_generateKey = false;
        memmove(m_codec.m_secretRaw, addr.m_key.data(), std::min((int)sizeof(m_codec.m_secretRaw), (int)addr.m_key.size()));
        return (uint16_t)addr.m_localCPort;
    }
    void handleWorldIdChange(int64_t worldId) override {
        m_worldId = worldId;
        protobuf::GameToPhone gtf;
        gtf.set_player_id(m_playerId);
        gtf.set_world_id(worldId);
        send_game_to_phone(&gtf);
    }
    void tcp_connect(const AuxiliaryControllerAddress &addr) {
        auto port = init_encryption(addr);
        //OMIT AuxiliaryControllerStatistics::setEncryptionEnabled((_Mtx_t)this->m_stat, this->m_encryption);
        NetworkingLogInfo("Auxiliary Controller attempting to connect to phone at: %s:%d%s", addr.m_localIp.c_str(), port, m_encryption ? " (secure)" : "");
        boost::system::error_code ec;
        auto ip_addr = boost::asio::ip::make_address(addr.m_localIp.c_str(), ec);
        if (ec) {
            NetworkingLogError("Auxiliary Controller failed to get IP address: '%s' (%d)", ec.message().c_str(), ec.value());
        } else {
            boost::asio::ip::tcp::endpoint ep(ip_addr, port);
            if (m_connInProgress) {
                NetworkingLogWarn("Auxiliary Controller connection attempt already in progress");
            } else {
                m_connInProgress = true;
                m_tmrConnect.expires_after(std::chrono::seconds(5));
                m_tmrConnect.async_wait([this](const std::error_code &ec) {
                    //socket_connect_timer_handler
                    if (ec) {
                        if (ec.category() != boost::asio::error::system_category || ec.value() != ERROR_OPERATION_ABORTED) {
                            NetworkingLogError("Auxiliary Controller socket connect timeout timer: '%s' (%d)", ec.message().c_str(), ec.value());
                            this->m_tmrConnect.cancel();
                        }
                    } else {
                        this->disconnect();
                    }
                });
                m_socket.async_connect(ep, [this](const std::error_code &ec) {
                    //tcp_connection_handler
                    this->m_tmrConnect.cancel();
                    if (ec || !this->m_connInProgress) {
                        NetworkingLogInfo("Auxiliary Controller failed to connect to socket: '%s' (%d)", ec.message().c_str(), ec.value());
                        this->m_connectedOK = false;
                        this->m_lastStatus = false;
                        this->m_ci2telemetry_sent = false;
                        //OMITAuxiliaryControllerStatistics::setPaired((_Mtx_t)v18->m_stat, 0);
                        boost::system::error_code cec;
                        this->m_socket.close(cec);
                        if (cec)
                            NetworkingLogInfo("Auxiliary Controller failed to close socket: '%s' (%d)", cec.message().c_str(), cec.value());
                    } else {
                        if (!this->m_connectedOK || !this->m_lastStatus) {
                            this->m_connectedOK = true;
                            //OMIT AuxiliaryControllerStatistics::setPaired((_Mtx_t)this->m_stat, 1);
                        }
                        if (!this->m_codec.newConnection(&this->m_errString)) {
                            //OMIT AuxiliaryControllerStatistics::increaseEncryptionNewConnectionError((_Mtx_t)this->m_stat);
                            NetworkingLogError("Auxiliary Controller failed to get new connection: %s", this->m_errString.c_str());
                        }
                        NetworkingLogInfo("Auxiliary Controller connected successfully");
                        this->repeat_keep_alive(0);
                        this->do_tcp_receive_encoded_message_length();
                        _InterlockedExchange(&this->m_maxPtgSeqno, 0);
                    }
                });
            }
        }
    }
    void do_stop() {
        m_tmrKeepAliveIOS.cancel();
        m_asioCtx.stop();
        disconnect();
        m_stopped = true;
        std::lock_guard l2(m_mutex2);
        {
            m_hasMotionData = false;
            m_max_ptg_f9 = 0i64;
            std::lock_guard l3(m_mutex3);
            m_last_command_seq_num_phone_reports = 0;
            m_commands.clear();
        }
        NetworkingLogInfo("Auxiliary Controller stopped");
    }
    void stop() {
        std::lock_guard l(m_mutex1);
        if (!m_stopped)
            do_stop();
    }
    ~AuxiliaryController() {
        if (!m_stopped) {
            std::lock_guard l(m_mutex1);
            do_stop();
        }
    }
    bool motion_data(zwift_network::Motion *pDest) {
        std::lock_guard l(m_mutex2);
        if (m_hasMotionData) {
            if (m_writeToMot2)
                *pDest = m_motion1;
            else
                *pDest = m_motion2;
            return true;
        }
        return false;
    }
    void disconnect() {
        m_tmrKeepAlive.cancel();
        m_tmrConnect.cancel();
        if (true) {
            boost::system::error_code ec;
            if (m_connInProgress && !m_socket.is_open()) {
                ec.assign(10009, boost::asio::error::system_category);
            } else {
                m_socket.shutdown(m_socket.shutdown_both, ec);
            }
            if (ec)
                NetworkingLogWarn("Auxiliary Controller failed to shut down tcp socket: '%s' (%d)", ec.message().c_str(), ec.value());
            if (m_connInProgress || m_socket.is_open())
                m_socket.close(ec);
            if (ec)
                NetworkingLogWarn("Auxiliary Controller failed to close tcp socket: '%s' (%d)", ec.message().c_str(), ec.value());
        }
        m_connInProgress = false;
        m_connectedOK = false;
        m_lastStatus = false;
        m_ci2telemetry_sent = false;
        //OMIT AuxiliaryControllerStatistics::setPaired((_Mtx_t)this->m_stat, 0);
    }
    void start(const AuxiliaryControllerAddress &addr, bool shouldEncrypt, uint32_t hardPort, const std::string &hardKey) {
        std::lock_guard l(m_mutex1);
        if (m_stopped) {
            ++m_starts;
            m_proto = addr.m_proto;
            if (m_proto) {
                m_hardPort = hardPort;
                m_hardKey = hardKey;
                m_encryption = shouldEncrypt;
                keep_io_service_alive();
                //OMIT AuxiliaryControllerStatistics::setProtocol((_Mtx_t)this->m_stat, m_proto);
                tcp_connect(addr);
                m_asioCtx.restart();
                boost::asio::detail::win_thread t([this]() {
                    m_asioCtx.run();
                });
                NetworkingLogInfo("Auxiliary Controller started");
                this->m_stopped = false;
            }
        } else {
            NetworkingLogWarn("Auxiliary Controller controller already started");
        }
    }
    void keep_io_service_alive() { //QUEST: what is this for
        m_tmrKeepAliveIOS.expires_after(std::chrono::seconds(30));
        m_tmrKeepAliveIOS.async_wait([this](const boost::system::error_code &ec) {
            if (ec) {
                if (ec.category() == boost::asio::error::system_category && ec.value() == ERROR_OPERATION_ABORTED)
                    NetworkingLogInfo("Auxiliary Controller shutting down io_service keep alive loop");
                else
                    NetworkingLogError("Auxiliary Controller received '%s' (%d) while processing the keep alive timer.", ec.message().c_str(), ec.value());
            } else {
                this->keep_io_service_alive();
            }
        });
    }
    void send_pairing_status(bool good) {
        protobuf::GameToPhoneCommand gtpc;
        gtpc.set_type(protobuf::GAME_TO_PHONE_PAIRING_STATUS);
        gtpc.set_status_good(good);
        NetworkingLogInfo("Sending Pairing Status of: %s", good ? "true" : "false");
        {
            std::lock_guard l(m_mutex3);
            m_seqNoCmd++;
            gtpc.set_seqno(m_seqNoCmd);
            m_commands.emplace_back(std::move(gtpc));
        }
        protobuf::GameToPhone gtp;
        gtp.set_player_id(m_playerId);
        gtp.set_world_id(m_worldId);
        send_game_to_phone(&gtp);
        m_lastStatus = good;
    }
    void repeat_keep_alive(int sec) {
        m_tmrKeepAlive.expires_after(std::chrono::seconds(sec));
        m_tmrKeepAlive.async_wait([this](const boost::system::error_code &ec) {
            if (ec) {
                if (ec.category() == boost::asio::error::system_category && ec.value() == ERROR_OPERATION_ABORTED)
                    return NetworkingLogInfo("Auxiliary Controller shutting down keep alive loop");
                else
                    NetworkingLogError("Auxiliary Controller failed to send keep alive packet: '%s' (%d) during send of keep alive packet.", ec.message().c_str(), ec.value());
            }
            this->send_keep_alive_packet();
        });
    }
    void send_keep_alive_packet() {
        if (!m_buf[0]) {
            protobuf::GameToPhone v14;
            v14.set_player_id(m_playerId);
            v14.set_world_id(m_worldId);
            send_game_to_phone(&v14);
        }
        m_buf[0] = 0;
        repeat_keep_alive(5);
    }
    void do_tcp_receive_encoded_message(uint32_t) {
        //not present
    }
    bool pop_phone_to_game_command(protobuf::PhoneToGameCommand *dest) {
        std::lock_guard l(m_mutex4);
        if (!m_ptgc_queue.empty()) {
            dest->Swap(&m_ptgc_queue.front());
            m_ptgc_queue.pop();
            return true;
        } 
        return false;
    }
    void reconnect(const AuxiliaryControllerAddress &addr) {
        std::lock_guard l(m_mutex1);
        if (!this->m_connectedOK && !this->m_stopped && !this->m_connInProgress) {
            disconnect();
            m_proto = addr.m_proto;
            //OMIT AuxiliaryControllerStatistics::setProtocol((_Mtx_t)this->m_stat, m_proto);
            tcp_connect(addr);
        }
    }
    void do_tcp_receive_encoded_message_length() {
        m_socket.async_receive(boost::asio::buffer(m_buf + 1, 4), [this](const boost::system::error_code &err, std::size_t bytes_transferred) {
            if (err) {
                if (err.category() == boost::asio::error::system_category && err.value() == ERROR_OPERATION_ABORTED)
                    NetworkingLogError("Auxiliary Controller received '%s' (%d) during message length receive handler", err.message().c_str(), err.value());
                this->disconnect();
                return;
            }
            if (!bytes_transferred) {
                NetworkingLogError("Auxiliary Controller received empty message in message length receive handler");
                this->disconnect();
                return;
            }
            //OMIT AuxiliaryControllerStatistics::registerNetUseIn((_Mtx_t)(*a3)->m_stat, bytes_transferred);
            auto len = ntohl(*(uint32_t *)(this->m_buf + 1));
            if (len >= 0x400) {
                NetworkingLogError("Auxiliary Controller received a message length above the threshold");
                this->disconnect();
                return;
            }
            m_socket.async_receive(boost::asio::buffer(m_buf + 1, len), [this, len](const boost::system::error_code &err, std::size_t bytes_transferred) {
                if (err) {
                    if (err.category() == boost::asio::error::system_category && err.value() == ERROR_OPERATION_ABORTED)
                        NetworkingLogError("Auxiliary Controller received '%s' (%d) during message receive handler", err.message().c_str(), err.value());
                    this->disconnect();
                    return;
                }
                if (!bytes_transferred) {
                    NetworkingLogError("Auxiliary Controller received empty message in message receive handler");
                    this->disconnect();
                    return;
                }
                auto pData = this->m_buf + 1;
                //OMIT AuxiliaryControllerStatistics::registerNetUseIn((_Mtx_t)(*a3)->m_stat, bytes_transferred);
                if (this->m_encryption) {
                    this->m_decrVector.clear();
                    if (!this->m_codec.decode(pData, bytes_transferred, &this->m_decrVector, &this->m_errString)) {
                        //OMIT AuxiliaryControllerStatistics::increaseEncryptionDecodeError((_Mtx_t)(*this)->m_stat);
                        NetworkingLogError("Auxiliary Controller failed to decode message: %s", this->m_errString.c_str());
                        this->disconnect();
                        return;
                    }
                    pData = this->m_decrVector.data();
                    bytes_transferred = this->m_decrVector.size();
                }
                if (pData) {
                    protobuf::PhoneToGame ptg;
                    if (ptg.ParseFromArray(pData, bytes_transferred)) {
                        this->process_phone_to_game(ptg);
                        this->do_tcp_receive_encoded_message_length();
                    } else {
                        NetworkingLogError("Auxiliary Controller failed to parse message");
                        this->disconnect();
                    }
                }
            });
        });
    }
    void process_phone_to_game(const protobuf::PhoneToGame &ptg) {
        if (ptg.commands_size()) {
            for(auto &i : ptg.commands()) {
                if (i.seqno() <= m_maxPtgSeqno) {
                    NetworkingLogWarn("Discarding command from phone: %d", (int)i.command());
                } else {
                    if (i.command() == protobuf::PAIRING_AS) {
                        if (i.player_id() == m_playerId) {
                            send_pairing_status(true);
                        } else {
                            NetworkingLogWarn("BAD PAIRING! Mobile Player ID: %d is not equal to Game Player ID: %d", i.player_id(), m_playerId);
                            send_pairing_status(false);
                            m_func();
                        }
                    } else {
                        /* OMIT if (!this->m_ci2telemetry_sent && i.command() == protobuf::PHONE_TO_GAME_PACKET)
                        {
                            memset(v37, 0, 0xD8ui64);
                            protobuf::GamePacket::GamePacket((__int64)v37, 0i64, 0);
                            v37[0] = (__int64)&protobuf::GamePacket::`vftable';
                                if ((unsigned __int8)protobuf::MessageLite::ParseFromString(
                                    v37,
                                    *(_QWORD *)(v10 + 48) & 0xFFFFFFFFFFFFFFF8ui64))
                                {
                                    if ((v37[2] & 8) != 0)
                                    {
                                        v17 = &off_7FF76B03B9A0;
                                        if (v37[9])
                                            v17 = (void ***)v37[9];
                                        AuxiliaryControllerStatistics::setZcInfo((_Mtx_t)this->m_stat, (__int64)v17);
                                        this->m_field_103C0 = 1;
                                    }
                                } else
                                {
                                    v16 = Logger::getLogger();
                                    NetworkingLog(NL_ERROR, &v16->m_errorPrefix, "Auxiliary Controller failed to parse GamePacket message");
                                }
                                sub_7FF76A4BE070(v37);
                        }*/
                        std::lock_guard l(m_mutex4);
                        m_ptgc_queue.emplace(i);
                    }
                    m_maxPtgSeqno = i.seqno();
                }
            }
        }
        if (ptg.has_f3()) {
            if (ptg.f9() > m_max_ptg_f9) {
                m_max_ptg_f9 = ptg.f9();
                auto pDestMot = m_writeToMot2 ? &m_motion2 : &m_motion1;
                pDestMot->m_ptg_f3 = ptg.f3();
                pDestMot->m_ptg_f4 = ptg.f4();
                pDestMot->m_ptg_f5 = ptg.f5();
                pDestMot->m_phoneRot = ptg.phone_rot();
                pDestMot->m_ptg_f7 = ptg.f7();
                pDestMot->m_ptg_f8 = ptg.f8();
                pDestMot->m_ptg_f9 = m_max_ptg_f9;
                std::lock_guard l(m_mutex2);
                m_writeToMot2 = !m_writeToMot2;
                m_hasMotionData = true;
            }
        }
        {
            std::lock_guard l(m_mutex3);
            if (m_last_command_seq_num_phone_reports < ptg.seqno())
                m_last_command_seq_num_phone_reports = ptg.seqno();
        }
    }
    void send_clear_power_up_command() {
        protobuf::GameToPhoneCommand gtpc;
        gtpc.set_type(protobuf::GAME_TO_PHONE_CLEAR_POWER_UP);
        std::lock_guard l(m_mutex3);
        m_seqNoCmd++;
        gtpc.set_seqno(m_seqNoCmd);
        m_commands.emplace_back(std::move(gtpc));
    }
    void send_activate_power_up_command(int powerupId, uint32_t powerupParam) {
        protobuf::GameToPhoneCommand gtpc;
        gtpc.set_type(protobuf::GAME_TO_PHONE_ACTIVATE_POWER_UP);
        std::lock_guard l(m_mutex3);
        m_seqNoCmd++;
        gtpc.set_seqno(m_seqNoCmd);
        gtpc.set_powerup_id(powerupId);
        gtpc.set_powerup_param(powerupParam);
        m_commands.emplace_back(std::move(gtpc));
    }
    void send_ble_peripheral_request(const protobuf::BLEPeripheralRequest &req) {
        protobuf::GameToPhoneCommand gtpc;
        gtpc.set_type(protobuf::GAME_TO_PHONE_BLEPERIPHERAL_REQUEST);
        auto rq = req.New(gtpc.GetArena());
        rq->CopyFrom(req);
        gtpc.set_allocated_ble_rq(rq);
        std::lock_guard l(m_mutex3);
        m_seqNoCmd++;
        gtpc.set_seqno(m_seqNoCmd);
        m_commands.emplace_back(std::move(gtpc));
    }
    void send_customize_action_button_command(uint32_t a2, uint32_t a3, const std::string &a4, const std::string &a5, bool a6) {
        protobuf::GameToPhoneCommand gtpc;
        gtpc.set_type(protobuf::GAME_TO_PHONE_CUSTOMIZE_ACTION_BUTTON);
        gtpc.set_f8(a2);
        gtpc.set_f9(a3);
        gtpc.set_f10(a4);
        gtpc.set_f11(a5);
        gtpc.set_f13(a6);
        std::lock_guard l(m_mutex3);
        m_seqNoCmd++;
        gtpc.set_seqno(m_seqNoCmd);
        m_commands.emplace_back(std::move(gtpc));
    }
    void send_default_activity_name(const std::string &name) {
        protobuf::GameToPhoneCommand gtpc;
        gtpc.set_type(protobuf::GAME_TO_PHONE_DEFAULT_ACTIVITY_NAME);
        gtpc.set_act_name(name);
        std::lock_guard l(m_mutex3);
        m_seqNoCmd++;
        gtpc.set_seqno(m_seqNoCmd);
        m_commands.emplace_back(std::move(gtpc));
    }
    void send_game_packet(const std::string &a2, bool force) {
        protobuf::GameToPhoneCommand gtpc;
        gtpc.set_type(protobuf::GAME_TO_PHONE_PACKET);
        gtpc.set_f21(a2);
        {
            std::lock_guard l(m_mutex3);
            m_seqNoCmd++;
            gtpc.set_seqno(m_seqNoCmd);
            m_commands.emplace_back(std::move(gtpc));
        }
        if (force) {
            protobuf::GameToPhone gtp;
            gtp.set_player_id(m_playerId);
            gtp.set_world_id(m_worldId);
            send_game_to_phone(&gtp);
        }
    }
    void send_image_to_mobile_app(const std::string &pathName, const std::string &imgName) {
        if (m_proto == protobuf::TCP) {
            NetworkingLogInfo("Uploading image to mobile device %s", pathName.c_str());
            std::ifstream v19(pathName, std::ios::binary);
            if (!v19.is_open()) {
                NetworkingLogError("Activity Upload failed to read image file");
            } else {
                protobuf::GameToPhoneCommand gtpc;
                gtpc.set_type(protobuf::GAME_TO_PHONE_SEND_IMAGE);
                std::string content((std::istreambuf_iterator<char>(v19)), std::istreambuf_iterator<char>());
                gtpc.set_img_bits(content);
                gtpc.set_img_name(imgName);
                std::lock_guard l(m_mutex3);
                m_seqNoCmd++;
                gtpc.set_seqno(m_seqNoCmd);
                m_commands.emplace_back(std::move(gtpc));
            }
        }
    }
    void send_mobile_alert(const protobuf::MobileAlert &alert) {
        protobuf::GameToPhoneCommand gtpc;
        gtpc.set_type(protobuf::GAME_TO_PHONE_MOBILE_ALERT);
        auto newAlert = alert.New(gtpc.GetArena());
        newAlert->CopyFrom(alert);
        gtpc.set_allocated_mobile_alert(newAlert);
        std::lock_guard l(m_mutex3);
        m_seqNoCmd++;
        gtpc.set_seqno(m_seqNoCmd);
        m_commands.emplace_back(std::move(gtpc));
    }
    void send_mobile_alert_cancel_command(const protobuf::MobileAlert &alert) {
        protobuf::GameToPhoneCommand gtpc;
        gtpc.set_type(protobuf::GAME_TO_PHONE_MOBILE_ALERT_CANCEL);
        auto newAlert = alert.New(gtpc.GetArena());
        newAlert->CopyFrom(alert);
        gtpc.set_allocated_mobile_alert(newAlert);
        std::lock_guard l(m_mutex3);
        m_seqNoCmd++;
        gtpc.set_seqno(m_seqNoCmd);
        m_commands.emplace_back(std::move(gtpc));
    }
    void send_player_profile(const protobuf::PlayerProfile &prof) {
        protobuf::GameToPhone gtp;
        auto newProf = prof.New(gtp.GetArena());
        newProf->CopyFrom(prof);
        newProf->clear_email();
        newProf->clear_dob();
        gtp.set_allocated_player_profile(newProf);
        gtp.set_player_id(m_playerId);
        gtp.set_world_id(m_worldId);
        send_game_to_phone(&gtp);
    }
    void send_player_state(const protobuf::PlayerState &state) {
        protobuf::GameToPhone gtp;
        auto newState = state.New(gtp.GetArena());
        newState->CopyFrom(state);
        gtp.set_allocated_player_state(newState);
        gtp.set_player_id(m_playerId);
        gtp.set_world_id(m_worldId);
        send_game_to_phone(&gtp);
    }
    void send_rider_list_entries(const std::list<protobuf::RiderListEntry> &riders) {
        protobuf::GameToPhone gtp;
        for (auto &i : riders)
            gtp.add_riders()->CopyFrom(i);
        gtp.set_player_id(m_playerId);
        gtp.set_world_id(m_worldId);
        send_game_to_phone(&gtp);
    }
    void send_set_power_up_command(const std::string &locName, const std::string &color, const std::string &mask, int powerupId) {
        protobuf::GameToPhoneCommand gtpc;
        gtpc.set_type(protobuf::GAME_TO_PHONE_SET_POWER_UP);
        gtpc.set_loc_name(locName);
        gtpc.set_color(color);
        gtpc.set_mask(mask);
        gtpc.set_powerup_id(powerupId);
        std::lock_guard l(m_mutex3);
        m_seqNoCmd++;
        gtpc.set_seqno(m_seqNoCmd);
        m_commands.emplace_back(std::move(gtpc));
    }
    void send_social_player_action(const protobuf::SocialPlayerAction &SPA) {
        protobuf::GameToPhoneCommand gtpc;
        gtpc.set_type(protobuf::GAME_TO_PHONE_SOCIAL_PLAYER_ACTION);
        auto newSPA = SPA.New(gtpc.GetArena());
        newSPA->CopyFrom(SPA);
        gtpc.set_allocated_spa(newSPA);
        std::lock_guard l(m_mutex3);
        m_seqNoCmd++;
        gtpc.set_seqno(m_seqNoCmd);
        m_commands.emplace_back(std::move(gtpc));
    }
        /* inlined: set_connection_handlers()
register_bytes_out(uint64_t)
get_world_id()
clear_telemetry()
add_pending_command(std::shared_ptr<protobuf::GameToPhoneCommand> const&)
set_client_info_to_telemetry(protobuf::PhoneToGameCommand const&)
connect(zwift_network::AuxiliaryControllerAddress const&)
do_receive()
*/
};
#define InitializeCNLfirst(T, cond) if (!(cond)) return makeNetworkResponseFuture<T>(NRO_NOT_INITIALIZED, "Initialize CNL first"s);
struct NetworkClientImpl { //0x400 bytes, calloc
    std::string m_server;
    MachineIdProviderFactory m_machine;
    CurlHttpConnectionFactory m_curlf;
    NetworkClientOptions m_nco{ false, 20, 300, /*NL_INFO RTL*/ NL_DEBUG };
    time_t m_netStartTime1 = 0, m_netStartTime2 = 0;
    std::unique_ptr<EventLoop> m_evLoop;
    std::unique_ptr<GenericHttpConnectionManager> m_httpConnMgr0, m_httpConnMgr1, m_httpConnMgr2;
    std::unique_ptr<ZwiftHttpConnectionManager> m_httpConnMgr3, m_httpConnMgr4;
    std::unique_ptr<ZwiftAuthenticationManager> m_authMgr;
    std::unique_ptr<AuthServerRestInvoker> m_authInvoker;
    std::unique_ptr<GlobalState> m_globalState;
    std::unique_ptr<HashSeedService> m_hashSeed1, m_hashSeed2;
    std::unique_ptr<WorldAttributeService> m_wat;
    std::unique_ptr<ProfileRequestDebouncer> m_profRqDebouncer;
    std::unique_ptr<UdpClient> m_udpClient;
    std::unique_ptr<TcpClient> m_tcpClient;
    std::unique_ptr<WorldClockService> m_wclock;
    std::unique_ptr<RelayServerRestInvoker> m_relay;
    std::unique_ptr<NetworkClockService> m_netClock;
    std::unique_ptr<UdpStatistics> m_udpStat;
    std::unique_ptr<RestServerRestInvoker> m_restInvoker;
    std::unique_ptr<ExperimentsRestInvoker> m_expRi;
    std::unique_ptr<ActivityRecommendationRestInvoker> m_arRi;
    std::unique_ptr<AchievementsRestInvoker> m_achRi;
    std::unique_ptr<CampaignRestInvoker> m_camRi;
    std::unique_ptr<ClubsRestInvoker> m_clubsRi;
    std::unique_ptr<EventCoreRestInvoker> m_ecRi;
    std::unique_ptr<EventFeedRestInvoker> m_efRi;
    std::unique_ptr<FirmwareUpdateRestInvoker> m_fuRi;
    std::unique_ptr<GenericRestInvoker> m_gRi;
    std::unique_ptr<PrivateEventsRestInvoker> m_peRi;
    std::unique_ptr<RaceResultRestInvoker> m_rarRi;
    std::unique_ptr<RouteResultsRestInvoker> m_rorRi;
    std::unique_ptr<PlayerPlaybackRestInvoker> m_ppbRi;
    std::unique_ptr<SegmentResultsRestInvoker> m_srRi;
    std::unique_ptr<PowerCurveRestInvoker> m_pcRi;
    std::unique_ptr<ZFileRestInvoker> m_zfRi;
    std::unique_ptr<ZwiftWorkoutsRestInvoker> m_zwRi;
    std::unique_ptr<WorkoutServiceRestInvoker> m_wsRi;
    std::unique_ptr<TcpStatistics> m_tcpStat;
    std::unique_ptr<WorldClockStatistics> m_wcStat;
    std::unique_ptr<LanExerciseDeviceStatistics> m_lanStat;
    std::unique_ptr<AuxiliaryControllerStatistics> m_auxStat;
    std::unique_ptr<WorldAttributeStatistics> m_waStat;
    std::unique_ptr<LanExerciseDeviceService> m_lanService;
    std::unique_ptr<AuxiliaryController> m_aux;
    moodycamel::ReaderWriterQueue<const AuxiliaryControllerAddress> m_rwqAux;
    AuxiliaryControllerAddress m_curAux;
    uint32_t m_limitProfPerRq = 100;
    bool m_tcpDisconnected = false, m_initOK = false, m_loginOK = false, m_someFunc0 = false;
    NetworkClientImpl() : m_rwqAux(1) {
        google::protobuf::internal::VerifyVersion(3021000 /* URSOFT FIX: slightly up from 3020000*/, 3020000, __FILE__);
    }
    NetworkLogLevel GetNetworkMaxLogLevel() const { return m_nco.m_maxLogLevel; }
    void startTcpClient() {
        if (!m_tcpClient && !m_nco.m_bHttpOnly)
            m_tcpClient.reset(new TcpClient(m_globalState.get(), m_wclock.get(), m_hashSeed1.get(), m_wat.get(), m_relay.get(), m_srRi.get(), this));
    }
    void shutdownTcpClient() {
        if (m_tcpClient) {
            m_tcpClient->shutdown();
            m_tcpClient.reset(nullptr);
        }
    }
    void handleDisconnectRequested(bool mode) { //1st vfunc
        if (mode)
            shutdownTcpClient();
        else
            startTcpClient();
        m_tcpDisconnected = mode;
    }
    void handleWorldAndMapRevisionChanged(int64_t worldId, uint32_t mapRevision) { //2nd vfunc
        if (m_tcpClient)
            m_tcpClient->handleWorldAndMapRevisionChanged(worldId, mapRevision);
    }
    ~NetworkClientImpl() { //3rd vfunc
        //IDA NetworkClientImpl_destroy - automatic
    }
    void shutdownUdpClient() {
        if (m_udpClient) {
            m_wat->removeListener(m_udpClient.get());
            m_udpClient->shutdown();
            m_udpClient.reset(nullptr);
        }
    }
    void shutdownAuxiliaryController() {
        if (m_aux) {
            m_aux->stop();
            m_aux.reset(nullptr);
        }
    }
    void logLibSummary() {
        NetworkingLogInfo("CNL %s", g_CNL_VER.c_str());
        NetworkingLogInfo("Machine Id: %s", m_machine.m_id.c_str());
    }
    void initialize(const std::string &serv, const std::string &certs, const std::string &zaVersion) {
        m_server = serv;
        //SetNetworkMaxLogLevel(maxLogLevel); combined with g_MinLogLevel (why two???)
        m_curlf.initialize(zaVersion, m_machine.m_id);
        _time64(&m_netStartTime1);
        m_netStartTime2 = m_netStartTime1;
        logLibSummary();
        bool skipCertCheck = m_nco.m_skipCertCheck;
        //smart shared pointers actively used here, but what for???
        m_httpConnMgr0.reset(new GenericHttpConnectionManager(&m_curlf, certs, skipCertCheck, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_SEQUENTIAL));
        m_httpConnMgr1.reset(new GenericHttpConnectionManager(&m_curlf, certs, skipCertCheck, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_CONCURRENT));
        m_httpConnMgr2.reset(new GenericHttpConnectionManager(&m_curlf, certs, skipCertCheck, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_SEQUENTIAL));
        m_authMgr.reset(new ZwiftAuthenticationManager(m_server));
        m_httpConnMgr3.reset(new ZwiftHttpConnectionManager(&m_curlf, certs, m_nco.m_skipCertCheck, m_authMgr.get(), &m_tcpDisconnected, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_SEQUENTIAL, 3));
        m_httpConnMgr4.reset(new ZwiftHttpConnectionManager(&m_curlf, certs, m_nco.m_skipCertCheck, m_authMgr.get(), &m_tcpDisconnected, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_CONCURRENT, 3));
        m_expRi.reset(new ExperimentsRestInvoker(m_httpConnMgr3.get(), m_server));
        m_arRi.reset(new ActivityRecommendationRestInvoker(m_httpConnMgr4.get(), m_server));
        m_achRi.reset(new AchievementsRestInvoker(m_httpConnMgr4.get(), m_server));
        m_authInvoker.reset(new AuthServerRestInvoker(m_machine.m_id, m_authMgr.get(), m_httpConnMgr3.get(), m_expRi.get(), m_server));
        m_camRi.reset(new CampaignRestInvoker(m_httpConnMgr3.get(), m_server));
        m_clubsRi.reset(new ClubsRestInvoker(m_httpConnMgr3.get(), m_server));
        m_ecRi.reset(new EventCoreRestInvoker(m_httpConnMgr3.get(), m_server));
        m_efRi.reset(new EventFeedRestInvoker(m_httpConnMgr4.get(), m_server));
        m_fuRi.reset(new FirmwareUpdateRestInvoker(m_httpConnMgr3.get(), m_server));
        m_gRi.reset(new GenericRestInvoker(m_httpConnMgr1.get()));
        m_peRi.reset(new PrivateEventsRestInvoker(m_httpConnMgr3.get(), m_server));
        m_rarRi.reset(new RaceResultRestInvoker(m_httpConnMgr3.get(), m_server));
        m_rorRi.reset(new RouteResultsRestInvoker(m_httpConnMgr3.get(), m_server));
        m_ppbRi.reset(new PlayerPlaybackRestInvoker(m_httpConnMgr4.get(), m_server));
        m_srRi.reset(new SegmentResultsRestInvoker(m_httpConnMgr4.get(), m_server));
        m_pcRi.reset(new PowerCurveRestInvoker(m_httpConnMgr4.get(), m_server));
        m_zfRi.reset(new ZFileRestInvoker(m_httpConnMgr3.get(), m_server));
        m_zwRi.reset(new ZwiftWorkoutsRestInvoker(m_httpConnMgr3.get(), m_server));
        m_wsRi.reset(new WorkoutServiceRestInvoker(m_httpConnMgr4.get(), m_server));
        m_udpStat.reset(new UdpStatistics());
        m_tcpStat.reset(new TcpStatistics());
        m_wcStat.reset(new WorldClockStatistics());
        m_lanStat.reset(new LanExerciseDeviceStatistics());
        m_auxStat.reset(new AuxiliaryControllerStatistics());
        m_waStat.reset(new WorldAttributeStatistics());
        //OMIT telemetry
        m_restInvoker.reset(new RestServerRestInvoker(m_machine.m_id, m_httpConnMgr3.get(), m_server, zaVersion));
        m_lanService.reset(new LanExerciseDeviceService(zaVersion, 0x14E9, 1, 100, 5, 30));
        NetworkingLogInfo("CNL initialized");
        m_initOK = true;
    }
    void onLoggedIn(const protobuf::PerSessionInfo &psi, const std::string &sessionId, const EncryptionInfo &ei) {
        m_netClock.reset(new NetworkClockService(psi.time()));
        m_loginOK = false;
        shutdownUdpClient();
        shutdownServiceEventLoop();
        m_evLoop.reset(new EventLoop());
        m_globalState.reset(new GlobalState(m_evLoop.get(), psi, sessionId, ei));
        m_httpConnMgr3->setGlobalState(m_globalState.get());
        //OMIT TelemetryService::setGlobalState(this->m_ts, &v88);
        m_relay.reset(new RelayServerRestInvoker(m_httpConnMgr3.get(), HttpHelper::sanitizeUrl(psi.relay_url())));
        m_wat.reset(new WorldAttributeService());
        m_wclock.reset(new WorldClockService(m_evLoop.get(), m_netClock.get()));
        m_hashSeed1.reset(new HashSeedService(m_evLoop.get(), m_globalState.get(), m_relay.get(), m_wclock.get(), false));
        m_hashSeed1->start();
        m_hashSeed2.reset(new HashSeedService(m_evLoop.get(), m_globalState.get(), m_relay.get(), m_wclock.get(), true));
        m_hashSeed2->start();
        m_profRqDebouncer.reset(new ProfileRequestDebouncer(m_evLoop.get(), m_restInvoker.get(), m_limitProfPerRq));
        if (!m_nco.m_bHttpOnly) {
            m_udpClient.reset(new UdpClient(m_globalState.get(), m_wclock.get(), m_hashSeed1.get(), m_hashSeed2.get(), m_udpStat.get(), m_relay.get(), this));
            m_globalState->registerUdpConfigListener(m_udpClient.get());
            m_globalState->registerEncryptionListener(m_udpClient.get());
            m_wat->registerListener(m_udpClient.get());
        }
        NetworkingLogInfo("Session ID: %s%s", sessionId.c_str(), m_globalState->shouldUseEncryption() ? " (secure)" : "");
        NetworkingLogInfo("Logged in");
        m_loginOK = true;
        //OMIT TelemetryService::remoteLogF(*(_DWORD **)v49, 0, 3u, "session", "Session ID %s", v58->_Bx._Buf);
    }
    std::future<NetworkResponse<std::string>> logInWithOauth2Credentials(/*ret a2,*/ const std::string &sOauth, const std::vector<std::string> &anEventProps, const std::string &oauthClient) {
        InitializeCNLfirst(std::string, m_authMgr && m_authInvoker);
        auto ret = m_authMgr->setCredentialsMid(sOauth, oauthClient);
        return ret.m_errCode ? makeNetworkResponseFuture<std::string>(ret.m_errCode, std::move(ret.m_msg)) :
            m_authInvoker->logIn({ m_nco.m_disableEncr, m_nco.m_disableEncryptionWithServer, m_nco.m_ignoreEncryptionFeatureFlag, m_nco.m_secretKeyBase64 },
                anEventProps, 
                [this](const protobuf::PerSessionInfo &psi, const std::string &str, const EncryptionInfo &ei) { onLoggedIn(psi, str, ei); });
    }
    std::future<NetworkResponse<std::string>> logInWithEmailAndPassword(const std::string &email, const std::string &pwd, const std::vector<std::string> &anEventProps, bool reserved, const std::string &oauthClient) {
        InitializeCNLfirst(std::string, m_authMgr && m_authInvoker);
        m_authMgr->setCredentialsOld(email, pwd, oauthClient);
        if (reserved)
            return makeNetworkResponseFuture<std::string>(NRO_NO_LOG_IN_ATTEMPTED, "Good luck, soldier"s);
        else
            return m_authInvoker->logIn(
                { m_nco.m_disableEncr, m_nco.m_disableEncryptionWithServer, m_nco.m_ignoreEncryptionFeatureFlag, m_nco.m_secretKeyBase64},
                anEventProps, [this](const protobuf::PerSessionInfo &psi, const std::string &str, const EncryptionInfo &ei) { 
                    onLoggedIn(psi, str, ei); 
                });
    }
    std::future<NetworkResponse<std::string>> logOut() {
        InitializeCNLfirst(std::string, m_initOK);
        shutdownTcpClient();
        if (m_loginOK) {
            auto WorldId = m_globalState->getWorldId();
            if (WorldId > 0) {
                auto v5 = m_relay->leaveWorld(WorldId);
                v5.wait();
            }
            m_globalState->setWorldId(0);
        }
        //OMIT if (m_ts->isEnabled())
        //    m_ts->lastSampleMetrics();
        m_loginOK = false;
        return m_authInvoker->logOut([this]() {
            m_netClock.reset(nullptr);
            m_wclock.reset(nullptr);
            m_hashSeed1.reset(nullptr);
            m_hashSeed2.reset(nullptr);
            m_profRqDebouncer.reset(nullptr);
            m_relay.reset(nullptr);
            shutdownUdpClient();
            m_wat.reset(nullptr);
            shutdownAuxiliaryController();
            shutdownServiceEventLoop();
            m_globalState.reset(nullptr);
            //omit m_ts->clearUserState();
            NetworkingLogInfo("Logged out");
            });
    }
    void shutdownServiceEventLoop() {
        if (m_evLoop) {
            m_evLoop->shutdown();
            m_evLoop = nullptr;
        }
    }
    std::future<NetworkResponse<void>> resetPassword(const std::string &newPwd) {
        InitializeCNLfirst(void, m_initOK);
        return m_authInvoker->resetPassword(newPwd);
    }
    std::future<NetworkResponse<protobuf::PlayerState>> latestPlayerState(int64_t worldId, int64_t playerId) {
        if (!m_loginOK)
            return makeNetworkResponseFuture<protobuf::PlayerState>(NRO_NOT_LOGGED_IN, "Log in first"s);
        if (worldId <= 0)
            return makeNetworkResponseFuture<protobuf::PlayerState>(NRO_INVALID_ARGUMENT, "Invalid world id"s);
        if (playerId <= 0)
            return makeNetworkResponseFuture<protobuf::PlayerState>(NRO_INVALID_ARGUMENT, "Invalid player id"s);
        return m_relay->latestPlayerState(worldId, playerId);
    }
    std::future<NetworkResponse<void>> removeFollowee(int64_t playerId, int64_t followeeId) {
        InitializeCNLfirst(void, m_restInvoker);
        if (playerId <= 0)
            return makeNetworkResponseFuture<void>(NRO_INVALID_ARGUMENT, "Invalid player id"s);
        if (followeeId <= 0)
            return makeNetworkResponseFuture<void>(NRO_INVALID_ARGUMENT, "Invalid followee player id"s);
        return m_restInvoker->removeFollowee(playerId, followeeId);
    }
    std::future<NetworkResponse<protobuf::SocialNetworkStatus>> addFollowee(int64_t playerId, int64_t followeeId, bool a5, protobuf::ProfileFollowStatus pfs) {
        InitializeCNLfirst(protobuf::SocialNetworkStatus, m_restInvoker);
        if (playerId <= 0)
            return makeNetworkResponseFuture<protobuf::SocialNetworkStatus>(NRO_INVALID_ARGUMENT, "Invalid player id"s);
        if (followeeId <= 0)
            return makeNetworkResponseFuture<protobuf::SocialNetworkStatus>(NRO_INVALID_ARGUMENT, "Invalid followee player id"s);
        return m_restInvoker->addFollowee(playerId, followeeId, a5, pfs);
    }
    void handleAuxiliaryControllerAddress(AuxiliaryControllerAddress &&aux) {
        m_rwqAux.enqueue(aux);
    }
    void onMyPlayerProfileReceived(const NetworkResponse<protobuf::PlayerProfile> &prof) {
        if (!prof.m_errCode) {
            auto playerId = prof.m_T.id();
            NetworkingLogInfo("Player ID: %d", playerId);
            auto &aid = prof.m_T.mix_panel_distinct_id();
            NetworkingLogInfo("Analytics ID: %s", aid.c_str());
            //OMIT TelemetryService::setProfileInfo((__int64)this->m_ts, (__int64)m_pb, v8, v9);
            if (m_globalState->getPlayerId() != playerId) {
                m_globalState->setPlayerId(playerId);
                shutdownAuxiliaryController();
                m_aux.reset(new AuxiliaryController(playerId, m_globalState.get(), m_wclock.get(), /*m_auxStat,*/ [this]() {
                    auto fut = m_relay->setPhoneAddress(""s, 0, protobuf::TCP, 0, ""s);
                    fut.get();
                }));
                m_globalState->registerWorldIdListener(m_aux.get());
                initializeWorkoutManager(prof.m_T);
                shutdownTcpClient();
                startTcpClient();
            }
            if (m_aux)
                m_aux->m_use_metric = prof.m_T.use_metric();
        }
    }
    void initializeWorkoutManager(const protobuf::PlayerProfile &prof) {
        //TODO or OMIT as external workout sources
    }
    std::future<NetworkResponse<protobuf::Goals>> getGoals(int64_t playerId) {
        InitializeCNLfirst(protobuf::Goals, m_restInvoker);
        if (playerId > 0)
            return m_restInvoker->getGoals(playerId);
        else
            return makeNetworkResponseFuture<protobuf::Goals>(NRO_INVALID_ARGUMENT, "Invalid player id"s);
    }
    std::future<NetworkResponse<protobuf::Goal>> saveGoal(const protobuf::Goal &g) {
        InitializeCNLfirst(protobuf::Goal, m_restInvoker);
        return m_restInvoker->saveGoal(g);
    }
    std::future<NetworkResponse<Json::Value>> getActivityRecommendations(const std::string &aGoal) {
        return m_arRi->getActivityRecommendations(aGoal);
    }
    std::future<NetworkResponse<protobuf::PowerCurveAggregationMsg>> getBestEffortsPowerCurveFromAllTime() {
        return m_pcRi->getBestEffortsPowerCurveFromAllTime();
    }
    NetworkRequestOutcome sendClearPowerUpCommand() {
        if (!m_aux || !m_aux->m_connectedOK || !m_aux->m_lastStatus)
            return NRO_NOT_PAIRED_TO_PHONE;
        m_aux->send_clear_power_up_command();
        return NRO_OK;
    }
    NetworkRequestOutcome sendActivatePowerUpCommand(int powerupId, int powerupParam) {
        if (!m_aux || !m_aux->m_connectedOK || !m_aux->m_lastStatus)
            return NRO_NOT_PAIRED_TO_PHONE;
        m_aux->send_activate_power_up_command(powerupId, powerupParam);
        return NRO_OK;
    }
    NetworkRequestOutcome unsubscribeFromSegment(int64_t id) {
        if (!m_loginOK)
            return NRO_NOT_LOGGED_IN;
        if (m_someFunc0)
            return NRO_DISCONNECTED_DUE_TO_SIMULTANEOUS_LOGINS;
        if (!m_tcpClient)
            return NRO_NO_PLAYER_ID_YET;
        m_tcpClient->unsubscribeFromSegment(id);
        return NRO_OK;
    }
    NetworkRequestOutcome sendCustomizeActionButtonCommand(uint32_t a2, uint32_t a3, char *a4, char *a5, bool a6) {
        if (!m_aux || !m_aux->m_connectedOK || !m_aux->m_lastStatus)
            return NRO_NOT_PAIRED_TO_PHONE;
        m_aux->send_customize_action_button_command(a2, a3, a4, a5, a6);
        return NRO_OK;
    }
    bool popServerToClient(std::shared_ptr<protobuf::ServerToClient> &dest) {
        if (!m_loginOK)
            return false;
        if (m_udpClient->popServerToClient(dest))
            return true;
        if (!m_tcpClient)
            return false;
        if (m_tcpClient->popServerToClient(dest))
            return true;
        return false;
    }
    std::future<NetworkResponse<void>> acceptPrivateEventInvitation(int64_t id) {
        InitializeCNLfirst(void, m_peRi);
        return m_peRi->accept(id);
    }
    std::future<NetworkResponse<void>> createRaceResultEntry(const protobuf::RaceResultEntrySaveRequest &rq) {
        InitializeCNLfirst(void, m_rarRi);
        return m_rarRi->createRaceResultEntry(rq);
    }
    std::future<NetworkResponse<protobuf::ZFileProto>> createZFile(const protobuf::ZFileProto &p) {
        InitializeCNLfirst(protobuf::ZFileProto, m_zfRi);
        return m_zfRi->create(p);
    }
    std::future<NetworkResponse<protobuf_bytes>> downloadZFile(int64_t id) {
        InitializeCNLfirst(protobuf_bytes, m_zfRi);
        return m_zfRi->download(id);
    }
    std::future<NetworkResponse<void>> eraseZFile(int64_t id) {
        InitializeCNLfirst(void, m_zfRi);
        return m_zfRi->erase(id);
    }
    std::future<NetworkResponse<protobuf::Achievements>> getAchievements() {
        InitializeCNLfirst(protobuf::Achievements, m_achRi);
        return m_achRi->load();
    }
    std::future<NetworkResponse<protobuf::ListPublicActiveCampaignResponse>> getActiveCampaigns() {
        InitializeCNLfirst(protobuf::ListPublicActiveCampaignResponse, m_camRi);
        return m_camRi->getActiveCampaigns();
    }
    std::future<NetworkResponse<protobuf::EventsProtobuf>> getEvents(const model::EventsSearch &es) {
        InitializeCNLfirst(protobuf::EventsProtobuf, m_efRi);
        return m_efRi->getEvents(es);
    }
    std::future<NetworkResponse<protobuf::FeatureResponse>> getFeatureResponse(const protobuf::FeatureRequest &rq) {
        InitializeCNLfirst(protobuf::FeatureResponse, m_expRi);
        return m_expRi->getFeatureResponse(rq);
    }
    std::future<NetworkResponse<protobuf::LateJoinInformation>> getLateJoinInformation(int64_t meetupId) {
        InitializeCNLfirst(protobuf::LateJoinInformation, m_relay);
        return m_relay->getLateJoinInformation(meetupId);
    }
    std::future<NetworkResponse<protobuf::PlaybackMetadataList>> getMyPlaybacks(int64_t a2) {
        InitializeCNLfirst(protobuf::PlaybackMetadataList, m_ppbRi);
        return m_ppbRi->getMyPlaybacks(a2);
    }
    std::future<NetworkResponse<protobuf::PlaybackData>> getPlaybackData(const protobuf::PlaybackMetadata &md) {
        InitializeCNLfirst(protobuf::PlaybackData, m_ppbRi);
        return m_ppbRi->getPlaybackData(md);
    }
    std::future<NetworkResponse<protobuf::PrivateEventProto>> getPrivateEvent(int64_t id) {
        InitializeCNLfirst(protobuf::PrivateEventProto, m_peRi);
        return m_peRi->get(id);
    }
    std::future<NetworkResponse<protobuf::ZFilesProto>> listZFiles(const std::string &folder) {
        InitializeCNLfirst(protobuf::ZFilesProto, m_zfRi);
        return m_zfRi->list(folder);
    }
    std::future<NetworkResponse<protobuf::ProfileEntitlements>> myProfileEntitlements() {
        InitializeCNLfirst(protobuf::ProfileEntitlements, m_restInvoker);
        return m_restInvoker->myProfileEntitlements();
    }
    std::future<NetworkResponse<void>> rejectPrivateEventInvitation(int64_t id) {
        InitializeCNLfirst(void, m_peRi);
        return m_peRi->reject(id);
    }
    std::future<NetworkResponse<void>> resetMyActiveClub() {
        InitializeCNLfirst(void, m_clubsRi);
        return m_clubsRi->resetMyActiveClub();
    }
    std::future<NetworkResponse<std::string>> savePlayback(const protobuf::PlaybackData &data) {
        InitializeCNLfirst(std::string, m_ppbRi);
        return m_ppbRi->savePlayback(data);
    }
    std::future<NetworkResponse<void>> saveRouteResult(const protobuf::RouteResultSaveRequest &r) {
        InitializeCNLfirst(void, m_rorRi);
        return m_rorRi->save(r);
    }
    std::future<NetworkResponse<int64_t>> saveSegmentResult(const protobuf::SegmentResult &sr) {
        InitializeCNLfirst(int64_t, m_restInvoker);
        return m_restInvoker->saveSegmentResult(sr);
    }
    std::future<NetworkResponse<void>> setMyActiveClub(const protobuf::UUID &id) {
        InitializeCNLfirst(void, m_clubsRi);
        return m_clubsRi->setMyActiveClub(id);
    }
    std::future<NetworkResponse<void>> unlockAchievements(const protobuf::AchievementUnlockRequest &rq) {
        InitializeCNLfirst(void, m_achRi);
        return m_achRi->unlock(rq);
    }
    std::future<NetworkResponse<protobuf::CampaignRegistrationResponse>> enrollInCampaignV2(const std::string &sn) {
        InitializeCNLfirst(protobuf::CampaignRegistrationResponse, m_camRi);
        return m_camRi->enrollPlayer(sn);
    }
    std::future<NetworkResponse<protobuf::ListCampaignRegistrationSummaryResponse>> getCampaignsV2() {
        InitializeCNLfirst(protobuf::ListCampaignRegistrationSummaryResponse, m_camRi);
        return m_camRi->getCampaigns();
    }
    std::future<NetworkResponse<protobuf::CampaignRegistrationResponse>> getRegistrationInCampaignV2(const std::string &sn) {
        InitializeCNLfirst(protobuf::CampaignRegistrationResponse, m_camRi);
        return m_camRi->getRegistration(sn);
    }
    std::future<NetworkResponse<protobuf::EventsProtobuf>> getEventsInInterval(const std::string &start, const std::string &end, int limit) {
        InitializeCNLfirst(protobuf::EventsProtobuf, m_restInvoker);
        return m_restInvoker->getEventsInInterval(start, end, limit);
    }
    std::future<NetworkResponse<protobuf::PlaybackMetadata>> getMyPlaybackLatest(int64_t a2, uint64_t after, uint64_t before) {
        InitializeCNLfirst(protobuf::PlaybackMetadata, m_ppbRi);
        return m_ppbRi->getMyPlaybackLatest(a2, after, before);
    }
    std::future<NetworkResponse<protobuf::PlaybackMetadata>> getMyPlaybackPr(int64_t a2, uint64_t after, uint64_t before) {
        InitializeCNLfirst(protobuf::PlaybackMetadata, m_ppbRi);
        return m_ppbRi->getMyPlaybackPr(a2, after, before);
    }
    std::future<NetworkResponse<protobuf::Clubs>> listMyClubs(Optional<protobuf::Membership_Status> status, Optional<int> start, Optional<int> limit) {
        InitializeCNLfirst(protobuf::Clubs, m_clubsRi);
        return m_clubsRi->listMyClubs(status, start, limit);
    }
    std::future<NetworkResponse<protobuf::PrivateEventFeedListProto>> privateEventFeed(int64_t start_date, int64_t end_date, Optional<protobuf::EventInviteStatus> status, bool organizer_only_past_events) {
        InitializeCNLfirst(protobuf::PrivateEventFeedListProto, m_peRi);
        return m_peRi->feed(start_date, end_date, status, organizer_only_past_events);
    }
    std::future<NetworkResponse<void>> createUser(const std::string &email, const std::string &pwd, const std::string &firstN, const std::string &lastN) {
        InitializeCNLfirst(void, m_restInvoker);
        return m_restInvoker->createUser(email, pwd, firstN, lastN);
    }
    std::future<NetworkResponse<protobuf::PlayerProfile>> myProfile() {
        InitializeCNLfirst(protobuf::PlayerProfile, m_restInvoker);
        return m_restInvoker->myProfile([this](const NetworkResponse<protobuf::PlayerProfile> &p) {
            onMyPlayerProfileReceived(p);
        });
    }
    std::future<NetworkResponse<int64_t>> saveWorldAttribute(const protobuf::WorldAttribute &wa) {
        if (m_globalState && m_globalState->isInWorld())
            return m_relay->saveWorldAttribute(m_globalState->getWorldId(), wa);
        else
            return makeNetworkResponseFuture<int64_t>(NRO_NO_WORLD_SELECTED, "Join world first"s);
    }
    std::future<NetworkResponse<protobuf::EventProtobuf>> getEvent(int64_t id) {
        InitializeCNLfirst(protobuf::EventProtobuf, m_restInvoker);
        return (id > 0) ? m_restInvoker->getEvent(id) : makeNetworkResponseFuture<protobuf::EventProtobuf>(NRO_INVALID_ARGUMENT, "Invalid event id"s);
    }
    std::future<NetworkResponse<protobuf::PlayerSocialNetwork>> getFollowees(int64_t profileId, bool followRequests) {
        InitializeCNLfirst(protobuf::PlayerSocialNetwork, m_restInvoker);
        return (profileId > 0) ? m_restInvoker->getFollowees(profileId, followRequests) : makeNetworkResponseFuture<protobuf::PlayerSocialNetwork>(NRO_INVALID_ARGUMENT, "Invalid player id"s);
    }
    std::future<NetworkResponse<protobuf::RaceResultSummary>> getSubgroupRaceResultSummary(int64_t sid) {
        InitializeCNLfirst(protobuf::RaceResultSummary, m_rarRi);
        return (sid > 0) ? m_rarRi->getSubgroupRaceResultSummary(sid) : makeNetworkResponseFuture<protobuf::RaceResultSummary>(NRO_INVALID_ARGUMENT, "Invalid event subgroup id"s);
    }
    std::future<NetworkResponse<bool>> registerForEventSubgroup(int64_t id) {
        InitializeCNLfirst(bool, m_restInvoker);
        return (id > 0) ? m_restInvoker->registerForEventSubgroup(id) : makeNetworkResponseFuture<bool>(NRO_INVALID_ARGUMENT, "Invalid event subgroup id"s);
    }
    std::future<NetworkResponse<int64_t>> createActivityRideOn(int64_t playerIdSender, int64_t playerIdTarget) {
        InitializeCNLfirst(int64_t, m_restInvoker);
        if (playerIdSender > 0) {
            if (playerIdTarget > 0) {
                //OMIT TelemetryService::remoteLogF(m_ts, 0, 4u, (__int64)"rideon", "Target Player ID %lli", playerIdTarget);
                return m_restInvoker->createActivityRideOn(playerIdSender, playerIdTarget);
            }
            //OMIT TelemetryService::remoteLogF(m_ts, 0, 2u, (__int64)"rideon", "Invalid target Player ID %lli", playerIdTarget)
            return makeNetworkResponseFuture<int64_t>(NRO_INVALID_ARGUMENT, "Invalid player id receiving ride on"s);
        }
        //OMIT TelemetryService::remoteLogF(m_ts, 0, 2u, (__int64)"rideon", "Invalid sender Player ID %lli", playerIdSender);
        return makeNetworkResponseFuture<int64_t>(NRO_INVALID_ARGUMENT, "Invalid player id giving ride on"s);
    }
    std::future<NetworkResponse<bool>> createSubgroupRegistration(int64_t id) {
        InitializeCNLfirst(bool, m_ecRi);
        if (m_loginOK) {
            if (id > 0)
                return m_ecRi->createRegistration(id);
            else
                return makeNetworkResponseFuture<bool>(NRO_INVALID_ARGUMENT, "Invalid event subgroup id"s);
        } else {
            return makeNetworkResponseFuture<bool>(NRO_NOT_LOGGED_IN, "Log in first"s);
        }
    }
    std::future<NetworkResponse<model::EventSignupResponse>> createSubgroupSignup(int64_t id) {
        InitializeCNLfirst(model::EventSignupResponse, m_ecRi);
        if (m_loginOK) {
            if (id > 0)
                return m_ecRi->createSignup(id);
            else
                return makeNetworkResponseFuture<model::EventSignupResponse>(NRO_INVALID_ARGUMENT, "Invalid event subgroup id"s);
        } else {
            return makeNetworkResponseFuture<model::EventSignupResponse>(NRO_NOT_LOGGED_IN, "Log in first"s);
        }
    }
    static int FileCompression_compressFileToGzip(const std::string &fileName, std::string *pDest, uint32_t maxSizePacked) {
        z_stream strm = {};
        int ret = 0; //no error
        const int bufSize = 0x20000;
        const char *fmt = "File could not be read:";
        std::unique_ptr<Bytef[]> in(new Bytef[bufSize]), out(new Bytef[bufSize]);
        auto err = deflateInit2(&strm, -1, Z_DEFLATED, 31, 8, 0);
        if (err) {
            NetworkingLogError("ZLIB deflateInit2() error: (%d) %s", err, strm.msg);
            ret = 1;
        } else {
            std::ifstream ifs(fileName.c_str(), std::ios::binary);
            if (!ifs.good()) {
                fmt = "Could not open";
                ret = 1;
            } else {
                strm.next_out = out.get();
                strm.avail_out = bufSize;
                while (!ifs.eof()) {
                    if (strm.avail_in == 0) {
                        strm.avail_in = ifs.readsome((char *)in.get(), bufSize);
                        strm.next_in = in.get();
                        if (strm.avail_in == 0)
                            break;
                    }
                    if (strm.avail_out == 0) {
                        strm.avail_out = bufSize;
                        pDest->append((char *)out.get(), bufSize);
                        if (pDest->size() > maxSizePacked) {
                            fmt = "Overflow in file";
                            ret = 1;
                            break;
                        }
                    }
                    if (deflate(&strm, Z_NO_FLUSH)) {
                        fmt = "Cannot deflate file";
                        ret = 1;
                        break;
                    }
                }
                if (deflate(&strm, Z_FINISH)) {
                    fmt = "Cannot finally deflate file";
                    ret = 1;
                }
                if (strm.next_out > out.get())
                    pDest->append((char *)out.get(), strm.next_out - out.get());
            }
            if (ret)
                NetworkingLogError("%s %s (%d)", fmt, fileName.c_str(), errno);
        }
        return ret;
    }
    static NetworkRequestOutcome generateZFileGzip(protobuf::ZFileProto *pDest, const std::string &name, const std::string &folder, const std::string &filePath) {
        auto s = new std::string;
        s->reserve(51200);
        if (0 == FileCompression_compressFileToGzip(filePath, s, 0xA00000)) {
            pDest->set_allocated_file(s);
            pDest->set_folder(folder);
            pDest->set_filename(name + ".gz"s);
            return NRO_OK;
        }
        return NRO_ERROR_READING_FILE;
    }
    std::future<NetworkResponse<protobuf::ZFileProto>> createZFileGzip(const std::string &name, const std::string &folder, const std::string &filePath) {
        InitializeCNLfirst(protobuf::ZFileProto, m_zfRi);
        protobuf::ZFileProto f;
        auto c = generateZFileGzip(&f, name, folder, filePath);
        if (c == NRO_OK)
            return m_zfRi->create(f);
        else
            return makeNetworkResponseFuture<protobuf::ZFileProto>(c, "Could not compress file."s);
    }
    std::future<NetworkResponse<void>> deleteActivity(int64_t playerId, int64_t actId) {
        InitializeCNLfirst(void, m_restInvoker);
        if (playerId <= 0)
            return makeNetworkResponseFuture<void>(NRO_INVALID_ARGUMENT, "Invalid player id"s);
        if (actId <= 0)
            return makeNetworkResponseFuture<void>(NRO_INVALID_ARGUMENT, "Invalid activity id"s);
        return m_restInvoker->deleteActivity(playerId, actId);
    }
    std::future<NetworkResponse<bool>> deleteSubgroupSignup(int64_t id) {
        InitializeCNLfirst(bool, m_ecRi);
        if (m_loginOK) {
            if (id > 0)
                return m_ecRi->deleteSignup(id);
            else
                return makeNetworkResponseFuture<bool>(NRO_INVALID_ARGUMENT, "Invalid event id"s);
        } else {
            return makeNetworkResponseFuture<bool>(NRO_NOT_LOGGED_IN, "Log in first"s);
        }
    }
    std::future<NetworkResponse<bool>> removeSignupForEvent(int64_t id) {
        InitializeCNLfirst(bool, m_restInvoker);
        if (id > 0)
            return m_restInvoker->removeSignupForEvent(id);
        else
            return makeNetworkResponseFuture<bool>(NRO_INVALID_ARGUMENT, "Invalid event id"s);
    }
    std::future<NetworkResponse<void>> saveTimeCrossingStartLine(int64_t eventId, const protobuf::CrossingStartingLineProto &csl) {
        InitializeCNLfirst(void, m_relay);
        if (eventId > 0)
            return m_relay->saveTimeCrossingStartLine(eventId, csl);
        else
            return makeNetworkResponseFuture<void>(NRO_INVALID_ARGUMENT, "Invalid event subgroup id"s);
    }
    std::future<NetworkResponse<model::EventSignupResponse>> signupForEventSubgroup(int64_t eventId) {
        InitializeCNLfirst(model::EventSignupResponse, m_restInvoker);
        if (eventId > 0)
            return m_restInvoker->signupForEventSubgroup(eventId);
        else
            return makeNetworkResponseFuture<model::EventSignupResponse>(NRO_INVALID_ARGUMENT, "Invalid event subgroup id"s);
    }
    std::future<NetworkResponse<protobuf::PlayerProfiles>> getEventSubgroupEntrants(protobuf::EventParticipation ep, int64_t eventId, uint32_t limit) {
        InitializeCNLfirst(protobuf::PlayerProfiles, m_restInvoker);
        if (eventId > 0)
            return m_restInvoker->getEventSubgroupEntrants(ep, eventId, limit);
        else
            return makeNetworkResponseFuture<protobuf::PlayerProfiles>(NRO_INVALID_ARGUMENT, "Invalid event subgroup id"s);
    }
    std::future<NetworkResponse<int64_t>> saveActivityImage(int64_t profileId, const protobuf::ActivityImage &img, const std::string &imgPath) {
        InitializeCNLfirst(int64_t, m_restInvoker);
        if (profileId > 0)
            return m_restInvoker->saveActivityImage(profileId, img, imgPath);
        else
            return makeNetworkResponseFuture<int64_t>(NRO_INVALID_ARGUMENT, "Invalid player id"s);
    }
    std::future<NetworkResponse<void>> removeGoal(int64_t playerId, int64_t goalId) {
        InitializeCNLfirst(void, m_restInvoker);
        if (playerId > 0)
            if (goalId > 0)
                return m_restInvoker->removeGoal(playerId, goalId);
            else
                return makeNetworkResponseFuture<void>(NRO_INVALID_ARGUMENT, "Invalid goal id"s);
        else
            return makeNetworkResponseFuture<void>(NRO_INVALID_ARGUMENT, "Invalid player id"s);
    }
    std::future<NetworkResponse<int64_t>> saveActivity(const protobuf::Activity &act, bool bFinalSave, const std::string &fitPath) {
        InitializeCNLfirst(int64_t, m_restInvoker);
        return m_restInvoker->saveActivity(act, bFinalSave, fitPath, [this](const NetworkResponse<int64_t> &nr) {
            //OMIT if (!nr)
            //    TelemetryService::setActivityId();
        });
    }
    std::future<NetworkResponse<protobuf::SegmentResults>> querySegmentResults(int64_t serverRealm, int64_t segmentId, const std::string &from, const std::string &to, bool full) {
        InitializeCNLfirst(protobuf::SegmentResults, m_restInvoker);
        if (serverRealm > 0)
            return m_restInvoker->querySegmentResults(serverRealm, segmentId, from, to, full);
        else
            return makeNetworkResponseFuture<protobuf::SegmentResults>(NRO_INVALID_ARGUMENT, "Invalid world id"s);
    }
    std::future<NetworkResponse<void>> updateProfile(bool inGameFields, const protobuf::PlayerProfile &prof, bool udp) {
        InitializeCNLfirst(void, m_restInvoker);
        bool m = prof.use_metric();
        return m_restInvoker->updateProfile(prof, inGameFields, [this, udp, m]() {
            if (udp && this->m_udpClient)
                this->m_udpClient->setPlayerProfileUpdated(this->m_wclock->getWorldTime());
            //TelemetryService::setPowerSourceType((__int64)v4->m_ts, v1);
            if (this->m_aux)
                m_aux->m_use_metric = m;
        });
    }
    std::future<NetworkResponse<protobuf::ActivityList>> getActivities(int64_t profileId, const Optional<int64_t> &startsAfter, const Optional<int64_t> &startsBefore, bool fetchSnapshots) {
        InitializeCNLfirst(protobuf::ActivityList, m_restInvoker);
        if (profileId > 0)
            return m_restInvoker->getActivities(profileId, startsAfter, startsBefore, fetchSnapshots);
        else
            return makeNetworkResponseFuture<protobuf::ActivityList>(NRO_INVALID_ARGUMENT, "Invalid player id"s);
    }
    void onLoggedOut() {
        shutdownUdpClient();
        shutdownAuxiliaryController();
        shutdownServiceEventLoop();
        //OMIT sub_7FF76A52BBE0((__int64)a1->m_ts);
        m_globalState.reset(nullptr);
        m_netClock.reset(nullptr);
        m_wat.reset(nullptr);
        m_wclock.reset(nullptr);
        m_hashSeed1.reset(nullptr);
        m_hashSeed2.reset(nullptr);
        m_profRqDebouncer.reset(nullptr);
        m_relay.reset(nullptr);
        NetworkingLogInfo("Logged out");
    }
    std::future<NetworkResponse<protobuf::PlayerProfile>> profile(int64_t profileId, bool bSocial) {
        InitializeCNLfirst(protobuf::PlayerProfile, m_restInvoker);
        if (profileId > 0) {
            if (!m_profRqDebouncer || bSocial)
                return m_restInvoker->profileByPlayerId(profileId, bSocial);
            else
                return m_profRqDebouncer->addRequest(profileId);
        } else {
            return makeNetworkResponseFuture<protobuf::PlayerProfile>(NRO_INVALID_ARGUMENT, "Invalid player id"s);
        }
    }
    std::future<NetworkResponse<protobuf::SegmentResults>> subscribeToSegmentAndGetLeaderboard(int64_t sid) {
        if (m_loginOK) {
            if (m_someFunc0) {
                return makeNetworkResponseFuture<protobuf::SegmentResults>(NRO_DISCONNECTED_DUE_TO_SIMULTANEOUS_LOGINS, "Log out other connection first"s);
            } else {
                if (m_tcpClient)
                    return m_tcpClient->subscribeToSegmentAndGetLeaderboard(sid);
                else
                    return makeNetworkResponseFuture<protobuf::SegmentResults>(NRO_NO_PLAYER_ID_YET, "Get my profile first"s);
            }
        } else {
            return makeNetworkResponseFuture<protobuf::SegmentResults>(NRO_NOT_LOGGED_IN, "Log in first"s);
        }
    }
    std::future<NetworkResponse<protobuf::PlayerProfiles>> profiles(const std::unordered_set<int64_t> &rq) {
        InitializeCNLfirst(protobuf::PlayerProfiles, m_restInvoker);
        if (rq.empty())
            return makeNetworkResponseFuture<protobuf::PlayerProfiles>(NRO_INVALID_ARGUMENT, "Empty player ids"s);
        for (auto i : rq)
            if (i <= 0)
                return makeNetworkResponseFuture<protobuf::PlayerProfiles>(NRO_INVALID_ARGUMENT, "Invalid player ids"s);
        if (rq.size() <= m_limitProfPerRq)
            return m_restInvoker->profilesByPlayerIds(rq);
        return makeNetworkResponseFuture<protobuf::PlayerProfiles>(NRO_INVALID_ARGUMENT, 
            "Requested number of players "s + std::to_string(rq.size()) + " exceeded maximum allowed of " + 
            std::to_string(m_limitProfPerRq) + " per request"s);
    }
    bool shouldTryToEnableEncryptionWithZc() {
        if (m_nco.m_disableEncryptionWithZc)
            return false;
        if (m_nco.m_zcSecretKeyBase64.length() || m_nco.m_ignoreEncryptionFeatureFlag)
            return true;
        else
            return m_globalState->shouldUseEncryption();
    }
    NetworkRequestOutcome sendPlayerState(int64_t id, const protobuf::PlayerState &pst) {
        if (!m_loginOK)
            return NRO_NOT_LOGGED_IN;
        if (!m_aux || !m_aux->m_connectedOK || !m_aux->m_lastStatus) {
            auto tmNow = _time64(nullptr);
            if (m_aux) {
                m_rwqAux.try_dequeue(m_curAux);
                if (m_curAux.m_localIp.length()) {
                    auto need_pause = (_difftime64(tmNow, m_netStartTime1) < 120.0) ? 1.0 : 15.0;
                    if (_difftime64(tmNow, m_netStartTime2) > need_pause) {
                        if (m_aux->m_stopped) {
                            auto en = shouldTryToEnableEncryptionWithZc();
                            m_aux->start(m_curAux, en, m_nco.m_zcSecurePort, m_nco.m_zcSecretKeyBase64);
                        } else {
                            m_aux->reconnect(m_curAux);
                        }
                        _time64(&m_netStartTime2);
                    }
                }
            }
        }
        auto ret = m_udpClient->sendPlayerState(id, pst);
        if (m_aux && m_aux->m_connectedOK && m_aux->m_lastStatus && ret == NRO_OK && m_globalState->getPlayerId() == pst.id())
            m_aux->send_player_state(pst);
        return ret;
    }
    std::future<NetworkResponse<protobuf::DropInWorldList>> fetchDropInWorldList() {
        InitializeCNLfirst(protobuf::DropInWorldList, m_relay);
        return m_relay->fetchDropInWorldList(false);
    }
    std::future<NetworkResponse<protobuf::DropInWorldList>> fetchWorldsCountsAndCapacities() {
        InitializeCNLfirst(protobuf::DropInWorldList, m_relay);
        return m_relay->fetchDropInWorldList(true);
    }
    std::future<NetworkResponse<protobuf::SegmentResults>> getSegmentJerseyLeaders() {
        return m_srRi->getSegmentJerseyLeaders();
    }
    std::list<ValidateProperty> parseValidationErrorMessage(const std::string &msg) {
        std::list<ValidateProperty> ret;
        size_t parsePos = 0;
        auto pref = "[PARAMETER]\r"s, suff = "]\r"s;
        const int skipSuf = 3; //or less?
        while (parsePos < msg.length()) {
            parsePos = msg.find(pref, parsePos);
            if (parsePos == -1)
                break;
            parsePos += pref.length();
            for (int i = 0; i < skipSuf; i++) {
                parsePos = msg.find(suff, parsePos + 1);
                if (parsePos == -1)
                    break;
            }
            if (parsePos == -1)
                break;
            parsePos += 3;
            if (parsePos < msg.length()) {
                if (msg.find("email_required"s, parsePos) != -1) ret.push_back(VP_EMAIL_REQ);
                else if (msg.find("email_wrong_length"s, parsePos) != -1) ret.push_back(VP_EMAIL_WRONG_LEN);
                else if (msg.find("email_too_short"s, parsePos) != -1) ret.push_back(VP_EMAIL_SHORT);
                else if (msg.find("email_too_long"s, parsePos) != -1) ret.push_back(VP_EMAIL_LONG);
                else if (msg.find("email_invalid"s, parsePos) != -1) ret.push_back(VP_EMAIL_FORMAT);
                else if (msg.find("emails_must_match"s, parsePos) != -1) ret.push_back(VP_EMAIL_UNEQ);
                else if (msg.find("profile_required"s, parsePos) != -1) ret.push_back(VP_PROF_REQ);
                else if (msg.find("profile_not_allowed"s, parsePos) != -1) ret.push_back(VP_PROF_UN);
                else if (msg.find("profile_id_must_be_0"s, parsePos) != -1) ret.push_back(VP_PROF_NN);
                else if (msg.find("profile_ids_must_match"s, parsePos) != -1) ret.push_back(VP_PROF_UNEQ);
                else if (msg.find("profile_invalid"s, parsePos) != -1) ret.push_back(VP_PROF_INV);
                else if (msg.find("password_required"s, parsePos) != -1) ret.push_back(VP_PSW_REQ);
                else if (msg.find("password_wrong_length"s, parsePos) != -1) ret.push_back(VP_PSW_LEN);
                else if (msg.find("password_too_short"s, parsePos) != -1) ret.push_back(VP_PSW_SHORT);
                else if (msg.find("password_too_long"s, parsePos) != -1) ret.push_back(VP_PSW_LONG);
                else if (msg.find("password_invalid"s, parsePos) != -1) ret.push_back(VP_PSW_FORMAT);
                else if (msg.find("password_mismatch"s, parsePos) != -1) ret.push_back(VP_PSW_UNEQ);
                else if (msg.find("first_name_required"s, parsePos) != -1) ret.push_back(VP_FN_REQ);
                else if (msg.find("first_name_wrong_length"s, parsePos) != -1) ret.push_back(VP_FN_LEN);
                else if (msg.find("first_name_too_short"s, parsePos) != -1) ret.push_back(VP_FN_SHORT);
                else if (msg.find("first_name_too_long"s, parsePos) != -1) ret.push_back(VP_FN_LONG);
                else if (msg.find("last_name_required"s, parsePos) != -1) ret.push_back(VP_LN_REQ);
                else if (msg.find("last_name_wrong_length"s, parsePos) != -1) ret.push_back(VP_LN_LEN);
                else if (msg.find("last_name_too_short"s, parsePos) != -1) ret.push_back(VP_LN_SHORT);
                else if (msg.find("last_name_too_long"s, parsePos) != -1) ret.push_back(VP_LN_LONG);
            }
        }
        if (ret.empty()) {
            NetworkingLogError("Could not parse error_message string: %s", msg.c_str());
            ret.push_back(VP_PARSE_ERROR);
        }
        return ret;
    }
    bool popPhoneToGameCommand(protobuf::PhoneToGameCommand *pDest) {
        if (m_aux)
            return m_aux->pop_phone_to_game_command(pDest);
        else
            return false;
    }
    void getConnectionMetrics(ConnectivityInfo *pDest) {
        pDest->m_tcpPackets = m_tcpStat->m_packets;
        pDest->m_udpPackets = m_udpStat->m_packets;
    }

    /*
LATER: void connectLanExerciseDevice(uint32_t,std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>,std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>)
void disconnectLanExerciseDevice(uint32_t)
void registerLanExerciseDeviceMessageReceivedCallback(std::function<void ()(zwift_network::LanExerciseDeviceInfo const&,const std::vector<uchar> &)> const&)
void registerLanExerciseDeviceStatusCallback(std::function<void ()(zwift_network::LanExerciseDeviceInfo const&)> const&)
void startScanningForLanExerciseDevices()
void stopScanningForLanExerciseDevices()
void getConnectionQuality()
void sendDeviceDiagnostics(const std::string &,const std::string &,const std::vector<uchar> &)
void sendMessageToLanExerciseDevice(uint32_t,const std::vector<uchar> &)

ABSENT in PC void createWorkout(const std::string &,protobuf::Sport,const std::string &)
void deleteActivityImage(int64_t,int64_t,int64_t)
void deletePlayback(const std::string &)
void deleteSubgroupRegistration(int64_t)
void deleteWorkout(const std::string &)
void editWorkout(const std::string &,const std::string &,protobuf::Sport,const std::string &)
void fetchAssetSummary(const std::string &)
void fetchCustomWorkouts(zwift_network::Optional<std::string>)
void getActivity(int64_t,int64_t,bool)
void getActivityImage(int64_t,int64_t,int64_t)
void getAvailablePowerCurveYears()
void getEventRaceResult(int64_t,zwift_network::Optional<int>,zwift_network::Optional<int>)
void getEventRaceResultSummary(int64_t)
void getSubgroupRaceResult(int64_t,zwift_network::Optional<int>,zwift_network::Optional<int>)
void updateFollower(int64_t,int64_t,bool,protobuf::ProfileFollowStatus)
void getFollowers(int64_t,bool)
void getFollowers(int64_t,int64_t,bool)
void getSegmentResult(int64_t)
void returnToHome()
void resumeSubscription()
void resetPassword(const std::string &)
void getBestEffortsPowerCurveByDays(zwift_network::PowerCurveAggregationDays)
void getBestEffortsPowerCurveByYear(int)
void getMyEvents(zwift_network::model::BaseEventsSearch const&)
void removeFollower(int64_t,int64_t)
void saveProfileReminders(protobuf::PlayerProfile const&)
void getNotifications()
void removeRegistrationForEvent(int64_t)
void updateNotificationReadStatus(int64_t,int64_t,bool)
void myActiveClub()
void signUrls(const std::string &,std::vector<std::string> const&)
void uploadReceipt(protobuf::InAppPurchaseReceipt &)

OMIT void initializeTelemetry()
void getFirmwareUpdates(std::vector<zwift_network::model::FirmwareRequest> const&)
void sendMixpanelEvent(const std::string &,std::vector<std::string> const&)
void campaignSummary(const std::string &)
void deregisterTelemetryJson(const std::string &)
void enrollInCampaign(const std::string &)
void enrollInCampaignV2(const std::string &)
void fetchUpcomingWorkouts()
void fetchWorkout(zwift_network::model::WorkoutPartnerEnum,const std::string &)
void getAllFirmwareReleases(const std::string &)
void getFirmwareRelease(const std::string &,const std::string &)
void registerInCampaign(const std::string &)
void registerInCampaignV2(const std::string &)
void getProgressInCampaignV2(const std::string &)
void getRegistrationInCampaignV2(const std::string &)
void redeemCoupon(const std::string &)
void withdrawFromCampaign(const std::string &)
void withdrawFromCampaignV2(const std::string &)
void sendAnalyticsEvent(const std::string &,std::vector<std::string> const&)
void registerTelemetryJson(const std::string &,std::function<Json::Value ()(std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>)> const&)
void onSaveActivityReceived(std::shared_ptr<zwift_network::NetworkResponse<int64_t> const> const&)
void remoteLog(zwift_network::LogLevel,char const*,char const*)
void remoteLogAndFlush(zwift_network::LogLevel,char const*,char const*)
*/
};
NetworkClient::NetworkClient() { m_pImpl = new(calloc(sizeof(NetworkClientImpl), 1)) NetworkClientImpl; }
NetworkClient::~NetworkClient() { m_pImpl->~NetworkClientImpl(); free(m_pImpl); }
void NetworkClient::globalInitialize() { curl_global_init(CURL_GLOBAL_ALL); }
void NetworkClient::globalCleanup() { curl_global_cleanup(); }
void NetworkClient::initialize(const std::string &server, const std::string &certs, const std::string &version) { m_pImpl->initialize(server, certs, version); }
NetworkRequestOutcome ZNETWORK_ClearPlayerPowerups() { return g_networkClient->m_pImpl->sendClearPowerUpCommand(); }
std::future<NetworkResponse<int64_t>> g_BroadcastLocalPlayerFlaggedFuture, g_SendPacePartnerInfoFuture;
struct BroadcastLocalPlayerFlagged { //32 bytes
    uint16_t m_ver, m_len;
    int64_t m_playerId;
    double m_worldTimeSec;
    PLAYER_FLAGGED_REASONS m_pfr;
    /*char field_1C;
    char field_1D;
    char field_1E;
    char field_1F;*/
};
void ZNETWORK_BroadcastLocalPlayerFlagged(PLAYER_FLAGGED_REASONS pfr) {
    auto t_now = _time64(nullptr);
    if (t_now - g_lastBroadcastLocalPlayerFlagged >= 600) {
        g_lastBroadcastLocalPlayerFlagged = t_now;
        protobuf::WorldAttribute wa_proto;
        wa_proto.set_wa_type(protobuf::WAT_FLAG);
        wa_proto.set_world_time_expire(g_CachedWorldTime + 3'600'000);
        BroadcastLocalPlayerFlagged v11{ sizeof(v11) - 4, 1, BikeManager::Instance()->m_mainBike->m_playerIdTx, g_CachedWorldTime * 0.001, pfr};
        static_assert(sizeof(v11) == 32);
        wa_proto.set_payload((char *)&v11, sizeof(v11));
        g_BroadcastLocalPlayerFlaggedFuture = zwift_network::save_world_attribute(wa_proto);
    }
}
std::map<int64_t, bool> g_sessionRideonsGivenTo; //playerId->rideOn given
std::vector<int64_t> g_sessionRideonsFrom;
std::vector<protobuf::RideOn> g_sessionRideonsRx;
int g_ZNETWORK_Stats[8], g_totalRideonsGiven;
int64_t g_lastRideonTarget, g_lastRideonDebounce;
time_t g_magicLeaderboardBirthday;
uint32_t g_lastPlayerStateTime, g_playerStatesCnt;
void ZNETWORK_ResetSessionData() {
    g_sessionRideonsGivenTo.clear();
    g_sessionRideonsRx.clear();
    //OMIT *g_ZNETWORK_Stats = 0i64;
    g_lastPlayerStateTime = 0;
    g_playerStatesCnt = 0;
    //OMIT *&g_ZNETWORK_Stats[4] = 0i64;
}
int g_currentPhonePowerupID;
void ZNETWORK_GivePlayerPowerup(protobuf::POWERUP_TYPE ty) {
    g_currentPhonePowerupID = 1 + (int)ty;
    std::string clr;
    const char *loc;
    switch (ty) {
    case protobuf::LIGHTNESS:
        clr = "0x9ECC11"s;
        loc = "LOC_POWERUP_FEATHERWEIGHT_NAME_UPPER";
        break;
    case protobuf::DRAFTBOOST:
        clr = "0x1192CC"s;
        loc = "LOC_POWERUP_DRAFT_BOOST_NAME_UPPER";
        break;
    case protobuf::UNDRAFTABLE:
        clr = "0xB07F46"s;
        loc = "LOC_POWERUP_UNDRAFTABLE_NAME_UPPER";
        break;
    case protobuf::AERO:
        clr = "0x1BF4FF"s;
        loc = "LOC_POWERUP_AERO_BOOST_NAME_UPPER";
        break;
    case protobuf::NINJA:
        clr = "0x1B1B1B"s;
        loc = "LOC_POWERUP_NINJA_NAME_UPPER";
        break;
    case protobuf::STEAMROLLER:
        clr = "0x6600CC"s;
        loc = "LOC_POWERUP_STEAMROLLER_NAME_UPPER";
        break;
    case protobuf::ANVIL:
        clr = "0x585858"s;
        loc = "LOC_POWERUP_ANVIL_NAME_UPPER";
        break;
    default:
        return;
    }
    zwift_network::send_set_power_up_command(GetText(loc), clr, "0xffffff"s, g_currentPhonePowerupID);
}
void ZNETWORK_SendPacePartnerInfo(int64_t playerId, ZNETWORK_PacePartnerInfo::BroadcastState bcs, float fpar, uint32_t importance) {
    ZNETWORK_PacePartnerInfo ppi{ .m_playerIdTx = BikeManager::Instance()->m_mainBike->m_playerIdTx, .m_playerId = playerId, 
        .m_ver = 1, .m_len = sizeof(ZNETWORK_PacePartnerInfo) - 4, .m_float = fpar, .m_bcs = bcs };
    protobuf::WorldAttribute wa;
    wa.set_wa_type(protobuf::WAT_PPI);
    wa.set_world_time_expire(zwift_network::world_time() + 10000);
    if (importance <= 0)
        wa.set_rel_id(playerId);
    else
        wa.set_importance(100 * importance);
    static_assert(sizeof(ZNETWORK_PacePartnerInfo) == 32);
    wa.set_payload((char *)&ppi, sizeof(ZNETWORK_PacePartnerInfo));
    g_SendPacePartnerInfoFuture = zwift_network::save_world_attribute(wa);
}
float g_delayed_latejoin_time = -1.0, g_ptg_f7 = 0.0f, g_ptg_f8 = 0.0f, g_ptg_f9 = 0.0f, g_delayed_RouteHash_time, g_phoneRotation;
ZNETWORK_LateJoinRequest g_lateJoinRequest;
void (*g_LateJoinCB)(int64_t, int32_t, int32_t, uint32_t, const VEC3 &);
void FinishLateJoin(int64_t lateJoinPlayerId_notused, int32_t decisionIndex, int32_t a3, uint32_t a4, const VEC3 &pos) {
    auto evSt = GroupEvents::FindSubgroupEvent(g_GroupEventsActive_CurrentEventId);
    if (evSt) {
        auto mainBike = BikeManager::Instance()->m_mainBike;
        auto routeComp = mainBike->m_routeComp;
        if (routeComp && routeComp->m_selRoute) {
            Log("GroupEvents: Finishing late-join (%d, %d)", decisionIndex, a3);
            auto x = pos.m_data[0], y = pos.m_data[1], z = pos.m_data[2];
            int segmId[4];
            RoadManager::FindClosestPointOnRoad(pos, 3, segmId, 1ui64 << mainBike->m_bc->m_sport, true, -1);
            auto v16 = g_pRoadManager->GetRoadSegment(segmId[0]);
            if (v16) {
                auto v42 = v16->CalculateRoadPositionAtTime(0.0 /*QUEST*/, false) - pos;
                if (v42.lenSquared() <= 36'000'000.0f) {
                    auto v21 = -1.7976931348623158e308;
                    if (g_pGameWorld->WorldID() == WID_BOLOGNA)
                        v21 = 0.03139999881386757;
                    double v15 = (double)pos.m_data[0], v24;
                    if (v16 && !v16->IsPlaceholder() && v16->IsPaddock()
                        && ((v24 = v16->GetPaddockExitRoadTime(), v24 <= -1.0) || (v15 > v21 && v15 < v24))) {
                        Log("GroupEvents: late-join failed - player did not late join off the paddock. { %3.2f, %3.2f, %3.2f }", x, y, z);
                    } else {
                        mainBike->m_bc->m_field_1E8 = a4;
                        mainBike->m_routeComp->m_field_C = a4;
                        mainBike->m_bc->m_field_1B8 = mainBike->m_ipsc->m_field_EC;
                        mainBike->m_field_8BB = mainBike->m_field_8BC = false;
                        auto v27 = a3 + 1;
                        TeleportManagerComponent::SetupDropIn(mainBike, evSt->m_field_4F8, false, false, nullptr);
                        mainBike->m_routeComp->m_decisionIndex = decisionIndex;
                        Log("Forced decision index to %d", decisionIndex);
                        mainBike->m_routeComp->m_field_14 = a3;
                        auto v29 = mainBike->m_routeComp->m_selRoute->m_field_110.size();
                        if (a3 + 1 >= v29) {
                            v27 -= (int)v29;
                            ++mainBike->m_routeComp->m_field_C;
                        }
                        int v31 = 0;
                        for(auto &i : mainBike->m_routeComp->m_selRoute->m_field_110) {
                            if (v31 >= v27) {
                                mainBike->m_field_8EC = i.m_field_10;
                                break;
                            }
                            i.m_bitField |= 6u;
                            v31++;
                        }
                        g_LateJoinCB = nullptr;
                        mainBike->m_routeComp->m_field_10 = v27;
                    }
                } else {
                    Log("GroupEvents: late-join failed - location not on a viable road. { %3.2f, %3.2f, %3.2f }", x, y, z);
                }
            } else {
                Log("GroupEvents: late-join failed - location not on a viable road. { %3.2f, %3.2f, %3.2f }", x, y, z);
            }
        }
    }
}
void ZNETWORK_INTERNAL_HandleLateJoinRequest(const ZNETWORK_LateJoinRequest &eljr, const VEC3 &pos) {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    auto v9 = g_pRoadManager->GetRoadSegment(mainBike->m_road ? mainBike->m_road->m_segmentId : 0);
    Log("Handle latejoin request...");
    int64_t eventId = mainBike->GetEventID();
    auto evSt = GroupEvents::FindSubgroupEvent(eventId);
    if (!evSt) {
        Log("Cannot find sub group state in handle late join request!!!");
        return;
    }
    if (eljr.m_ver != 2) {
        evSt->m_field_500.push_back(evSt->m_field_4F8);
        return;
    }
    auto IsPaddock = RoadIsPaddock(g_pGameWorld->WorldID(), mainBike->m_road ? mainBike->m_road->m_segmentId : 0, mainBike->m_field_888, 
        mainBike->m_field_8EC == 0.0f, nullptr, nullptr);
    if (eljr.m_cmd == ZNETWORK_LateJoinRequest::LJC_1) {
        if (!v9 || IsPaddock || v9->GetIntersectionMarkerBasedOnRoadTime(mainBike->m_field_888) || !mainBike->m_routeComp || mainBike->m_routeComp->m_field_A8) {
            g_lateJoinRequest = eljr; //ZNETWORK_INTERNAL_DelayLateJoinResponse
            g_delayed_latejoin_time = 15.0f;
            //OMIT AnalyticsHelper_inst();
            Log("latejoin Delayed...");
        } else {
            ZNETWORK_RespondToLateJoinRequest(eljr.m_lateJoinPlayerId);
            Log("Responded to late join request");
        }
    }
    if (eljr.m_cmd != eljr.LJC_2) {
        //OMIT AnalyticsHelper_inst();
        Log("Unknown Latejoin command: %d", eljr.m_cmd);
    }
    if (g_LateJoinCB) {
        Log("Latejoin: received response! %d %d", eljr.m_decisionIndex, eljr.m_cb_a3);
        g_LateJoinCB(eljr.m_lateJoinPlayerId, eljr.m_decisionIndex, eljr.m_cb_a3, eljr.m_cb_a4, pos);
        //OMIT AnalyticsHelper_inst();
    } else {
        Log("latejoin: received response but no lj-cb");
        //OMIT AnalyticsHelper_inst();
    }
}
std::future<NetworkResponse<int64_t>> g_BroadcastLateJoinReturnCode;
void ZNETWORK_RespondToLateJoinRequest(int64_t lateJoinPlayerId) {
    auto bk = BikeManager::Instance()->m_mainBike;
    if (lateJoinPlayerId != bk->m_playerIdTx && (g_GameMode & 0x10) && g_subscriptionMode != SM_INACTIVE) {
        // OMIT AnalyticsHelper::IncrementLogoutCounter "Game - Latejoin - Sent responses"
        protobuf::WorldAttribute v28;
        v28.set_wa_type(protobuf::WAT_LATE);
        v28.set_world_time_expire(g_CachedWorldTime + 10000);
        v28.set_x(bk->m_pos.m_data[0]);
        v28.set_y_altitude(bk->m_pos.m_data[1]);
        v28.set_z(bk->m_pos.m_data[2]);
        v28.set_importance(5'000'000);
        v28.set_rel_id(lateJoinPlayerId);
        static_assert(sizeof(ZNETWORK_LateJoinResponse) == 0x28);
        ZNETWORK_LateJoinResponse v19{ .m_ver = 2, .m_len = sizeof(ZNETWORK_LateJoinResponse) - 4, .m_field_4 = 2, .m_playerId = bk->m_playerIdTx, .m_lateJoinPlayerId = lateJoinPlayerId, .m_field_20 = bk->m_bc->m_field_1E8 };
        if (bk->m_routeComp && bk->m_routeComp->m_selRoute) { // RouteComputer::GetSelectedRoute
            v19.m_decisionIndex = bk->m_routeComp->m_decisionIndex; //oldnapalm guess: road ID and road(spline) time
            v19.m_field_1C = bk->m_routeComp->m_field_10;
        }
        v28.set_payload((char *)&v19, sizeof(ZNETWORK_LateJoinResponse));
        Log("GroupEvents: late-join: respond: [%d] -> [%d]  (%d, %d)", lateJoinPlayerId, bk->m_playerIdTx, v19.m_decisionIndex, v19.m_field_1C);
        g_BroadcastLateJoinReturnCode = zwift_network::save_world_attribute(v28);
    }
}
double g_accumulatedTime;
int g_timeUntilNextSubscrModeCheck;
int64_t g_RouteHashTargetID;
bool g_pairToPhone, g_WasPairedToAux, g_SelectedAuxSampleHR, g_SelectedAuxSampleWRS;
void ZNETWORK_Update(float dt) {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    static uint32_t g_lastUpdate = timeGetTime();
    auto Time = timeGetTime();
    auto v4 = Time - g_lastUpdate;
    ZNet::API::Inst()->Update(v4);
    g_accumulatedTime += ((Time - g_lastUpdate) * 0.001f);
    g_lastUpdate = Time;
    g_CachedWorldTime = zwift_network::world_time();
    auto v9 = g_timeUntilNextSubscrModeCheck - (dt * 1000.0);
    if (v9 > 1000)
        v9 = 1000;
    g_timeUntilNextSubscrModeCheck = v9;
    if (v9 <= 0) {
        g_subscriptionMode = ZNETWORK_CalculateSubscriptionMode();
        g_timeUntilNextSubscrModeCheck = 1000;
    }
    if (g_delayed_latejoin_time > 0.0f) {
        g_delayed_latejoin_time -= dt;
        if (g_delayed_latejoin_time <= 0.0f) {
            g_delayed_latejoin_time = -1.0;
            ZNETWORK_LateJoinRequest v102 = g_lateJoinRequest;
            VEC3 v89{};
            v102 = g_lateJoinRequest;
            ZNETWORK_INTERNAL_HandleLateJoinRequest(v102, v89);
        }
    }
    if (g_delayed_RouteHash_time > 0.0f) {
        g_delayed_RouteHash_time -= dt;
        if (g_delayed_RouteHash_time - dt <= 0.0f) {
            g_delayed_RouteHash_time = -1.0f;
            ZNETWORK_SendRouteHashRequest(g_RouteHashTargetID);
        }
    }
    if (zwift_network::is_paired_to_phone()) {
        if (!g_pairToPhone) {
            if (mainBike) {
                auto v13 = mainBike->m_pendPU;
                if (v13 == protobuf::POWERUP_NONE)
                    zwift_network::send_activate_power_up_command(g_currentPhonePowerupID, 0);
                else
                    ZNETWORK_GivePlayerPowerup(v13);
                ZML_SendGameSessionInfo();
            }
            if (FitnessDeviceManager::m_pSelectedHRDevice && FitnessDeviceManager::m_pSelectedHRDevice->m_protocol == DP_BLE
                && ((BLEDevice *)FitnessDeviceManager::m_pSelectedHRDevice)->m_bleSrc == BLES_ZCA)
                FitnessDeviceManager::m_pSelectedHRDevice->Pair(true);
            if (FitnessDeviceManager::m_pSelectedPowerDevice && FitnessDeviceManager::m_pSelectedPowerDevice->m_protocol == DP_BLE
                && ((BLEDevice *)FitnessDeviceManager::m_pSelectedPowerDevice)->m_bleSrc == BLES_ZCA)
                FitnessDeviceManager::m_pSelectedPowerDevice->Pair(true);
            if (FitnessDeviceManager::m_pSelectedSpeedDevice && FitnessDeviceManager::m_pSelectedSpeedDevice->m_protocol == DP_BLE
                && ((BLEDevice *)FitnessDeviceManager::m_pSelectedSpeedDevice)->m_bleSrc == BLES_ZCA)
                FitnessDeviceManager::m_pSelectedSpeedDevice->Pair(true);
            if (FitnessDeviceManager::m_pSelectedCadenceDevice && FitnessDeviceManager::m_pSelectedCadenceDevice->m_protocol == DP_BLE
                && ((BLEDevice *)FitnessDeviceManager::m_pSelectedCadenceDevice)->m_bleSrc == BLES_ZCA)
                FitnessDeviceManager::m_pSelectedCadenceDevice->Pair(true);
            if (FitnessDeviceManager::m_pSelectedControllableTrainerDevice && FitnessDeviceManager::m_pSelectedControllableTrainerDevice->m_protocol == DP_BLE
                && ((BLEDevice *)FitnessDeviceManager::m_pSelectedControllableTrainerDevice)->m_bleSrc == BLES_ZCA)
                FitnessDeviceManager::m_pSelectedControllableTrainerDevice->Pair(true);
            if (FitnessDeviceManager::m_pSelectedSteeringDevice && FitnessDeviceManager::m_pSelectedSteeringDevice->m_protocol == DP_BLE
                && ((BLEDevice *)FitnessDeviceManager::m_pSelectedSteeringDevice)->m_bleSrc == BLES_ZCA)
                FitnessDeviceManager::m_pSelectedSteeringDevice->Pair(true);
            if (FitnessDeviceManager::m_pSelectedBrakingDevice && FitnessDeviceManager::m_pSelectedBrakingDevice->m_protocol == DP_BLE
                && ((BLEDevice *)FitnessDeviceManager::m_pSelectedBrakingDevice)->m_bleSrc == BLES_ZCA)
                FitnessDeviceManager::m_pSelectedBrakingDevice->Pair(true);
            if (FitnessDeviceManager::m_pSelectedRunSpeedDevice && FitnessDeviceManager::m_pSelectedRunSpeedDevice->m_protocol == DP_BLE
                && ((BLEDevice *)FitnessDeviceManager::m_pSelectedRunSpeedDevice)->m_bleSrc == BLES_ZCA)
                FitnessDeviceManager::m_pSelectedRunSpeedDevice->Pair(true);
            if (FitnessDeviceManager::m_pSelectedRunCadenceDevice && FitnessDeviceManager::m_pSelectedRunCadenceDevice->m_protocol == DP_BLE
                && ((BLEDevice *)FitnessDeviceManager::m_pSelectedRunCadenceDevice)->m_bleSrc == BLES_ZCA)
                FitnessDeviceManager::m_pSelectedRunCadenceDevice->Pair(true);
            if (g_WasPairedToAux) {                  // ZML_RestoreAuxPairing
                ZML_StartAuxPairing();
                ZML_SendAuxPairingSelection(g_SelectedAuxSampleHR, protobuf::HEART_RATE);
                ZML_SendAuxPairingSelection(g_SelectedAuxSampleWRS, protobuf::WALK_RUN_SPEED);
                ZML_EndAuxPairing();
            }
            if (!HasPairedToZML()) {
                auto sg = mainBike->GetSaveGame();// SaveFirstPairingToZML
                auto sig = SIG_CalcCaseInsensitiveSignature("rode_with_zml");
                static_assert(sizeof(GenericTrackingData) == 16);
                sg->SetTrackingData(sig, GenericTrackingData(1, sig));
                sg->m_field_1E1 = true;
                mainBike->SaveProfile(false, false); // SaveFirstPairingToZML }
            }
            for (auto &i : BikeManager::Instance()->m_allBikes)
                i->m_field_488 = 0;
        }
        g_pairToPhone = true;
    } else {
        g_pairToPhone = false;
    }
    zwift_network::Motion v105; //ZNETWORK_INTERNAL_ProcessMotionData
    if (zwift_network::motion_data(&v105)) {
        if (g_ptg_f9 > 0.0f) {
            g_phoneRotation = -v105.m_phoneRot;
            if (fabsf(g_phoneRotation) < 0.01f)
                g_phoneRotation = 0.0f;
            g_ptg_f7 = v105.m_ptg_f7;
            if (fabsf(g_ptg_f7) < 0.01f)
                g_ptg_f7 = 0.0f;
            g_ptg_f8 = v105.m_ptg_f8;
            if (fabs(g_ptg_f8) < 0.01f)
                g_ptg_f8 = 0.0;
        }
        g_ptg_f9 = v105.m_ptg_f9;
    }
    protobuf::WorldAttribute v90;                                   // ZNETWORK_INTERNAL_ProcessGlobalMessages
    if (ZNETWORK_IsLoggedIn()) //ZNETWORK_INTERNAL_ProcessProfileUpdates
        while (zwift_network::pop_world_attribute(&v90))
            ZNETWORK_INTERNAL_ProcessReceivedWorldAttribute(v90);
    int64_t updatedPlayerId;
    while (zwift_network::pop_player_id_with_updated_profile(&updatedPlayerId) && updatedPlayerId) {
        auto v32 = BikeManager::Instance()->FindBikeWithNetworkID(updatedPlayerId, false);
        if (v32)
            v32->RequestProfileFromServer();
    }
    ZNETWORK_INTERNAL_ProcessPhoneInput();
    ZNETWORK_INTERNAL_ProcessPlayerPackets();
    ZNETWORK_INTERNAL_ProcessUpcomingWorkouts();
    //OMIT ANALYTICS_InGameEvent();
    for (auto &i : g_Leaderboards.m_srwList) {
        if (i.m_hasFuture && i.m_future.valid() && std::future_status::ready == i.m_future.wait_for(std::chrono::seconds(0))) {
            auto res = i.m_future.get();
            if (res.m_errCode) {
                LogTyped(LOG_NETWORK, "LEADERBOARDS: Local Player request failed for results for segment hash %d", i.m_hash);
                LogTyped(LOG_NETWORK, "LEADERBOARDS: ERROR: \"%s\" (Outcome: %d) querying Local Player results", res.m_errCode, res.m_msg.c_str());
                i.m_hasFuture = false;
            } else {
                for (auto &sr : res.m_T.segment_results()) {
                    std::list<protobuf::SegmentResult>::iterator improvedPlace;
                    bool improved = false, known = false;
                    if (!i.m_srList1.empty()) {
                        for (auto i1 = i.m_srList1.begin(); i1 != i.m_srList1.end(); ++i1) {
                            if (sr.world_time() == i1->world_time())
                                known = true;
                            if (!improved  && sr.elapsed_ms() < i1->elapsed_ms()) {
                                improved = true;
                                improvedPlace = i1;
                            }
                        }
                    } 
                    if (!known) {
                        ZML_SendSegmentLeaderboardResult(protobuf::THIRTY_DAY, sr);
                        /*TODO v60 = &v36->field_90;
                        v61 = v36->field_90._Myfirst;
                        v62 = v61[1];
                        *&v94 = v62;
                        DWORD2(v94) = 0;
                        v63 = v61;
                        if (!v62->field_0[25])
                        {
                            do
                            {
                                *&v94 = v62;
                                if (*&v62->field_0[32] >= 1)
                                {
                                    DWORD2(v94) = 1;
                                    v63 = v62;
                                    v62 = *v62->field_0;
                                } else
                                {
                                    DWORD2(v94) = 0;
                                    v62 = *&v62->field_0[16];
                                }
                            } while (!v62->field_0[25]);
                            p_m_srList1 = srList1;
                        }
                        if (*(v63 + 25) || v63[4] > 1)
                        {
                            if (v60->_Mylast == 0x555555555555555i64)
                                std::vector<void *>::_Xlen();
                            v95 = v60;
                            v96 = 0i64;
                            v64 = operator new(0x30ui64);
                            v64[4] = 1;
                            *(v64 + 40) = 0;
                            *v64 = v61;
                            v64[1] = v61;
                            v64[2] = v61;
                            *(v64 + 12) = 0;
                            v96 = 0i64;
                            v97 = v94;
                            json_stuff_30(v60, &v97, v64);
                        }*/
                        if (improved)
                            i.m_srList1.insert(improvedPlace, sr);
                        else
                            i.m_srList1.push_back(sr);
                    }
                }
                i.m_hasFuture = true;
                i.m_field_21 = true;
            }
        }
    }
}
bool ZNETWORK_HasGivenRideOnToPlayer(int64_t playerId) {
    auto f = g_sessionRideonsGivenTo.find(playerId);
    if (f == g_sessionRideonsGivenTo.end())
        return false;
    return f->second;
}
std::future<NetworkResponse<int64_t>> g_SaveSegmentResultReturnCode;
void ZNETWORK_RegisterLocalPlayersSegmentResult(/*int64_t a1,*/ double wtSec, float durSec, float avgPower, bool isMale, float avgHr) { //IDA: ZNETWORK_RegisterLocalPlayersSegmentResult_0
    auto mainBike = BikeManager::Instance()->m_mainBike;
    zassert(mainBike != nullptr);
    if (mainBike && g_pGameWorld->WorldID()) {
        protobuf::SegmentResult sr;
        Leaderboards::SetPlayerSegmentResult(&sr, 0, 1, wtSec, durSec, avgPower, isMale, mainBike->m_ipsc->m_pty,
            mainBike->GetRiderWeightKG(false) * 1000.0f, nullptr, nullptr, mainBike->GetEventID(), avgHr, g_ActivityID);
        g_SaveSegmentResultReturnCode = zwift_network::save_segment_result(sr);
        Log("LEADERBOARDS: Saving segment results for segment %d with a time of %3.2f", 1, wtSec);
    }
}
void ZNETWORK_RegisterLocalPlayersSegmentResult(Leaderboards *lbs, int64_t segmentId, double wtSec, float durSec, float avgPower, bool isMale, int64_t eventId) {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    zassert(mainBike != nullptr);
    if (mainBike && g_pGameWorld->WorldID()) {
        protobuf::SegmentResult sr;
        Leaderboards::SetPlayerSegmentResult(&sr, 0, segmentId, wtSec, durSec, avgPower, isMale, mainBike->m_ipsc->m_pty,
            mainBike->GetRiderWeightKG(false) * 1000.0f, nullptr, nullptr, eventId, 0.0f, g_ActivityID);
        lbs->m_segSaveReqs.push_back(ZNet::SaveSegmentResult(sr, [sr](const int64_t &res) {
            Log("LEADERBOARDS: SUCCESS Segment result uploaded (result-id: %lld) (segment hash: %lld) (event subgroup id: %lld) (duration: %lldms)",
                res, sr.event_subgroup_id(), sr.world_time()/*CHECKME*/);
        }, [sr](ZNet::Error e) {
            Log("LEADERBOARDS: ERROR failed to upload segment result (segment hash: %lld) (event subgroup id: %lld) (duration: %lldms) error: %s",
                sr.segment_id(), sr.event_subgroup_id(), sr.elapsed_ms()/*CHECKME*/, e.m_msg.data());
        }));
        Log("LEADERBOARDS: Saving segment results for segment %lld with a time of %3.2f", segmentId, wtSec);
    }
}
void ZNETWORK_RegisterRideLeader(uint64_t eventId, int64_t leaderId, time_t worldTime, time_t timeExpire) {
    auto f = std::ranges::find_if(g_RideLeaderActions, [eventId](const RideLeaderActionInfo *i) { return i->m_eventId == eventId; });
    if (f == g_RideLeaderActions.end()) {
        g_RideLeaderActions.push_back(new RideLeaderActionInfo{ eventId, leaderId, worldTime, timeExpire });
    } else if ((*f)->m_worldTime < worldTime) {
        (*f)->m_leaderId = leaderId;
        (*f)->m_worldTime = worldTime;
        (*f)->m_worldTimeExpire = timeExpire;
    }
}
enum DropInWorldsStatus { DIW_INIT, DIW_WAIT, DIW_OK_PAUSE, DIW_GIVEUP } g_DropInWorldsStatus;
auto g_UpdateDropInWorldsStatusLam = []() { return zwift_network::fetch_worlds_counts_and_capacities(); };
std::future<NetworkResponse<protobuf::DropInWorldList>> g_UpdateDropInWorldsStatusFut;
int g_UpdateDropInWorldsStatusRepeats[2]{ 3, 3 };
protobuf::DropInWorldList g_DropInWorlds;
DWORD g_UpdateDropInWorldsStatusTicks;
void ZNETWORK_UpdateDropInWorldsStatus() {
    switch(g_DropInWorldsStatus) {
    case DIW_INIT:
        break;
    case DIW_WAIT:
        if (g_UpdateDropInWorldsStatusFut.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            auto v14 = g_UpdateDropInWorldsStatusFut.get();
            if (v14.m_errCode == 0) {
                g_DropInWorlds.CopyFrom(v14.m_T);
                g_UpdateDropInWorldsStatusTicks = GetTickCount();
                g_DropInWorldsStatus = DIW_OK_PAUSE;
                g_UpdateDropInWorldsStatusRepeats[1] = g_UpdateDropInWorldsStatusRepeats[0];
                return;
            }
            if (g_UpdateDropInWorldsStatusRepeats[1]) {
                --g_UpdateDropInWorldsStatusRepeats[1];
                break;
            }
            g_DropInWorldsStatus = DIW_GIVEUP;
        }
        return;
    case DIW_OK_PAUSE:
        if (g_UpdateDropInWorldsStatusTicks && int(GetTickCount() - g_UpdateDropInWorldsStatusTicks) < 30'000)
            return;
        break;
    default:
        return;
    }
    g_UpdateDropInWorldsStatusFut = g_UpdateDropInWorldsStatusLam();
    g_DropInWorldsStatus = DIW_WAIT;
}
void ZNETWORK_INTERNAL_ProcessReceivedWorldAttribute(const protobuf::WorldAttribute &wa) {
    auto ignoreGod = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\IGNOREGODMESSAGES", 0, true); (void)ignoreGod;
    auto mgr = BikeManager::Instance();
    auto mainBike = mgr->m_mainBike;
    switch (wa.wa_type()) {
    case protobuf::WAT_LEAVE:
        return;
    case protobuf::WAT_RELOGIN: {
        protobuf::PlayerLeftWorld plw;
        if (plw.ParseFromString(wa.payload())) {
            if (plw.another_login() && plw.player_id() == mainBike->m_playerIdTx) {
                if (!g_CachedWorldTime || g_CachedWorldTime - wa.world_time_born() > 300000)
                    return;
                mainBike->m_joinedWorld = false;
                LogTyped(LOG_NETWORK, "You were forced out of the world by another log-in");
            }
            g_Leaderboards.UserLeftWorld(plw.player_id());
            if (mainBike->m_profile.player_type() == protobuf::PACER_BOT)
                PacerBotSweatOMeter::Instance()->RemoveFollower(plw.player_id());
        }}
        return;
    case protobuf::WAT_RIDE_ON: {
        if (wa.world_time_expire() - wa.world_time_born() > 86'400'000) {
            ++g_nPhantomRidesOnReceived;
            return;
        }
        Log("Ride on received");
        protobuf::RideOn rio;
        if (rio.ParseFromString(wa.payload()) && std::find(g_sessionRideonsFrom.begin(), g_sessionRideonsFrom.end(), rio.player_id()) == g_sessionRideonsFrom.end()) {
            g_sessionRideonsRx.push_back(rio);
            ZML_RequestEffectPlay(protobuf::RECEIVED_RIDE_ON, true, true);
            if (g_tweakArray[TWI_MINIMALUI].IntValue()) { // HUD_GiveRideOn inlined
                auto suf = GetText("LOC_SAYS_RIDE_ON");
                char buf[0x400];
                if (rio.first_name().empty())
                    sprintf_s(buf, sizeof(buf), "%s %s", rio.last_name().c_str(), suf);
                else
                    sprintf_s(buf, sizeof(buf), "%c.%s %s", rio.first_name()[0], rio.last_name().c_str(), suf);
                auto flagTex = HUD_GetFlagTextureFromISO3166(rio.country_code(), true);
                if (flagTex == HUD_flagIcons)
                    flagTex = -1;
                HUD_Notify(buf, 6.0f, HNT_RIDEON_RX, flagTex, 0 /*not used*/, 0.0f/*not used*/, rio.player_id());
            }
            mainBike->GiveRideOn(rio.player_id());
            g_sessionRideonsFrom.push_back(rio.player_id());
            g_pNotableMomentsMgr.OnNotableMoment(protobuf::NMT_RIDEON_INT, mainBike->m_pos, rio.player_id(), 0, 0.0);
        }}
        return;
    case protobuf::WAT_SPA: {
        protobuf::SocialPlayerAction spa;
        if (spa.ParseFromString(wa.payload())) {
            if (spa.spa_type() != protobuf::SOCIAL_TEXT_MESSAGE && spa.spa_type() != protobuf::SOCIAL_FLAG)
                return;
            auto bk = mgr->FindBikeWithNetworkID(spa.player_id(), false);
            auto curLead = g_GroupEventsActive_CurrentLeader;
            if (g_currentPrivateEvent && g_currentPrivateEvent->m_leaderId) // GroupEvents::GetCurrentLeader
                curLead = g_currentPrivateEvent->m_leaderId;
            bool isLeader = curLead == spa.player_id();
            if (spa.spa_type() == protobuf::SOCIAL_TEXT_MESSAGE) {
                if (ProfanityFilter::PlayerOldEnoughToMessage() || isLeader || g_GroupEventsActive_CurrentSweeper == spa.player_id()) {
                    VEC3 pos{};
                    if (bk)
                        pos = mainBike->GetPosition();
                    else
                        if (wa.has_x()) {
                            pos.m_data[0] = wa.x();
                            pos.m_data[1] = wa.y_altitude();
                            pos.m_data[2] = wa.z();
                        }
                    if (g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\SHOWGROUPMSGS", true, true) && spa.message().size())
                        HUD_PushSPA(spa, pos);
                }
            } else if (spa.spa_type() == protobuf::SOCIAL_FLAG && spa.to_player_id() == mainBike->m_playerIdTx) {
                auto testerAdminWeight = ((bk && (bk->m_profile.f104() == 3)) || isLeader) ? 3 : 1;
                switch (spa.flag_type()) {
                case protobuf::FLAG_TYPE_BAD_LANGUAGE:
                    g_FlaggedAsPottyMouth += testerAdminWeight;
                    if (g_FlaggedAsHarasser + g_FlaggedAsPottyMouth >= 4 && !GroupEvents::GetCurrentEvent(spa.event_subgroup()))
                        ZNETWORK_FlagLocalPlayer(PFR_HARASSER);
                    break;
                case protobuf::FLAG_TYPE_HARASSMENT:
                    g_FlaggedAsHarasser += testerAdminWeight;
                    if (g_FlaggedAsPottyMouth + g_FlaggedAsHarasser >= 4 && !GroupEvents::GetCurrentEvent(spa.event_subgroup()))
                        ZNETWORK_FlagLocalPlayer(PFR_POTTY_MOUTH);
                    break;
                case protobuf::FLAG_TYPE_FLIER:
                    g_FlaggedAsFlier += testerAdminWeight;
                    if (g_FlaggedAsFlier >= 25 || (g_FlaggedAsFlier >= 10 && g_FlaggedAsFlier >= (g_currentEventTotalRiders * 0.05f)))
                        ZNETWORK_FlagLocalPlayer(PFR_FLIER);
                    break;
                }
            }
        }}
        return;
    case protobuf::WAT_EVENT:
    case protobuf::WAT_JOIN_E:
    case protobuf::WAT_LEFT_E:
        GroupEvents::OnWorldAttribute(wa);
        return;
    case protobuf::WAT_RQ_PROF:
        if (!mainBike || wa.world_time_expire() <= g_CachedWorldTime || (mainBike->m_playerIdTx < 0 && !mainBike->m_field_806))
            return;
        mainBike->RequestProfileFromServer();
        Log("ZwiftNetwork player profile update request for networkId: %d", mainBike->m_playerIdTx);
        return;
    case protobuf::WAT_INV_W: {
        if (wa.rel_id() != mainBike->m_playerIdTx) // PrivateEventsManagerV2::ReceiveInvitationWorldAttribute
            return;
        protobuf::EventInviteProto v199;
        v199.ParseFromString(wa.payload());
        Log("Received a meetup invitation notification");
        g_PrivateEvents.m_field_DC = true;
        g_PrivateEvents.m_field_DD = false;
        g_PrivateEvents.m_field_DE = false;
        g_PrivateEvents.GetPrivateEvents();
        auto d = (UI_DropInDialog *)UI_DialogPointer(UID_DROP_IN);
        if (d)
            d->RefreshGroupList();
        return;
    }
    case protobuf::WAT_KICKED:
        Log("kick me");
        GUI_CreateTwoButtonsDialog(GetText("LOC_GOT_IT_TITLE"), nullptr, GetText("LOC_KICKED_TITLE"), GetText("LOC_KICKED_CONTENT"), "", 30.0f, kickCallback, 600.0f, 180.0f, false, 0.0f, false);
        return;
    case protobuf::WAT_WTIME:
        if (ignoreGod)
            return;
        g_WorldTime = *(float *)wa.payload().c_str();
        return;
    case protobuf::WAT_RTIME: {
        if (ignoreGod)
            return;
        auto em = EntityManager::GetInst();
        auto b = (BlimpEntity *)em->FindFirstEntityOfType(Entity::ET_BLIMP);
        if (b) {
            auto rt = fmod(*(double *)wa.payload().c_str(), 1.0);
            b->SetRoadTime(rt);
        }
        return;
    }
    case protobuf::WAT_B_ACT:
        if (wa.has_world_time_expire()) {
            struct watBact { int64_t id; protobuf::UserBikeAction uba; } *pba = (watBact *)wa.payload().c_str();
            auto v103 = mgr->FindBikeWithNetworkID(pba->id, false);
            if (v103 && v103 != mainBike)
                v103->PerformAction(pba->uba);
        } else {
            LogTyped(LOG_NETWORK, "Non-expiring user action found");
        }
        return;
    case protobuf::WAT_GRP_M: {
        auto wgm = (ZNETWORK_TextMessage *)wa.payload().c_str();
        if (g_UserConfigDoc.GetBool("ZWIFT\\CONFIG\\SHOWGROUPMSGS", true, true)) {
            auto bk = mgr->FindBikeWithNetworkID(wgm->m_srcProfileId, false);
            VEC3 dv = mainBike->GetPosition() - (bk ? bk->GetPosition() : wgm->m_msgPos);
            if (dv.lenSquared() < wgm->m_msgRadius * wgm->m_msgRadius) {
                wgm->m_msg[511] = 0;
                HUD_PushTextMessage(*wgm);
            }
        } else {
            LogTyped(LOG_NETWORK, "Filtered area message due to user prefs");
        }}
        return;
    case protobuf::WAT_PRI_M: {
        auto wgm = (ZNETWORK_TextMessage *)wa.payload().c_str();
        if (wgm->m_destProfileId != mainBike->m_playerIdTx)
            return;
        wgm->m_msg[1022] = 0;
        wgm->m_msg[1023] = 0;
        LogTyped(LOG_NETWORK, "Received private message from %d (%s)", int(wgm->m_srcProfileId), wgm->m_msg);
        HUD_PushTextMessage(*wgm);
        //OMIT ++g_ZNETWORK_Stats[2];
        return;
    }
    case protobuf::WAT_SR: {
        protobuf::SegmentResult sr;
        sr.ParseFromString(wa.payload());
        double world_time_s = sr.world_time() * 0.001;
        bool male = true;
        if (sr.has_is_male())
            male = sr.is_male();
        if (sr.segment_id() != 1 || !sr.event_subgroup_id()) {
            auto elapsed_s = sr.elapsed_ms() * 0.001f;
            g_Leaderboards.SaveUserSegmentPR(sr.player_id(), sr.segment_id(), world_time_s, elapsed_s, sr.avg_power(), male, sr.power_source_model(), sr.weight_in_grams(), sr.first_name(), sr.last_name(), sr.event_subgroup_id());
        } else {
            if (GroupEvents::FindSubgroupEvent(sr.event_subgroup_id()))
                GroupEvents::ReportUserResult(sr);
            else
                PrivateEventV2::ReportUserResult(sr);
        }
        return;
    }
    case protobuf::WAT_FLAG: {
        auto flag = (BroadcastLocalPlayerFlagged *)wa.payload().c_str();
        if (flag->m_len != sizeof(*flag) - 4)
            return;
        if (flag->m_ver == 1) {
            Log("User %lld flagged", flag->m_playerId);
            if (flag->m_pfr == PFR_SANDBAGGER) {
                if (mainBike) { // Leaderboards::FlagSandbagger
                    auto fb = mgr->FindBikeWithNetworkID(flag->m_playerId, true);
                    if (fb && !fb->m_immuneFromCheating) {
                        g_Leaderboards.m_sandbaggers.emplace_back(BadGuyInfo{ flag->m_playerId, flag->m_worldTimeSec, 3600.0 });
                        fb->m_isSandbagger = true;
                        if (mainBike->m_playerIdTx == flag->m_playerId)
                            mainBike->m_isSandbagger = true;
                        auto c = (CriticalPowerCurve *)DataRecorder::Instance()->GetComponent(RecorderComponent::T_CPC);
                        if (c && c->m_sandbaggerCoeff < 1.0f && c->m_sandbaggerCoeff > 0.0f)
                            g_sandbaggerPenaltyScale = c->m_sandbaggerCoeff;
                    }
                }
            } else if (mainBike) { // Leaderboards::FlagCheater
                auto fb = mgr->FindBikeWithNetworkID(flag->m_playerId, true);
                if (fb && !fb->m_immuneFromCheating) {
                    g_Leaderboards.m_cheaters.emplace_back(BadGuyInfo{ flag->m_playerId, flag->m_worldTimeSec, 3600.0 });
                    fb->m_isCheater = 1;
                    if (mainBike->m_playerIdTx == flag->m_playerId)
                        mainBike->m_isCheater = true;
                }
            }
        } else {
            Log("Received invalid user flagged message. Wrong version (%d)", flag->m_ver);
        }}
        return;
    case protobuf::WAT_BC_PROMPT: {
        auto bcp = (WebEventStartMsg *)wa.payload().c_str();
        if (bcp->m_len != sizeof(WebEventStartMsg) - 4)
            return;
        if (bcp->m_ver == 1)
            EventSystem::GetInst()->TriggerEvent(EV_BC_PROMPT, 1, &bcp);
        else
            Log("Received invalid web event start message. Wrong version (%d)", bcp->m_ver);
        return;
    }
    case protobuf::WAT_RLA: { //ZNETWORK_BroadcastRideLeaderAction
        static_assert(sizeof(BroadcastRideLeaderAction) == 0x68);
        auto brla = (BroadcastRideLeaderAction *)wa.payload().c_str();
        if (brla->m_len != sizeof(BroadcastRideLeaderAction) - 4 || brla->m_ver != 1)
            return;
        switch (brla->m_rideLeaderAction) {
        case RLA_1: case RLA_2: case RLA_3: case RLA_4:
            ZNETWORK_RegisterRideLeader(brla->m_eventId, brla->m_leaderId, brla->m_worldTime, wa.world_time_expire());
            break;
        case RLA_6:
            for (auto i : g_RideLeaderActions) {
                if (i->m_eventId == brla->m_eventId) {
                    if (i->m_leaderId == brla->m_leaderId) {
                        if (brla->m_worldTime > i->m_worldTime) {
                            i->m_leaderId = 0;
                            i->m_worldTime = brla->m_worldTime;
                            i->m_worldTimeExpire = wa.world_time_expire();
                        }
                    }
                    break;
                }
            }
            break;
        default:
            break;
        }}
        return;
    case protobuf::WAT_GE: {
        static_assert(sizeof(GroupEvents::EntrantRaw) == 0x28);
        static_assert(sizeof(GroupEvents::Entrant) == 0x20);
        auto entr = (GroupEvents::EntrantRaw *)wa.payload().c_str();
        if (entr->m_len != sizeof(GroupEvents::EntrantRaw) - 4 || entr->m_ver != 1) {
            auto ge = GroupEvents::FindSubgroupEvent(entr->m_sgEventId); // GroupEvents::UserSignedup
            if (entr->m_isSignUp) {
                if (ge) {
                    GroupEvents::Subgroup_AddEntrantToSignups(ge, entr->m_playerIdTx, entr->m_wt_sec, entr->m_field_19);
                    return;
                }
                g_SignupList_Orphan.emplace_back(*entr);
            } else { // GroupEvents::UserRegistered
                if (ge) {
                    GroupEvents::Subgroup_AddEntrantToRegistrants(ge, entr->m_playerIdTx, entr->m_wt_sec, entr->m_field_19);
                    return;
                }
                g_RegistrantList_Orphan.emplace_back(*entr);
            }
        } else {
            Log("Received invalid entrant message. Wrong version (%d) or size (%d)" /*fixed by URSOFT, was 'invalid user flagged message'*/, entr->m_ver, entr->m_len);
        }
        return;
    }
    case protobuf::WAT_NM: {
        BroadcastLocalPlayerNotableMoment *pnm = (BroadcastLocalPlayerNotableMoment *)wa.payload().c_str();
        static_assert(sizeof(BroadcastLocalPlayerNotableMoment) == 48);
        if (pnm->m_len == sizeof(BroadcastLocalPlayerNotableMoment) - 4 && pnm->m_ver == 1) {
            if (pnm->m_playerIdTx != mainBike->m_playerIdTx) {
                auto bk = mgr->FindBikeWithNetworkID(pnm->m_playerIdTx, false);
                if (bk)
                    ConfettiComponent::SpawnPersonalConfetti(bk, pnm->m_nmt, pnm->m_field_24, pnm->m_field_1C);
            }
        } else {
            Log("Received invalid notable moment message. Wrong version (%d) or size (%d)", pnm->m_ver, pnm->m_len);
        }}
        return;
    case protobuf::WAT_LATE: {
        ZNETWORK_LateJoinRequest *ljr = (ZNETWORK_LateJoinRequest *)wa.payload().c_str();
        static_assert(sizeof(ZNETWORK_LateJoinRequest) == 40);
        if (ljr->m_len == 36 && ljr->m_playerId == mainBike->m_playerIdTx)
            ZNETWORK_INTERNAL_HandleLateJoinRequest(*ljr, VEC3{(float)wa.x(), (float)wa.y_altitude(), (float)wa.z()});
        }
        return;
    case protobuf::WAT_RH: {
        auto *prh = (ZNETWORK_RouteHashRequest *)wa.payload().c_str();
        static_assert(sizeof(ZNETWORK_RouteHashRequest) == 32);
        if (prh->m_len != sizeof(ZNETWORK_RouteHashRequest) - 4 || prh->m_playerIdTx != mainBike->m_playerIdTx)
            return;
        Log("Handle RouteHash request..."); //ZNETWORK_INTERNAL_HandleRouteHashRequest
        if (prh->m_kind == 1) {
            ZNETWORK_RespondToRouteHashRequest(prh->m_otherPlayerId);
            Log("Responded to RouteHash request");
        } else if (prh->m_kind == 2) {
            g_delayed_RouteHash_time = -1.0;
            g_routehashCt = 0;
            if (g_RouteHashTargetID == prh->m_otherPlayerId) {
                assert(prh->m_ver == 2);
                Log("RouteHash: received response! %u, %u", prh->m_routeHash, prh->m_decisionIndex);
                if (mainBike) {
                    auto routeComp = mainBike->m_routeComp;    // VirtualBikeComputer::RouteHashSet
                    if (routeComp) {
                        auto v178 = RouteManager::Instance()->GetRoute(prh->m_routeHash);
                        routeComp->SetRoute(v178, false, false, "default"s);
                        routeComp->SetDecisionStateToOffroute();
                        routeComp->m_decisionIndex = prh->m_decisionIndex;
                        Log("Forced decision index to %d", prh->m_decisionIndex);
                    }
                }
            } else {
                Log("RouteHash: received response, but it's from the wrong target...");
            }
        } else {
            Log("Unknown RouteHash command");
        }}
        return;
    case protobuf::WAT_STATS: {
        auto st = (ZNETWORK_GRFenceRiderStats *)wa.payload().c_str();
        static_assert(sizeof(ZNETWORK_GRFenceRiderStats) == 32);
        if (st->m_len == sizeof(ZNETWORK_GRFenceRiderStats) - 4)
            mainBike->m_grFenceComponent->OnReceiveRiderStats(*st);
        else
            LogTyped(LOG_WARNING, "[ZwiftNetwork] GLOBAL_MESSAGE_TYPE_RIDER_FENCE_STATS received but has invalid header!");
        }
        return;
    case protobuf::WAT_FENCE: {
        auto fc = (ZNETWORK_GRFenceConfig *)wa.payload().c_str();
        static_assert(sizeof(ZNETWORK_GRFenceConfig) == 48);
        if (fc->m_len == sizeof(ZNETWORK_GRFenceConfig) - 4) { // GroupRideFence::Component::OnReceiveFenceConfig
            mainBike->m_grFenceComponent->m_ver = fc->m_ver;
            mainBike->m_grFenceComponent->m_len = fc->m_len;
            if (fc->m_ver <= 1) {
                if (mainBike->m_grFenceComponent->m_field_128 == fc->m_field_8 && !mainBike->m_grFenceComponent->IsFenceGenerator()) {
                    mainBike->m_grFenceComponent->m_field_150 = 0;
                    mainBike->m_grFenceComponent->m_field_30 = fc->m_field_10;
                    mainBike->m_grFenceComponent->m_field_34 = fc->m_field_14;
                    mainBike->m_grFenceComponent->m_field_40 = fc->m_field_20;
                    mainBike->m_grFenceComponent->m_bool2C = fc->m_field_2C;
                    mainBike->m_grFenceComponent->m_field_144 = fc->m_field_28;
                }
            } else {
                mainBike->m_grFenceComponent->m_bool2C = false;
                LogTyped(LOG_WARNING, "Incompatible Group Ride Fence API between clients! Client: %d, Leader: %d. Disabling fence on this client.", 1, fc->m_ver);
            }
        } else {
            LogTyped(LOG_WARNING, "[ZwiftNetwork] GLOBAL_MESSAGE_TYPE_GRFENCE_CONFIG received but has invalid header!");
        }
    }
        return;
    case protobuf::WAT_BN_GE: {
        ZNETWORK_BibNumberForGroupEvent *pbn = (ZNETWORK_BibNumberForGroupEvent *)wa.payload().c_str();
        static_assert(sizeof(ZNETWORK_BibNumberForGroupEvent) == 40);
        if (mainBike && pbn->m_len == sizeof(ZNETWORK_BibNumberForGroupEvent) - 4 && pbn->m_ver == 1) {
            auto e = mainBike->GetEventID();
            if (e && e == pbn->m_eventId) {
                auto sg = GroupEvents::FindSubgroupEvent(e);
                if (sg)
                    sg->m_map[pbn->m_key] = pbn->m_val;
            }
        }
    }
    return;
    case protobuf::WAT_PPI: {
        auto ppi = (ZNETWORK_PacePartnerInfo *)wa.payload().c_str();
        static_assert(sizeof(ZNETWORK_PacePartnerInfo) == 32);
        if (ppi->m_len == sizeof(ZNETWORK_PacePartnerInfo) - 4 && ppi->m_ver == 1 && (g_GameMode & 0x34 /*TODO*/) == 0) { // ZNETWORK_INTERNAL_HandlePacePartnerInfo
            auto isBot = mainBike && mainBike->m_profile.player_type() == protobuf::PACER_BOT; // ZNETWORK_INTERNAL_HandlePacePartnerInfo
            switch (ppi->m_bcs) {
            case ZNETWORK_PacePartnerInfo::BS_2: case ZNETWORK_PacePartnerInfo::BS_5:
                if (!isBot)
                    PacerBotSweatOMeter::Instance()->UpdateDistanceRanges(ppi->m_float, *(int64_t *)ppi, ppi->m_bcs);
                break;
            case ZNETWORK_PacePartnerInfo::BS_1: case ZNETWORK_PacePartnerInfo::BS_10:
                if (isBot)
                    PacerBotSweatOMeter::Instance()->AdjustPacePartnerRanges(*ppi);
                break;
            }
        }
    }
        return;
    default:
        LogTyped(LOG_NETWORK, "Unhandled world message type %d", wa.wa_type());
    }
}
std::future<NetworkResponse<int64_t>> g_CreateActivityRideonFuture, g_BroadcastRouteHashReturnCode;
void ZNETWORK_RespondToRouteHashRequest(int64_t playerId) {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    if (mainBike && playerId != mainBike->m_playerIdTx && g_subscriptionMode != SM_INACTIVE) {
        protobuf::WorldAttribute wa;
        wa.set_x(mainBike->m_pos.m_data[0]);
        wa.set_y_altitude(mainBike->m_pos.m_data[1]);
        wa.set_z(mainBike->m_pos.m_data[2]);
        wa.set_wa_type(protobuf::WAT_RH);
        wa.set_world_time_expire(g_CachedWorldTime + 10000);
        wa.set_importance(5000000);
        wa.set_rel_id(playerId);
        ZNETWORK_RouteHashRequest v32{ .m_ver = 2, .m_len = sizeof(ZNETWORK_RouteHashRequest) - 4, .m_kind = 2, .m_otherPlayerId = mainBike->m_playerIdTx, .m_playerIdTx = playerId };
        static_assert(sizeof(ZNETWORK_RouteHashRequest) == 32);
        if (mainBike->m_routeComp && mainBike->m_routeComp->m_selRoute && mainBike->m_routeComp->m_selRoute->m_hash) {
            v32.m_routeHash = mainBike->m_routeComp->m_selRoute->m_hash;
            v32.m_decisionIndex = mainBike->m_routeComp->m_decisionIndex;
        } else if (mainBike->m_field_1814 == -1) {
            std::vector<Route *> v30;
            RouteManager::Instance()->FindRoutesByRoadInfo(mainBike->m_road ? mainBike->m_road->m_segmentId : 0, 0.0/*notused double*/, mainBike->m_field_8B8, &v30);
            if (!v30.empty()) /*QUEST: looks like a bug here, must be !empty - fixed*/ {
                int v11 = 0;
                float v10 = 3.4028235e38f;
                for (int i = 0; i < std::ssize(v30); i++) {
                    auto v16 = v30[i]->m_field_1BC;
                    if (v10 > v16) {
                        v11 = i;
                        v10 = v16;
                    }
                }
                auto v20 = RouteManager::Instance()->GetRoute(v30[v11]->m_hash);
                mainBike->m_routeComp->SetRoute(v20, false, false, "default"s);
                v32.m_routeHash = v30[v11]->m_hash;
            }
        }
        wa.set_payload((char *)&v32, sizeof(ZNETWORK_RouteHashRequest));
        Log("routeHash: respond: [%lld] -> [%lld]  (%u)", playerId, mainBike->m_playerIdTx, v32.m_routeHash);
        g_BroadcastRouteHashReturnCode = zwift_network::save_world_attribute(wa);
    }
}
void ZNETWORK_FlagLocalPlayer(PLAYER_FLAGGED_REASONS pfr, bool changeCheatBits /*= true*/) {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    if (mainBike && !mainBike->m_boolCheatSmth && !mainBike->m_immuneFromCheating) {
        ZNETWORK_BroadcastLocalPlayerFlagged(pfr);
        if (changeCheatBits)
            mainBike->m_cheatBits |= 2;
    }
}
void ZNETWORK_GiveRideOn(int64_t playerTo, bool a2) {
    auto f = g_sessionRideonsGivenTo.find(playerTo);
    if (f == g_sessionRideonsGivenTo.end() || f->second == false /*not sure*/) {
        Log("Total Ride Ons Given: %d", ++g_totalRideonsGiven);
        if (!a2) {
            auto mainBike = BikeManager::Instance()->m_mainBike;
            if (mainBike) {
                g_CreateActivityRideonFuture = zwift_network::create_activity_ride_on(mainBike->m_playerIdTx, playerTo);
                //OMIT ++g_ZNETWORK_Stats[3];
            }
        }
        g_sessionRideonsGivenTo[playerTo] = true;
        g_lastRideonTarget = playerTo;
        LARGE_INTEGER pc;
        QueryPerformanceCounter(&pc);
        g_lastRideonDebounce = pc.QuadPart + 3000;
    }
}
std::future<NetworkResponse<int64_t>> g_BroadcastRiderFenceStats, g_BroadcastGRFenceConfig, g_BroadcastLocalPlayerBibNumberForGroupEvent;
void ZNETWORK_BroadcastGRFenceStats(uint64_t a1, int64_t playerId, bool a3, bool a4/*, uint32_t a5*/) {
    static_assert(sizeof(ZNETWORK_GRFenceRiderStats) == 32);
    ZNETWORK_GRFenceRiderStats v12{ .m_ver = 1, .m_len = sizeof(ZNETWORK_GRFenceRiderStats) - 4, .m_field_8 = a1, 
        .m_playerIdTx = BikeManager::Instance()->m_mainBike->m_playerIdTx, .m_field_18 = a3, .m_field_19 = a4 };
    protobuf::WorldAttribute v13;
    v13.set_payload((char *)&v12, sizeof(ZNETWORK_GRFenceRiderStats));
    v13.set_wa_type(protobuf::WAT_STATS);
    v13.set_world_time_expire(g_CachedWorldTime + 30000);
    v13.set_rel_id(playerId);
    g_BroadcastRiderFenceStats = zwift_network::save_world_attribute(v13);
}
void ZNETWORK_BroadcastGRFenceConfig(uint64_t a1, uint32_t a2, float a3, float a4, float a5, float a6, float a7, uint32_t a8, uint32_t a9, bool a10) {
    static_assert(sizeof(ZNETWORK_GRFenceConfig) == 48);
    ZNETWORK_GRFenceConfig v17{ .m_ver = 1, .m_len = sizeof(ZNETWORK_GRFenceConfig) - 4, .m_field_8 = a1, .m_field_10 = a3, .m_field_14 = a4, .m_field_18 = a5,
        .m_field_1C = a6, .m_field_20 = a7, .m_field_24 = 30, .m_field_28 = a9, .m_field_2C = a10 };
    protobuf::WorldAttribute v28;
    v28.set_payload((char *)&v17, sizeof(ZNETWORK_GRFenceConfig));
    v28.set_wa_type(protobuf::WAT_FENCE);
    v28.set_world_time_expire(g_CachedWorldTime + 30000);
    g_BroadcastGRFenceConfig = zwift_network::save_world_attribute(v28);
}
void ZNETWORK_BroadcastBibNumberForGroupEvent(int64_t eventId, uint32_t expire, uint32_t a3) {
    static_assert(sizeof(ZNETWORK_BibNumberForGroupEvent) == 40);
    ZNETWORK_BibNumberForGroupEvent v17{ .m_ver = 1, .m_len = sizeof(ZNETWORK_BibNumberForGroupEvent) - 4, .m_key = BikeManager::Instance()->m_mainBike->m_playerIdTx,
        .m_eventId = eventId, .m_val = a3, .m_wt_sec = zwift_network::world_time() * 0.001 };
    protobuf::WorldAttribute v20;
    v20.set_payload((char *)&v17, sizeof(ZNETWORK_BibNumberForGroupEvent));
    v20.set_wa_type(protobuf::WAT_BN_GE);
    v20.set_world_time_expire(zwift_network::world_time() + 1000 * expire);
    g_BroadcastLocalPlayerBibNumberForGroupEvent = zwift_network::save_world_attribute(v20);
}
int64_t g_lastBclaTime;
std::future<NetworkResponse<int64_t>> g_BroadcastLocalPlayerRideLeaderAction, g_BroadcastLocalPlayerRegisterForGroupEvent;
void ZNETWORK_BroadcastRideLeaderAction(RideLeaderAction act, uint32_t a2_notused, uint64_t eventId) {
    static_assert(sizeof(BroadcastRideLeaderAction) == 0x68);
    auto v5 = (int64_t)_time64(nullptr);
    if (v5 - g_lastBclaTime >= 10) {
        g_lastBclaTime = v5;
        BroadcastRideLeaderAction v13{.m_ver = 1, .m_len = sizeof(BroadcastRideLeaderAction) - 4, .m_leaderId = BikeManager::Instance()->m_mainBike->m_playerIdTx, 
            .m_worldTime = g_CachedWorldTime, .m_eventId = eventId, .m_rideLeaderAction = act };
        protobuf::WorldAttribute v14;
        v14.set_payload((char *)&v13, sizeof(ZNETWORK_BibNumberForGroupEvent));
        v14.set_wa_type(protobuf::WAT_RLA);
        v14.set_world_time_expire(g_CachedWorldTime + 4'500'000);
        g_BroadcastLocalPlayerRideLeaderAction = zwift_network::save_world_attribute(v14);
    }
}
void ZNETWORK_BroadcastRegisterForGroupEvent(int64_t sgEventId, uint32_t exp, bool isSignUp, bool a4) {
    static_assert(sizeof(GroupEvents::EntrantRaw) == 0x24 + 4);
    GroupEvents::EntrantRaw v18{ .m_ver = 1, .m_len = sizeof(GroupEvents::EntrantRaw) - 4, .m_playerIdTx = BikeManager::Instance()->m_mainBike->m_playerIdTx,
        .m_sgEventId = sgEventId, .m_isSignUp = isSignUp, .m_field_19 = a4, .m_wt_sec = zwift_network::world_time() * 0.001 };
    protobuf::WorldAttribute v19;
    v19.set_payload((char *)&v18, sizeof(GroupEvents::EntrantRaw));
    v19.set_wa_type(protobuf::WAT_GE);
    v19.set_world_time_expire(zwift_network::world_time() + 1000 * exp);
    g_BroadcastLocalPlayerRegisterForGroupEvent = zwift_network::save_world_attribute(v19);
}
std::future<NetworkResponse<int64_t>> g_PrivateTextReturnCode, g_BroadcastAreaTextReturnCode, g_BroadcastSPAReturnCode;
void ZNETWORK_SendPrivateText(int64_t playerIdDest, int64_t playerIdSrc, const UChar *msg) {
    static_assert(sizeof(ZNETWORK_TextMessage) == 0x4B0);
    if (g_subscriptionMode != SM_INACTIVE) {
        ZNETWORK_TextMessage v16{ .m_srcProfileId = playerIdSrc, .m_destProfileId = playerIdDest, .m_worldTime = g_CachedWorldTime };
        int cnt = 0;
        while (*msg && cnt < 140) {
            v16.m_msg[cnt++] = *msg++;
        }
        protobuf::WorldAttribute v15;
        v15.set_payload((char *)&v16, sizeof(ZNETWORK_TextMessage)); //QUEST: why so much
        v15.set_wa_type(protobuf::WAT_PRI_M);
        v15.set_world_time_expire(g_CachedWorldTime + 10000);
        v15.set_rel_id(playerIdDest);
        g_PrivateTextReturnCode = zwift_network::save_world_attribute(v15);
    }
}
void ZNETWORK_BroadcastAreaText(int64_t playerId, const UChar *msg, const VEC3 &pos, float importance) {
    int64_t eventId = g_GroupEventsActive_BroadcastEventId;
    if (!g_GroupEventsActive_BroadcastEventId) {
        if (g_currentPrivateEvent)
            eventId = g_currentPrivateEvent->m_eventId;
        else
            eventId = 0;
    }
    static_assert(sizeof(ZNETWORK_TextMessage) == 0x4B0);
    if (g_subscriptionMode != SM_INACTIVE) {
        protobuf::SocialPlayerAction v28;
        TempUTF16 v30;
        v28.set_spa_type(protobuf::SOCIAL_TEXT_MESSAGE);
        auto &prof = BikeManager::Instance()->m_mainBike->m_profile;
        v28.set_first_name(prof.first_name());
        v28.set_last_name(prof.last_name());
        v28.set_country_code(prof.country_code());
        v28.set_avatar(prof.large_avatar_url());
        v28.set_player_id(playerId);
        v28.set_to_player_id(0);
        v28.set_message(SafeFromUTF8(msg, &v30));
        v28.set_event_subgroup(eventId);
        protobuf::WorldAttribute v29;
        v29.set_wa_type(protobuf::WAT_SPA);
        v29.set_world_time_expire(g_CachedWorldTime + 10000);
        v29.set_x(pos.m_data[0]);
        v29.set_y_altitude(pos.m_data[1]);
        v29.set_z(pos.m_data[2]);
        v29.set_importance(int32_t(importance));
        /* WHY? if (v28.m_to_player_id > 0) {
            v29.m_hasBits = v19 | 0xBD0;
            v29.m_rel_id = v28.m_to_player_id;
        }*/
        auto p = new std::string();
        v28.SerializeToString(p);
        v29.set_allocated_payload(p);
        g_BroadcastAreaTextReturnCode = zwift_network::save_world_attribute(v29);
    }
}
void ZNETWORK_SendSPA(protobuf::SocialPlayerAction *spa, const VEC3 &pos, float importance, int64_t eventSubgroupId) {
    //QUEST v16[2] = spa;
    if (g_subscriptionMode != SM_INACTIVE) {
        spa->set_player_id(BikeManager::Instance()->m_mainBike->m_profile.id());
        spa->set_event_subgroup(eventSubgroupId);
        protobuf::WorldAttribute v17;
        v17.set_wa_type(protobuf::WAT_SPA);
        v17.set_world_time_expire(g_CachedWorldTime + 10000);
        v17.set_x(pos.m_data[0]);
        v17.set_y_altitude(pos.m_data[1]);
        v17.set_z(pos.m_data[2]);
        v17.set_importance(int32_t(importance));
        if (spa->to_player_id() > 0)
            v17.set_rel_id(spa->to_player_id());
        auto p = new std::string();
        spa->SerializeToString(p);
        v17.set_allocated_payload(p);
        g_BroadcastSPAReturnCode = zwift_network::save_world_attribute(v17);
    }
}
int64_t g_lastBcnmTime;
std::future<NetworkResponse<int64_t>> g_BroadcastNotableMomentFuture, g_BroadcastPlayerFlagReturnCode;
void ZNETWORK_BroadcastLocalPlayerNotableMoment(NOTABLEMOMENT_TYPE nmt, uint32_t a2, float a4) {
    static_assert(sizeof(BroadcastLocalPlayerNotableMoment) == 48);
    auto v6 = (int64_t)_time64(nullptr);
    if (v6 - g_lastBcnmTime < 10)
        return;
    g_lastBcnmTime = v6;
    auto mainBike = BikeManager::Instance()->m_mainBike;
    BroadcastLocalPlayerNotableMoment v16{ .m_ver = 1, .m_len = sizeof(BroadcastLocalPlayerNotableMoment) - 4, .m_playerIdTx = mainBike->m_playerIdTx,
        .m_worldTime = g_CachedWorldTime, .m_nmt = nmt, .m_field_1C = a2, .m_field_24 = a4 };
    protobuf::WorldAttribute v17;
    auto pos = mainBike->GetPosition();
    v17.set_payload((char *)&v16, sizeof(BroadcastLocalPlayerNotableMoment));
    v17.set_wa_type(protobuf::WAT_NM);
    v17.set_world_time_expire(g_CachedWorldTime + 20000);
    v17.set_x(pos.m_data[0]);
    v17.set_y_altitude(pos.m_data[1]);
    v17.set_z(pos.m_data[2]);
    v17.set_importance(50000);
    g_BroadcastNotableMomentFuture = zwift_network::save_world_attribute(v17);
}
void ZNETWORK_SendLateJoinRequest(int64_t playerId) {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    //OMIT AnalyticsHelper_inst
    static_assert(sizeof(ZNETWORK_LateJoinRequest) == 0x28);
    ZNETWORK_LateJoinRequest v20{ .m_ver = 2, .m_len = sizeof(ZNETWORK_LateJoinRequest) - 4, .m_cmd = ZNETWORK_LateJoinRequest::LJC_1, .m_lateJoinPlayerId = mainBike->m_playerIdTx, .m_playerId = playerId };
    protobuf::WorldAttribute v21;
    if (g_subscriptionMode != SM_INACTIVE) {
        g_LateJoinCB = FinishLateJoin;
        v21.set_wa_type(protobuf::WAT_LATE);
        v21.set_world_time_expire(g_CachedWorldTime + 10000);
        v21.set_x(mainBike->m_pos.m_data[0]);
        v21.set_y_altitude(mainBike->m_pos.m_data[1]);
        v21.set_z(mainBike->m_pos.m_data[2]);
        v21.set_importance(5'000'000);
        v21.set_rel_id(playerId);
        v21.set_payload((char *)&v20, sizeof(ZNETWORK_LateJoinRequest));
        g_BroadcastLateJoinReturnCode = zwift_network::save_world_attribute(v21);
    }
}
void ZNETWORK_SendPlayerFlag(int64_t srcPlayerId, int64_t destPlayerId, protobuf::SocialFlagType sft) {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    if (g_subscriptionMode != SM_INACTIVE) {
        protobuf::SocialPlayerAction spa;
        spa.set_spa_type(protobuf::SOCIAL_FLAG);
        auto &prof = mainBike->m_profile;
        spa.set_first_name(prof.first_name());
        spa.set_last_name(prof.last_name());
        spa.set_country_code(prof.country_code());
        spa.set_avatar(prof.large_avatar_url());
        spa.set_player_id(srcPlayerId);
        spa.set_to_player_id(destPlayerId);
        spa.set_flag_type(sft);
        protobuf::WorldAttribute v29;
        v29.set_wa_type(protobuf::WAT_SPA);
        v29.set_world_time_expire(g_CachedWorldTime + 10000);
        if (destPlayerId > 0)
            v29.set_rel_id(destPlayerId);
        auto p = new std::string();
        spa.SerializeToString(p);
        v29.set_allocated_payload(p);
        g_BroadcastPlayerFlagReturnCode = zwift_network::save_world_attribute(v29);
    }
}
namespace zwift_network {
NetworkRequestOutcome send_image_to_mobile_app(const std::string &pathName, const std::string &imgName) {
    auto aux = g_networkClient->m_pImpl->m_aux.get();
    if (!aux || !aux->m_connectedOK || !aux)
        return NRO_NOT_PAIRED_TO_PHONE;
    aux->send_image_to_mobile_app(pathName, imgName);
    return NRO_OK;
}
NetworkRequestOutcome send_set_power_up_command(const std::string &locName, const std::string &color, const std::string &mask, int puId) {
    auto aux = g_networkClient->m_pImpl->m_aux.get();
    if (!aux || !aux->m_connectedOK || !aux)
        return NRO_NOT_PAIRED_TO_PHONE;
    aux->send_set_power_up_command(locName, color, mask, puId);
    return NRO_OK;
}
NetworkRequestOutcome send_rider_list_entries(const std::list<protobuf::RiderListEntry> &list) {
    auto aux = g_networkClient->m_pImpl->m_aux.get();
    if (!aux || !aux->m_connectedOK || !aux)
        return NRO_NOT_PAIRED_TO_PHONE;
    aux->send_rider_list_entries(list);
    return NRO_OK;
}
NetworkRequestOutcome send_mobile_alert_cancel_command(const protobuf::MobileAlert &ma) {
    auto aux = g_networkClient->m_pImpl->m_aux.get();
    if (!aux || !aux->m_connectedOK || !aux)
        return NRO_NOT_PAIRED_TO_PHONE;
    aux->send_mobile_alert_cancel_command(ma);
    return NRO_OK;
}
NetworkRequestOutcome send_player_profile(const protobuf::PlayerProfile &pp) {
    auto aux = g_networkClient->m_pImpl->m_aux.get();
    if (!aux || !aux->m_connectedOK || !aux)
        return NRO_NOT_PAIRED_TO_PHONE;
    aux->send_player_profile(pp);
    return NRO_OK;
}
NetworkRequestOutcome send_social_player_action(const protobuf::SocialPlayerAction &spa) {
    auto aux = g_networkClient->m_pImpl->m_aux.get();
    if (!aux || !aux->m_connectedOK || !aux)
        return NRO_NOT_PAIRED_TO_PHONE;
    aux->send_social_player_action(spa);
    return NRO_OK;
}
NetworkRequestOutcome send_mobile_alert(const protobuf::MobileAlert &ma) {
    auto aux = g_networkClient->m_pImpl->m_aux.get();
    if (!aux || !aux->m_connectedOK || !aux)
        return NRO_NOT_PAIRED_TO_PHONE;
    aux->send_mobile_alert(ma);
    return NRO_OK;
}
NetworkRequestOutcome send_default_activity_name_command(const std::string &name) {
    auto aux = g_networkClient->m_pImpl->m_aux.get();
    if (!aux || !aux->m_connectedOK || !aux)
        return NRO_NOT_PAIRED_TO_PHONE;
    aux->send_default_activity_name(name);
    return NRO_OK;
}
NetworkRequestOutcome send_game_packet(const std::string &a2, bool force) {
    auto aux = g_networkClient->m_pImpl->m_aux.get();
    if (!aux || !aux->m_connectedOK || !aux)
        return NRO_NOT_PAIRED_TO_PHONE;
    aux->send_game_packet(a2, force);
    return NRO_OK;
}
void get_connection_metrics(ConnectivityInfo *ptr) {
    g_networkClient->m_pImpl->getConnectionMetrics(ptr);
}
NetworkRequestOutcome send_ble_peripheral_request(const protobuf::BLEPeripheralRequest &rq) {
    auto aux = g_networkClient->m_pImpl->m_aux.get();
    if (!aux || !aux->m_connectedOK || !aux)
        return NRO_NOT_PAIRED_TO_PHONE;
    aux->send_ble_peripheral_request(rq);
    return NRO_OK;
}
NetworkRequestOutcome set_teleporting_allowed(bool a) {
    if (!g_networkClient->m_pImpl->m_aux)
        return NRO_NO_PLAYER_ID_YET;
    g_networkClient->m_pImpl->m_aux->m_teleportAllowed = a;
    return NRO_OK;
}
bool pop_world_attribute(protobuf::WorldAttribute *dest) {
    if (g_networkClient->m_pImpl->m_loginOK)
        return g_networkClient->m_pImpl->m_wat->popWorldAttribute(*dest);
    return false;
}
bool pop_player_id_with_updated_profile(int64_t *ret) {
    if (!g_networkClient->m_pImpl->m_loginOK)
        return false;
    return g_networkClient->m_pImpl->m_udpClient->m_updatedIds.try_dequeue(*ret); // UdpClient::popPlayerIdWithUpdatedProfile
}
bool pop_phone_to_game_command(protobuf::PhoneToGameCommand *pDest) { return g_networkClient->m_pImpl->popPhoneToGameCommand(pDest); }
std::future<NetworkResponse<protobuf::PlayerProfiles>> profiles(const std::unordered_set<int64_t> &rq) { return g_networkClient->m_pImpl->profiles(rq); }
bool motion_data(Motion *dest) {
    auto aux = g_networkClient->m_pImpl->m_aux.get();
    if (aux)
        return aux->motion_data(dest);
    else
        return false;
}
time_t from_iso_8601(const std::string &sdt) { return DateTime::fromIso8601StringUtc(sdt).m_value; }
std::string to_iso_8601(time_t dt) { return DateTime{ dt }.toIso8601String(); }
std::future<NetworkResponse<protobuf::DropInWorldList>> fetch_drop_in_world_list() { return g_networkClient->m_pImpl->fetchDropInWorldList(); }
std::list<ValidateProperty> parse_validation_error_message(const std::string &msg) { return g_networkClient->m_pImpl->parseValidationErrorMessage(msg); }
std::future<NetworkResponse<protobuf::DropInWorldList>> fetch_worlds_counts_and_capacities() { return g_networkClient->m_pImpl->fetchWorldsCountsAndCapacities(); }
bool is_paired_to_phone() {
    auto aux = g_networkClient->m_pImpl->m_aux.get();
    return aux && aux->m_connectedOK && aux->m_lastStatus;
}
ValidateProperty validateProperty(ProfileProperties pp, const std::string &ppval) {
    switch (pp) {
    case PP_EMAIL:
        if (ppval.length() < 6)
            return VP_EMAIL_SHORT;
        if (ppval.length() > 255)
            return VP_EMAIL_LONG;
        {
            const std::regex pattern("^[_A-Za-z0-9-\\+]+(\\.[_A-Za-z0-9-\\+]+)*@[A-Za-z0-9-]+(\\.[A-Za-z0-9]+)*(\\.[A-Za-z]{2,})$"
            /*"(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+"*/);
            if (std::regex_match(ppval, pattern))
                return VP_OK;
        }
        return VP_EMAIL_FORMAT;
    case PP_PASSWORD:
        if (ppval.length() < 8)
            return VP_PSW_SHORT;
        if (ppval.length() > 255)
            return VP_PSW_LONG;
        {
            const std::regex pattern("^((?=.*[0-9])(?=.*[A-Za-z])(?=\\S+$).{8,255})$");
            if (std::regex_match(ppval, pattern))
                return VP_OK;
        }
        return VP_PSW_FORMAT;
    case PP_FIRST_NAME:
        if (ppval.length() == 0)
            return VP_FN_SHORT;
        if (ppval.length() > 255)
            return VP_FN_LONG;
        return VP_OK;
    case PP_LAST_NAME:
        if (ppval.length() == 0)
            return VP_LN_SHORT;
        if (ppval.length() > 255)
            return VP_LN_LONG;
        return VP_OK;
    default:
        return VP_OK;
    }
}
std::future<NetworkResponse<bool>> remove_signup_for_event(int64_t id) { return g_networkClient->m_pImpl->removeSignupForEvent(id); }
std::future<NetworkResponse<protobuf::SegmentResults>> query_segment_results(int64_t serverRealm, int64_t segmentId, const std::string &from, const std::string &to, bool full) {
    return g_networkClient->m_pImpl->querySegmentResults(serverRealm, segmentId, from, to, full);
}
std::future<NetworkResponse<protobuf::SegmentResults>> subscribe_to_segment_and_get_leaderboard(int64_t sid) { return g_networkClient->m_pImpl->subscribeToSegmentAndGetLeaderboard(sid); }
std::future<NetworkResponse<protobuf::SegmentResults>> get_segment_jersey_leaders() { return g_networkClient->m_pImpl->getSegmentJerseyLeaders(); }
std::future<NetworkResponse<protobuf::PlayerProfile>> profile(int64_t profileId, bool bSocial) { return g_networkClient->m_pImpl->profile(profileId, bSocial); }
std::future<NetworkResponse<protobuf::ActivityList>> get_activities(int64_t profileId, const Optional<int64_t> &startsAfter, const Optional<int64_t> &startsBefore, bool fetchSnapshots) {
    return g_networkClient->m_pImpl->getActivities(profileId, startsAfter, startsBefore, fetchSnapshots);
}
std::future<NetworkResponse<void>> update_profile(bool inGameFields, const protobuf::PlayerProfile &prof, bool udp) { return g_networkClient->m_pImpl->updateProfile(inGameFields, prof, udp); }
std::future<NetworkResponse<int64_t>> save_activity(const protobuf::Activity &act, bool bFinalSave, const std::string &fitPath) { return g_networkClient->m_pImpl->saveActivity(act, bFinalSave, fitPath); }
std::future<NetworkResponse<void>> remove_goal(int64_t playerId, int64_t goalId) { return g_networkClient->m_pImpl->removeGoal(playerId, goalId); }
std::future<NetworkResponse<int64_t>> save_activity_image(int64_t profileId, const protobuf::ActivityImage &img, const std::string &imgPath) { return g_networkClient->m_pImpl->saveActivityImage(profileId, img, imgPath); }
std::future<NetworkResponse<protobuf::PlayerProfiles>> get_event_subgroup_entrants(protobuf::EventParticipation ep, int64_t eventId, uint32_t limit) { return g_networkClient->m_pImpl->getEventSubgroupEntrants(ep, eventId, limit); }
std::future<NetworkResponse<model::EventSignupResponse>> signup_for_event_subgroup(int64_t eventId) { return g_networkClient->m_pImpl->signupForEventSubgroup(eventId); }
std::future<NetworkResponse<void>> save_time_crossing_start_line(int64_t eventId, const protobuf::CrossingStartingLineProto &csl) { return g_networkClient->m_pImpl->saveTimeCrossingStartLine(eventId, csl); }
std::future<NetworkResponse<bool>> delete_subgroup_signup(int64_t id) { return g_networkClient->m_pImpl->deleteSubgroupSignup(id); }
std::future<NetworkResponse<void>> delete_activity(int64_t playerId, int64_t actId) { return g_networkClient->m_pImpl->deleteActivity(playerId, actId); }
std::future<NetworkResponse<protobuf::ZFileProto>> create_zfile_gzip(const std::string &name, const std::string &folder, const std::string &filePath) {
    return g_networkClient->m_pImpl->createZFileGzip(name, folder, filePath);
}
std::future<NetworkResponse<model::EventSignupResponse>> create_subgroup_signup(int64_t id) { return g_networkClient->m_pImpl->createSubgroupSignup(id); }
std::future<NetworkResponse<bool>> create_subgroup_registration(int64_t id) { return g_networkClient->m_pImpl->createSubgroupRegistration(id); }
std::future<NetworkResponse<int64_t>> create_activity_ride_on(int64_t playerIdSender, int64_t playerIdTarget) {
    return g_networkClient->m_pImpl->createActivityRideOn(playerIdSender, playerIdTarget);
}
std::future<NetworkResponse<protobuf::PrivateEventFeedListProto>> private_event_feed(int64_t start_date, int64_t end_date, Optional<protobuf::EventInviteStatus> status, bool organizer_only_past_events) {
    return g_networkClient->m_pImpl->privateEventFeed(start_date, end_date, status, organizer_only_past_events);
}
std::future<NetworkResponse<bool>> register_for_event_subgroup(int64_t id) { return g_networkClient->m_pImpl->registerForEventSubgroup(id); }
std::future<NetworkResponse<protobuf::PlayerSocialNetwork>> get_followees(int64_t profileId, bool followRequests) { return g_networkClient->m_pImpl->getFollowees(profileId, followRequests); }
std::future<NetworkResponse<protobuf::EventProtobuf>> get_event(int64_t id) { return g_networkClient->m_pImpl->getEvent(id); }
std::future<NetworkResponse<int64_t>> save_world_attribute(const protobuf::WorldAttribute &wa) { return g_networkClient->m_pImpl->saveWorldAttribute(wa); }
std::future<NetworkResponse<void>> create_user(const std::string &email, const std::string &pwd, const std::string &firstN, const std::string &lastN) {
    return g_networkClient->m_pImpl->createUser(email, pwd, firstN, lastN);
}
std::future<NetworkResponse<protobuf::RaceResultSummary>> get_subgroup_race_result_summary(int64_t sid) { return g_networkClient->m_pImpl->getSubgroupRaceResultSummary(sid); }
std::future<NetworkResponse<protobuf::PlayerProfile>> my_profile() { return g_networkClient->m_pImpl->myProfile(); }
std::future<NetworkResponse<protobuf::Clubs>> list_my_clubs(Optional<protobuf::Membership_Status> status, Optional<int> start, Optional<int> limit) { return g_networkClient->m_pImpl->listMyClubs(status, start, limit); }
std::future<NetworkResponse<protobuf::PlaybackMetadata>> get_my_playback_latest(int64_t a2, uint64_t after, uint64_t before) { return g_networkClient->m_pImpl->getMyPlaybackLatest(a2, after, before); }
std::future<NetworkResponse<protobuf::PlaybackMetadata>> get_my_playback_pr(int64_t a2, uint64_t after, uint64_t before) { return g_networkClient->m_pImpl->getMyPlaybackPr(a2, after, before); }
std::future<NetworkResponse<protobuf::EventsProtobuf>> get_events_in_interval(const std::string &start, const std::string &end, int limit) { return g_networkClient->m_pImpl->getEventsInInterval(start, end, limit); }
std::future<NetworkResponse<protobuf::CampaignRegistrationResponse>> get_registration_in_campaign_v2(const std::string &sn) { return g_networkClient->m_pImpl->getRegistrationInCampaignV2(sn); }
std::future<NetworkResponse<protobuf::ListCampaignRegistrationSummaryResponse>> get_campaigns_v2() { return g_networkClient->m_pImpl->getCampaignsV2(); }
std::future<NetworkResponse<protobuf::CampaignRegistrationResponse>> enroll_in_campaign_v2(const std::string &sn) { return g_networkClient->m_pImpl->enrollInCampaignV2(sn); }
std::future<NetworkResponse<void>> unlock_achievements(const protobuf::AchievementUnlockRequest &rq) { return g_networkClient->m_pImpl->unlockAchievements(rq); }
std::future<NetworkResponse<void>> set_my_active_club(const protobuf::UUID &id) { return g_networkClient->m_pImpl->setMyActiveClub(id); }
std::future<NetworkResponse<int64_t>> save_segment_result(const protobuf::SegmentResult &sr) { return g_networkClient->m_pImpl->saveSegmentResult(sr); }
std::future<NetworkResponse<std::string>> save_playback(const protobuf::PlaybackData &data) { return g_networkClient->m_pImpl->savePlayback(data); }
std::future<NetworkResponse<void>> save_route_result(const protobuf::RouteResultSaveRequest &r) { return g_networkClient->m_pImpl->saveRouteResult(r); }
std::future<NetworkResponse<void>> reset_my_active_club() { return g_networkClient->m_pImpl->resetMyActiveClub(); }
std::future<NetworkResponse<void>> reject_private_event_invitation(int64_t id) { return g_networkClient->m_pImpl->rejectPrivateEventInvitation(id); }
std::future<NetworkResponse<protobuf::ProfileEntitlements>> my_profile_entitlements() { return g_networkClient->m_pImpl->myProfileEntitlements(); }
std::future<NetworkResponse<protobuf::ZFilesProto>> list_zfiles(const std::string &folder) { return g_networkClient->m_pImpl->listZFiles(folder); }
std::future<NetworkResponse<protobuf::PrivateEventProto>> get_private_event(int64_t id) { return g_networkClient->m_pImpl->getPrivateEvent(id); }
std::future<NetworkResponse<protobuf::PlaybackMetadataList>> get_my_playbacks(int64_t a2) { return g_networkClient->m_pImpl->getMyPlaybacks(a2); }
std::future<NetworkResponse<protobuf::PlaybackData>> get_playback_data(const protobuf::PlaybackMetadata &md) { return g_networkClient->m_pImpl->getPlaybackData(md); }
std::future<NetworkResponse<protobuf::LateJoinInformation>> get_late_join_information(int64_t meetupId) { return g_networkClient->m_pImpl->getLateJoinInformation(meetupId); }
std::future<NetworkResponse<protobuf::FeatureResponse>> get_feature_response(const protobuf::FeatureRequest &rq) { return g_networkClient->m_pImpl->getFeatureResponse(rq); }
std::future<NetworkResponse<protobuf::ListPublicActiveCampaignResponse >> get_active_campaigns() { return g_networkClient->m_pImpl->getActiveCampaigns(); }
std::future<NetworkResponse<protobuf::Achievements>> get_achievements() { return g_networkClient->m_pImpl->getAchievements(); }
std::future<NetworkResponse<void>> erase_zfile(int64_t id) { return g_networkClient->m_pImpl->eraseZFile(id); }
std::future<NetworkResponse<protobuf::EventsProtobuf>> get_events(const model::EventsSearch &es) { return g_networkClient->m_pImpl->getEvents(es); }
std::future<NetworkResponse<protobuf::ZFileProto>> create_zfile(const protobuf::ZFileProto &p) { return g_networkClient->m_pImpl->createZFile(p); }
std::future<NetworkResponse<void>> remove_followee(int64_t playerId, int64_t followeeId) { return g_networkClient->m_pImpl->removeFollowee(playerId, followeeId); }
std::future<NetworkResponse<protobuf::SocialNetworkStatus>> add_followee(int64_t playerId, int64_t followeeId, bool a5, protobuf::ProfileFollowStatus pfs) { return g_networkClient->m_pImpl->addFollowee(playerId, followeeId, a5, pfs); }
std::future<NetworkResponse<protobuf_bytes>> download_zfile(int64_t id) { return g_networkClient->m_pImpl->downloadZFile(id); }
std::future<NetworkResponse<void>> create_race_result_entry(const protobuf::RaceResultEntrySaveRequest &rq) { return g_networkClient->m_pImpl->createRaceResultEntry(rq); }
std::future<NetworkResponse<void>> accept_private_event_invitation(int64_t id) { return g_networkClient->m_pImpl->acceptPrivateEventInvitation(id); }
bool pop_server_to_client(std::shared_ptr<protobuf::ServerToClient> &dest) { return g_networkClient->m_pImpl->popServerToClient(dest); }
NetworkRequestOutcome unsubscribe_from_segment(int64_t id) { return g_networkClient->m_pImpl->unsubscribeFromSegment(id); }
NetworkRequestOutcome send_activate_power_up_command(int powerupId, int powerupParam) { return g_networkClient->m_pImpl->sendActivatePowerUpCommand(powerupId, powerupParam); }
NetworkRequestOutcome send_customize_action_button_command(uint32_t a2, uint32_t a3, char *a4, char *a5, bool a6) { return g_networkClient->m_pImpl->sendCustomizeActionButtonCommand(a2, a3, a4, a5, a6); }
std::future<NetworkResponse<Json::Value>> get_activity_recommendations(const std::string &aGoal) { return g_networkClient->m_pImpl->getActivityRecommendations(aGoal); }
std::future<NetworkResponse<void>> reset_password(const std::string &newPwd) { return g_networkClient->m_pImpl->resetPassword(newPwd); }
    std::future<NetworkResponse<protobuf::PowerCurveAggregationMsg>> get_best_efforts_power_curve_from_all_time() { return g_networkClient->m_pImpl->getBestEffortsPowerCurveFromAllTime(); }
std::future<NetworkResponse<protobuf::Goals>> get_goals(int64_t playerId) {
    return g_networkClient->m_pImpl->getGoals(playerId);
}
std::future<NetworkResponse<protobuf::Goal>> save_goal(const protobuf::Goal &g) {
    return g_networkClient->m_pImpl->saveGoal(g);
}
std::future<NetworkResponse<std::string>> log_out() { return g_networkClient->m_pImpl->logOut(); }
std::future<NetworkResponse<protobuf::PlayerState>> latest_player_state(int64_t worldId, int64_t playerId) {
    return g_networkClient->m_pImpl->latestPlayerState(worldId, playerId);
}
std::future<NetworkResponse<std::string>> log_in_with_oauth2_credentials(const std::string &sOauth, const std::vector<std::string> &anEventProps, const std::string &oauthClient) {
    return g_networkClient->m_pImpl->logInWithOauth2Credentials(sOauth, anEventProps, oauthClient);
}
std::future<NetworkResponse<std::string>> log_in_with_email_and_password(const std::string &email, const std::string &pwd, const std::vector<std::string> &anEventProps, bool reserved, const std::string &oauthClient) {
    return g_networkClient->m_pImpl->logInWithEmailAndPassword(email, pwd, anEventProps, reserved, oauthClient);
}
uint64_t world_time() { auto w = g_networkClient->m_pImpl->m_wclock.get(); return w ? w->getWorldTime() : 0; }
}
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
uint64_t g_serverTime;
uint64_t ZNETWORK_GetNetworkSyncedTimeGMT() {
    static const double bc = 9.223372036854776e18;
    if (!g_serverTime)
        return 0i64;
    uint64_t ovf_corr = 0i64;
    if (g_accumulatedTime >= bc) {
        g_accumulatedTime -= bc;
        if (g_accumulatedTime - bc < bc)
            ovf_corr = 0x8000000000000000ui64;
    }
    return g_serverTime + ovf_corr + (uint64_t)g_accumulatedTime;
}
bool ZNETWORK_IsLoggedIn() {
    if (g_networkClient->m_pImpl && g_networkClient->m_pImpl->m_authMgr) {
        return g_networkClient->m_pImpl->m_authMgr->getLoggedIn();
    }
    return false;
}
void ZNETWORK_Initialize() {
    auto server = g_UserConfigDoc.GetCStr("ZWIFT\\CONFIG\\SERVER_URL", "https://us-or-rly101.zwift.com", false);
    g_IsOnProductionServer = (strstr(server, "us-or-rly101") != nullptr);
    struct tm v51 { 16, 15, 14, 13, 0, 70 };
    g_magicLeaderboardBirthday = _mktime64(&v51);
    //OMIT CrashReporting::SetServer
    Log("Calling initialize_zwift_network()");
    LogDebug("  With serverurl: %s", server);
    g_NetworkOn = initialize_zwift_network(server, GAMEPATH("data/cacert.pem"), "1.0.106405");
    //zwift_network::register_logging_function inlined by URSOFT
    LogDebug("End ZNETWORK_Initilize() - serverURL = %s", server); //URSOFT: should be ZNETWORK_Initialize, but...
    PopulateBotInfo(server);
}
GlobalState::GlobalState(EventLoop *el, const protobuf::PerSessionInfo &psi, const std::string &sessionId, const EncryptionInfo &ei) :
    m_evloop(el), m_psi(psi), m_sessionInfo(sessionId), m_ei(ei) {
    m_shouldUseEncryption = ei.m_sk.length() != 0;
    m_time = g_steadyClock.now();
    /*TODO (_QWORD *)&this->field_108 = 0i64;
    v9 = operator new(0x20ui64);
    *v9 = v9;
    v9[1] = v9;
    this->field_100 = v9;
    *(_QWORD *)&this->field_288 = 0i64;
    v10 = operator new(0x20ui64);
    *v10 = v10;
    v10[1] = v10;
    *(_QWORD *)&this->field_280 = v10;
    *(_QWORD *)&this->field_3C8 = 0i64;
    v11 = operator new(0x20ui64);
    *v11 = v11;
    v11[1] = v11;
    this->field_3C0 = v11;*/
}
void GlobalState::registerUdpConfigListener(UdpConfigListener *lis) {
    m_evloop->post([this, lis]() {
        this->m_ucLis += lis;
    });
}
void GlobalState::registerEncryptionListener(EncryptionListener *lis) {
    m_evloop->post([this, lis]() {
        this->m_encLis += lis;
    });
}
void GlobalState::registerWorldIdListener(WorldIdListener *lis) {
    m_evloop->post([this, lis]() {
        this->m_widLis += lis;
    });
}
void GlobalState::setWorldId(int64_t worldId) {
    m_worldId = worldId;
    m_evloop->post([this, worldId]() {
        this->m_widLis.notify([this, worldId](WorldIdListener &wil) {
            wil.handleWorldIdChange(m_worldId);
        });
    });
}
void GlobalState::setEncryptionInfo(const EncryptionInfo &ei) { 
    std::lock_guard l(m_mutex);
    m_shouldUseEncryption = !ei.m_sk.empty();
    m_ei = ei;
    m_evloop->post([this, ei]() {
        this->m_encLis.notify([this, ei](EncryptionListener &el) {
            el.handleEncryptionChange(ei);
        });
    });
}
void GlobalState::setUdpConfig(const protobuf::UdpConfigVOD &uc, uint64_t a3) {
    m_evloop->post([this, uc, a3]() {
        this->m_ucLis.notify([this, uc, a3](UdpConfigListener &ul) {
            ul.handleUdpConfigChange(uc, a3);
        });
    });
}
void UdpClient::handleWorldAndMapRevisionChanges(uint64_t time, int64_t world, uint32_t map) {
    if (world != -1 && (world != m_world || map != m_map)) {
        m_world = world;
        m_map = map;
        m_field_15C = false;
        if (m_field_1208 == 2)
            m_field_1208 = 0;
        NetworkingLogInfo("Changed to world-map [%lld,%u]", world, map);
        handleRelayAddress(time);
        m_gs->setWorldId(m_world);
        m_netImpl->handleWorldAndMapRevisionChanged(m_world, m_map);
    }
}
void UdpClient::handleServerToClient(const std::shared_ptr<protobuf::ServerToClient> &src) {
    m_netImpl->m_udpStat->m_packets++;
    if (src->has_has_simult_login() && src->has_simult_login()) {
        if (!m_field_F20) {
            m_field_F20 = true;
            m_netImpl->handleDisconnectRequested(true);
            NetworkingLogWarn("Disconnected due to simultaneous login");
        }
        m_us->inspectServerToClient(src, m_wcs->getWorldTime(), m_host);
    } else {
        if (m_field_F20) {
            m_field_F20 = false;
            m_netImpl->handleDisconnectRequested(false);
            NetworkingLogWarn("Simultaneous login has ceased");
        }
        if (!src->has_server_realm() || src->server_realm() == 0)
            m_us->inspectServerToClient(src, m_wcs->getWorldTime(), m_host);
        else if (src->server_realm() != m_world) {
            m_periodicLogger.log();
            m_us->inspectServerToClient(src, m_wcs->getWorldTime(), m_host);
        } else {
            if (src->has_expunge_reason()) {
                m_expungeReason = src->expunge_reason();
                m_us->incrementExpungeReasonReceived();
                NetworkingLogInfo("World-map [%lld,%u] expunge reason: %d", m_world, m_map, m_expungeReason);
            }
            if (src->has_expunge_reason() && m_expungeReason) {
                m_us->inspectServerToClient(src, m_wcs->getWorldTime(), m_host);
            } else {
                m_wcs->handleServerToClient(*src);
                m_us->inspectServerToClient(src, m_wcs->getWorldTime(), m_host);
                for (const auto &s : src->states1()) {
                    if (s.has_customization_id()) {
                        uint64_t &v21 = m_field_FC0[s.id()];
                        auto v22 = s.customization_id();
                        if (v21 != v22 && v22 < src->world_time() - 2000) {
                            v21 = v22;
                            m_updatedIds.enqueue(s.id());
                        }
                    }
                }
                m_stcs.emplace(src, g_steadyClock.now());
            }
        }
    }
}
void TcpClient::processPayload(uint64_t len) {
    //OMIT TcpStatistics::registerNetUseIn(*((TcpStatistics **)this + 24), len + 2);
    auto decodedPtr = decodeMessage(m_buf64k + 2, &len);
    if (!decodedPtr) {
        handleCommunicationError(boost::asio::error::make_error_code(boost::asio::error::broken_pipe), "Failed to decode TCP StC"s);
    } else {
        protobuf::ServerToClient stc;
        if (stc.ParseFromArray(decodedPtr, len)) {
            auto wcs_time = m_wcs->getWorldTime();
            m_ncli->m_tcpStat->m_packets++;
            //OMIT ? TcpStatistics::inspectServerToClient((_Mtx_t)this->m_stat, (__int64)&_stc_ptr->m_stc, wcs_time);
            m_wat->handleServerToClient(stc);
            if (stc.has_zc_local_ip()) {
                auto &zc_key = stc.zc_key();
                if (m_ncli->GetNetworkMaxLogLevel() == NL_DEBUG) {
                    if (zc_key.length()) {
                        auto sk = base64::toString(zc_key);
                        NetworkingLogDebug("ZC encryption info from TCP StC: port %d secure port %d secret key %s", stc.zc_local_port(), stc.zc_secure_port(), sk.c_str());
                    } else {
                        NetworkingLogDebug("ZC encryption info missing in TCP StC (no auxiliary_controller_secret_key)");
                    }
                }
                AuxiliaryControllerAddress auxAddr(stc.zc_local_ip(), stc.zc_local_port(), stc.zc_protocol(), stc.zc_secure_port(), fix_google(zc_key));
                m_ncli->handleAuxiliaryControllerAddress(std::move(auxAddr));
            }
            if (stc.has_udp_config_vod_1())
                m_gs->setUdpConfig(stc.udp_config_vod_1(), wcs_time);
            if (stc.has_tcp_config()) {
                NetworkingLogDebug("TCP TcpConfig from StC");
                handleTcpConfig(stc.tcp_config());
            }
            processSubscribedSegment(stc); //QUEST я передвинул перед m_rwq.emplace, чтобы объект был еще тут - надеюсь, не повлияет
            if (stc.has_race_placements())
                m_rwq.emplace(wcs_time, std::make_shared<protobuf::ServerToClient>(std::move(stc)));
        } else {
            //OMIT TcpStatistics::increaseParseErrorCount_0((_Mtx_t)this->m_stat);
            handleCommunicationError(boost::asio::error::make_error_code(boost::asio::error::broken_pipe), "Failed to parse TCP StC"s);
        }
    }
}
int g_segmentCnt;
SegmentResultsWrapper *ZNETWORK_RegisterSegmentID(int64_t hash, TimingArchEntity *tae /*= nullptr*/) {
    ++g_segmentCnt;
    Log("Registered segment %d with hash id %lld", g_segmentCnt, hash);
    SegmentResultsWrapper item;
    item.m_hash = hash;
    //TODO *(_QWORD *)item_->field_50 = 0i64;    *(_WORD *)&item_->field_50[8] = 0;    *(_WORD *)&item_->field_50[32] = 0;
    item.m_srpList.m_tae = tae;
    return &g_Leaderboards.m_srwList.emplace_back(std::move(item));
}
void ZNETWORK_RegisterLocalPlayersRouteResult(const RouteFinishData &rfd) {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    auto v3 = g_pGameWorld->WorldID();
    if (mainBike && (mainBike->m_playerIdTx >= 0 || mainBike->m_field_806) && v3) {
        protobuf::RouteResultSaveRequest v22;
        v22.set_server_realm(g_CurrentServerRealmID);
        v22.set_map_id(v3);
        if (g_ActivityID > 0)
            v22.set_activity_id(g_ActivityID);
        v22.set_route_hash(rfd.m_routeHash);
        v22.set_event_id(rfd.m_eventId);
        v22.set_world_time(rfd.m_world_time);
        v22.set_elapsed_ms(rfd.m_elapsed_ms);
        v22.set_power_type(rfd.m_powerMeter ? protobuf::PT_METER : protobuf::PT_VIRTUAL);
        v22.set_height_in_cm(rfd.m_height_in_cm);
        v22.set_weight_in_grams(rfd.m_weight_in_grams);
        v22.set_gender(rfd.m_isMale ? protobuf::MALE : protobuf::FEMALE);
        v22.set_avg_power((int)rfd.m_avg_power);
        v22.set_max_power((int)rfd.m_max_power);
        v22.set_avg_hr((int)rfd.m_avg_hr);
        v22.set_max_hr((int)rfd.m_max_hr);
        v22.set_ftp(rfd.m_ftp);
        v22.set_steering(rfd.m_steering);
        v22.set_player_type(rfd.m_player_type);
        v22.set_calories((int)rfd.m_calories);
        v22.set_sport(rfd.m_sport);
        v22.set_cadence_sensor(FitnessDeviceManager::GetDeviceNameAndModel(*FitnessDeviceManager::m_pSelectedCadenceDevice));
        v22.set_controllable(FitnessDeviceManager::GetDeviceNameAndModel(*FitnessDeviceManager::m_pSelectedControllableTrainerDevice));
        v22.set_hr_monitor(FitnessDeviceManager::GetDeviceNameAndModel(*FitnessDeviceManager::m_pSelectedHRDevice));
        v22.set_power_meter(FitnessDeviceManager::GetDeviceNameAndModel(*FitnessDeviceManager::m_pSelectedPowerDevice));
        Log(": Saving Full route results for route %lld with a time of %f", rfd.m_routeHash, rfd.m_world_time);
        zwift_network::save_route_result(v22).get();
    }
}
std::future<NetworkResponse<void>> ZNETWORK_RaceResultEntrySaveRequest(double w_time, float resultTimeInSeconds, BikeEntity *pBike, bool lj, float data_f4) {
    auto groupEvent = pBike->GetEventID();
    Log("RaceResult: (player) groupEvent %d seconds %3.2f for %3.2f meter", groupEvent, resultTimeInSeconds, data_f4, lj ? " (LJ)" : "");
    protobuf::RaceResultEntrySaveRequest rq;
    rq.set_late_join(lj);
    rq.set_bib_num(pBike->m_raceBibNumber);
    rq.set_is_cheater(pBike->m_isCheater);
    rq.set_is_sandbagger(pBike->m_isSandbagger);
    auto data = rq.mutable_data();
    data->set_activity_id(g_ActivityID);
    data->set_sport(pBike->m_bc->m_sport);
    data->set_map_id(g_pGameWorld->WorldID());
    data->set_server_realm(g_CurrentServerRealmID);
    data->set_world_time_ms(uint64_t(w_time * 1000.0));
    data->set_result_time_ms(uint64_t(resultTimeInSeconds * 1000.0f));
    data->set_f4((int)data_f4); //distance?
    /* TODO if (g_NoesisFeatureFlag && *(&g_NoesisFeatureFlag + 2))// UI_Refactor::NoesisFeatureFlags::GetNoesisFeatureFlag
    {
        pb_clear((unsigned __int64 *)&qword_7FF76B36DF78);
        dword_7FF76B36DF70 &= ~1u;
    }
    if ((dword_7FF76B36DF70 & 1) != 0)
    {
        data->m_f10 = *(std::string *)(qword_7FF76B36DF78));
    }*/
    auto sensor = rq.mutable_sensor();
    auto sensor_f2 = pBike->m_bc->m_sensor_f2 / fmaxf(pBike->m_bc->m_total_smth, 0.0001f);
    sensor->set_f2((int)sensor_f2);
    sensor->set_power_type(pBike->m_ipsc->m_pty);
    auto heart = sensor->mutable_heart();
    heart->set_f2(pBike->m_bc->m_heart_f2 / fmaxf(pBike->m_bc->m_total_smth, 0.0001f));
    heart->set_actual(FitnessDeviceManager::m_pSelectedHRDevice && timeGetTime() - FitnessDeviceManager::m_pSelectedHRDevice->m_last_time_ms < 10000);
    sensor->set_f11(pBike->m_sensor_f11);
    auto crit = rq.mutable_crit();
    float bta_20h = 0.0f, bta_5h = 0.0f, bta_hour = 0.0, bta_15min = 0.0;
    auto cpc = (CriticalPowerCurve *)DataRecorder::Instance()->GetComponent(RecorderComponent::T_CPC); // ZNETWORK_INTERNAL_GetCPValues
    if (cpc) {
        if (resultTimeInSeconds >= 900.0f)
            bta_15min = cpc->GetBestTimeAverage(900.0f, resultTimeInSeconds);
        if (resultTimeInSeconds > 3600.0f)
            bta_hour = cpc->GetBestTimeAverage(3600.0f, resultTimeInSeconds);
        if (resultTimeInSeconds > 18000.0f)
            bta_5h = cpc->GetBestTimeAverage(18000.0f, resultTimeInSeconds);
        if (resultTimeInSeconds > 72000.0f)
            bta_20h = cpc->GetBestTimeAverage(72000.0f, resultTimeInSeconds);
    } else {
        LogTyped(LOG_ERROR, "CritPower ptr null (saveRequest)");
    }
    crit->set_bta_15min((int)bta_15min);
    crit->set_bta_hour((int)bta_hour);
    crit->set_bta_5h((int)bta_5h);
    crit->set_bta_20h((int)bta_20h);
    return zwift_network::create_race_result_entry(rq);
}
void ZNETWORK_INTERNAL_ProcessPlayerPackets() {
    std::shared_ptr<protobuf::ServerToClient> stc;
    auto bikeManager = BikeManager::Instance();
    while (zwift_network::pop_server_to_client(stc)) {
        auto pStc = stc.get();// pStc = stc[0];
        if (pStc->has_zwifters())
            g_ServerReportedPlayerCount = pStc->zwifters();
        if (pStc->has_race_placements()) {
            g_ServerReportedRacePlacements = 1;
            auto &ess = pStc->race_placements();
            g_currentEventTotalRiders = ess.event_total_riders();
            auto pos = ess.position();
            auto bni = ess.bike_network_id();
            auto mainBike = bikeManager->m_mainBike;
            auto pBni = (ess.bike_network_id() && bni != mainBike->m_playerIdTx) ? bikeManager->FindBikeWithNetworkID(bni, true) : mainBike;
            if (pBni) {
                pBni->m_field_11DC = 0;
                pBni->m_eventPos = pos;
                if (ess.has_millisec_to_leader())
                    pBni->m_msToLeader = ess.millisec_to_leader();
            }
            if (ess.player_rd5_size()) {
                for (auto &v15 : ess.player_rd5()) {
                    auto v17 = bikeManager->FindBikeWithNetworkID(v15.player_id(), false);
                    if (v17) {
                        v17->m_field_11DC = 0;
                        v17->m_eventPos = v15.event_pos();
                        if (v15.has_millisec_to_leader())
                            v17->m_msToLeader = v15.millisec_to_leader();
                    }
                }
            } else {
                int v19 = 1;
                for (auto &v23 : ess.player_rd1()) {
                    auto v17 = bikeManager->FindBikeWithNetworkID(v23.player_id(), false);
                    if (v17) {
                        v17->m_field_11DC = 0;
                        v17->m_eventPos = v19;
                        if (v23.has_millisec_to_leader())
                            v17->m_msToLeader = v23.millisec_to_leader();
                    }
                    ++v19;
                }
                int v26 = pos - 1;
                for (auto &v31 : ess.event_rider_positions()) {
                    BikeEntity *bk1 = nullptr;
                    if (!v31.player_id() || v31.player_id() == mainBike->m_playerIdTx) {
                        bk1 = mainBike;
                    } else {
                        for (auto pbe : bikeManager->m_allBikes)
                            if (pbe->m_playerIdTx == v31.player_id()) {
                                bk1 = pbe;
                                break;
                            }
                        for (auto pbe : bikeManager->m_field_9CD8)
                            if (pbe->m_playerIdTx == v31.player_id()) {
                                bk1 = pbe;
                                break;
                            }
                    }
                    if (bk1) {
                        bk1->m_field_11DC = 0;
                        bk1->m_eventPos = v26;
                        if (v31.has_millisec_to_leader())
                            bk1->m_msToLeader = v31.millisec_to_leader();
                    }
                    --v26;
                }
                int v42 = pos + 1;
                for (auto &v47 : ess.player_rd4()) {
                    BikeEntity *bk2 = nullptr;
                    if (!v47.player_id() || v47.player_id() == mainBike->m_playerIdTx) {
                        bk2 = mainBike;
                    } else {
                        for (auto pbe : bikeManager->m_allBikes)
                            if (pbe->m_playerIdTx == v47.player_id()) {
                                bk2 = pbe;
                                break;
                            }
                        for (auto pbe : bikeManager->m_field_9CD8)
                            if (pbe->m_playerIdTx == v47.player_id()) {
                                bk2 = pbe;
                                break;
                            }
                    }
                    if (bk2) {
                        bk2->m_field_11DC = 0;
                        bk2->m_eventPos = v42;
                        if (v47.has_millisec_to_leader())
                            bk2->m_msToLeader = v47.millisec_to_leader();
                    }
                    ++v42;
                }
            }
        }
        g_playerStatesCnt += pStc->states1_size();
        zwiftUpdateContext zuc;
        for (auto &pst : pStc->states1())
            bikeManager->ProcessPlayerState(&zuc, pst);
    }
}
void ZNETWORK_JoinWorld(int64_t serverRealm, bool enableTeleport) {
    zwift_network::set_teleporting_allowed(enableTeleport); //QUEST: not present in PC, optimized?
    LogTyped(LOG_NETWORK, "Joining world %ld", serverRealm);
    auto f = zwift_network::fetch_drop_in_world_list().get();
    if (f.m_errCode) {
        LogTyped(LOG_NETWORK, "Problem getting the list of worlds [%d] %s", f.m_errCode, f.m_msg.c_str());
    } else {
        LogTyped(LOG_NETWORK, "Got the list of worlds");
        g_lastPlayerStateTime = timeGetTime();
        g_CurrentServerRealmID = serverRealm;
        int v6 = 5;
        for (auto &i : g_Leaderboards.m_srwList)
            i.m_time = (timeGetTime() + 2000 * v6++);
        BikeManager::Instance()->m_mainBike->m_joinedWorld = true;
        Leaderboards::FetchJerseyLeadersForAllSegments();
    }
}
void ZNETWORK_SendRouteHashRequest(int64_t toPlayerId) {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    Log("Sending Route hash request (to %d)", toPlayerId);
    if (g_subscriptionMode != SM_INACTIVE) {
        if (++g_routehashCt <= 3) {
            g_RouteHashTargetID = toPlayerId;
            protobuf::WorldAttribute v17;
            v17.set_wa_type(protobuf::WAT_RH);
            v17.set_world_time_expire(g_CachedWorldTime + 10'000);
            v17.set_importance(5'000'000);
            v17.set_rel_id(toPlayerId);
            v17.set_x(mainBike->m_pos.m_data[0]);
            v17.set_y_altitude(mainBike->m_pos.m_data[1]);
            v17.set_z(mainBike->m_pos.m_data[2]);
            static_assert(sizeof(ZNETWORK_RouteHashRequest) == 32);
            ZNETWORK_RouteHashRequest v16{ .m_ver = 2, .m_len = sizeof(ZNETWORK_RouteHashRequest) - 4, .m_kind = 1, .m_otherPlayerId = mainBike->m_playerIdTx, .m_playerIdTx = toPlayerId };
            g_delayed_RouteHash_time = 5.0f;
            v17.set_payload((char *)&v16, sizeof(v16));
            Log("------------- routehash sent");
            g_BroadcastRouteHashReturnCode = zwift_network::save_world_attribute(v17);
        } else {
            g_delayed_RouteHash_time = -1.0f;
            g_routehashCt = 0;
        }
    }
}
protobuf::ActivityList g_LastActivityList;
std::future<NetworkResponse<protobuf::ActivityList>> g_UserActivitiesFuture;
int64_t g_LastActivityListRequestPlayerID;
protobuf::ActivityList *ZNETWORK_GetActivities() {
    protobuf::ActivityList *ret = nullptr;
    if (ZNETWORK_IsLoggedIn()) {
        auto playerIdTx = BikeManager::Instance()->m_mainBike->m_playerIdTx;
        if (playerIdTx == g_LastActivityListRequestPlayerID)
            return &g_LastActivityList;
        if (!g_UserActivitiesFuture.valid())
            g_UserActivitiesFuture = zwift_network::get_activities(playerIdTx, Optional<int64_t>(), Optional<int64_t>(), false);
        if (g_UserActivitiesFuture.valid()) {
            bool ready = (std::future_status::ready == g_UserActivitiesFuture.wait_for(std::chrono::seconds(0)));
            if (ready) {
                auto res = g_UserActivitiesFuture.get();
                if (res.m_errCode) {
                    Log("zwift_network::get_activities() failure: %d!", res.m_errCode);
                } else {
                    g_LastActivityListRequestPlayerID = playerIdTx;
                    g_LastActivityList.CopyFrom(res.m_T);
                    ret = &g_LastActivityList;
                }
            }
        }
    }
    return ret;
}
void ZNETWORK_INTERNAL_ProcessUpcomingWorkouts() {
    //OMIT as server-side workouts:
}
void ZNETWORK_INTERNAL_ProcessPhoneInput() {
    protobuf::PhoneToGameCommand ptg;
    auto mainBike = BikeManager::Instance()->m_mainBike;
    auto exp = Experimentation::Instance();
    while (zwift_network::pop_phone_to_game_command(&ptg)) {
        switch (ptg.command()) {
        case protobuf::CUSTOM_ACTION:
            ZML_HandleCustomButton(ptg.custom_button());
            break;
        case protobuf::CHANGE_CAMERA_ANGLE: {
            auto curCam = g_CameraManager.GetSelectedCamera();
            if (curCam == g_HeadCam)
                g_DesiredCam = g_FollowCam;
            else if (curCam == g_FollowCam)
                g_DesiredCam = g_WheelCam;
            else if (curCam == g_WheelCam)
                g_DesiredCam = g_SideCam;
            else if (curCam == g_SideCam)
                g_DesiredCam = g_LeadCam;
            else if (curCam == g_LeadCam)
                g_DesiredCam = g_DollyCam;
            else
                g_DesiredCam = g_HeadCam;
            if (g_friendsListGUIObj)
                g_friendsListGUIObj->m_changeCamera = true;
        }
            break;
        case protobuf::JOIN_ANOTHER_PLAYER: {
            auto b = BikeManager::Instance()->FindBikeWithNetworkID(ptg.rel_id() /*QUEST why SLODWORD here*/, false);
            if (b && mainBike) {
                mainBike->Respawn(b->m_road ? b->m_road->m_segmentId : 0, b->m_field_888, b->m_field_8B8, false);
                EventSystem::GetInst()->TriggerEvent(EV_FRAME_CH, 0);
                ZNETWORK_SendRouteHashRequest(ptg.rel_id());
            }}
            break;
        case protobuf::TELEPORT_TO_START:
            if (mainBike) {
                assert(g_pRoadManager);
                auto v64 = mainBike->m_road;
                if (!v64)
                    v64 = g_pRoadManager->GetRoadSegment(0);
                mainBike->m_field_888 = g_startLineRoadTime;// BikeEntity::TeleportToStart
                auto pos = v64->CalculateRoadPositionAtTime(/*double*/0.0, false);
                mainBike->SetPos(pos);
                mainBike->AdjustRandomXZ();
                EventSystem::GetInst()->TriggerEvent(EV_FRAME_CH, 0);
                pos = v64->CalculateRoadPositionAtTime(/*double*/0.0, false);
                auto v69 = atan2f(mainBike->m_pos.m_data[2] - pos.m_data[2], mainBike->m_pos.m_data[0] - pos.m_data[0]);
                mainBike->m_heading.SetDir(-v69, g_startLineRoadTime - 0.01f);
                mainBike->m_field_8EC = 0.0f;
                mainBike->m_field_940 &= ~2u;
                mainBike->m_field_8F0 = -1;
                mainBike->m_teleportPos = mainBike->m_pos;
            }
            break;
        case protobuf::ELBOW_FLICK:
            TriggerLocalPlayerAction(BikeEntity::UA_ELBOW);
            break;
        case protobuf::WAVE:
            TriggerLocalPlayerAction(BikeEntity::UA_WAVE);
            break;
        case protobuf::RIDE_ON:
            TriggerLocalPlayerAction(BikeEntity::UA_RIDEON);
            break;
        case protobuf::BELL:
            TriggerLocalPlayerAction(BikeEntity::UA_BELL);
            break;
        case protobuf::HAMMER_TIME:
            TriggerLocalPlayerAction(BikeEntity::UA_HAMMERTIME);
            break;
        case protobuf::TOAST:
            TriggerLocalPlayerAction(BikeEntity::UA_TOAST);
            break;
        case protobuf::NICE:
            TriggerLocalPlayerAction(BikeEntity::UA_NICE);
            break;
        case protobuf::BRING_IT:
            TriggerLocalPlayerAction(BikeEntity::UA_BRINGIT);
            break;
        case protobuf::DISCARD_ACTIVITY:
            if (g_pDialogs[UID_QUIT])
                ((UI_QuitDialog *)g_pDialogs[UID_QUIT])->FinalizeDontSaveAndQuit();
            break;
        case protobuf::SAVE_ACTIVITY: {
            auto &text = ptg.activity_title();
            auto v75 = (UI_QuitDialog *)g_pDialogs[UID_QUIT];
            if (v75) {
                auto v76 = v75->m_editBox;
                if (v76 && text.length()) {
                    v76->SetText(text);
                    v76->SetCaretPosition((int)text.length(), 0);
                }
                UI_IOS_QuitScreen::OnSavePressed(v75);
            }}
            break;
        case protobuf::DONE_RIDING:
            if (!g_pDialogs[UID_QUIT]) {
                UI_QuitDialog::s_bQuitFromZC = true;
                UI_CreateDialog(UID_QUIT, OnQuit, nullptr);
            }
            break;
        case protobuf::CANCEL_DONE_RIDING:
            if (g_pDialogs[UID_QUIT]) {
                GUI_TakeFocus(nullptr);
                auto v70 = (UI_QuitDialog *)g_pDialogs[UID_QUIT];
                if (v70->m_onCancel)
                    v70->m_onCancel(1);
            }
            break;
        case protobuf::REQUEST_FOR_PROFILE: {
            auto v16 = BikeManager::Instance()->FindBikeWithNetworkID(ptg.request_profile_id(), false);
            if (v16) {
                if (v16->m_playerIdTx == v16->m_profile.id())
                    zwift_network::send_player_profile(v16->m_profile);
                else
                    Log("Phone requested profile.  We know about player %lld but don't have the profile yet. Still waiting on the server", ptg.request_profile_id());
            }}
            break;
        case protobuf::TAKE_SCREENSHOT:
            if (timeGetTime() - g_LastScreenshotTime > 500 && !g_pDialogs[UID_QUIT]) {
                auto imgprefs = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\IMGPREFS", 0, true);
                GAME_ScreenshotParams p(SCS_USER_TRIGGERED_COMPANION_APP); //was GAME_ScreenshotParams::Default
                if (imgprefs == 2 || GAME_NumberStoredScreenshots() >= GAME_MaxStoredScreenshots() || g_ActivityID == -1) {
                    auto gsc = new GAME_Screenshot();
                    p.m_useContentKinds = GAME_Screenshot::CK_DIRTY;
                    p.m_gameScreenshot = gsc;
                    p.m_field_4 = false;
                    p.m_func = [](GAME_Screenshot *gsc) {
                        zwift_network::send_image_to_mobile_app(std::string(gsc->m_contents[1].m_pathName), std::string(gsc->m_contents[1].m_imageName));
                        delete gsc;
                    };
                    if (GAME_QueueScreenshot(p) >= 0)
                        Log("[ZNET]: Saving phone triggered screenshot");
                } else {
                    p.m_func = [](GAME_Screenshot *gsc) {
                        zwift_network::send_image_to_mobile_app(std::string(gsc->m_contents[1].m_pathName), std::string(gsc->m_contents[1].m_imageName));
                        delete gsc;
                    };
                    if (GAME_QueueScreenshot(p) >= 0) {
                        Log("[ZNET]: Saving phone triggered screenshot to server");
                        //OMIT analytics
                    }
                }
                g_LastScreenshotTime = timeGetTime();
            }
            break;
        case protobuf::OBSOLETE_GROUP_TEXT_MESSAGE:
            //nop
            break;
        case protobuf::OBSOLETE_SINGLE_PLAYER_TEXT_MESSAGE: {
            BufSafeToUTF8 buf;
            auto cmsg = ptg.msg().c_str();
            LogTyped(LOG_NETWORK, "Phone message sent to id %d (%s)", ptg.player_msg_dst(), cmsg);
            ZNETWORK_SendPrivateText(ptg.player_msg_dst(), mainBike->m_playerIdTx, SafeToUTF8(cmsg, &buf));
        }
            break;
        case protobuf::MOBILE_API_VERSION:
            g_PhoneAPIVersion = ptg.api_ver();
            break;
        case protobuf::ACTIVATE_POWER_UP:
            if (mainBike)
                mainBike->ActivatePowerUp();
            break;
        case protobuf::U_TURN:
            zassert(exp);
            if (exp->IsEnabled(FID_EN_LSUT) || mainBike->m_bc->m_speed > 5.0f)
                RouteComputer::FlipRoute(mainBike);
            break;
        case protobuf::FAN_VIEW: {
            auto v83 = BikeManager::Instance()->FindBikeWithNetworkID(ptg.rel_id(), false);
            if (v83) {
                if ((g_GameMode & 4 /*TODO*/) == 0) {
                    Log("ZML fan view triggered");
                    g_friendsListGUIObj->FanView(v83, false);
                    g_friendsListGUIObj->m_fanView = true;
                }
            } else {
                g_farFanviewFocusRequestID = ptg.rel_id();
                g_farFanviewFocusRequestTimeout = 0;
                if (mainBike)
                    mainBike->m_field_CC1 = true;
            }}
            break;
        case protobuf::SOCIAL_PLAYER_ACTION:
            if (ptg.spa().spa_type() == protobuf::SOCIAL_TEXT_MESSAGE) {
                auto eventId = GroupEvents::GetCurrentBroadcastId_ex();
                protobuf::SocialPlayerAction newSpa(ptg.spa());
                auto importance = 75'000.0f;
                if (ptg.spa().to_player_id())
                    importance = 5'000'000.0f;
                auto leaderId = (g_currentPrivateEvent && g_currentPrivateEvent->m_leaderId) ? g_currentPrivateEvent->m_leaderId : g_GroupEventsActive_CurrentLeader;
                if (leaderId == ptg.spa().player_id() || g_GroupEventsActive_CurrentSweeper == ptg.spa().player_id())
                    importance = 5'000'000.0f;
                ZNETWORK_SendSPA(&newSpa, mainBike->m_pos, importance, eventId);
                if (UI_GroupEventChat::g_GroupChat && UI_GroupEventChat::g_GroupChat->m_field_72) {
                    BikeEntity *v109 = nullptr;
                    if (newSpa.player_id())
                        v109 = BikeManager::Instance()->FindBikeWithNetworkID(newSpa.player_id(), true);
                    UChar dest[0x400], dest1[0x80];
                    ToUTF8(newSpa.message().c_str(), dest, _countof(dest));
                    if (newSpa.player_id()) {
                        if (v109) {
                            u_snprintf(dest1, _countof(dest1), "%S -> %S", mainBike->m_uname, v109->m_uname);
                            UI_GroupEventChat::g_GroupChat->AddChatMessage(dest, dest1, newSpa.player_id(), true);
                        }
                    } else {
                        UI_GroupEventChat::g_GroupChat->AddChatMessage(dest, mainBike->m_uname, newSpa.player_id(), false);
                    }
                }
            } else if (ptg.spa().spa_type() == protobuf::SOCIAL_FLAG) {
                if (ptg.spa().has_flag_type())
                    switch (ptg.spa().flag_type()) {
                    case protobuf::FLAG_TYPE_BAD_LANGUAGE: case protobuf::FLAG_TYPE_HARASSMENT: case protobuf::FLAG_TYPE_FLIER:
                        ZNETWORK_SendPlayerFlag(mainBike->m_playerIdTx, ptg.spa().to_player_id(), ptg.spa().flag_type());
                    }
            } else {
                Log("Unknown SPA received from phone");
            }
            break;
        case protobuf::MOBILE_ALERT_RESPONSE:
            EventSystem::GetInst()->TriggerEvent(EV_ZCA_ALERT_RESP, 2, ptg.alert_resp().f1(), ptg.alert_resp().f2());
            break;
        case protobuf::BLEPERIPHERAL_RESPONSE:
            BLEModule::Instance()->ProcessBLEResponse(ptg.ble_resp(), BLES_ZCA);
            break;
        case protobuf::PHONE_TO_GAME_PACKET: {
            protobuf::GamePacket v195;
            v195.ParseFromString(ptg.game_packet());
            switch (v195.type()) {
            case protobuf::SPORTS_DATA_RESPONSE:
                if (v195.has_client_info())
                    switch (v195.sports_data_response().type()) {
                    case protobuf::AVAILABLE_SAMPLE_TYPES: {
                        auto hasHR = false, hasSPEED = false;
                        for (auto v133 : v195.sports_data_response().sample_types()) {
                            if (v133 == protobuf::HEART_RATE)
                                hasHR = true;
                            else if (v133 == protobuf::WALK_RUN_SPEED)
                                hasSPEED = true;
                        }
                        auto v135 = (ZMLAUXDevice *)FitnessDeviceManager::FindDevice(ZMLAUXDevice::PREFS_ID);
                        if (v135)
                            v135->m_tsLastSPD = v135->m_tsLastHR = timeGetTime();
                        else
                            FitnessDeviceManager::AddDevice(new ZMLAUXDevice(hasHR, hasSPEED), "Apple Watch");
                        }
                        break;
                    case protobuf::SELECTED_SAMPLE_TYPES:
                        /* looks like empty cycle: for (j = 0; j < (sds_f5); ++j)
                            google::protobuf::RepeatedField<int>::operator[](sds_f5, j);*/
                        break;
                    case protobuf::SAMPLE:
                        if (v195.sports_data_response().sample().type() == protobuf::HEART_RATE) {
                            auto v125 = (ZMLAUXDevice *)FitnessDeviceManager::FindDevice(ZMLAUXDevice::PREFS_ID);
                            if (v125) {
                                auto v127 = (SensorValueComponent *)v125->FindComponentOfType(DeviceComponent::CPT_HR);
                                if (v127) {
                                    v127->m_val = (float)v195.sports_data_response().sample().value();
                                    v125->m_tsLastHR = timeGetTime();
                                }
                            }
                        } else if (v195.sports_data_response().sample().type() == protobuf::WALK_RUN_SPEED) {
                            auto v120 = (ZMLAUXDevice *)FitnessDeviceManager::FindDevice(ZMLAUXDevice::PREFS_ID);
                            if (v120) {
                                auto v122 = (SensorValueComponent *)v120->FindComponentOfType(DeviceComponent::CPT_RUN_SPD);
                                if (v122) {
                                    v122->m_val = float(v195.sports_data_response().sample().value() * 3.6);
                                    v120->m_tsLastSPD = timeGetTime();
                                }
                            }
                        }
                        break;
                    }
                break;
            case protobuf::RIDE_ON_BOMB_REQUEST:
                if (v195.rideon_bomb_request().has_f1() && v195.rideon_bomb_request().f1()) {
                    ZML_DetonateRideOnBomb();
                    //OMIT ++g_ZNETWORK_Stats[4];
                }
                break;
            case protobuf::WORKOUT_ACTION_REQUEST:
                if (v195.workout_action_rq().has_type())
                    ZML_ReceivedWorkoutAction(v195.workout_action_rq().type());
                break;
            case protobuf::CLIENT_ACTION:
                if (v195.client_action().has_type())
                    ZML_ReceivedClientAction(v195.client_action());
                break;
            }}
            break;
        case protobuf::BLEPERIPHERAL_DISCOVERY:
            BLEModule::Instance()->ProcessDiscovery(ptg.ble_adv(), BLES_ZCA);
            break;
        default:
            Log("Received a phone command of type: %d", ptg.command());
            break;
        }
    }
}
namespace ZNet {
void API::Update(uint32_t dt) {
    std::vector<std::pair<ZNet::RequestId, std::unique_ptr<ZNet::RPCBase>>> copy;
    {
        std::lock_guard l(m_mutex);
        if (m_map.size())
            copy.reserve(m_map.size());
        for (auto &i : m_map)
            if (i.second->ShouldRemove(dt))
                copy.emplace_back(std::move(i));
    }
    for (auto &i : copy) {
        /*auto v39 =*/ i.second->Post(false);
        m_map.erase(i.first);
    }
}
RequestId NetworkService::GetAllTimeBestEffortsPowerCurve(std::function<void(const protobuf::PowerCurveAggregationMsg &)> &&f, Params *pParams) {
    pParams->m_funcName = "GetBestEffortsPowerCurveAllTime";
    pParams->m_retry = RetryParams { .m_count = 3, .m_timeout = 2000 };
    pParams->m_has_retry = true;
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::PowerCurveAggregationMsg>>()>([]() { return zwift_network::get_best_efforts_power_curve_from_all_time(); }),
        std::move(f), pParams);
}
RequestId UpdateProfile(bool inGameFields, const protobuf::PlayerProfile &prof, bool udp, std::function<void(void)> &&f, Params *pParams) {
    zassert(ZNETWORK_IsLoggedIn() && "UpdateProfile called when not logged in!");
    if (ZNETWORK_IsLoggedIn()) {
        pParams->m_funcName = "UpdateProile";
        return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<void>>()>([inGameFields, prof, udp]() {
            return zwift_network::update_profile(inGameFields, prof, udp);
        }), std::move(f), pParams);
    }
    return 0;
}
RequestId NetworkService::UpdateProfile(bool inGameFields, const protobuf::PlayerProfile &prof, bool udp, std::function<void(void)> &&func, Params *pParams) {
    return ZNet::UpdateProfile(inGameFields, prof, udp, std::move(func), pParams);
}
RequestId GetProfile(int64_t playerId, std::function<void(const protobuf::PlayerProfile &)> &&f, Params *pParams) {
    pParams->m_funcName = "GetProfile";
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::PlayerProfile>>()>([playerId]() {
        return zwift_network::profile(playerId, false);
    }), std::move(f), pParams);
}
RequestId NetworkService::GetProfile(int64_t playerId, std::function<void(const protobuf::PlayerProfile &)> &&func, Params *pParams) {
    return ZNet::GetProfile(playerId, std::move(func), pParams);
}
RequestId GetProfiles(const std::unordered_set<int64_t> &playerIDs, std::function<void(const protobuf::PlayerProfiles &)> &&f, Params *pParams) {
    zassert(playerIDs.size() < MAX_IDS);
    pParams->m_funcName = "GetProfiles";
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::PlayerProfiles>>(void)>([playerIDs]() {
        return zwift_network::profiles(playerIDs);
    }), std::move(f), pParams);
}
bool WaitForPendingRequests(const std::vector<RequestId> &vec, std::string_view sv) {
    bool ret = true;
    if (!sv.length())
        sv = "Unknown";
    auto inst = ZNet::API::Inst();
    for (auto v6 : vec) {
        //ZNet::API::GetInst()->WaitFor(v6); inlined
        inst->m_mutex.lock();
        auto f = inst->m_map.find(v6);
        if (f != inst->m_map.end()) {
            auto rid = f->first;
            if (rid) {
                auto v15 = f->second->Post(true);
                inst->m_map.erase(f);
                inst->m_mutex.unlock();
                if (!v15.second || v15.first) {
                    ret = false;
                    int outcome = -1;
                    if (v15.second)
                        outcome = v15.first;
                    LogTyped(LOG_ERROR, "[ZNet] RPC wait failure for [%s]! Outcome: %d", sv.data(), outcome);
                }
            } else {
                inst->m_mutex.unlock();
                zassert(!"[ZNet] RequestId is empty!");
            }
        } else {
            inst->m_mutex.unlock();
            ::Sleep(0);
        }
    }
    return ret;
}
RequestId DeleteActivity(int64_t playerID, uint64_t actId, std::function<void(void)> &&f, std::function<void(Error)> &&ef) {
    Params params{.m_funcName = "DeleteActivity", .m_onError = std::move(ef)};
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<void>>(void)>([playerID, actId]() {
        return zwift_network::delete_activity(playerID, actId);
    }), std::move(f), &params);
}
RequestId DownloadPlayback(const protobuf::PlaybackMetadata &proto, std::function<void(const protobuf::PlaybackData &)> &&f, std::function<void(Error)> &&ef) {
    Params params{ .m_funcName = "DownloadPlayback", .m_onError = std::move(ef), .m_retry{.m_count = 5, .m_timeout = 1000}, .m_has_retry = true };
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::PlaybackData>>(void)>([proto]() {
        return zwift_network::get_playback_data(proto);
    }), std::move(f), &params);
}
RequestId EnrollInCampaign(std::string &proto, std::function<void(const protobuf::CampaignRegistrationResponse &)> &&f, std::function<void(Error)> &&ef, Params *pParams) {
    //OMIT
    return 0;
}
RequestId FetchSegmentJerseyLeaders(std::function<void(const protobuf::SegmentResults &)> &&f, std::function<void(Error)> &&ef) {
    Params params{ .m_funcName = "FetchJerseyLeaders", .m_onError = std::move(ef) };
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::SegmentResults>>(void)>([]() {
        return zwift_network::get_segment_jersey_leaders();
    }), std::move(f), &params);
}
RequestId GetAchievements(std::function<void(const protobuf::Achievements &)> &&f, std::function<void(Error)> &&ef) {
    Params params{ .m_funcName = "GetAchievements", .m_onError = std::move(ef) };
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::Achievements>>(void)>([]() {
        return zwift_network::get_achievements();
    }), std::move(f), &params);
}
RequestId GetActiveCampaigns(std::function<void(const protobuf::ListPublicActiveCampaignResponse &)> &&f, std::function<void(Error)> &&ef) {
    Params params{ .m_funcName = "GetActiveCampaigns", .m_onError = std::move(ef) };
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::ListPublicActiveCampaignResponse>>(void)>([]() {
        return zwift_network::get_active_campaigns();
    }), std::move(f), &params);
}
RequestId GetActivities(int64_t profileId, std::function<void(const protobuf::ActivityList &)> &&f, Params *pParams) {
    pParams->m_funcName = "GetActivities";
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::ActivityList>>(void)>([profileId]() {
        Optional<int64_t> startsAfter, startsBefore;
        return zwift_network::get_activities(profileId, startsAfter, startsBefore, false);
    }), std::move(f), pParams);
}
RequestId GetCampaignRegistration(std::string &proto, std::function<void(const protobuf::CampaignRegistrationResponse &)> &&f, std::function<void(Error)> &&ef, Params *pParams) {
    //OMIT
    return 0;
}
RequestId GetCampaigns(std::function<void(const protobuf::ListCampaignRegistrationSummaryResponse &)> &&f, Params *pParams) {
    //OMIT
    return 0;
}
RequestId GetClubList(std::function<void(const protobuf::Clubs &)> &&f, Params *pParams) {
    pParams->m_funcName = "GetClubList";
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::Clubs>>(void)>([]() {
        Optional<protobuf::Membership_Status> status;
        Optional<int> start, limit;
        return zwift_network::list_my_clubs(status, start, limit);
    }), std::move(f), pParams);
}
RequestId GetDropInWorldList(std::function<void(const protobuf::DropInWorldList &)> &&f, Params *pParams) {
    pParams->m_funcName = "GetDropInWorldList";
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::DropInWorldList>>(void)>([]() {
        return zwift_network::fetch_drop_in_world_list();
    }), std::move(f), pParams);
}
RequestId GetFeatureVariant(const protobuf::FeatureRequest &&proto, std::function<void(const protobuf::FeatureResponse)> &&f, std::function<void(Error)> &&ef) {
    Params params{ .m_funcName = "GetFeatureVariant", .m_onError = std::move(ef) };
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::FeatureResponse>>(void)>([proto]() {
        return zwift_network::get_feature_response(proto);
    }), std::move(f), &params);
}
RequestId GetFollowees(int64_t profileId, bool follow, std::function<void(const protobuf::PlayerSocialNetwork &)> &&f, Params *pParams) {
    pParams->m_funcName = "GetFollowees";
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::PlayerSocialNetwork>>(void)>([profileId, follow]() {
        return zwift_network::get_followees(profileId, follow);
    }), std::move(f), pParams);
}
RequestId GetGroupEvent(int64_t id, std::function<void(const protobuf::EventProtobuf &)> &&f, std::function<void(Error)> &&ef) {
    Params params{ .m_funcName = "GetGroupEvent", .m_onError = std::move(ef) };
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::EventProtobuf>>(void)>([id]() {
        return zwift_network::get_event(id);
    }), std::move(f), &params);
}
RequestId GetMyPlaybackLatest(int64_t a2, uint64_t after, uint64_t before, std::function<void(const protobuf::PlaybackMetadata &)> &&f, std::function<void(Error)> &&ef) {
    Params params{ .m_funcName = "GetMyPlaybackLatest", .m_onError = std::move(ef), .m_retry{.m_count = 5, .m_timeout = 1000}, .m_has_retry = true };
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::PlaybackMetadata>>(void)>([a2, after, before]() {
        return zwift_network::get_my_playback_latest(a2, after, before);
    }), std::move(f), &params);
}
RequestId GetMyPlaybackPr(int64_t a2, uint64_t after, uint64_t before, std::function<void(const protobuf::PlaybackMetadata &)> &&f, std::function<void(Error)> &&ef) {
    Params params{ .m_funcName = "GetMyPlaybackPr", .m_onError = std::move(ef), .m_retry{.m_count = 5, .m_timeout = 1000}, .m_has_retry = true };
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::PlaybackMetadata>>(void)>([a2, after, before]() {
        return zwift_network::get_my_playback_pr(a2, after, before);
    }), std::move(f), &params);
}
RequestId GetMyPlaybacks(int64_t a2, std::function<void(const protobuf::PlaybackMetadataList &)> &&f, std::function<void(Error)> &&ef) {
    Params params{ .m_funcName = "GetMyPlaybackPr", .m_onError = std::move(ef) };
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::PlaybackMetadataList>>(void)>([a2]() {
        return zwift_network::get_my_playbacks(a2);
    }), std::move(f), &params);
}
RequestId GetPlayerState(int64_t serverRealm, int64_t playerId, std::function<void(const protobuf::PlayerState &)> &&f, Params *pParams) {
    pParams->m_funcName = "GetPlayerState";
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::PlayerState>>(void)>([serverRealm, playerId]() {
        return zwift_network::latest_player_state(serverRealm, playerId);
    }), std::move(f), pParams);
}
RequestId GetProfile(std::function<void(const protobuf::PlayerProfile &)> &&f, Params *pParams) {
    //OMIT
    return 0;
}
RequestId GetProfile(std::string &proto, std::function<void(const protobuf::PlayerProfile &)> &&f, Params *pParams) {
    //OMIT
    return 0;
}
RequestId GetProfile(std::string_view, std::function<void(const protobuf::PlayerProfile &)> &&f, Params *pParams) {
    //OMIT
    return 0;
}
RequestId GetProgressInCampaign(std::string &proto, std::function<void(const protobuf::CampaignRegistrationDetailResponse &)> &&f, std::function<void(Error)> &&ef, Params *pParams) {
    //OMIT
    return 0;
}
RequestId RegisterInCampaign(std::string &proto, std::function<void(const protobuf::CampaignRegistrationResponse &)> &&f, std::function<void(Error)> &&ef, Params *pParams) {
    //OMIT
    return 0;
}
RequestId SaveActivity(const protobuf::Activity &proto, bool bFinalSave, const std::string &fitPath, std::function<void(int64_t)> &&f, std::function<void(Error)> &&ef) {
    zassert(!ZU_IsInPreReleaseRestrictedMode(proto.course_id()) || proto.privacy() == protobuf::ActivityPrivacyType::PRIVATE);
    Params params{ .m_funcName = "SaveActivity", .m_onError = std::move(ef) };
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<int64_t>>(void)>([proto, bFinalSave, fitPath]() {
        Log("ZNet::SaveActivity calling zwift_network::save_activity with {name: %s, bFinalSave: %s, fitFileNameToUpload: %s, fitFileNameShort: %s}",
            proto.name().c_str(), bFinalSave ? "True" : "False", fitPath.c_str(), proto.fit_filename().c_str());
        return zwift_network::save_activity(proto, bFinalSave, fitPath);
    }), std::move(f), &params);
}
RequestId SavePlayback(const protobuf::PlaybackData &proto, std::function<void(const std::string &)> &&f, std::function<void(Error)> &&ef) {
    Params params{ .m_funcName = "SavePlayback", .m_onError = std::move(ef) };
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<std::string>>(void)>([proto]() {
        return zwift_network::save_playback(proto);
    }), std::move(f), &params);
}
RequestId SaveSegmentResult(const protobuf::SegmentResult &proto, std::function<void(const int64_t &)> &&f, std::function<void(Error)> &&ef) {
    Params params{ .m_funcName = "SaveSegmentResult", .m_onError = std::move(ef) };
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<int64_t>>(void)>([proto]() {
        return zwift_network::save_segment_result(proto);
    }), std::move(f), &params);
}
RequestId SubscribeToRouteSegment(int64_t id, std::function<void(const protobuf::SegmentResults &)> &&f, std::function<void(Error)> &&ef) {
    Params params{ .m_funcName = "SubscribeToRouteSegment", .m_onError = std::move(ef) };
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<protobuf::SegmentResults>>(void)>([id]() {
        return zwift_network::subscribe_to_segment_and_get_leaderboard(id);
    }), std::move(f), &params);
}
RequestId UnlockAchievements(const std::vector<int> &ach, std::function<void(void)> &&f, std::function<void(Error)> &&ef) {
    if (ach.empty()) {
        Log("ZNet::UnlockAchievements aborting - achievementIds is empty");
        return 0;
    }
    Params params{ .m_funcName = "UnlockAchievements", .m_onError = std::move(ef) };
    protobuf::AchievementUnlockRequest rq;
    for (auto i : ach)
        rq.add_unlocks()->set_id(i);
    return ZNet::API::Inst()->Enqueue(std::function<std::future<NetworkResponse<void>>(void)>([rq = std::move(rq)]() {
        return zwift_network::unlock_achievements(rq);
    }), std::move(f), &params);
}
RequestId WithdrawFromCampaign(std::string &proto, std::function<void(const protobuf::CampaignRegistrationResponse &)> &&f, std::function<void(Error)> &&ef, Params *pParams) {
    //OMIT
    return 0;
}
};

//Units
TEST(SmokeTestNet, JsonWebToken) {
    Oauth2Credentials o;
    //zoffline first
    std::string at("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjkiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiQmVhcmVyIiwiYXpwIjoiR2FtZV9MYXVuY2hlciIsImF1dGhfdGltZSI6MTUzNTUwNzI0OSwic2Vzc2lvbl9zdGF0ZSI6IjA4NDZubzluLTc2NXEtNHAzcy1uMjBwLTZwbnA5cjg2cjVzMyIsImFjciI6IjAiLCJhbGxvd2VkLW9yaWdpbnMiOlsiaHR0cHM6Ly9sYXVuY2hlci56d2lmdC5jb20qIiwiaHR0cDovL3p3aWZ0Il0sInJlYWxtX2FjY2VzcyI6eyJyb2xlcyI6WyJldmVyeWJvZHkiLCJ0cmlhbC1zdWJzY3JpYmVyIiwiZXZlcnlvbmUiLCJiZXRhLXRlc3RlciJdfSwicmVzb3VyY2VfYWNjZXNzIjp7Im15LXp3aWZ0Ijp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiR2FtZV9MYXVuY2hlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFJFU1QgQVBJIC0tIHByb2R1Y3Rpb24iOnsicm9sZXMiOlsiYXV0aG9yaXplZC1wbGF5ZXIiLCJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFplbmRlc2siOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSZWxheSBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIl19LCJlY29tLXNlcnZlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sImFjY291bnQiOnsicm9sZXMiOlsibWFuYWdlLWFjY291bnQiLCJtYW5hZ2UtYWNjb3VudC1saW5rcyIsInZpZXctcHJvZmlsZSJdfX0sIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20iLCJzZXNzaW9uX2Nvb2tpZSI6IjZ8YTJjNWM1MWY5ZDA4YzY4NWUyMDRlNzkyOWU0ZmMyMDAyOWI5ODE1OGYwYjdmNzk0MmZiMmYyMzkwYWMzNjExMDMzN2E3YTQyYjVlNTcwNmVhODM0YjQzYzFlNDU1NzJkMTQ2MzIwMTQxOWU5NzZjNTkzZWZjZjE0M2UwNWNiZjgifQ._kPfXO8MdM7j0meG4MVzprSa-3pdQqKyzYMHm4d494w"),
    rt("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjgiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiUmVmcmVzaCIsImF6cCI6IkdhbWVfTGF1bmNoZXIiLCJhdXRoX3RpbWUiOjAsInNlc3Npb25fc3RhdGUiOiIwODQ2bm85bi03NjVxLTRwM3MtbjIwcC02cG5wOXI4NnI1czMiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZXZlcnlib2R5IiwidHJpYWwtc3Vic2NyaWJlciIsImV2ZXJ5b25lIiwiYmV0YS10ZXN0ZXIiXX0sInJlc291cmNlX2FjY2VzcyI6eyJteS16d2lmdCI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIkdhbWVfTGF1bmNoZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIiwiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBaZW5kZXNrIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiWndpZnQgUmVsYXkgUkVTVCBBUEkgLS0gcHJvZHVjdGlvbiI6eyJyb2xlcyI6WyJhdXRob3JpemVkLXBsYXllciJdfSwiZWNvbS1zZXJ2ZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJhY2NvdW50Ijp7InJvbGVzIjpbIm1hbmFnZS1hY2NvdW50IiwibWFuYWdlLWFjY291bnQtbGlua3MiLCJ2aWV3LXByb2ZpbGUiXX19LCJzZXNzaW9uX2Nvb2tpZSI6IjZ8YTJjNWM1MWY5ZDA4YzY4NWUyMDRlNzkyOWU0ZmMyMDAyOWI5ODE1OGYwYjdmNzk0MmZiMmYyMzkwYWMzNjExMDMzN2E3YTQyYjVlNTcwNmVhODM0YjQzYzFlNDU1NzJkMTQ2MzIwMTQxOWU5NzZjNTkzZWZjZjE0M2UwNWNiZjgifQ.5e1X1imPlVfXfhDHE_OGmG9CNGvz7hpPYPXcNkPJ5lw"),
        json("{\"access_token\":\"" + at + "\",\"expires_in\":1000021600,\"id_token\":\"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjciLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiSUQiLCJhenAiOiJHYW1lX0xhdW5jaGVyIiwiYXV0aF90aW1lIjoxNTM1NTA3MjQ5LCJzZXNzaW9uX3N0YXRlIjoiMDg0Nm5vOW4tNzY1cS00cDNzLW4yMHAtNnBucDlyODZyNXMzIiwiYWNyIjoiMCIsIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20ifQ.rWGSvv5TFO-i6LKczHNUUcB87Hfd5ow9IMG9O5EGR4Y\",\"not-before-policy\":1408478984,\"refresh_expires_in\":611975560,\"refresh_token\":\"" + rt + "\",\"scope\":\"\",\"session_state\":\"0846ab9a-765d-4c3f-a20c-6cac9e86e5f3\",\"token_type\":\"bearer\"}");
    EXPECT_TRUE(o.parse(json));
    EXPECT_TRUE(o.m_msg.empty());
    EXPECT_EQ(0, o.m_errCode);
    EXPECT_EQ(json, o.m_json);
    EXPECT_EQ(0x000000003b9b1e60, o.m_exp);

    auto &a = o.getAccessToken();
    EXPECT_TRUE(a.m_msg.empty());
    EXPECT_EQ(0, a.m_errCode);
    EXPECT_STREQ("02r3deb5-nq9q-476s-9ss0-034q977sp2r1", a.getSubject().c_str());
    EXPECT_STREQ("0846no9n-765q-4p3s-n20p-6pnp9r86r5s3", a.getSessionState().c_str());
    EXPECT_EQ(at, a.asString());
    EXPECT_EQ(0x004c4b3fff676980, a.m_exp);

    auto &r = o.getRefreshToken();
    EXPECT_TRUE(r.m_msg.empty());
    EXPECT_EQ(0, r.m_errCode);
    EXPECT_STREQ("02r3deb5-nq9q-476s-9ss0-034q977sp2r1", r.getSubject().c_str());
    EXPECT_STREQ("0846no9n-765q-4p3s-n20p-6pnp9r86r5s3", r.getSessionState().c_str());
    EXPECT_EQ(rt, r.asString());
    EXPECT_EQ(0x004c4b3fff676980, r.m_exp);
    
    //retail (zca)
    std::string retail_at("eyJhbGciOiJSUzI1NiIsInR5cCIgOiAiSldUIiwia2lkIiA6ICJPLUVjXzJJNjg5bW9peGJIZzFfNDZDVFlGeEdZMDViaDluYm5Mcjl0RzY4In0.eyJleHAiOjE2MzY5MTAwODksImlhdCI6MTYzNjg4ODQ4OSwianRpIjoiMmI4YzI0OWEtMGU3MS00YWUyLThlY2ItNzgyYjFiYjZkNjlmIiwiaXNzIjoiaHR0cHM6Ly9zZWN1cmUuendpZnQuY29tL2F1dGgvcmVhbG1zL3p3aWZ0IiwiYXVkIjpbImVtYWlsLXByZWZzLXNlcnZpY2UiLCJteS16d2lmdCIsInNzby1nYXRld2F5Iiwic3Vic2NyaXB0aW9uLXNlcnZpY2UiLCJHYW1lX0xhdW5jaGVyIiwiWndpZnQgWmVuZGVzayIsIlp3aWZ0IFJFU1QgQVBJIC0tIHByb2R1Y3Rpb24iLCJad2lmdCBSZWxheSBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIiwiZWNvbS1zZXJ2ZXIiLCJhY2NvdW50Il0sInN1YiI6IjFhNzM2ZWNjLTFjYTYtNGFmZi1hMTc2LWU1NzgzMTk3YTE1NyIsInR5cCI6IkJlYXJlciIsImF6cCI6Ilp3aWZ0X01vYmlsZV9MaW5rIiwic2Vzc2lvbl9zdGF0ZSI6ImVjNzJmYWIyLWQ2NDItNDU5Ny04YmZmLTUwOTM5MjRjNTEyMCIsImFjciI6IjEiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZXZlcnlib2R5IiwidHJpYWwtc3Vic2NyaWJlciIsImV2ZXJ5b25lIiwiYmV0YS10ZXN0ZXIiXX0sInJlc291cmNlX2FjY2VzcyI6eyJlbWFpbC1wcmVmcy1zZXJ2aWNlIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwibXktendpZnQiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJzc28tZ2F0ZXdheSI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sInN1YnNjcmlwdGlvbi1zZXJ2aWNlIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiR2FtZV9MYXVuY2hlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFplbmRlc2siOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIiwiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSZWxheSBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIl19LCJlY29tLXNlcnZlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sImFjY291bnQiOnsicm9sZXMiOlsibWFuYWdlLWFjY291bnQiLCJtYW5hZ2UtYWNjb3VudC1saW5rcyIsInZpZXctcHJvZmlsZSJdfX0sInNjb3BlIjoiIiwibmFtZSI6IllvdXJ5IFBlcnNoaW4iLCJwcmVmZXJyZWRfdXNlcm5hbWUiOiJzdWxpbW92YTA4QG1haWwucnUiLCJnaXZlbl9uYW1lIjoiWW91cnkiLCJmYW1pbHlfbmFtZSI6IlBlcnNoaW4iLCJlbWFpbCI6InN1bGltb3ZhMDhAbWFpbC5ydSJ9.VfuMKYGZzRCBMk7JOCsEIhVsUTHEBfIY7za8no_YtgNXbjxmnwcxMXRRUz_rCzKQDYvo4aTqThhuVMz9DpAMv4csrmWuST8KS4NlkwMBj-IqrGIr5ZI5mkKfFRDXrD44e5wk-3-6Z2F2oWxd3JoyzyuvIcu6CYEYYl4xtWj3TlN_GhlYyWPLJrcCBOHVtEX5diYyqWbHrpfeQ9dat3N3of0v_PXG4cjAMYV6DR-K9nIpKWGWE3siUIkt7pTY-cyJldRYCWzHjo6bwrZgwN5gB6wO-q3A0_gXgpr2oOriuToP-CAqtM60AdwGkckE6h4r-nFVcHV9j0Mo-I0-mgZbxw");
    JsonWebToken rat;
    EXPECT_TRUE(rat.parse(retail_at));
    EXPECT_TRUE(rat.m_msg.empty());
    EXPECT_EQ(0, rat.m_errCode);
    EXPECT_STREQ("1a736ecc-1ca6-4aff-a176-e5783197a157", rat.getSubject().c_str());
    EXPECT_STREQ("ec72fab2-d642-4597-8bff-5093924c5120", rat.getSessionState().c_str());
    EXPECT_EQ(retail_at, rat.asString());
    EXPECT_EQ(0x003a279c4bd74a80, rat.m_exp);

    std::string retail_rt("eyJhbGciOiJIUzI1NiIsInR5cCIgOiAiSldUIiwia2lkIiA6ICIxYTY0ZDZkNC1iODVhLTQxZjQtOTFiMy01ZmJhNGQ4Y2FhMzMifQ.eyJleHAiOjE2MzgwOTgwODksImlhdCI6MTYzNjg4ODQ4OSwianRpIjoiZjJhMzhiNDgtMjlhMC00ZDZkLTkxYWQtMTg3ZTliMmQ4ZmViIiwiaXNzIjoiaHR0cHM6Ly9zZWN1cmUuendpZnQuY29tL2F1dGgvcmVhbG1zL3p3aWZ0IiwiYXVkIjoiaHR0cHM6Ly9zZWN1cmUuendpZnQuY29tL2F1dGgvcmVhbG1zL3p3aWZ0Iiwic3ViIjoiMWE3MzZlY2MtMWNhNi00YWZmLWExNzYtZTU3ODMxOTdhMTU3IiwidHlwIjoiUmVmcmVzaCIsImF6cCI6Ilp3aWZ0X01vYmlsZV9MaW5rIiwic2Vzc2lvbl9zdGF0ZSI6ImVjNzJmYWIyLWQ2NDItNDU5Ny04YmZmLTUwOTM5MjRjNTEyMCIsInNjb3BlIjoiIn0.vDBIPqDaOaJKcXK0MPJthsK_0nyHA3iKikE9oroPS3A");
    JsonWebToken rrt;
    EXPECT_TRUE(rrt.parse(retail_rt));
    EXPECT_TRUE(rrt.m_msg.empty());
    EXPECT_EQ(0, rrt.m_errCode);
    EXPECT_STREQ("1a736ecc-1ca6-4aff-a176-e5783197a157", rrt.getSubject().c_str());
    EXPECT_STREQ("ec72fab2-d642-4597-8bff-5093924c5120", rrt.getSessionState().c_str());
    EXPECT_EQ(retail_rt, rrt.asString());
    EXPECT_EQ(0x003a326a53055a80, rrt.m_exp);
}
TEST(SmokeTestNet, EventLoopTest) {
    google::protobuf::internal::VerifyVersion(3021000, 3020000, __FILE__);
    EventLoop el;
    //Sleep(100);
    //el.shutdown();
    //el.enqueueShutdown();
}
#ifdef OLDNAPALM_SERVER
auto g_rt = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjgiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiUmVmcmVzaCIsImF6cCI6IkdhbWVfTGF1bmNoZXIiLCJhdXRoX3RpbWUiOjAsInNlc3Npb25fc3RhdGUiOiIwODQ2bm85bi03NjVxLTRwM3MtbjIwcC02cG5wOXI4NnI1czMiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZXZlcnlib2R5IiwidHJpYWwtc3Vic2NyaWJlciIsImV2ZXJ5b25lIiwiYmV0YS10ZXN0ZXIiXX0sInJlc291cmNlX2FjY2VzcyI6eyJteS16d2lmdCI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIkdhbWVfTGF1bmNoZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIiwiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBaZW5kZXNrIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiWndpZnQgUmVsYXkgUkVTVCBBUEkgLS0gcHJvZHVjdGlvbiI6eyJyb2xlcyI6WyJhdXRob3JpemVkLXBsYXllciJdfSwiZWNvbS1zZXJ2ZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJhY2NvdW50Ijp7InJvbGVzIjpbIm1hbmFnZS1hY2NvdW50IiwibWFuYWdlLWFjY291bnQtbGlua3MiLCJ2aWV3LXByb2ZpbGUiXX19LCJzZXNzaW9uX2Nvb2tpZSI6IjV8ODU0Mjc1MjBmOTQ5MjQxMThkZjc1YjhhNDc5ZmIzN2ZlY2I3MjEwODU1YzFkYTgxOWE1Yzk4Y2U1Yjk1OTVjZTVmNzgyOGYwNzMwMzQ4YmM4MmRhY2U0ZjM3NWU4OGUxNTUxZjg1ZWEzY2FkMjM3M2RmMDRhYzNlMGJjOTg4YzYifQ.ZB87s5lo7jde09q9IKsFCWc22U65kroBYz0aVb46Lmk"s;
auto g_token = "{\"access_token\":\"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjkiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiQmVhcmVyIiwiYXpwIjoiR2FtZV9MYXVuY2hlciIsImF1dGhfdGltZSI6MTUzNTUwNzI0OSwic2Vzc2lvbl9zdGF0ZSI6IjA4NDZubzluLTc2NXEtNHAzcy1uMjBwLTZwbnA5cjg2cjVzMyIsImFjciI6IjAiLCJhbGxvd2VkLW9yaWdpbnMiOlsiaHR0cHM6Ly9sYXVuY2hlci56d2lmdC5jb20qIiwiaHR0cDovL3p3aWZ0Il0sInJlYWxtX2FjY2VzcyI6eyJyb2xlcyI6WyJldmVyeWJvZHkiLCJ0cmlhbC1zdWJzY3JpYmVyIiwiZXZlcnlvbmUiLCJiZXRhLXRlc3RlciJdfSwicmVzb3VyY2VfYWNjZXNzIjp7Im15LXp3aWZ0Ijp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiR2FtZV9MYXVuY2hlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFJFU1QgQVBJIC0tIHByb2R1Y3Rpb24iOnsicm9sZXMiOlsiYXV0aG9yaXplZC1wbGF5ZXIiLCJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFplbmRlc2siOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSZWxheSBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIl19LCJlY29tLXNlcnZlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sImFjY291bnQiOnsicm9sZXMiOlsibWFuYWdlLWFjY291bnQiLCJtYW5hZ2UtYWNjb3VudC1saW5rcyIsInZpZXctcHJvZmlsZSJdfX0sIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20iLCJzZXNzaW9uX2Nvb2tpZSI6IjV8ODU0Mjc1MjBmOTQ5MjQxMThkZjc1YjhhNDc5ZmIzN2ZlY2I3MjEwODU1YzFkYTgxOWE1Yzk4Y2U1Yjk1OTVjZTVmNzgyOGYwNzMwMzQ4YmM4MmRhY2U0ZjM3NWU4OGUxNTUxZjg1ZWEzY2FkMjM3M2RmMDRhYzNlMGJjOTg4YzYifQ.Z3lqUfV7UXSwrB7UoLTdoyupnzoBNP-uaGG1Yh45cKU\",\"expires_in\":1000021600,\"id_token\":\"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjciLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiSUQiLCJhenAiOiJHYW1lX0xhdW5jaGVyIiwiYXV0aF90aW1lIjoxNTM1NTA3MjQ5LCJzZXNzaW9uX3N0YXRlIjoiMDg0Nm5vOW4tNzY1cS00cDNzLW4yMHAtNnBucDlyODZyNXMzIiwiYWNyIjoiMCIsIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20ifQ.rWGSvv5TFO-i6LKczHNUUcB87Hfd5ow9IMG9O5EGR4Y\",\"not-before-policy\":1408478984,\"refresh_expires_in\":611975560,\"refresh_token\":\"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjgiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiUmVmcmVzaCIsImF6cCI6IkdhbWVfTGF1bmNoZXIiLCJhdXRoX3RpbWUiOjAsInNlc3Npb25fc3RhdGUiOiIwODQ2bm85bi03NjVxLTRwM3MtbjIwcC02cG5wOXI4NnI1czMiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZXZlcnlib2R5IiwidHJpYWwtc3Vic2NyaWJlciIsImV2ZXJ5b25lIiwiYmV0YS10ZXN0ZXIiXX0sInJlc291cmNlX2FjY2VzcyI6eyJteS16d2lmdCI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIkdhbWVfTGF1bmNoZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIiwiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBaZW5kZXNrIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiWndpZnQgUmVsYXkgUkVTVCBBUEkgLS0gcHJvZHVjdGlvbiI6eyJyb2xlcyI6WyJhdXRob3JpemVkLXBsYXllciJdfSwiZWNvbS1zZXJ2ZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJhY2NvdW50Ijp7InJvbGVzIjpbIm1hbmFnZS1hY2NvdW50IiwibWFuYWdlLWFjY291bnQtbGlua3MiLCJ2aWV3LXByb2ZpbGUiXX19LCJzZXNzaW9uX2Nvb2tpZSI6IjV8ODU0Mjc1MjBmOTQ5MjQxMThkZjc1YjhhNDc5ZmIzN2ZlY2I3MjEwODU1YzFkYTgxOWE1Yzk4Y2U1Yjk1OTVjZTVmNzgyOGYwNzMwMzQ4YmM4MmRhY2U0ZjM3NWU4OGUxNTUxZjg1ZWEzY2FkMjM3M2RmMDRhYzNlMGJjOTg4YzYifQ.ZB87s5lo7jde09q9IKsFCWc22U65kroBYz0aVb46Lmk\",\"scope\":\"\",\"session_state\":\"0846ab9a-765d-4c3f-a20c-6cac9e86e5f3\",\"token_type\":\"bearer\"}"s;
#else
auto g_rt = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjgiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiUmVmcmVzaCIsImF6cCI6IkdhbWVfTGF1bmNoZXIiLCJhdXRoX3RpbWUiOjAsInNlc3Npb25fc3RhdGUiOiIwODQ2bm85bi03NjVxLTRwM3MtbjIwcC02cG5wOXI4NnI1czMiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZXZlcnlib2R5IiwidHJpYWwtc3Vic2NyaWJlciIsImV2ZXJ5b25lIiwiYmV0YS10ZXN0ZXIiXX0sInJlc291cmNlX2FjY2VzcyI6eyJteS16d2lmdCI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIkdhbWVfTGF1bmNoZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIiwiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBaZW5kZXNrIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiWndpZnQgUmVsYXkgUkVTVCBBUEkgLS0gcHJvZHVjdGlvbiI6eyJyb2xlcyI6WyJhdXRob3JpemVkLXBsYXllciJdfSwiZWNvbS1zZXJ2ZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJhY2NvdW50Ijp7InJvbGVzIjpbIm1hbmFnZS1hY2NvdW50IiwibWFuYWdlLWFjY291bnQtbGlua3MiLCJ2aWV3LXByb2ZpbGUiXX19LCJzZXNzaW9uX2Nvb2tpZSI6IjJ8ZjNjYTdiYmQwZTNkNjRlNjViODE1Y2EzZmQ2Y2E5ZjMyMDQ1OGE4OTUyY2UwMWE4M2UzMGQyYmJhNDI5OWJlNjI2ZDFlMmY5YmIxY2FlMjcwMzViZTA5ZjNlMTMzNDZhMTM5NTc3NWY0ODEwYTAxYTRlMTNmODU2NjkxOGU0NzcifQ.__xW1OygoBGtTl_T1rlkxoJtPjlE6KRZhTMvnVWCXRo"s;
auto g_token = "{\"access_token\":\"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjkiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiQmVhcmVyIiwiYXpwIjoiR2FtZV9MYXVuY2hlciIsImF1dGhfdGltZSI6MTUzNTUwNzI0OSwic2Vzc2lvbl9zdGF0ZSI6IjA4NDZubzluLTc2NXEtNHAzcy1uMjBwLTZwbnA5cjg2cjVzMyIsImFjciI6IjAiLCJhbGxvd2VkLW9yaWdpbnMiOlsiaHR0cHM6Ly9sYXVuY2hlci56d2lmdC5jb20qIiwiaHR0cDovL3p3aWZ0Il0sInJlYWxtX2FjY2VzcyI6eyJyb2xlcyI6WyJldmVyeWJvZHkiLCJ0cmlhbC1zdWJzY3JpYmVyIiwiZXZlcnlvbmUiLCJiZXRhLXRlc3RlciJdfSwicmVzb3VyY2VfYWNjZXNzIjp7Im15LXp3aWZ0Ijp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiR2FtZV9MYXVuY2hlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFJFU1QgQVBJIC0tIHByb2R1Y3Rpb24iOnsicm9sZXMiOlsiYXV0aG9yaXplZC1wbGF5ZXIiLCJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFplbmRlc2siOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSZWxheSBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIl19LCJlY29tLXNlcnZlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sImFjY291bnQiOnsicm9sZXMiOlsibWFuYWdlLWFjY291bnQiLCJtYW5hZ2UtYWNjb3VudC1saW5rcyIsInZpZXctcHJvZmlsZSJdfX0sIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20iLCJzZXNzaW9uX2Nvb2tpZSI6IjJ8ZjNjYTdiYmQwZTNkNjRlNjViODE1Y2EzZmQ2Y2E5ZjMyMDQ1OGE4OTUyY2UwMWE4M2UzMGQyYmJhNDI5OWJlNjI2ZDFlMmY5YmIxY2FlMjcwMzViZTA5ZjNlMTMzNDZhMTM5NTc3NWY0ODEwYTAxYTRlMTNmODU2NjkxOGU0NzcifQ.iCHigZvPvh6P3zX9NLjjfbTpBpHYO0_s_T-9edJS5qE\",\"expires_in\":1000021600,\"id_token\":\"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjciLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiSUQiLCJhenAiOiJHYW1lX0xhdW5jaGVyIiwiYXV0aF90aW1lIjoxNTM1NTA3MjQ5LCJzZXNzaW9uX3N0YXRlIjoiMDg0Nm5vOW4tNzY1cS00cDNzLW4yMHAtNnBucDlyODZyNXMzIiwiYWNyIjoiMCIsIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20ifQ.rWGSvv5TFO-i6LKczHNUUcB87Hfd5ow9IMG9O5EGR4Y\",\"not-before-policy\":1408478984,\"refresh_expires_in\":611975560,\"refresh_token\":\"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjgiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiUmVmcmVzaCIsImF6cCI6IkdhbWVfTGF1bmNoZXIiLCJhdXRoX3RpbWUiOjAsInNlc3Npb25fc3RhdGUiOiIwODQ2bm85bi03NjVxLTRwM3MtbjIwcC02cG5wOXI4NnI1czMiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZXZlcnlib2R5IiwidHJpYWwtc3Vic2NyaWJlciIsImV2ZXJ5b25lIiwiYmV0YS10ZXN0ZXIiXX0sInJlc291cmNlX2FjY2VzcyI6eyJteS16d2lmdCI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIkdhbWVfTGF1bmNoZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIiwiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBaZW5kZXNrIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiWndpZnQgUmVsYXkgUkVTVCBBUEkgLS0gcHJvZHVjdGlvbiI6eyJyb2xlcyI6WyJhdXRob3JpemVkLXBsYXllciJdfSwiZWNvbS1zZXJ2ZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJhY2NvdW50Ijp7InJvbGVzIjpbIm1hbmFnZS1hY2NvdW50IiwibWFuYWdlLWFjY291bnQtbGlua3MiLCJ2aWV3LXByb2ZpbGUiXX19LCJzZXNzaW9uX2Nvb2tpZSI6IjJ8ZjNjYTdiYmQwZTNkNjRlNjViODE1Y2EzZmQ2Y2E5ZjMyMDQ1OGE4OTUyY2UwMWE4M2UzMGQyYmJhNDI5OWJlNjI2ZDFlMmY5YmIxY2FlMjcwMzViZTA5ZjNlMTMzNDZhMTM5NTc3NWY0ODEwYTAxYTRlMTNmODU2NjkxOGU0NzcifQ.__xW1OygoBGtTl_T1rlkxoJtPjlE6KRZhTMvnVWCXRo\",\"scope\":\"\",\"session_state\":\"0846ab9a-765d-4c3f-a20c-6cac9e86e5f3\",\"token_type\":\"bearer\"}\""s;
#endif
TEST(SmokeTestNet, DISABLED_LoginTestPwd) {
    std::vector<std::string> v{ "OS"s, "Windows"s };
    {
        /*TODO auto ret0 = zwift_network::log_in_with_email_and_password(""s, ""s, v, false, "Game_Launcher"s);
        EXPECT_TRUE(ret0.valid());
        EXPECT_EQ(std::future_status::ready, ret0.wait_for(std::chrono::milliseconds(20)));
        auto r0 = ret0.get();
        EXPECT_EQ(2, r0.m_errCode) << r0.m_msg;
        EXPECT_EQ("Initialize CNL first"s, r0.m_msg);*/
    }
    ZNETWORK_Initialize();
    EXPECT_FALSE(ZNETWORK_IsLoggedIn());
    auto ret2 = zwift_network::log_out();
    EXPECT_TRUE(ret2.valid());
    EXPECT_EQ(std::future_status::ready, ret2.wait_for(std::chrono::milliseconds(20)));
    EXPECT_TRUE(is_ready(ret2));
    auto r2 = ret2.get();
    EXPECT_EQ(3, (int)r2.m_errCode) << r2.m_msg;
    EXPECT_EQ("Not logged in"s, r2.m_msg);
    auto ret1 = g_networkClient->m_pImpl->logInWithEmailAndPassword(""s, ""s, v, true, "Game_Launcher"s);
    EXPECT_TRUE(ret1.valid());
    EXPECT_EQ(std::future_status::ready, ret1.wait_for(std::chrono::milliseconds(20)));
    auto r1 = ret1.get();
    EXPECT_EQ(4, (int)r1.m_errCode) << r1.m_msg;
    EXPECT_EQ("Good luck, soldier"s, r1.m_msg);
    auto ret = g_networkClient->m_pImpl->logInWithEmailAndPassword("olyen2007@gmail.com"s, "123"s, v, false, "Game_Launcher"s);
    EXPECT_TRUE(ret.valid());
    while (!ZNETWORK_IsLoggedIn())
        Sleep(100);
    EXPECT_TRUE(ret.valid());
    EXPECT_EQ(std::future_status::ready, ret.wait_for(std::chrono::milliseconds(20)));
    EXPECT_TRUE(is_ready(ret));
    auto r = ret.get();
    EXPECT_EQ(0, (int)r.m_errCode) << r.m_msg;
    //zoffline has no fantasy
    auto env_rt = std::getenv("rt");
    auto senv_rt = g_rt;
    if (env_rt != nullptr) //token for olyen2007@gmail.com
        senv_rt = env_rt;
    EXPECT_EQ(senv_rt, r.m_msg);
    ret = zwift_network::log_out();
    EXPECT_TRUE(ret.valid());
    while (ZNETWORK_IsLoggedIn())
        Sleep(100);
    EXPECT_TRUE(ret.valid());
    EXPECT_EQ(std::future_status::ready, ret.wait_for(std::chrono::milliseconds(20)));
    r = ret.get();
    EXPECT_EQ(0, r.m_errCode) << r.m_msg;
    EXPECT_EQ(""s, r.m_msg);
    ZNETWORK_Shutdown();
}
#define LOGIN_VIA_TOKEN \
    EXPECT_FALSE(ZNETWORK_IsLoggedIn()); \
    std::vector<std::string> v{"OS"s, "Windows"s}; \
    auto oauth = std::getenv("oauth"), rt = std::getenv("rt"); \
    std::string s_oauth(g_token), s_rt(g_rt); \
    if (oauth != nullptr) /*token for olyen2007@gmail.com*/ \
        s_oauth = oauth; \
    if (rt != nullptr) /*token for olyen2007@gmail.com*/ \
        s_rt = rt; \
    auto ret = zwift_network::log_in_with_oauth2_credentials(s_oauth, v, "Game_Launcher"s); \
    EXPECT_TRUE(ret.valid()); \
    while (!ZNETWORK_IsLoggedIn()) \
        Sleep(100); \
    EXPECT_TRUE(ret.valid()); \
    EXPECT_EQ(std::future_status::ready, ret.wait_for(std::chrono::milliseconds(20))); \
    auto r = ret.get(); \
    EXPECT_EQ(0, r.m_errCode) << r.m_msg; \
    EXPECT_EQ(s_rt, r.m_msg)

TEST(SmokeTestNet, DISABLED_LoginTestToken) {
    ZNETWORK_Initialize();
    LOGIN_VIA_TOKEN;
    ZNETWORK_Shutdown();
}
TEST(SmokeTestNet, B64) {
    uint8_t rawData[] = { "\x1\x2\x3-lorem\0ipsum" };
    auto obj = base64::toString(rawData, _countof(rawData));
    EXPECT_STREQ("AQIDLWxvcmVtAGlwc3VtAA==", obj.c_str());
    std::string b64copy(obj);
    auto obj1 = base64::toBin(b64copy);
    EXPECT_EQ(obj1.size(), _countof(rawData));
    EXPECT_EQ(0, memcmp(rawData, obj1.data(), _countof(rawData)));
}
/*TEST(SmokeTest, Timer) {
    boost::asio::io_context asioCtx;
    boost::asio::steady_timer asioTimer(asioCtx);
    boost::asio::deadline_timer dt(asioCtx);
    int so = sizeof(asioTimer), dtso = sizeof(dt);
    asioTimer.expires_after(std::chrono::milliseconds(100));
    asioTimer.async_wait([](boost::system::error_code const &ec) {
        ;
        });
    boost::asio::ip::tcp::socket tcpSocket(asioCtx, boost::asio::ip::tcp::v4());
    int so2 = sizeof(tcpSocket);
}*/
TEST(SmokeTestNet, Protobuf) {
    protobuf::EventSubgroupPlacements EventSubgroupPlacements; //0x90
    protobuf::CrossingStartingLineProto CrossingStartingLineProto;
    protobuf::Activity Activity; //0x158
    protobuf::ActivityImage ActivityImage; //0x60
    protobuf::FeatureRequest FeatureRequest; //0x68
    protobuf::UdpConfigVOD UdpConfigVOD;
    protobuf::RelayAddressesVOD RelayAddressesVOD;
    protobuf::RelayAddress RelayAddress;
    protobuf::PlayerSummary PlayerSummary;
}
TEST(SmokeTestNet, NetworkResponse) {
    NetworkResponse<std::string> obj;
    obj.storeError(NRO_CURL_ERROR, "123"s);
    EXPECT_EQ(NRO_CURL_ERROR, obj.m_errCode);
    EXPECT_STREQ("123", obj.m_msg.c_str());
    obj.storeError(NRO_ENCRYPTION_FAILURE, "1234");
    EXPECT_EQ(NRO_ENCRYPTION_FAILURE, obj.m_errCode);
    EXPECT_STREQ("1234", obj.m_msg.c_str());
    obj.m_T = "12345"s;
    const auto &cobj = obj.storeError(NRO_ERROR_READING_FILE, obj.m_T);
    EXPECT_EQ(NRO_ERROR_READING_FILE, cobj.m_errCode);
    EXPECT_STREQ("12345", obj.m_msg.c_str());
    const auto &cobj2 = obj;
    EXPECT_STREQ("12345", std::string(cobj2).c_str());
    struct testPlayerIdProvider : public ProfileRequestLazyContext::PlayerIdProvider {
        std::unordered_set<int64_t> getPlayerIds(uint32_t key) override {
            if (key == 321)
                return std::unordered_set<int64_t>{3, 2, 1};
            else
                return std::unordered_set<int64_t>{};
        }
    } tp;
    ProfileRequestLazyContext ctx(123456, nullptr), ctx2(321, &tp);
    EXPECT_EQ(123456, ctx.m_key);
    auto ids = ctx.getPlayerIds();
    EXPECT_EQ(0u, ids.size());
    auto ids2 = ctx2.getPlayerIds();
    EXPECT_EQ(3u, ids2.size());
    EXPECT_TRUE(ids2.contains(1) && ids2.contains(2) && ids2.contains(3));
    ZNet::Error e1("NRO_HTTP_STATUS_CONFLICT"), e2("msg", NRO_CURL_ERROR);
    EXPECT_EQ(NRO_OK, e1.m_netReqOutcome);
    EXPECT_FALSE(e1.m_hasNetReqOutcome);
    EXPECT_STREQ("NRO_HTTP_STATUS_CONFLICT", e1.m_msg.data());
    EXPECT_EQ(NRO_CURL_ERROR, e2.m_netReqOutcome);
    EXPECT_TRUE(e2.m_hasNetReqOutcome);
    EXPECT_STREQ("msg", e2.m_msg.data());
    ZNETWORK_PacePartnerInfo ppi;
    EXPECT_EQ(0, ppi.field_1C + ppi.field_1A);
    ZNet::RetryParams rp;
    EXPECT_EQ(0, rp.m_count + rp.m_timeout);
    ZNet::Params p;
    EXPECT_FALSE(p.m_has_retry);
    EXPECT_EQ(0, p.m_timeout);
    auto zapi = ZNet::API::Inst();
    EXPECT_FALSE(zapi->Dequeue(123));
    auto rid = zapi->Enqueue<int>([]() {
        return std::future<NetworkResponse<int>>();
    }, [](const int &val) {
        ;
    }, &p);
    EXPECT_TRUE(rid > 0);
    p.m_has_retry = true;
    auto ridr = zapi->Enqueue<int>([]() {
        return std::future<NetworkResponse<int>>();
    }, [](const int &val) {
        ;
    }, &p);
    EXPECT_TRUE(ridr > 0);
    EXPECT_TRUE(zapi->GetStatus(ridr));
    EXPECT_TRUE(zapi->Dequeue(ridr));
    EXPECT_FALSE(zapi->GetStatus(ridr));
    EXPECT_TRUE(zapi->IsBusy());
    EXPECT_TRUE(zapi->Dequeue(rid));
    EXPECT_FALSE(zapi->IsBusy());
    EXPECT_FALSE(ZNet::NetworkService::IsInitialized());
    ZNet::NetworkService::Initialize();
    EXPECT_TRUE(ZNet::NetworkService::IsInitialized());
    EXPECT_TRUE(ZNet::NetworkService::Instance() != nullptr);
    ZNet::NetworkService::Shutdown();
    EXPECT_FALSE(ZNet::NetworkService::IsInitialized());
}
TEST(SmokeTestNet, RPC) {
    bool errFlag = false;
    auto err = [&errFlag](ZNet::Error) {
        errFlag = true;
    };
    ZNet::Params p{.m_funcName = "RPC_test", .m_onError = err, .m_timeout = 2000 };
    auto zapi = ZNet::API::Inst();
    std::promise<NetworkResponse<int>> prom;
    int v = -1;
    auto rid = zapi->Enqueue<int>([&prom]() {
        return prom.get_future();
    }, [&v](const int &val) {
        v = val;
    }, &p);
    zapi->Update(1);
    EXPECT_TRUE(zapi->GetStatus(rid));
    EXPECT_EQ(-1, v);
    prom.set_value(NetworkResponse<int>{.m_T = 5});
    EXPECT_EQ(-1, v);
    zapi->Update(1);
    EXPECT_FALSE(zapi->GetStatus(rid));
    EXPECT_EQ(5, v);
    EXPECT_FALSE(errFlag);
}
TEST(SmokeTestNet, RPC_err) {
    ZNet::Error errFlag("init");
    std::string msg_copy;
    auto err = [&errFlag, &msg_copy](ZNet::Error e) {
        errFlag = e;
        msg_copy = e.m_msg;
    };
    {
        ZNet::Params p{ .m_funcName = "RPCerr_test", .m_onError = err, .m_timeout = 2000 };
        auto zapi = ZNet::API::Inst();
        std::promise<NetworkResponse<int>> prom;
        int v = -1;
        auto rid = zapi->Enqueue<int>([&prom]() {
            return prom.get_future();
            }, [&v](const int &val) {
                v = val;
            }, &p);
        zapi->Update(1);
        EXPECT_TRUE(zapi->GetStatus(rid));
        prom.set_value(NetworkResponse<int>{NetworkResponseBase{ "message", NRO_CURL_ERROR }, 5});
        zapi->Update(1);
        EXPECT_FALSE(zapi->GetStatus(rid));
        EXPECT_EQ(-1, v);
        EXPECT_EQ(NRO_CURL_ERROR, errFlag.m_netReqOutcome);
        EXPECT_STREQ("message", errFlag.m_msg.data()); //future object is not destructed yet
    }
    //no! already destructed!!! EXPECT_STREQ("message", errFlag.m_msg.data());
    EXPECT_STREQ("message", msg_copy.c_str());
    EXPECT_TRUE(errFlag.m_hasNetReqOutcome);
}
TEST(SmokeTestNet, RPC_timeout) {
    ZNet::Error errFlag("init", NRO_ERROR_READING_FILE);
    auto err = [&errFlag](ZNet::Error e) {
        errFlag = e;
    };
    ZNet::Params p{ .m_funcName = "RPCto_test", .m_onError = err, .m_timeout = 2 };
    auto zapi = ZNet::API::Inst();
    std::promise<NetworkResponse<int>> prom;
    int v = -1;
    auto rid = zapi->Enqueue<int>([&prom]() {
        return prom.get_future();
        }, [&v](const int &val) {
            v = val;
        }, &p);
    EXPECT_TRUE(zapi->GetStatus(rid));
    zapi->Update(1);
    EXPECT_TRUE(zapi->GetStatus(rid));
    EXPECT_EQ(NRO_ERROR_READING_FILE, errFlag.m_netReqOutcome);
    zapi->Update(10);
    EXPECT_FALSE(zapi->GetStatus(rid));
    EXPECT_EQ(-1, v);
    EXPECT_EQ(NRO_OK, errFlag.m_netReqOutcome);
    EXPECT_STREQ("Timeout", errFlag.m_msg.data()); //future object is not destructed yet
    EXPECT_FALSE(errFlag.m_hasNetReqOutcome);
}
TEST(SmokeTestNet, RPC_block) {
    std::unique_ptr<ZNet::RPCBase> r;
    ZNet::Params p{ .m_funcName = "RPCblock_test" };
    std::promise<NetworkResponse<int>> prom;
    int v = -1;
    r.reset(new ZNet::RPC<int>([&prom]() {
        return prom.get_future();
    }, [&v](const int &val) {
        v = val;
    }, &p));
    std::jthread t([&prom]() {
        ::Sleep(100);
        prom.set_value(NetworkResponse<int>{.m_T = 5});
    });
    EXPECT_EQ(-1, v);
    auto postResult = r->Post(true);
    EXPECT_EQ(NRO_OK, postResult.first);
    EXPECT_TRUE(postResult.second);
    EXPECT_EQ(5, v);
}
TEST(SmokeTestNet, RPC_void) {
    ZNet::Params p{.m_funcName = "RPC_test_void", .m_timeout = 2000 };
    auto zapi = ZNet::API::Inst();
    std::promise<NetworkResponse<void>> prom;
    bool onSucc = false;
    auto rid = zapi->Enqueue<void>([&prom]() {
        return prom.get_future();
    }, [&onSucc]() {
        onSucc = true;
    }, &p);
    zapi->Update(1);
    EXPECT_TRUE(zapi->GetStatus(rid));
    EXPECT_FALSE(onSucc);
    prom.set_value(NetworkResponse<void>{});
    EXPECT_FALSE(onSucc);
    zapi->Update(1);
    EXPECT_TRUE(onSucc);
    EXPECT_FALSE(zapi->GetStatus(rid));
}
TEST(SmokeTestNet, RetrRPC) {
    bool errFlag = false;
    auto err = [&errFlag](ZNet::Error) {
        errFlag = true;
    };
    ZNet::Params p{ .m_funcName = "RPC_retry_test", .m_onError = err, .m_retry = {.m_count = 2, .m_timeout = 2000}, .m_timeout = 2000, .m_has_retry = true };
    auto zapi = ZNet::API::Inst();
    std::promise<NetworkResponse<int>> prom;
    int v = -1;
    auto rid = zapi->Enqueue<int>([&prom]() {
        return prom.get_future();
    }, [&v](const int &val) {
        v = val;
    }, &p);
    zapi->Update(1);
    EXPECT_TRUE(zapi->GetStatus(rid));
    EXPECT_EQ(-1, v);
    prom.set_value(NetworkResponse<int>{.m_T = 5});
    EXPECT_EQ(-1, v);
    zapi->Update(1);
    EXPECT_FALSE(zapi->GetStatus(rid));
    EXPECT_EQ(5, v);
    EXPECT_FALSE(errFlag);
}
TEST(SmokeTestNet, Retr2RPC) {
    bool errFlag = false;
    auto err = [&errFlag](ZNet::Error) {
        errFlag = true;
    };
    ZNet::Params p{ .m_funcName = "RPC_retry2_test", .m_onError = err, .m_retry = {.m_count = 2, .m_timeout = 20'000}, .m_timeout = 20'000, .m_has_retry = true };
    auto zapi = ZNet::API::Inst();
    std::promise<NetworkResponse<int>> prom[2];
    int v = -1, idx = 0;
    auto rid = zapi->Enqueue<int>([&prom, &idx]() {
        return prom[idx].get_future();
    }, [&v](const int &val) {
        v = val;
    }, &p);
    zapi->Update(1);
    EXPECT_TRUE(zapi->GetStatus(rid));
    EXPECT_EQ(-1, v);
    prom[idx++].set_value(NetworkResponse<int>{NetworkResponseBase{"message", NRO_CURL_ERROR}, 5});
    EXPECT_EQ(-1, v);
    zapi->Update(1);
    EXPECT_TRUE(zapi->GetStatus(rid));
    EXPECT_EQ(-1, v);
    prom[idx].set_value(NetworkResponse<int>{.m_T = 5});
    EXPECT_EQ(-1, v);
    zapi->Update(500);
    EXPECT_TRUE(zapi->GetStatus(rid));
    EXPECT_EQ(-1, v);
    zapi->Update(5000);
    EXPECT_TRUE(zapi->GetStatus(rid));
    zapi->Update(1);
    EXPECT_FALSE(zapi->GetStatus(rid));
    EXPECT_EQ(5, v);
    EXPECT_FALSE(errFlag);
}
TEST(SmokeTestNet, Retr3RPC) {
    bool errFlag = false;
    auto err = [&errFlag](ZNet::Error) {
        errFlag = true;
    };
    ZNet::Params p{ .m_funcName = "RPC_retry3_test", .m_onError = err, .m_retry = {.m_count = 3, .m_timeout = 6'000}, .m_timeout = 6'000, .m_has_retry = true };
    auto zapi = ZNet::API::Inst();
    std::promise<NetworkResponse<int>> prom[3];
    int v = -1, idx = 0;
    auto rid = zapi->Enqueue<int>([&prom, &idx]() {
        return prom[idx].get_future();
    }, [&v](const int &val) {
        v = val;
    }, &p);
    zapi->Update(3500);
    zapi->Update(3500);
    EXPECT_TRUE(zapi->GetStatus(rid));
    EXPECT_EQ(-1, v);
    zapi->Update(500);
    prom[++idx].set_value(NetworkResponse<int>{NetworkResponseBase{"message", NRO_CURL_ERROR}, 5});
    EXPECT_EQ(-1, v);
    zapi->Update(5000);
    EXPECT_TRUE(zapi->GetStatus(rid));
    EXPECT_EQ(-1, v);
    prom[++idx].set_value(NetworkResponse<int>{.m_T = 5});
    EXPECT_EQ(-1, v);
    zapi->Update(500);
    zapi->Update(500);
    EXPECT_TRUE(zapi->GetStatus(rid));
    EXPECT_EQ(-1, v);
    zapi->Update(5000);
    EXPECT_TRUE(zapi->GetStatus(rid));
    zapi->Update(1);
    EXPECT_FALSE(zapi->GetStatus(rid));
    EXPECT_EQ(5, v);
    EXPECT_FALSE(errFlag);
}
TEST(SmokeTestNet, SubscriptionStuff) {
    EXPECT_EQ(SM_NAMED_SRC, ZNETWORK_CalculateSubscriptionMode());
    auto sm = ZNETWORK_CalculateThenGetSubscriptionMode();
    EXPECT_TRUE(sm == SM_UNDEFINED || sm == SM_NAMED_SRC);
    EXPECT_STREQ("Ursoft premium", ZNETWORK_GetPromoNameOfCurrentRideEntitlement());
    EXPECT_STREQ("Ursoft platinum", ZNETWORK_GetPromoNameOfNextRideEntitlement());
    EXPECT_EQ(100'000.0f, ZNETWORK_GetTrialKMLeft());
    EXPECT_EQ(-1, ZNETWORK_GetSubscriptionDaysLeft());
}
TEST(SmokeTestNet, Nro2Str) {
    int64_t act = 0;
    for (auto i = 0; i < NRO_CNT; i++)
        for (auto ch : NetworkRequestOutcomeToString((NetworkRequestOutcome)i))
            act += ch;
    EXPECT_EQ(406868, act);
}
TEST(SmokeTestNet, Qsb) {
    QueryStringBuilder obj; //string values must be escaped outside!
    obj.addOptional("skip"s, Optional<int>());
    obj.addOptional("b0"s, Optional(false));
    obj.addOptional("b1"s, Optional(true));
    obj.addOptional("str"s, Optional<const char *>("sval"));
    obj.addOptional("int32_t"s, Optional<int32_t>(-1));
    obj.addOptional("int32_t"s, Optional<int32_t>(123));
    obj.addOptional("int64_t"s, Optional<int64_t>(0x7FFFFFFFFFFFFFFFi64));
    obj.addOptional("uint32_t"s, Optional<uint32_t>(0xFFFFFFFF));
    obj.addOptional("uint64_t"s, Optional<uint64_t>(0xFFFFFFFFFFFFFFFFui64));
    obj.addOptional("std_str"s, Optional("strval"s));
    obj.addIfNotEmpty("skip"s, "");
    obj.addIfNotEmpty("skip"s, (const char *)nullptr);
    obj.addIfNotEmpty("str2"s, "sval2");
    obj.addIfNotFalse("skip"s, false);
    obj.addIfNotFalse("if_bool"s, true);
    obj.addIfNotZero("skip"s, (int32_t)0);
    obj.addIfNotZero("skip"s, (int64_t)0);
    obj.addIfNotZero("skip"s, (uint32_t)0);
    obj.addIfNotZero("skip"s, (uint64_t)0);
    obj.addIfNotZero("if_int32"s, (int32_t)1);
    obj.addIfNotZero("if_int64"s, (int64_t)2);
    obj.addIfNotZero("if_uint32"s, (uint32_t)3);
    obj.addIfNotZero("if_uint64"s, (uint64_t)4);
    EXPECT_STREQ("?b0=false&b1=true&if_bool=true&if_int32=1&if_int64=2&if_uint32=3&if_uint64=4&int32_t=-1&int32_t=123&int64_t=9223372036854775807&std_str=strval&str=sval&str2=sval2&uint32_t=4294967295&uint64_t=18446744073709551615", obj.getString(true).c_str());
    EXPECT_STREQ("b0=false&b1=true&if_bool=true&if_int32=1&if_int64=2&if_uint32=3&if_uint64=4&int32_t=-1&int32_t=123&int64_t=9223372036854775807&std_str=strval&str=sval&str2=sval2&uint32_t=4294967295&uint64_t=18446744073709551615", obj.getString(false).c_str());
}
TEST(SmokeTestNet, TestDropInWorldsStatus) {
    ZNETWORK_Initialize();
    //not logged in branch:
    auto waitUntil = GetTickCount() + 2000;
    do {
        ZNETWORK_UpdateDropInWorldsStatus();
        if (int(GetTickCount() - waitUntil) > 0) {
            EXPECT_STREQ("", "timeout");
            break;
        }
    } while (g_DropInWorldsStatus != DIW_GIVEUP);
    LOGIN_VIA_TOKEN;
    //logged in TestDropInWorldsStatus branch:
    g_DropInWorldsStatus = DIW_INIT;
    waitUntil = GetTickCount() + 20000;
    do {
        ZNETWORK_UpdateDropInWorldsStatus();
        if (int(GetTickCount() - waitUntil) > 0) {
            EXPECT_STREQ("", "timeout");
            break;
        }
        if (g_DropInWorldsStatus == DIW_GIVEUP) {
            EXPECT_STREQ("", "DIW_GIVEUP");
            break;
        }
    } while (g_DropInWorldsStatus != DIW_OK_PAUSE);
    EXPECT_EQ(14, g_DropInWorlds.worlds_size());
    if (g_DropInWorlds.worlds_size() == 14) {
        auto &w13name = g_DropInWorlds.worlds(13).name();
        EXPECT_EQ("Public Watopia"s, w13name);
    }
    g_DropInWorldsStatus = DIW_INIT;
    auto logOutResp = zwift_network::log_out().get();
    EXPECT_EQ(0, (int)logOutResp.m_errCode) << logOutResp.m_msg;
    EXPECT_EQ(""s, logOutResp.m_msg);
    ZNETWORK_Shutdown();
}
TEST(SmokeTestNet, Activities) {
    BikeManager::Instance()->m_mainBike->m_playerIdTx = 0;
    g_LastActivityListRequestPlayerID = 0;
    g_LastActivityList.Clear();
    ZNETWORK_Initialize();
    EXPECT_FALSE(ZNETWORK_IsLoggedIn());
    //not logged in branch:
    auto acts = ZNETWORK_GetActivities();
    EXPECT_EQ(nullptr, acts);
    LOGIN_VIA_TOKEN;
    //logged in ZNETWORK_GetActivities branch:
    acts = ZNETWORK_GetActivities();
    EXPECT_EQ(&g_LastActivityList, acts);
    //no m_playerIdTx
    EXPECT_EQ(0, g_LastActivityList.activities_size());
    auto mypf = zwift_network::my_profile();
    auto status = mypf.wait_for(std::chrono::seconds(5));
    EXPECT_EQ(std::future_status::ready, status);
    if (std::future_status::ready == status) {
        auto prof = mypf.get();
        EXPECT_EQ(NRO_OK, prof.m_errCode) << prof.m_msg;
        if (NRO_OK == prof.m_errCode) {
            auto pid = g_networkClient->m_pImpl->m_globalState->getPlayerId();
            EXPECT_EQ(pid, prof.m_T.id());
            EXPECT_EQ("P"s, prof.m_T.last_name());
            EXPECT_EQ("Olyen"s, prof.m_T.first_name());
            BikeManager::Instance()->m_mainBike->m_playerIdTx = pid;
            auto waitUntil = GetTickCount() + 20000;
            do {
                acts = ZNETWORK_GetActivities();
                if (int(GetTickCount() - waitUntil) > 0)
                    break;
            } while (acts != &g_LastActivityList);
            EXPECT_EQ(&g_LastActivityList, acts) << "timeout";
            EXPECT_GT(g_LastActivityList.activities_size(), 0);
        }
    }
    //no player branch:
    BikeManager::Instance()->m_mainBike->m_playerIdTx = 0;
    acts = ZNETWORK_GetActivities();
    EXPECT_EQ(nullptr, acts);

    auto logOutResp = zwift_network::log_out().get();
    EXPECT_EQ(0, (int)logOutResp.m_errCode) << logOutResp.m_msg;
    EXPECT_EQ(""s, logOutResp.m_msg);
    ZNETWORK_Shutdown();
}
TEST(SmokeTestNet, ClearPlayerPowerups) {
    ZNETWORK_Initialize();
    //not logged in branch:
    BikeManager::Instance()->m_mainBike->m_playerIdTx = 0;
    auto nro = ZNETWORK_ClearPlayerPowerups();
    EXPECT_FALSE(ZNETWORK_IsLoggedIn());
    EXPECT_EQ(NRO_NOT_PAIRED_TO_PHONE, nro);

    /*need pair to phone
    //log in:
    std::vector<std::string> v{"OS"s, "Windows"s};
    auto ret = zwift_network::log_in_with_oauth2_credentials(g_token, v, "Game_Launcher"s);
    EXPECT_TRUE(ret.valid());
    while (!ZNETWORK_IsLoggedIn())
        Sleep(100);
    EXPECT_TRUE(ret.valid());
    EXPECT_EQ(std::future_status::ready, ret.wait_for(std::chrono::milliseconds(20)));
    auto r = ret.get();
    EXPECT_EQ(0, r.m_errCode) << r.m_msg;
    EXPECT_EQ(g_rt, r.m_msg);

    //logged in ZNETWORK_ClearPlayerPowerups branch, no m_playerIdTx:
    nro = ZNETWORK_ClearPlayerPowerups();
    EXPECT_EQ(NRO_INVALID_ARGUMENT, nro);
    //with m_playerIdTx:
    auto mypf = zwift_network::my_profile();
    auto status = mypf.wait_for(std::chrono::seconds(5));
    EXPECT_EQ(std::future_status::ready, status);
    if (std::future_status::ready == status) {
        auto prof = mypf.get();
        EXPECT_EQ(NRO_OK, prof.m_errCode) << prof.m_msg;
        if (NRO_OK == prof.m_errCode) {
            auto pid = g_networkClient->m_pImpl->m_globalState->getPlayerId();
            EXPECT_EQ(pid, prof.m_T.id());
            EXPECT_EQ("P"s, prof.m_T.last_name());
            BikeManager::Instance()->m_mainBike->m_playerIdTx = pid;
            auto waitUntil = GetTickCount() + 20000;
            do {
                acts = ZNETWORK_ClearPlayerPowerups();
                if (int(GetTickCount() - waitUntil) > 0)
                    break;
            } while (acts != &g_LastActivityList);
            EXPECT_EQ(&g_LastActivityList, acts) << "timeout";
            EXPECT_GT(g_LastActivityList.activities_size(), 0);
        }
    }

    //no player branch:
    BikeManager::Instance()->m_mainBike->m_playerIdTx = 0;
    acts = ZNETWORK_GetActivities();
    EXPECT_EQ(nullptr, acts);*/

    auto logOutResp = zwift_network::log_out().get();
    EXPECT_EQ(3, (int)logOutResp.m_errCode) << logOutResp.m_msg;
    EXPECT_EQ("Not logged in"s, logOutResp.m_msg);
    ZNETWORK_Shutdown();
}
TEST(SmokeTestNet, SayHello) {
    auto evSysInst = EventSystem::GetInst();
    Experimentation::Initialize(evSysInst);
    ZNETWORK_Initialize();
    BikeManager::Instance()->m_mainBike->m_playerIdTx = 0;
    LOGIN_VIA_TOKEN;
    //who am i
    auto mypf = zwift_network::my_profile();
    auto status = mypf.wait_for(std::chrono::seconds(5));
    EXPECT_EQ(std::future_status::ready, status);

    for (int i = 0; i < 1000; i++) {
        auto receivedTCPs = g_networkClient->m_pImpl->m_tcpClient->m_codec.m_hostSeqNo[protocol_encryption::Codec::DEC];
        if (receivedTCPs > 1)
            break;
        ZNETWORK_Update(0.1f);
        Sleep(100);
    }

    auto logOutResp = zwift_network::log_out().get();
    EXPECT_EQ(0, (int)logOutResp.m_errCode) << logOutResp.m_msg;
    EXPECT_EQ(""s, logOutResp.m_msg);
    Experimentation::Shutdown();
    ZNETWORK_Shutdown();
    EventSystem::Destroy();
}
TEST(SmokeTestNet, Crypt) {
    auto header = (const uint8_t *)"\x00\xe3\x1f\xf6\xfb\x8a\x52\xbe\x2f\x44\xbf\x2b\x6e\x3c\xbd\x62\x8e\xae\xec\x14\x02\xb8\x9f\xdf\x7d\x8f\x61\x80\xc3\x6c\x00\xd0\x41\x15\x47\xa3\x89\x9f\x65\xc8\x17\x9e\x67\xcb\xa3\x04\x23\xef\x21\xbb\xbd\x3d\x44\x41\xc4\xf7\x6b\xb0\xf5\xff\x99\xc5\x58\xc6\x91\x45\x6e\x52\x43\x5e\xcc\x97\x57\x0d\xf5\xe7\xeb\xf3\xc7\x4a\x6f\xdf\x63\x62\x89\xf7\xff\x31\xb7\xe4\x8b\x6b\xce\x87\xcd\xa7\xd2\x46\x17\x2b\x13\x45\xcf\x6b\x2f\x52\x4b\x06\x2c\x0a\x98\x3c\x4b\x68\x92\x55\xf2\x29\x4d\xae\x41\xb1\x30\x21\xf9\x26\xeb\x8f";
    auto src = header + 1;
    int hlen = 1, len = 127;
    auto iv = (const uint8_t *)"\x00\x00\x00\x01\x00\x04\x00\x01\x00\x00\x00\x00";
    auto secretRaw = (const uint8_t *)"\xa9\x05\x6d\xcd\xa2\x1a\xad\xcc\xd4\xd2\xb4\x5b\xce\x76\x43\xd8";
    EVP_CIPHER_CTX *cipher = EVP_CIPHER_CTX_new();
    ASSERT_NE(nullptr, cipher);
    auto gcm = EVP_aes_128_gcm();
    ASSERT_NE(nullptr, gcm);
    auto ok = EVP_EncryptInit_ex(cipher, gcm, nullptr, nullptr, nullptr);
    ASSERT_NE(0, ok);
    ok = EVP_CIPHER_CTX_set_padding(cipher, 0);
    ASSERT_NE(0, ok);
    ok = EVP_CIPHER_CTX_ctrl(cipher, 0, 0, nullptr);
    ASSERT_NE(0, ok);
    ok = EVP_CIPHER_CTX_set_key_length(cipher, 16);
    ASSERT_NE(0, ok);
    int outl = EVP_CIPHER_CTX_block_size(cipher) + len, outlf = 0;
    std::vector<uint8_t> tmp(outl);
    uint32_t tag = *(uint32_t *)&src[len - 4];
    ok = EVP_DecryptInit_ex(cipher, nullptr, nullptr, secretRaw, iv);
    ASSERT_NE(0, ok);
    if (hlen) {
        ok = EVP_DecryptUpdate(cipher, nullptr, &outl, header, hlen);
        ASSERT_NE(0, ok);
    }
    ok = EVP_DecryptUpdate(cipher, &tmp[0], &outl, src, len - 4);
    ASSERT_NE(0, ok);
    ok = EVP_CIPHER_CTX_ctrl(cipher, EVP_CTRL_AEAD_SET_TAG, 4, &tag);
    ASSERT_NE(0, ok);
    ok = EVP_DecryptFinal_ex(cipher, &tmp[0] + outl, &outlf);
    outlf += outl;
    ASSERT_NE(0, ok);
    EVP_CIPHER_CTX_free(cipher);
    const char pay[] = "\x10\x02\x18\x00\xc2\x01\x34\x0a\x15\x08\x01\x10\x06\x1a\x0c\x31\x39\x32\x2e\x31\x36\x38\x2e\x31\x2e\x36\x30\x20\xce\x17\x0a\x15\x08\x00\x10\x00\x1a\x0c\x31\x39\x32\x2e\x31\x36\x38\x2e\x31\x2e\x36\x30\x20\xce\x17\x10\x0a\x18\x1e\x20\x03\xca\x01\x3d\x0a\x1b\x08\x01\x10\x06\x1a\x15\x08\x01\x10\x06\x1a\x0c\x31\x39\x32\x2e\x31\x36\x38\x2e\x31\x2e\x36\x30\x20\xce\x17\x0a\x1b\x08\x00\x10\x00\x1a\x15\x08\x00\x10\x00\x1a\x0c\x31\x39\x32\x2e\x31\x36\x38\x2e\x31\x2e\x36\x30\x20\xce\x17\x10\xce\x17";
    ASSERT_EQ(sizeof(pay) - 1 /*ending 0*/, outlf);
    ASSERT_GE(tmp.size(), outlf);
    tmp.resize(outlf);
    ASSERT_EQ(0, memcmp(&tmp[0], pay, outlf));
}