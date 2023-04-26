#include "ZwiftApp.h"
#include "readerwriterqueue/readerwriterqueue.h"
#include "concurrentqueue/concurrentqueue.h"
#include "openssl/md5.h"
bool g_NetworkOn;
void ZNETWORK_Shutdown() {
    if (g_NetworkOn) {
        shutdown_zwift_network();
        g_NetworkOn = false;
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
    //void addOptional<std::string>(const std::string &val, zwift_network::Optional<std::string>);
    std::string getString(bool needQuest) {
        int total = 0;
        for (const auto &i : *this)
            total += i.first.length() + i.second.length() + 2;
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
enum AcceptType { ATH_PB, ATH_JSON };
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
    std::string escapeUrl(const std::string &src) { /*vptr[23]*/ return curl_easy_escape(m_curl, src.c_str(), src.size()); }

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
        int v32 = 0;
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
    QueryResult performRequest(const std::string &op, const std::string &url, const std::string &descr, int payloadSize) {
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
    CurlHttpConnection *instance(const std::string &certs, bool ncoSkipCertCheck, bool a5, int ncoTimeoutSec, int ncoUploadTimeoutSec, HttpRequestMode hrm) {
        return new CurlHttpConnection(certs, ncoSkipCertCheck, a5, ncoTimeoutSec, ncoUploadTimeoutSec, m_curlVersion, m_zaVersion,
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
    std::vector<std::thread> m_pool;
    int m_ncoTimeoutSec, m_ncoUploadTimeoutSec, m_nThreads;
    HttpRequestMode m_hrm;
    bool m_ncoSkipCertCheck;
    HttpConnectionManager(CurlHttpConnectionFactory *curlf, const std::string &certs, bool ncoSkipCertCheck, int ncoTimeoutSec, int ncoUploadTimeoutSec, HttpRequestMode hrm, int nThreads) :
        m_curlf(curlf), m_certs(certs), m_ncoTimeoutSec(ncoTimeoutSec), m_ncoUploadTimeoutSec(ncoUploadTimeoutSec), m_hrm(hrm), m_nThreads(nThreads), m_ncoSkipCertCheck(ncoSkipCertCheck) {}
    ~HttpConnectionManager() {}
    virtual void worker(uint32_t nr) = 0;// { /*empty*/ }
    void startWorkers() {
        m_pool.reserve(m_nThreads);
        for (auto i = 0; i < m_nThreads; ++i)
            m_pool.emplace_back([i, this]() { worker(i); });
    }
    void shutdown() {
        setThreadPoolSize(0);
        for (auto &t : m_pool) {
            if (t.joinable())
                t.join();
        }
    }
    void setUploadTimeout(uint64_t to) { std::lock_guard l(m_mutex); m_ncoUploadTimeoutSec = to; }
    void setTimeout(uint64_t to) { std::lock_guard l(m_mutex); m_ncoTimeoutSec = to; }
    void setThreadPoolSize(int val) {
        int oldVal = m_nThreads;
        { std::lock_guard l(m_mutex); m_nThreads = val; }
        if (oldVal) {
            m_conditionVar.notify_all();
        } else while (oldVal < m_nThreads) {
            m_pool.emplace_back([this, oldVal]() { worker(oldVal); });
            ++oldVal;
        }
    }
};
struct GenericHttpConnectionManager : public HttpConnectionManager { //0x128 bytes
    using task_type = std::packaged_task<NetworkResponseBase(CurlHttpConnection *)>;
    std::queue<task_type> m_ptq;
    GenericHttpConnectionManager(CurlHttpConnectionFactory *curlf, const std::string &certs, bool ncoSkipCertCheck, int ncoTimeoutSec, int ncoUploadTimeoutSec, HttpRequestMode hrm) :
        HttpConnectionManager(curlf, certs, ncoSkipCertCheck, ncoTimeoutSec, ncoUploadTimeoutSec, hrm, 1) {}
    ~GenericHttpConnectionManager() { shutdown(); }
    void worker(uint32_t a2) override {
        auto conn = m_curlf->instance(m_certs, m_ncoSkipCertCheck, true, m_ncoTimeoutSec, m_ncoUploadTimeoutSec, HRM_SEQUENTIAL);
        while (a2 < m_nThreads) {
            task_type task;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_conditionVar.wait(lock, [this] { return !this->m_ptq.empty(); });
                task = std::move(m_ptq.front());
                m_ptq.pop();
            }
            task(conn);
        }
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
/* TODO:
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
struct WorldAttributeServiceListener {};
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
    bool shouldUseEncryption() { return m_shouldUseEncryption; }
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
struct base64 : public std::string { //IDA: Base64Url, base64::
    void decode(const std::string &src) { decode(src, 0, src.length()); }
    void decode(const std::string &src, int offset, int len) {
        const uint8_t *p = (uint8_t *)src.c_str() + offset;
        int pad = len > 0 && (len % 4 || p[len - 1] == '=');
        const size_t L = ((len + 3) / 4 - pad) * 4;
        resize(0);
        reserve(L / 4 * 3 + pad);
        for (size_t i = 0, j = 0; i < L; i += 4) {
            int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
            push_back(n >> 16);
            push_back(n >> 8 & 0xFF);
            push_back(n & 0xFF);
        }
        if (pad) {
            int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
            push_back(n >> 16);
            if (len > L + 2 && p[L + 2] != '=') {
                n |= B64index[p[L + 2]] << 6;
                push_back(n >> 8 & 0xFF);
            }
        }
    }
    void encode(const uint8_t *src, size_t len) {
        auto output_length = 4 * ((len + 2) / 3);
        resize(output_length);
        auto dest = data();
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
    }
};
namespace HttpHelper {
    static NetworkResponse<Json::Value> parseJson(const std::string &src) {
        NetworkResponse<Json::Value> ret;
        Json::Reader r;
        if (!r.parse(src, ret, false)) {
            NetworkingLogError("Error parsing JSON: %s\nJSON: %s", r.getFormattedErrorMessages().c_str(), src.c_str());
            ret.storeError(NRO_JSON_PARSING_ERROR, "Error parsing json"s);
        }
        return ret;
    }
    static NetworkResponse<Json::Value> parseJson(const std::vector<char> &src) {
        NetworkResponse<Json::Value> ret;
        Json::Reader r;
        if (src.size() == 0 || !r.parse(&src.front(), &src.back(), ret, false)) {
            NetworkingLogError("Error parsing JSON: %s\nJSON: %s", r.getFormattedErrorMessages().c_str(), src.size() ? &src.front() : "empty");
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
        auto size = src.ByteSize();
        dest->resize(size);
        if (!src.SerializeToArray(dest->data(), size))
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
            if (!ret.m_T.ParseFromArray(queryResult.m_T.data(), queryResult.m_T.size()))
                ret.storeError(NRO_PROTOBUF_FAILURE_TO_DECODE, "Failed to decode protobuf"s);
        }
        return ret;
    }
    static NetworkResponse<Json::Value> convertToJsonResponse(const QueryResult &src) {
        NetworkResponse<Json::Value> ret;
        return src.ok(&ret) ? parseJson(src) : ret;
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
    uint64_t m_exp = 0;
    const std::string &asString() const { return m_base64; }
    const std::string &getSessionState() const { return m_sessionState; }
    const std::string &getSubject() const { return m_sub; }
    bool parsePayload(const std::string &payload) {
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
        base64 payload;
        payload.decode(jwt, firstSep + 1, secondSep - firstSep - 1);
        if (parsePayload(payload)) {
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
        auto pr = HttpHelper::parseJson(src);
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
    void worker(uint32_t a2) override {
        auto conn = m_curlf->instance(m_certs, m_ncoSkipCertCheck, false, m_ncoTimeoutSec, m_ncoUploadTimeoutSec, m_hrm);
        while (a2 < m_nThreads) {
            RequestTaskContext task;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_conditionVar.wait(lock, [this] { return !this->m_needNewAcToken && !this->m_rtq.empty(); });
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
            task(conn, needNewAcToken);
        }
    }
    ~ZwiftHttpConnectionManager() { shutdown(); }
    template<class T>
    std::future<NetworkResponse<T>> pushRequestTask(const std::function<NetworkResponse<T>(CurlHttpConnection *)> &f, bool b1, bool b2) {
        if (b2 || !*m_tcpDisconnected) {
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
            stored_task.m_secured = b1; //TODO: check (not sure)
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
    base64 m_secret;
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
    void secretRawToString() { m_secret.encode(m_secretRaw, sizeof(m_secretRaw)); }
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
                    base64 b64;
                    b64.decode(m_secret, 0, m_secret.length());
                    if (b64.size() == sizeof(m_secretRaw))
                        memmove(m_secretRaw, b64.data(), sizeof(m_secretRaw));
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
    bool decode(const uint8_t *src, uint32_t len, std::vector<uint8_t> *dest, std::string *err) { /*vptr[DCNT]*/
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
        auto retHlen = 1;
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
                m_ivs[DEC].m_netSEQ = ntohl(m_hostSeqNo[DEC]);
            } else if (m_vitalCI && !m_hasConnId[DEC]) {
                return failM1("connection id not received yet"s, err);
            }
            if (flags & 1) {
                m_ivs[DEC].m_netSEQ = ntohl(m_hostSeqNo[DEC]);
                return retHlen;
            }
            if (len >= retHlen) {
                m_hostSeqNo[DEC] = ntohl(*(uint32_t *)&h[retHlen]);
                retHlen += 4;
                m_ivs[DEC].m_netSEQ = ntohl(m_hostSeqNo[DEC]);
                return retHlen;
            }
            return failM1("message too short"s, err);
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
                        ;
                else
                    return fail("EVP_EncryptFinal_ex"s, err);
            else
                return fail("EVP_EncryptUpdate"s, err);
        else
            return fail("EVP_EncryptInit_ex"s, err);
        dest->insert(dest->end(), m_tmp[ENC].begin(), m_tmp[ENC].begin() + outlf + outl);
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
                if (EVP_CIPHER_CTX_ctrl(m_ciphers[DEC], 0, 0, nullptr))
                    if (!EVP_DecryptFinal_ex(m_ciphers[DEC], &m_tmp[DEC][0] + outl, &outlf))
                        return fail("EVP_DecryptFinal_ex"s, err);
                    else
                        ;
                else
                    return fail("EVP_CIPHER_CTX_ctrl"s, err);
            else
                return fail("EVP_DecryptUpdate"s, err);
        else
            return fail("EVP_DecryptInit_ex"s, err);
        dest->insert(dest->end(), &m_tmp[DEC][0], &m_tmp[DEC][0] + outlf + outl);
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
    NetworkResponse<protobuf::LoginResponse> doLogIn(const std::string &sk, const std::vector<std::string> &anEventProps, CurlHttpConnection *conn) {
        std::string LogInV2;
        auto url = m_server + "/api/users/login"s;
        protobuf::LoginRequest lr;
        lr.set_key(sk);
        for (int i = 0; i < anEventProps.size(); i += 2) {
            auto p = lr.mutable_properties()->add_property();
            p->set_name(anEventProps[i]);
            p->set_value(anEventProps[i + 1]);
        }
        auto pm = lr.mutable_properties()->add_property();
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
    std::string getSecretKey(const EncryptionOptions &eo) {
        if (!eo.m_disableEncrWithServer && shouldEnableEncryptionBasedOnFeatureFlag(eo)) {
            if (!eo.m_secretKeyBase64.empty()) {
                base64 ret;
                ret.decode(eo.m_secretKeyBase64);
                return ret;
            } else {
                protocol_encryption::TcpRelayClientCodec codec;
                std::string err;
                if (codec.m_initOK || codec.initialize(&err))
                    return std::string((char *)codec.m_secretRaw, _countof(codec.m_secretRaw));
                else
                    NetworkingLogError("Failed to get secret key: %s", err.c_str());
            }
        }
        return std::string();
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
        *v49.add_params()->add_param() = "game_1_26_2_data_encryption"s;
        *v49.add_params()->add_param() = "game_1_27_0_disable_encryption_bypass"s;
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
    static void Execute(EventLoop *t) { t->m_asioCtx.run(); }
    EventLoop() : m_asioCtx(), m_thrd(Execute, this) {} //OMIT: get m_asioCtx->impl pointer and increment references count to it
    ~EventLoop() { shutdown(); }
    void shutdown() { 
        m_asioCtx.stop(); 
        if (m_thrd.joinable()) 
            m_thrd.join(); 
    }
    void enqueueShutdown() {
        boost::asio::post(m_asioCtx, [this]() { 
            m_asioCtx.stop(); });
        if (m_thrd.joinable()) 
            m_thrd.join();
    }
    void post(std::function<void()> &&f) { boost::asio::post(m_asioCtx, f); }
};
struct NetworkClockService;
struct WorldClockService { //0x2120 bytes
    NetworkClockService *m_ncs;
    int64_t m_twoLegsLatency = 0, m_field_40 = 0, m_calcWorldClockOffset[1024] = {};
    uint64_t m_idx = 0, m_stab = 2;
    volatile LONG64 m_worldTime = 0, m_worldClockOffset = 0;
    std::map<uint32_t, uint64_t> m_map38; //seq to send time
    bool m_stc_useF5 = true, m_bInit = false;
    WorldClockService(EventLoop *, NetworkClockService *ncs) : m_ncs(ncs) {
        //OMIT NumericStatisticsPeriodicLogger<long> ctr
    }
    bool isInitialized() /*vptr[1]*/ { return m_bInit; }
    uint64_t getWorldTime() /*vptr[2]*/ {
        if (!this->m_bInit)
            return 0;
        auto steadyDelta = g_steadyClock.nowInMilliseconds() - this->m_worldClockOffset;
        auto worldTime = this->m_worldTime;
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
        if ((1 << m_stab) & m_idx) {
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
struct HashSeedService { //0x370 bytes
    void start() {
        //TODO
    }
    HashSeedService(EventLoop *el, GlobalState *gs, RelayServerRestInvoker *ri, WorldClockService *wc, bool a6, int a7 = 300, int a8 = 10) {
    }
};
struct UdpStatistics;
struct RestServerRestInvoker { //0x70 bytes
    RestServerRestInvoker(const std::string &machId, ZwiftHttpConnectionManager *mgr, const std::string &server, const std::string &version) {
        //TODO
    }
};
struct UdpClient : public WorldAttributeServiceListener, UdpConfigListener, EncryptionListener { //0x1400-16 bytes
    //            UdpClient::UdpClient(GlobalState *, WorldClockService *, HashSeedService *, HashSeedService *, UdpStatistics *, RelayServerRestInvoker *, TelemetryService *, UdpClient::Listener &, std::chrono::duration<long long, std::ratio<1l, 1l>>, std::chrono::duration<long long, std::ratio<1l, 1000l>>)
    UdpClient(GlobalState *, WorldClockService *, HashSeedService *, HashSeedService *, UdpStatistics *, RelayServerRestInvoker *, void /*netImpl*/ *) {
        //TODO
    }
    void handleEncryptionChange(const EncryptionInfo &ei) override {
        //TODO
    }
    void handleUdpConfigChange(const protobuf::UdpConfigVOD &uc, uint64_t a3) override {
        //TODO
    }
    void shutdown() {
        //TODO
    }
};
struct WorldAttributeService { //0x270 bytes
    moodycamel::ReaderWriterQueue<void *> m_rwq;
    ServiceListeners<WorldAttributeServiceListener> m_lis;
    WorldAttributeService() : m_rwq(100) {
        /*TODO   *(_QWORD *)this->field_80 = 0i64;
          *(_QWORD *)&this->field_80[64] = 0i64;
          *(_QWORD *)&this->field_80[72] = 0i64;
          v4 = operator new(0x20ui64);
          *v4 = v4;
          v4[1] = v4;
          *(_QWORD *)&this->field_80[64] = v4;*/
    }
    void registerListener(WorldAttributeServiceListener *lis) { m_lis += lis; }
    void removeListener(WorldAttributeServiceListener *lis) { m_lis -= lis; }
/*WorldAttributeService::getLargestWorldAttributeTimestamp()
WorldAttributeService::handleServerToClient(zwift::protobuf::ServerToClient const&)
WorldAttributeService::logWorldAttribute(zwift::protobuf::WorldAttribute const&,bool)
WorldAttributeService::popWorldAttribute(std::shared_ptr<zwift::protobuf::WorldAttribute const> &)*/
};
struct ProfileRequestDebouncer { //0x1E0 bytes
    ProfileRequestDebouncer(EventLoop *, RestServerRestInvoker *, uint32_t) {
        //TODO
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
    //TODO
};
struct ActivityRecommendationRestInvoker { //0x30 bytes
    ActivityRecommendationRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    //TODO
};
struct AchievementsRestInvoker { //0x30 bytes
    AchievementsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO = ExperimentsRestInvoker
    }
    //TODO
};
struct CampaignRestInvoker { //0x70 bytes
    CampaignRestInvoker(const std::string &server) {
        //TODO
    }
    //TODO
};
struct ClubsRestInvoker { //0x30 bytes
    ClubsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    //TODO
};
struct EventCoreRestInvoker { //0x30 bytes
    EventCoreRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    //TODO
};
struct EventFeedRestInvoker { //0x30 bytes
    EventFeedRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    //TODO
};
struct FirmwareUpdateRestInvoker { //0x30 bytes
    FirmwareUpdateRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    //TODO
};
struct GenericRestInvoker { //0x10 bytes
    GenericRestInvoker(GenericHttpConnectionManager *mgr) {
        //TODO
    }
    //TODO
};
struct PrivateEventsRestInvoker { //0x30 bytes
    PrivateEventsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    //TODO
};
struct RaceResultRestInvoker { //0x30 bytes
    RaceResultRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    //TODO
};
struct RouteResultsRestInvoker { //0x30 bytes
    RouteResultsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO = Experiments
    }
    //TODO
};
struct PlayerPlaybackRestInvoker { //0x30 bytes
    PlayerPlaybackRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    //TODO
};
struct SegmentResultsRestInvoker { //0x30 bytes
    SegmentResultsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    //TODO
};
struct PowerCurveRestInvoker { //0x30 bytes
    PowerCurveRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    //TODO
};
struct ZFileRestInvoker { //0x30 bytes
    ZFileRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    //TODO
};
struct ZwiftWorkoutsRestInvoker { //0x38 bytes
    ZwiftWorkoutsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    //TODO
};
struct WorkoutServiceRestInvoker { //0x30 bytes
    WorkoutServiceRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    //TODO
};
struct UdpStatistics { //0x450 bytes
    UdpStatistics() {
        //TODO
    }
    //TODO
};
struct TcpStatistics { //0xC8 bytes
    TcpStatistics() {
        //TODO
    }
    //TODO
};
struct WorldClockStatistics { //0xB0 bytes
    WorldClockStatistics() {
        //TODO
    }
    //TODO
};
struct LanExerciseDeviceStatistics { //0x110 bytes
    LanExerciseDeviceStatistics() {
        //TODO
    }
    //TODO
};
struct AuxiliaryControllerStatistics { //0xB8 bytes
    AuxiliaryControllerStatistics() {
        //TODO
    }
    //TODO
};
struct WorldAttributeStatistics { //0x90 bytes
    WorldAttributeStatistics() {
        //TODO
    }
    //TODO
};
struct LanExerciseDeviceService { //0x3B0 bytes
    LanExerciseDeviceService(const std::string &zaVersion, uint16_t, int, int, int, int) {
        //TODO
    }
    //TODO
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
        return (cfg.nodes_size() && !foundCurrentNodeAsFirstNode) || !currentHostIsStillGood;
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
            if (m_obj.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
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
struct TcpClient {
    struct SegmentSubscription {
        std::promise<NetworkResponse<protobuf::SegmentResults> *> *m_promise = nullptr;
        bool m_hasValue = false;
        SegmentSubscription(boost::asio::io_context *ctx, std::promise<NetworkResponse<protobuf::SegmentResults> *> *promise) : m_promise(promise) {
            //TODO Android:
            /*  this_ = this;
            *this = *(_OWORD *)promise;
            *(_QWORD *)promise = 0LL;
            *(_QWORD *)(promise + 8) = 0LL;
            v5 = *ctx;
            v9 = &`typeinfo for'asio::detail::typeid_wrapper<asio::detail::deadline_timer_service<asio::detail::chrono_time_traits<std::chrono::steady_clock,asio::wait_traits<std::chrono::steady_clock>>>>;
                v10 = 0LL;
            v6 = asio::detail::service_registry::do_use_service(
                v5,
                (__int64 *)&v9,
                (__int64(__fastcall *)(__int64))asio::detail::service_registry::create<asio::detail::deadline_timer_service<asio::detail::chrono_time_traits<std::chrono::steady_clock, asio::wait_traits<std::chrono::steady_clock>>>, asio::io_context>,
                (__int64)ctx);
            *((_QWORD *)this_ + 10) = ctx;
            *((_QWORD *)this_ + 5) = 0LL;
            *((_QWORD *)this_ + 6) = 0LL;
            *((_QWORD *)this_ + 8) = 0LL;
            *((_QWORD *)this_ + 9) = 0LL;
            *((_QWORD *)this_ + 7) = -1LL;
            *((_DWORD *)this_ + 23) = 0;
            *((_QWORD *)this_ + 12) = ZZN4asio9execution6detail17any_executor_base16object_fns_tableINS_10io_context19basic_executor_typeINSt6__ndk19allocatorIvEELj0EEEEEPKNS2_10object_fnsEPNS6_9enable_ifIXaantsr7is_sameIT_vEE5valuentsr7is_sameISE_NS6_10shared_ptrIvEEEE5valueEvE4typeEE3fns;
            *((_QWORD *)this_ + 13) = this_ + 5;
            *((_QWORD *)this_ + 14) = ZZN4asio9execution6detail17any_executor_base16target_fns_tableINS_10io_context19basic_executor_typeINSt6__ndk19allocatorIvEELj0EEEEEPKNS2_10target_fnsEbPNS6_9enable_ifIXntsr7is_sameIT_vEE5valueEvE4typeEE16fns_with_execute;
            *((_QWORD *)this_ + 15) = asio::execution::any_executor<asio::execution::context_as_t<asio::execution_context &>, asio::execution::detail::blocking::never_t<0>, asio::execution::prefer_only<asio::execution::detail::blocking::possibly_t<0>>, asio::execution::prefer_only<asio::execution::detail::outstanding_work::tracked_t<0>>, asio::execution::prefer_only<asio::execution::detail::outstanding_work::untracked_t<0>>, asio::execution::prefer_only<asio::execution::detail::relationship::fork_t<0>>, asio::execution::prefer_only<asio::execution::detail::relationship::continuation_t<0>>>::prop_fns_table<asio::io_context::basic_executor_type<std::allocator<void>, 0u>>(void)::fns;
            *((_QWORD *)this_ + 2) = v6;
            *((_QWORD *)this_ + 3) = 0LL;
            *((_BYTE *)this_ + 32) = 0;
            v7 = *ctx;
            v9 = &`typeinfo for'asio::detail::typeid_wrapper<asio::detail::deadline_timer_service<asio::detail::chrono_time_traits<std::chrono::steady_clock,asio::wait_traits<std::chrono::steady_clock>>>>;
                v10 = 0LL;
            result = asio::detail::service_registry::do_use_service(
                v7,
                (__int64 *)&v9,
                (__int64(__fastcall *)(__int64))asio::detail::service_registry::create<asio::detail::deadline_timer_service<asio::detail::chrono_time_traits<std::chrono::steady_clock, asio::wait_traits<std::chrono::steady_clock>>>, asio::io_context>,
                (__int64)ctx);
            *((_QWORD *)this_ + 24) = ctx;
            this_ += 12;
            *((_QWORD *)this_ - 5) = 0LL;
            *((_QWORD *)this_ - 4) = 0LL;
            *((_QWORD *)this_ - 2) = 0LL;
            *((_QWORD *)this_ - 1) = 0LL;
            *((_QWORD *)this_ - 3) = -1LL;
            *((_DWORD *)this_ + 3) = 0;
            *((_QWORD *)this_ + 2) = ZZN4asio9execution6detail17any_executor_base16object_fns_tableINS_10io_context19basic_executor_typeINSt6__ndk19allocatorIvEELj0EEEEEPKNS2_10object_fnsEPNS6_9enable_ifIXaantsr7is_sameIT_vEE5valuentsr7is_sameISE_NS6_10shared_ptrIvEEEE5valueEvE4typeEE3fns;
            *((_QWORD *)this_ + 3) = this_;
            *((_QWORD *)this_ + 4) = ZZN4asio9execution6detail17any_executor_base16target_fns_tableINS_10io_context19basic_executor_typeINSt6__ndk19allocatorIvEELj0EEEEEPKNS2_10target_fnsEbPNS6_9enable_ifIXntsr7is_sameIT_vEE5valueEvE4typeEE16fns_with_execute;
            *((_QWORD *)this_ + 5) = asio::execution::any_executor<asio::execution::context_as_t<asio::execution_context &>, asio::execution::detail::blocking::never_t<0>, asio::execution::prefer_only<asio::execution::detail::blocking::possibly_t<0>>, asio::execution::prefer_only<asio::execution::detail::outstanding_work::tracked_t<0>>, asio::execution::prefer_only<asio::execution::detail::outstanding_work::untracked_t<0>>, asio::execution::prefer_only<asio::execution::detail::relationship::fork_t<0>>, asio::execution::prefer_only<asio::execution::detail::relationship::continuation_t<0>>>::prop_fns_table<asio::io_context::basic_executor_type<std::allocator<void>, 0u>>(void)::fns;
            *((_QWORD *)this_ - 8) = result;
            *((_QWORD *)this_ - 7) = 0LL;
            *((_BYTE *)this_ - 48) = 0;
            *((_QWORD *)this_ + 6) = 0LL;
            *((_QWORD *)this_ + 12) = 0LL;
            *((_BYTE *)this_ + 112) = 0;
            *((_WORD *)this_ + 64) = 0;
            *((_BYTE *)this_ + 130) = 0;
            *((_DWORD *)this_ + 33) = 0;
            *((_BYTE *)this_ + 136) = 0;*/
        }
        void setPromiseValue(uint64_t segment, NetworkResponse<protobuf::SegmentResults> *val) {
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
    moodycamel::ReaderWriterQueue<void * /*TODO*/> m_rwq;
    std::string m_ip;
    EventLoop m_eventLoop;
    int64_t m_worldId = 0, m_port = 0;
    RelayServerRestInvoker *m_relay;
    boost::asio::ip::tcp::socket m_tcpSocket;
    uint32_t m_mapRevision = 0;
    TcpAddressService m_tcpAddressService;
    FutureWaiter<NetworkResponse<protobuf::TcpConfig>> m_tcpConfigWaiter;
    boost::asio::steady_timer m_asioTimer2, m_asioTimer3, m_asioTimer4;
    void segmentSubscriptionsShutdown() {
        //TODO
    }
    void clearEndpoint() {
        m_ip.clear();
        m_port = 0;
        //TODO shared_release((__int64 *)&this->field_262[6], (__int64 *)&v3);
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
        //TODO
    }
    void disconnect() {
        if (!m_tcpSocket.is_open()) {
            NetworkingLogDebug("TCP disconnect socket is not open");
        } else {
            NetworkingLogDebug("TCP disconnect socket is open");
            NetworkingLogDebug("TCP disconnect socket shutdown");
            boost::system::error_code ec;
            m_tcpSocket.shutdown(m_tcpSocket.shutdown_both, ec);
            if (ec /*shutdown(m_tcpSocket, SD_BOTH)*/)
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
    TcpClient(GlobalState *gs, WorldClockService *wcs, HashSeedService *hss, WorldAttributeService *wat, RelayServerRestInvoker *relay, SegmentResultsRestInvoker *segRes, NetworkClientImpl *ncli, int t1 = 35000, int t2 = 5000) : 
        m_rwq(100), m_relay(relay),
        m_tcpSocket(m_eventLoop.m_asioCtx, boost::asio::ip::tcp::v4()), 
        m_tcpConfigWaiter(m_eventLoop.m_asioCtx), m_asioTimer2(m_eventLoop.m_asioCtx),
        m_asioTimer3(m_eventLoop.m_asioCtx), m_asioTimer4(m_eventLoop.m_asioCtx) {
        //TODO
    }
    void onInactivityTimeout(/*std::error_code*/) {
        NetworkingLogWarn("TCP connection timed out owing to inactivity");
        //OMIT m_stat->TcpStatistics::increaseConnectionTimeoutCount();
        reconnect(true);
    }
    void reconnect(bool refreshTcpConfigIfNeeded) {
        NetworkingLogDebug("TCP reconnect refreshTcpConfigIfNeeded: %d ", refreshTcpConfigIfNeeded);
        disconnect();
        waitDisconnection(refreshTcpConfigIfNeeded);
    }
        /*
TcpClient::Listener::~Listener()
TcpClient::connect(void)
TcpClient::decodeMessage(char *,ulong &)
TcpClient::encodeMessage(char *,ulong,uint &)
TcpClient::getClientToServerForHelloMessage(ulong,long)
TcpClient::getMaximumHelloMessageSize(void)
TcpClient::getTcpMessageSize(uint)
TcpClient::handleCommunicationError(std::error_code,std::string const&)
TcpClient::handleTcpConfigChanged(zwift::protobuf::TcpConfig const&)
TcpClient::onTcpConfigReceived(std::shared_ptr<zwift_network::NetworkResponse<zwift::protobuf::TcpConfig> const> const&)
TcpClient::popServerToClient(std::shared_ptr<zwift::protobuf::ServerToClient const> &)
TcpClient::processPayload(ulong)
TcpClient::processSegmentSubscription(long,std::shared_ptr<std::promise<std::shared_ptr<zwift_network::NetworkResponse<zwift::protobuf::SegmentResults> const>>>)
TcpClient::processSegmentUnsubscription(long)
TcpClient::processSubscribedSegment(zwift::protobuf::ServerToClient const&)
TcpClient::readHeader(void)
TcpClient::readPayload(uint)
TcpClient::resetTimeoutTimer(void)
TcpClient::sayHello(void)
TcpClient::sendClientToServer(TcpCommand,zwift::protobuf::ClientToServer &,std::function<void ()(void)> const&,std::function<void ()(void)> const&)
TcpClient::sendSubscribeToSegment(long,std::shared_ptr<TcpClient::SegmentSubscription> const&)
TcpClient::serializeToTcpMessage(TcpCommand,zwift::protobuf::ClientToServer const&,std::array<char,1492ul> &,uint &)
TcpClient::subscribeToSegmentAndGetLeaderboard(long)
TcpClient::unsubscribeFromSegment(long)
TcpClient::~TcpClient()    */
};
struct AuxiliaryController : public WorldIdListener {
    void handleWorldIdChange(int64_t worldId) override {
        //TODO
    }
    //TODO
};
struct NetworkClientImpl { //0x400 bytes, calloc
    std::string m_server;
    MachineIdProviderFactory m_machine;
    CurlHttpConnectionFactory m_curlf;
    NetworkClientOptions m_nco{ false, 20, 300, /*NL_INFO RTL*/ NL_DEBUG };
    time_t m_netStartTime1 = 0, m_netStartTime2 = 0;
    GenericHttpConnectionManager *m_httpConnMgr0 = nullptr, *m_httpConnMgr1 = nullptr, *m_httpConnMgr2 = nullptr;
    ZwiftHttpConnectionManager *m_httpConnMgr3 = nullptr, *m_httpConnMgr4 = nullptr;
    ZwiftAuthenticationManager *m_authMgr = nullptr;
    AuthServerRestInvoker *m_authInvoker = nullptr;
    GlobalState *m_globalState = nullptr;
    HashSeedService *m_hashSeed1 = nullptr, *m_hashSeed2 = nullptr;
    WorldAttributeService *m_wat = nullptr;
    ProfileRequestDebouncer *m_profRqDebouncer = nullptr;
    UdpClient *m_udpClient = nullptr;
    TcpClient *m_tcpClient = nullptr;
    WorldClockService *m_wclock = nullptr;
    RelayServerRestInvoker *m_relay = nullptr;
    EventLoop *m_evLoop = nullptr;
    NetworkClockService *m_netClock = nullptr;
    UdpStatistics *m_udpStat = nullptr;
    RestServerRestInvoker *m_restInvoker = nullptr;
    ExperimentsRestInvoker *m_expRi = nullptr;
    ActivityRecommendationRestInvoker *m_arRi = nullptr;
    AchievementsRestInvoker *m_achRi = nullptr;
    CampaignRestInvoker *m_camRi = nullptr;
    ClubsRestInvoker *m_clubsRi = nullptr;
    EventCoreRestInvoker *m_ecRi = nullptr;
    EventFeedRestInvoker *m_efRi = nullptr;
    FirmwareUpdateRestInvoker *m_fuRi = nullptr;
    GenericRestInvoker *m_gRi = nullptr;
    PrivateEventsRestInvoker *m_peRi = nullptr;
    RaceResultRestInvoker *m_rarRi = nullptr;
    RouteResultsRestInvoker *m_rorRi = nullptr;
    PlayerPlaybackRestInvoker *m_ppbRi = nullptr;
    SegmentResultsRestInvoker *m_srRi = nullptr;
    PowerCurveRestInvoker *m_pcRi = nullptr;
    ZFileRestInvoker *m_zfRi = nullptr;
    ZwiftWorkoutsRestInvoker *m_zwRi = nullptr;
    WorkoutServiceRestInvoker *m_wsRi = nullptr;
    TcpStatistics *m_tcpStat = nullptr;
    WorldClockStatistics *m_wcStat = nullptr;
    LanExerciseDeviceStatistics *m_lanStat = nullptr;
    AuxiliaryControllerStatistics *m_auxStat = nullptr;
    WorldAttributeStatistics *m_waStat = nullptr;
    LanExerciseDeviceService *m_lanService = nullptr;
    AuxiliaryController *m_aux = nullptr;
    moodycamel::ReaderWriterQueue<const AuxiliaryControllerAddress> m_rwqAux;
    AuxiliaryControllerAddress m_curAux;
    uint32_t m_field_10 = 100;
    bool m_tcpDisconnected = false, m_initOK = false, m_loginOK = false;
    NetworkClientImpl() : m_rwqAux(1) { //QUEST: why two vtables
        google::protobuf::internal::VerifyVersion(3021000 /* URSOFT FIX: slightly up from 3020000*/, 3020000, __FILE__);
    }
    void startTcpClient() {
        if (!m_tcpClient && !m_nco.m_bHttpOnly)
            m_tcpClient = new TcpClient(m_globalState, m_wclock, m_hashSeed1, m_wat, m_relay, m_srRi, this);
    }
    void shutdownTcpClient() {
        if (m_tcpClient) {
            m_tcpClient->shutdown();
            m_tcpClient = nullptr;
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
        //TODO IDA NetworkClientImpl_destroy
    }
    void shutdownUdpClient() {
        if (m_udpClient) {
            m_wat->removeListener(m_udpClient);
            m_udpClient->shutdown();
            FreeAndNil(m_udpClient);
        }
    }
    void shutdownAuxiliaryController() {
        //TODO
    }
    void LogStart() {
        NetworkingLogInfo("CNL %s", g_CNL_VER.c_str());
        NetworkingLogInfo("Machine Id: %s", m_machine.m_id.c_str());
    }
    void initialize(const std::string &serv, const std::string &certs, const std::string &zaVersion) {
        m_server = serv;
        //SetNetworkMaxLogLevel(maxLogLevel); combined with g_MinLogLevel (why two???)
        m_curlf.initialize(zaVersion, m_machine.m_id);
        _time64(&m_netStartTime1);
        m_netStartTime2 = m_netStartTime1;
        LogStart();
        bool skipCertCheck = m_nco.m_skipCertCheck;
        //smart shared pointers actively used here, but what for???
        m_httpConnMgr0 = new GenericHttpConnectionManager(&m_curlf, certs, skipCertCheck, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_SEQUENTIAL);
        m_httpConnMgr1 = new GenericHttpConnectionManager(&m_curlf, certs, skipCertCheck, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_CONCURRENT);
        m_httpConnMgr2 = new GenericHttpConnectionManager(&m_curlf, certs, skipCertCheck, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_SEQUENTIAL);
        m_authMgr = new ZwiftAuthenticationManager(m_server);
        m_httpConnMgr3 = new ZwiftHttpConnectionManager(&m_curlf, certs, m_nco.m_skipCertCheck, m_authMgr, &m_tcpDisconnected, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_SEQUENTIAL, 3);
        m_httpConnMgr4 = new ZwiftHttpConnectionManager(&m_curlf, certs, m_nco.m_skipCertCheck, m_authMgr, &m_tcpDisconnected, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_CONCURRENT, 3);
        m_expRi = new ExperimentsRestInvoker(m_httpConnMgr3, m_server);
        m_arRi = new ActivityRecommendationRestInvoker(m_httpConnMgr4, m_server);
        m_achRi = new AchievementsRestInvoker(m_httpConnMgr4, m_server);
        m_authInvoker = new AuthServerRestInvoker(m_machine.m_id, m_authMgr, m_httpConnMgr3, m_expRi, m_server);
        m_camRi = new CampaignRestInvoker(m_server);
        m_clubsRi = new ClubsRestInvoker(m_httpConnMgr3, m_server);
        m_ecRi = new EventCoreRestInvoker(m_httpConnMgr3, m_server);
        m_efRi = new EventFeedRestInvoker(m_httpConnMgr4, m_server);
        m_fuRi = new FirmwareUpdateRestInvoker(m_httpConnMgr3, m_server);
        m_gRi = new GenericRestInvoker(m_httpConnMgr1);
        m_peRi = new PrivateEventsRestInvoker(m_httpConnMgr3, m_server);
        m_rarRi = new RaceResultRestInvoker(m_httpConnMgr3, m_server);
        m_rorRi = new RouteResultsRestInvoker(m_httpConnMgr3, m_server);
        m_ppbRi = new PlayerPlaybackRestInvoker(m_httpConnMgr4, m_server);
        m_srRi = new SegmentResultsRestInvoker(m_httpConnMgr4, m_server);
        m_pcRi = new PowerCurveRestInvoker(m_httpConnMgr4, m_server);
        m_zfRi = new ZFileRestInvoker(m_httpConnMgr3, m_server);
        m_zwRi = new ZwiftWorkoutsRestInvoker(m_httpConnMgr3, m_server);
        m_wsRi = new WorkoutServiceRestInvoker(m_httpConnMgr4, m_server);
        m_udpStat = new UdpStatistics();
        m_tcpStat = new TcpStatistics();
        m_wcStat = new WorldClockStatistics();
        m_lanStat = new LanExerciseDeviceStatistics();
        m_auxStat = new AuxiliaryControllerStatistics();
        m_waStat = new WorldAttributeStatistics();
        //OMIT telemetry
        m_restInvoker = new RestServerRestInvoker(m_machine.m_id, m_httpConnMgr3, m_server, zaVersion);
        m_lanService = new LanExerciseDeviceService(zaVersion, 0x14E9, 1, 100, 5, 30);
        NetworkingLogInfo("CNL initialized");
        m_initOK = true;
    }
    void onLoggedIn(const protobuf::PerSessionInfo &psi, const std::string &sessionId, const EncryptionInfo &ei) {
        m_netClock = new NetworkClockService(psi.time());
        m_loginOK = false;
        shutdownUdpClient();
        shutdownServiceEventLoop();
        m_evLoop = new EventLoop();
        m_globalState = new GlobalState(m_evLoop, psi, sessionId, ei);
        m_httpConnMgr3->setGlobalState(m_globalState);
        //OMIT TelemetryService::setGlobalState(this->m_ts, &v88);
        m_relay = new RelayServerRestInvoker(m_httpConnMgr3, HttpHelper::sanitizeUrl(psi.relay_url()));
        m_wat = new WorldAttributeService();
        m_wclock = new WorldClockService(m_evLoop, m_netClock);
        m_hashSeed1 = new HashSeedService(m_evLoop, m_globalState, m_relay, m_wclock, false);
        m_hashSeed1->start();
        m_hashSeed2 = new HashSeedService(m_evLoop, m_globalState, m_relay, m_wclock, true);
        m_hashSeed2->start();
        m_profRqDebouncer = new ProfileRequestDebouncer(m_evLoop, m_restInvoker, m_field_10);
        if (!m_nco.m_bHttpOnly) {
            m_udpClient = new UdpClient(m_globalState, m_wclock, m_hashSeed1, m_hashSeed2, m_udpStat, m_relay, this);
            m_globalState->registerUdpConfigListener(m_udpClient);
            m_globalState->registerEncryptionListener(m_udpClient);
            m_wat->registerListener(m_udpClient);
        }
        NetworkingLogInfo("Session ID: %s%s", sessionId.c_str(), m_globalState->shouldUseEncryption() ? " (secure)" : "");
        NetworkingLogInfo("Logged in");
        m_loginOK = true;
        //OMIT TelemetryService::remoteLogF(*(_DWORD **)v49, 0, 3u, "session", "Session ID %s", v58->_Bx._Buf);
    }
    std::future<NetworkResponse<std::string>> logInWithOauth2Credentials(/*ret a2,*/ const std::string &sOauth, const std::vector<std::string> &anEventProps, const std::string &oauthClient) {
        if (m_authMgr && m_authInvoker) {
            auto ret = m_authMgr->setCredentialsMid(sOauth, oauthClient);
            return ret.m_errCode ? makeNetworkResponseFuture<std::string>(ret.m_errCode, std::move(ret.m_msg)) :
                m_authInvoker->logIn({ m_nco.m_disableEncr, m_nco.m_disableEncryptionWithServer, m_nco.m_ignoreEncryptionFeatureFlag, m_nco.m_secretKeyBase64 },
                    anEventProps, 
                    [this](const protobuf::PerSessionInfo &psi, const std::string &str, const EncryptionInfo &ei) { onLoggedIn(psi, str, ei); });
        }
        return makeNetworkResponseFuture<std::string>(NRO_NOT_INITIALIZED, "Initialize CNL first"s);
    }
    std::future<NetworkResponse<std::string>> logInWithEmailAndPassword(const std::string &email, const std::string &pwd, const std::vector<std::string> &anEventProps, bool reserved, const std::string &oauthClient) {
        if (m_authMgr && m_authInvoker) {
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
        return makeNetworkResponseFuture<std::string>(NRO_NOT_INITIALIZED, "Initialize CNL first"s);
    }
    std::future<NetworkResponse<std::string>> logOut() {
        if (m_initOK) {
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
                shutdownUdpClient();
                shutdownAuxiliaryController();
                shutdownServiceEventLoop();
                FreeAndNil(m_globalState);
                //omit m_ts->clearUserState();
                FreeAndNil(m_netClock);
                FreeAndNil(m_wat);
                FreeAndNil(m_wclock);
                FreeAndNil(m_hashSeed1);
                FreeAndNil(m_hashSeed2);
                FreeAndNil(m_profRqDebouncer);
                FreeAndNil(m_relay);
                NetworkingLogInfo("Logged out");
                });
        }
        return makeNetworkResponseFuture<std::string>(NRO_NOT_INITIALIZED, "Initialize CNL first"s);
    }
    void shutdownServiceEventLoop() {
        if (m_evLoop) {
            m_evLoop->shutdown();
            m_evLoop = nullptr;
        }
    }
    std::future<NetworkResponse<void>> resetPassword(const std::string &newPwd) {
        if (m_initOK)
            return m_authInvoker->resetPassword(newPwd);
        return makeNetworkResponseFuture<void>(NRO_NOT_INITIALIZED, "Initialize CNL first"s);
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
};
NetworkClient::NetworkClient() { m_pImpl = new(calloc(sizeof(NetworkClientImpl), 1)) NetworkClientImpl; }
NetworkClient::~NetworkClient() { m_pImpl->~NetworkClientImpl(); free(m_pImpl); }
void NetworkClient::globalInitialize() { curl_global_init(CURL_GLOBAL_ALL); }
void NetworkClient::globalCleanup() { curl_global_cleanup(); }
void NetworkClient::initialize(const std::string &server, const std::string &certs, const std::string &version) {
    m_pImpl->initialize(server, certs, version);
}
std::future<NetworkResponse<std::string>> NetworkClient::logOut() { return m_pImpl->logOut(); }
std::future<NetworkResponse<void>> NetworkClient::resetPassword(const std::string &newPwd) { return m_pImpl->resetPassword(newPwd); }
namespace zwift_network {
void get_goals(int64_t playerId) { 
    //TODO
}
void save_goal(const protobuf::Goal &g) {
    //TODO
}
std::future<NetworkResponse<std::string>> log_out() { return g_networkClient->logOut(); }
std::future<NetworkResponse<void>> reset_password(const std::string &newPwd) { return g_networkClient->resetPassword(newPwd); }
std::future<NetworkResponse<protobuf::PlayerState>> latest_player_state(int64_t worldId, int64_t playerId) {
    return g_networkClient->latestPlayerState(worldId, playerId); 
}
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
    if (g_networkClient->m_pImpl && g_networkClient->m_pImpl->m_authMgr) {
        return g_networkClient->m_pImpl->m_authMgr->getLoggedIn();
    }
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
std::future<NetworkResponse<std::string>> NetworkClient::logInWithOauth2Credentials(const std::string &sOauth, const std::vector<std::string> &anEventProps, const std::string &oauthClient) { return m_pImpl->logInWithOauth2Credentials(sOauth, anEventProps, oauthClient); }
std::future<NetworkResponse<std::string>> NetworkClient::logInWithEmailAndPassword(const std::string &email, const std::string &pwd, const std::vector<std::string> &anEventProps, bool reserved, const std::string &oauthClient) { return m_pImpl->logInWithEmailAndPassword(email, pwd, anEventProps, reserved, oauthClient); }
std::future<NetworkResponse<protobuf::PlayerState>> NetworkClient::latestPlayerState(int64_t worldId, int64_t playerId) {
    return m_pImpl->latestPlayerState(worldId, playerId);
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

//Units
TEST(SmokeTest, JsonWebToken) {
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
TEST(SmokeTest, EventLoopTest) {
    google::protobuf::internal::VerifyVersion(3021000, 3020000, __FILE__);
    EventLoop el;
    //Sleep(100);
    //el.shutdown();
    //el.enqueueShutdown();
}
TEST(SmokeTest, DISABLED_LoginTestPwd) {
    g_MainThread = GetCurrentThreadId();
    std::vector<std::string> v{ "OS"s, "Windows"s };
    {
        NetworkClient tmp;
        auto ret0 = tmp.logInWithEmailAndPassword(""s, ""s, v, false, "Game_Launcher"s);
        EXPECT_TRUE(ret0.valid());
        EXPECT_EQ(std::future_status::ready, ret0.wait_for(std::chrono::seconds(0)));
        auto r0 = ret0.get();
        EXPECT_EQ(2, r0.m_errCode) << r0.m_msg;
        EXPECT_EQ("Initialize CNL first"s, r0.m_msg);
    }
    ZNETWORK_Initialize();
    ZMUTEX_SystemInitialize();
    LogInitialize();
    EXPECT_FALSE(ZNETWORK_IsLoggedIn());
    auto ret2 = zwift_network::log_out();
    EXPECT_TRUE(ret2.valid());
    EXPECT_EQ(std::future_status::ready, ret2.wait_for(std::chrono::seconds(0)));
    auto r2 = ret2.get();
    EXPECT_EQ(3, (int)r2.m_errCode) << r2.m_msg;
    EXPECT_EQ("Not logged in"s, r2.m_msg);
    auto ret1 = g_networkClient->logInWithEmailAndPassword(""s, ""s, v, true, "Game_Launcher"s);
    EXPECT_TRUE(ret1.valid());
    EXPECT_EQ(std::future_status::ready, ret1.wait_for(std::chrono::seconds(0)));
    auto r1 = ret1.get();
    EXPECT_EQ(4, (int)r1.m_errCode) << r1.m_msg;
    EXPECT_EQ("Good luck, soldier"s, r1.m_msg);
    auto ret = g_networkClient->logInWithEmailAndPassword("olyen2007@gmail.com"s, "123"s, v, false, "Game_Launcher"s);
    EXPECT_TRUE(ret.valid());
    while (!ZNETWORK_IsLoggedIn())
        Sleep(100);
    EXPECT_TRUE(ret.valid());
    EXPECT_EQ(std::future_status::ready, ret.wait_for(std::chrono::seconds(0)));
    auto r = ret.get();
    EXPECT_EQ(0, (int)r.m_errCode) << r.m_msg;
    //zoffline has no fantasy
    EXPECT_EQ("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjgiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiUmVmcmVzaCIsImF6cCI6IkdhbWVfTGF1bmNoZXIiLCJhdXRoX3RpbWUiOjAsInNlc3Npb25fc3RhdGUiOiIwODQ2bm85bi03NjVxLTRwM3MtbjIwcC02cG5wOXI4NnI1czMiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZXZlcnlib2R5IiwidHJpYWwtc3Vic2NyaWJlciIsImV2ZXJ5b25lIiwiYmV0YS10ZXN0ZXIiXX0sInJlc291cmNlX2FjY2VzcyI6eyJteS16d2lmdCI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIkdhbWVfTGF1bmNoZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIiwiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBaZW5kZXNrIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiWndpZnQgUmVsYXkgUkVTVCBBUEkgLS0gcHJvZHVjdGlvbiI6eyJyb2xlcyI6WyJhdXRob3JpemVkLXBsYXllciJdfSwiZWNvbS1zZXJ2ZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJhY2NvdW50Ijp7InJvbGVzIjpbIm1hbmFnZS1hY2NvdW50IiwibWFuYWdlLWFjY291bnQtbGlua3MiLCJ2aWV3LXByb2ZpbGUiXX19LCJzZXNzaW9uX2Nvb2tpZSI6IjV8ODU0Mjc1MjBmOTQ5MjQxMThkZjc1YjhhNDc5ZmIzN2ZlY2I3MjEwODU1YzFkYTgxOWE1Yzk4Y2U1Yjk1OTVjZTVmNzgyOGYwNzMwMzQ4YmM4MmRhY2U0ZjM3NWU4OGUxNTUxZjg1ZWEzY2FkMjM3M2RmMDRhYzNlMGJjOTg4YzYifQ.ZB87s5lo7jde09q9IKsFCWc22U65kroBYz0aVb46Lmk"s, r.m_msg);
    ret = zwift_network::log_out();
    EXPECT_TRUE(ret.valid());
    while (ZNETWORK_IsLoggedIn())
        Sleep(100);
    EXPECT_TRUE(ret.valid());
    EXPECT_EQ(std::future_status::ready, ret.wait_for(std::chrono::seconds(0)));
    r = ret.get();
    EXPECT_EQ(0, r.m_errCode) << r.m_msg;
    EXPECT_EQ(""s, r.m_msg);
}
TEST(SmokeTest, DISABLED_LoginTestToken) {
    auto rt = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjgiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiUmVmcmVzaCIsImF6cCI6IkdhbWVfTGF1bmNoZXIiLCJhdXRoX3RpbWUiOjAsInNlc3Npb25fc3RhdGUiOiIwODQ2bm85bi03NjVxLTRwM3MtbjIwcC02cG5wOXI4NnI1czMiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZXZlcnlib2R5IiwidHJpYWwtc3Vic2NyaWJlciIsImV2ZXJ5b25lIiwiYmV0YS10ZXN0ZXIiXX0sInJlc291cmNlX2FjY2VzcyI6eyJteS16d2lmdCI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIkdhbWVfTGF1bmNoZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIiwiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBaZW5kZXNrIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiWndpZnQgUmVsYXkgUkVTVCBBUEkgLS0gcHJvZHVjdGlvbiI6eyJyb2xlcyI6WyJhdXRob3JpemVkLXBsYXllciJdfSwiZWNvbS1zZXJ2ZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJhY2NvdW50Ijp7InJvbGVzIjpbIm1hbmFnZS1hY2NvdW50IiwibWFuYWdlLWFjY291bnQtbGlua3MiLCJ2aWV3LXByb2ZpbGUiXX19LCJzZXNzaW9uX2Nvb2tpZSI6IjZ8YTJjNWM1MWY5ZDA4YzY4NWUyMDRlNzkyOWU0ZmMyMDAyOWI5ODE1OGYwYjdmNzk0MmZiMmYyMzkwYWMzNjExMDMzN2E3YTQyYjVlNTcwNmVhODM0YjQzYzFlNDU1NzJkMTQ2MzIwMTQxOWU5NzZjNTkzZWZjZjE0M2UwNWNiZjgifQ.5e1X1imPlVfXfhDHE_OGmG9CNGvz7hpPYPXcNkPJ5lw"s;
    auto token = "{\"access_token\":\"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjkiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiQmVhcmVyIiwiYXpwIjoiR2FtZV9MYXVuY2hlciIsImF1dGhfdGltZSI6MTUzNTUwNzI0OSwic2Vzc2lvbl9zdGF0ZSI6IjA4NDZubzluLTc2NXEtNHAzcy1uMjBwLTZwbnA5cjg2cjVzMyIsImFjciI6IjAiLCJhbGxvd2VkLW9yaWdpbnMiOlsiaHR0cHM6Ly9sYXVuY2hlci56d2lmdC5jb20qIiwiaHR0cDovL3p3aWZ0Il0sInJlYWxtX2FjY2VzcyI6eyJyb2xlcyI6WyJldmVyeWJvZHkiLCJ0cmlhbC1zdWJzY3JpYmVyIiwiZXZlcnlvbmUiLCJiZXRhLXRlc3RlciJdfSwicmVzb3VyY2VfYWNjZXNzIjp7Im15LXp3aWZ0Ijp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiR2FtZV9MYXVuY2hlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFJFU1QgQVBJIC0tIHByb2R1Y3Rpb24iOnsicm9sZXMiOlsiYXV0aG9yaXplZC1wbGF5ZXIiLCJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFplbmRlc2siOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSZWxheSBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIl19LCJlY29tLXNlcnZlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sImFjY291bnQiOnsicm9sZXMiOlsibWFuYWdlLWFjY291bnQiLCJtYW5hZ2UtYWNjb3VudC1saW5rcyIsInZpZXctcHJvZmlsZSJdfX0sIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20iLCJzZXNzaW9uX2Nvb2tpZSI6IjZ8YTJjNWM1MWY5ZDA4YzY4NWUyMDRlNzkyOWU0ZmMyMDAyOWI5ODE1OGYwYjdmNzk0MmZiMmYyMzkwYWMzNjExMDMzN2E3YTQyYjVlNTcwNmVhODM0YjQzYzFlNDU1NzJkMTQ2MzIwMTQxOWU5NzZjNTkzZWZjZjE0M2UwNWNiZjgifQ._kPfXO8MdM7j0meG4MVzprSa-3pdQqKyzYMHm4d494w\",\"expires_in\":1000021600,\"id_token\":\"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjciLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiSUQiLCJhenAiOiJHYW1lX0xhdW5jaGVyIiwiYXV0aF90aW1lIjoxNTM1NTA3MjQ5LCJzZXNzaW9uX3N0YXRlIjoiMDg0Nm5vOW4tNzY1cS00cDNzLW4yMHAtNnBucDlyODZyNXMzIiwiYWNyIjoiMCIsIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20ifQ.rWGSvv5TFO-i6LKczHNUUcB87Hfd5ow9IMG9O5EGR4Y\",\"not-before-policy\":1408478984,\"refresh_expires_in\":611975560,\"refresh_token\":\""s + rt + "\",\"scope\":\"\",\"session_state\":\"0846ab9a-765d-4c3f-a20c-6cac9e86e5f3\",\"token_type\":\"bearer\"}"s;
    g_MainThread = GetCurrentThreadId();
    ZNETWORK_Initialize();
    ZMUTEX_SystemInitialize();
    LogInitialize();
    EXPECT_FALSE(ZNETWORK_IsLoggedIn());
    std::vector<std::string> v{"OS"s, "Windows"s};
    auto ret = g_networkClient->logInWithOauth2Credentials(token, v, "Game_Launcher"s);
    EXPECT_TRUE(ret.valid());
    while(!ZNETWORK_IsLoggedIn())
        Sleep(100);
    EXPECT_TRUE(ret.valid());
    EXPECT_EQ(std::future_status::ready, ret.wait_for(std::chrono::seconds(0)));
    auto r = ret.get();
    EXPECT_EQ(0, r.m_errCode) << r.m_msg;
    EXPECT_EQ(rt, r.m_msg);
}
TEST(SmokeTest, B64) {
    base64 obj;
    uint8_t rawData[] = { "\x1\x2\x3-lorem\0ipsum" };
    obj.encode(rawData, _countof(rawData));
    EXPECT_STREQ("AQIDLWxvcmVtAGlwc3VtAA==", obj.c_str());
    std::string b64copy(obj);
    obj.decode(b64copy);
    EXPECT_EQ(obj.size(), _countof(rawData));
    EXPECT_EQ(0, memcmp(rawData, obj.c_str(), _countof(rawData)));
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