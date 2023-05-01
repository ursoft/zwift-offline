#include "ZwiftApp.h"
#include "readerwriterqueue/readerwriterqueue.h"
#include "concurrentqueue/concurrentqueue.h"
#include "openssl/md5.h"
#include "xxhash.h"
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
    template<class T>
    void addOptional(const std::string &name, const Optional<T> &val) {
        if (val.m_hasValue)
            add(name, val.m_T);
    }
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
        //OMIT NumericStatisticsPeriodicLogger<int64_t> ctr
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
    uint64_t getHashSeed(uint64_t a3) {
        if (a3) {
            std::lock_guard l(m_mutex);
            if (!m_isAi || !m_map.empty() || m_fwHashSeeds.m_inWait /*not sure*/) {
                auto f = m_map.find(a3);
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
    NetworkRequestOutcome signMessage(char *buf, uint32_t len, uint64_t a4) {
        auto hashSeed = getHashSeed(a4);
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
    std::future<NetworkResponse<int64_t>> createActivityRideOn(int64_t profileId, int64_t playerId) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<int64_t>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(m_url);
            url += "/api/profiles/"s + std::to_string(playerId) + "/activities/0/rideon"s;
            Json::Value json;
            json["profileId"] = profileId;
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
                return model::EventSignupResponse{
                    rx_json["riderStartTime"].asString(),
                    rx_json["signUpStatus"].asInt(),
                    rx_json["riderSlot"].asInt(),
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
    std::future<NetworkResponse<int64_t>> saveActivity(const protobuf::Activity &act, bool uploadToStrava, const std::string &fitPath, const std::function<void(const NetworkResponse<int64_t> &)> &func) {
        return m_mgr->pushRequestTask(std::function<NetworkResponse<int64_t>(CurlHttpConnection *)>([=](CurlHttpConnection *conn) {
            std::string url(this->m_url);
            url += "/api/profiles/"s + std::to_string(act.player_id()) + "/activities"s;
            auto pAct = &act;
            std::unique_ptr<protobuf::Activity> actMutable;
            if (act.has_id())
                url += "/"s + std::to_string(act.id());
            if (uploadToStrava)
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
    /* OMIT:
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
RestServerRestInvoker::partnerAccessToken(const std::string &) //OMIT
RestServerRestInvoker::profileByEmail(const std::string &)
RestServerRestInvoker::redeemCoupon(const std::string &)
RestServerRestInvoker::removeFollower(int64_t,int64_t)
RestServerRestInvoker::removeRegistrationForEvent(int64_t)
RestServerRestInvoker::resumeSubscription()
RestServerRestInvoker::saveProfileReminders(protobuf::PlayerProfile const&)
RestServerRestInvoker::sendMixpanelEvent(const std::string &,std::vector<std::string> const&) //OMIT
RestServerRestInvoker::signUrls(const std::string &,std::vector<std::string> const&)
RestServerRestInvoker::updateFollower(int64_t,int64_t,bool,protobuf::ProfileFollowStatus)
RestServerRestInvoker::updateNotificationReadStatus(int64_t,int64_t,bool)
RestServerRestInvoker::uploadReceipt(int64_t,protobuf::InAppPurchaseReceipt &)
RestServerRestInvoker::~RestServerRestInvoker()*/
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
    void handleWorldAttribute(const protobuf::WorldAttribute &wa) override {
        //TODO
    }
    void shutdown() {
        //TODO
    }
    /*
UdpClient::clearEndpoint()
UdpClient::connect()
UdpClient::decodeMessage(char const*,uint64_t &)
UdpClient::disconnect()
UdpClient::disconnectionRequested(protobuf::ServerToClient const&)
UdpClient::encodeMessage(char *,uint64_t,uint32_t &)
UdpClient::getExpungeReasonToBeInformed()
UdpClient::handleFanViewedPlayerChanges(uint64_t,protobuf::PlayerState const&)
UdpClient::handlePlayerProfileUpdates(uint64_t,protobuf::PlayerState const&)
UdpClient::handlePositionChanges(protobuf::PlayerState const&)
UdpClient::handleRelayAddress(uint64_t)
UdpClient::handleServerToClient(std::shared_ptr<protobuf::ServerToClient const>)
UdpClient::handleWorldAndMapRevisionChanges(uint64_t,int64_t,uint32_t)
UdpClient::handleWorldAttribute(protobuf::WorldAttribute const&)
UdpClient::isNoWorld(protobuf::ServerToClient const&)
UdpClient::mapExpungeReasonResponse(protobuf::ExpungeReason)
UdpClient::onConnectionTimeout(std::error_code)
UdpClient::popPlayerIdWithUpdatedProfile(int64_t &)
UdpClient::popServerToClient(std::shared_ptr<protobuf::ServerToClient const> &)
UdpClient::processDatagram(uint64_t)
UdpClient::receive()
UdpClient::receivedExpungeReason(protobuf::ServerToClient const&)
UdpClient::reconnect(bool)
UdpClient::reset()
UdpClient::resetConnectionTimeoutTimer(std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>)
UdpClient::resetGameInterpolationTimeoutTimer()
UdpClient::resetMetricsLogTimer()
UdpClient::resolveAndSetEndpoint(const std::string &,ushort)
UdpClient::selectHostAndPorts(uint64_t)
UdpClient::sendClientToServer(protobuf::ClientToServer const&,uint32_t)
UdpClient::sendDisconnectedClientToServer(int64_t)
UdpClient::sendPlayerState(int64_t,protobuf::PlayerState const&)
UdpClient::sendRegularClientToServer(int64_t,uint32_t,protobuf::PlayerState const&)
UdpClient::serializeToUdpMessage(protobuf::ClientToServer const&,uint32_t &)
UdpClient::setPlayerProfileUpdated(uint64_t)
UdpClient::shouldPrependDontForwardByte(protobuf::ClientToServer const&)
UdpClient::shouldPrependVoronoiOrDieByte(protobuf::ClientToServer const&)
UdpClient::~UdpClient()    */
};
struct WorldAttributeService { //0x270 bytes
    moodycamel::ReaderWriterQueue<protobuf::WorldAttribute> m_rwq;
    ServiceListeners<WorldAttributeServiceListener> m_lis;
    uint64_t m_largestTime;
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
    uint64_t getLargestWorldAttributeTimestamp() { return m_largestTime; }
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
struct ProfileRequestDebouncer { //0x1E0 bytes
    ProfileRequestDebouncer(EventLoop *, RestServerRestInvoker *, uint32_t) {
        //TODO
    }
/*
ProfileRequestDebouncer::addRequest(int64_t const&)
ProfileRequestDebouncer::getContextPromises(uint32_t)
ProfileRequestDebouncer::getPlayerIds(uint32_t)
ProfileRequestDebouncer::onProfilesReceived(uint32_t,std::shared_ptr<zwift_network::NetworkResponse<protobuf::PlayerProfiles> const> const&)
ProfileRequestDebouncer::requestProfiles()
ProfileRequestDebouncer::returnProfilePromises(std::unordered_multimap<int64_t,std::promise<std::shared_ptr<zwift_network::NetworkResponse<protobuf::PlayerProfile> const>>> &)
ProfileRequestDebouncer::startDebouncePeriod()
ProfileRequestDebouncer::~ProfileRequestDebouncer()*/
};
struct NetworkClockService { //0x18 bytes
    uint64_t m_localCreTime, m_netCreTime;
    NetworkClockService(uint64_t netTime) : m_localCreTime(g_steadyClock.now()), m_netCreTime(netTime) {}
    uint64_t getNetworkTimeInSeconds() {
        return (g_steadyClock.now() - m_localCreTime) / 1000000000 + m_netCreTime;
    }
};
struct AuxiliaryControllerAddress { //80 bytes
    std::string m_localIp, m_key;
    int m_localPort = 0, m_stc_f31 = 0;
    protobuf::IPProtocol m_proto = protobuf::TCP;
    AuxiliaryControllerAddress(const std::string &localIp, uint32_t localPort, protobuf::IPProtocol protocol, uint32_t stc_f31, const std::string &key) :
        m_localIp(localIp), m_key(key), m_localPort(localPort), m_stc_f31(stc_f31), m_proto(protocol) {}
    AuxiliaryControllerAddress() {}
};
struct ActivityRecommendationRestInvoker { //0x30 bytes
    ActivityRecommendationRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    void getActivityRecommendations(const std::string &a2) {
        //TODO
    }
};
struct AchievementsRestInvoker { //0x30 bytes
    AchievementsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO = ExperimentsRestInvoker
    }
    /*TODO
AchievementsRestInvoker::load()
AchievementsRestInvoker::unlock(protobuf::achievement::AchievementUnlockRequest const&)*/
};
struct CampaignRestInvoker { //0x70 bytes
    CampaignRestInvoker(const std::string &server) {
        //TODO
    }
    /*TODO
CampaignRestInvoker::CampaignRestInvoker(std::shared_ptr<ZwiftHttpConnectionManager>,const std::string &)
CampaignRestInvoker::enrollPlayer(int64_t,const std::string &)
CampaignRestInvoker::enrollPlayer(const std::string &)
CampaignRestInvoker::getActiveCampaigns()
CampaignRestInvoker::getCampaigns()
CampaignRestInvoker::getCompletedCampaigns()
CampaignRestInvoker::getProgress(const std::string &)
CampaignRestInvoker::getRegistration(const std::string &)
CampaignRestInvoker::playerSummary(int64_t,const std::string &)
CampaignRestInvoker::registerPlayer(int64_t,const std::string &)
CampaignRestInvoker::registerPlayer(const std::string &)
CampaignRestInvoker::withdrawPlayer(int64_t,const std::string &)
CampaignRestInvoker::withdrawPlayer(const std::string &)*/
};
struct ClubsRestInvoker { //0x30 bytes
    ClubsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    /*TODO
ClubsRestInvoker::listMyClubs(zwift_network::Optional<protobuf::club::Membership_Status>,zwift_network::Optional<int>,zwift_network::Optional<int>)
ClubsRestInvoker::myActiveClub()
ClubsRestInvoker::resetMyActiveClub()
ClubsRestInvoker::setMyActiveClub(protobuf::club::UUID const&)*/
};
struct EventCoreRestInvoker { //0x30 bytes
    EventCoreRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    /*TODO
EventCoreRestInvoker::createRegistration(int64_t)
EventCoreRestInvoker::createSignup(int64_t)
EventCoreRestInvoker::deleteRegistration(int64_t)
EventCoreRestInvoker::deleteSignup(int64_t)*/
};
struct EventFeedRestInvoker { //0x30 bytes
    EventFeedRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    /*TODO
EventFeedRestInvoker::getEvents(zwift_network::model::EventsSearch const&)
EventFeedRestInvoker::getMyEvents(zwift_network::model::BaseEventsSearch const&)*/
};
struct FirmwareUpdateRestInvoker { //0x30 bytes
    FirmwareUpdateRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //OMIT
    }
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
    GenericRestInvoker(GenericHttpConnectionManager *mgr) {
        //TODO
    }
    /*TODO GenericRestInvoker::get(const std::string &)*/
};
struct PrivateEventsRestInvoker { //0x30 bytes
    PrivateEventsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    /*TODO
PrivateEventsRestInvoker::accept(int64_t)
PrivateEventsRestInvoker::feed(int64_t,int64_t,zwift_network::Optional<protobuf::EventInviteStatusProto>,bool)
PrivateEventsRestInvoker::get(int64_t)
PrivateEventsRestInvoker::reject(int64_t)*/
};
struct RaceResultRestInvoker { //0x30 bytes
    RaceResultRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    /*TODO
RaceResultRestInvoker::createRaceResultEntry(protobuf::RaceResultEntrySaveRequest const&)
RaceResultRestInvoker::getEventRaceResult(int64_t,zwift_network::Optional<int>,zwift_network::Optional<int>)
RaceResultRestInvoker::getEventRaceResultSummary(int64_t)
RaceResultRestInvoker::getSubgroupRaceResult(int64_t,zwift_network::Optional<int>,zwift_network::Optional<int>)
RaceResultRestInvoker::getSubgroupRaceResultSummary(int64_t)*/
};
struct RouteResultsRestInvoker { //0x30 bytes
    RouteResultsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO = Experiments
    }
    /*TODO RouteResultsRestInvoker::save(protobuf::routeresults::RouteResultSaveRequest const&)*/
};
struct PlayerPlaybackRestInvoker { //0x30 bytes
    PlayerPlaybackRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    /*TODO
PlayerPlaybackRestInvoker::deletePlayback(const std::string &)
PlayerPlaybackRestInvoker::getMyPlaybackLatest(int64_t,uint64_t,uint64_t)
PlayerPlaybackRestInvoker::getMyPlaybackPr(int64_t,uint64_t,uint64_t)
PlayerPlaybackRestInvoker::getMyPlaybacks(int64_t)
PlayerPlaybackRestInvoker::getPlaybackData(protobuf::playback::PlaybackMetadata const&)
PlayerPlaybackRestInvoker::savePlayback(protobuf::playback::PlaybackData const&)*/
};
struct SegmentResultsRestInvoker { //0x30 bytes
    SegmentResultsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    /*TODO 
SegmentResultsRestInvoker::getLeaderboard(int64_t)
SegmentResultsRestInvoker::getSegmentJerseyLeaders()*/
};
struct PowerCurveRestInvoker { //0x30 bytes
    PowerCurveRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    /*TODO
PowerCurveRestInvoker::getAvailablePowerCurveYears()
PowerCurveRestInvoker::getBestEffortsPowerCurveByDays(int)
PowerCurveRestInvoker::getBestEffortsPowerCurveByYear(int)
PowerCurveRestInvoker::getBestEffortsPowerCurveFromAllTime()*/
};
struct ZFileRestInvoker { //0x30 bytes
    ZFileRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    /*TODO 
ZFileRestInvoker::create(protobuf::ZFileProto const&)
ZFileRestInvoker::download(int64_t)
ZFileRestInvoker::erase(int64_t)
ZFileRestInvoker::list(const std::string &)*/
};
struct ZwiftWorkoutsRestInvoker { //0x38 bytes
    ZwiftWorkoutsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    /*TODO 
ZwiftWorkoutsRestInvoker::createWorkout(const std::string &,protobuf::Sport,const std::string &)
ZwiftWorkoutsRestInvoker::createWorkoutProto(const std::string &,protobuf::Sport,const std::string &)
ZwiftWorkoutsRestInvoker::deleteWorkout(const std::string &)
ZwiftWorkoutsRestInvoker::editWorkout(const std::string &,const std::string &,protobuf::Sport,const std::string &)
ZwiftWorkoutsRestInvoker::fetchUpcomingWorkouts(RequestTaskComposer<std::vector<zwift_network::model::WorkoutsFromPartner>,std::multiset<zwift_network::model::Workout>>::Composable)
ZwiftWorkoutsRestInvoker::fetchWorkout(const std::string &)
ZwiftWorkoutsRestInvoker::~ZwiftWorkoutsRestInvoker()*/
};
struct WorkoutServiceRestInvoker { //0x30 bytes
    WorkoutServiceRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
    /*TODO
WorkoutServiceRestInvoker::fetchAssetSummary(const std::string &)
WorkoutServiceRestInvoker::fetchCustomWorkouts(zwift_network::Optional<std::string>)*/
};
struct UdpStatistics { //0x450 bytes
    UdpStatistics() {
        //OMIT
    }
    //OMIT
};
struct TcpStatistics { //0xC8 bytes
    TcpStatistics() {
        //OMIT
    }
    //OMIT
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
struct LanExerciseDeviceService { //0x3B0 bytes
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
        return (cfg.nodes_size() && !foundCurrentNodeAsFirstNode) || !currentHostIsStillGood;
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
            *((_QWORD *)this_ + 15) = asio::execution::any_executor<asio::execution::context_as_t<asio::execution_context &>, asio::execution::detail::blocking::never_t<0>, asio::execution::prefer_only<asio::execution::detail::blocking::possibly_t<0>>, asio::execution::prefer_only<asio::execution::detail::outstanding_work::tracked_t<0>>, asio::execution::prefer_only<asio::execution::detail::outstanding_work::untracked_t<0>>, asio::execution::prefer_only<asio::execution::detail::relationship::fork_t<0>>, asio::execution::prefer_only<asio::execution::detail::relationship::continuation_t<0>>>::prop_fns_table<asio::io_context::basic_executor_type<std::allocator<void>, 0u>>()::fns;
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
            *((_QWORD *)this_ + 5) = asio::execution::any_executor<asio::execution::context_as_t<asio::execution_context &>, asio::execution::detail::blocking::never_t<0>, asio::execution::prefer_only<asio::execution::detail::blocking::possibly_t<0>>, asio::execution::prefer_only<asio::execution::detail::outstanding_work::tracked_t<0>>, asio::execution::prefer_only<asio::execution::detail::outstanding_work::untracked_t<0>>, asio::execution::prefer_only<asio::execution::detail::relationship::fork_t<0>>, asio::execution::prefer_only<asio::execution::detail::relationship::continuation_t<0>>>::prop_fns_table<asio::io_context::basic_executor_type<std::allocator<void>, 0u>>()::fns;
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
TcpClient::connect()
TcpClient::decodeMessage(char *,uint64_t &)
TcpClient::encodeMessage(char *,uint64_t,uint32_t &)
TcpClient::getClientToServerForHelloMessage(uint64_t,int64_t)
TcpClient::getMaximumHelloMessageSize()
TcpClient::getTcpMessageSize(uint32_t)
TcpClient::handleCommunicationError(std::error_code,const std::string &)
TcpClient::handleTcpConfigChanged(protobuf::TcpConfig const&)
TcpClient::onTcpConfigReceived(std::shared_ptr<zwift_network::NetworkResponse<protobuf::TcpConfig> const> const&)
TcpClient::popServerToClient(std::shared_ptr<protobuf::ServerToClient const> &)
TcpClient::processPayload(uint64_t)
TcpClient::processSegmentSubscription(int64_t,std::shared_ptr<std::promise<std::shared_ptr<zwift_network::NetworkResponse<protobuf::SegmentResults> const>>>)
TcpClient::processSegmentUnsubscription(int64_t)
TcpClient::processSubscribedSegment(protobuf::ServerToClient const&)
TcpClient::readHeader()
TcpClient::readPayload(uint32_t)
TcpClient::resetTimeoutTimer()
TcpClient::sayHello()
TcpClient::sendClientToServer(TcpCommand,protobuf::ClientToServer &,std::function<void ()()> const&,std::function<void ()()> const&)
TcpClient::sendSubscribeToSegment(int64_t,std::shared_ptr<TcpClient::SegmentSubscription> const&)
TcpClient::serializeToTcpMessage(TcpCommand,protobuf::ClientToServer const&,std::array<char,1492ul> &,uint32_t &)
TcpClient::subscribeToSegmentAndGetLeaderboard(int64_t)
TcpClient::unsubscribeFromSegment(int64_t)
TcpClient::~TcpClient()    */
};
struct AuxiliaryController : public WorldIdListener {
    void handleWorldIdChange(int64_t worldId) override {
        //TODO
    }
    /*TODO zwift_network::AuxiliaryController::AuxiliaryController(int64_t,std::shared_ptr<GlobalState>,std::shared_ptr<WorldClockService>,std::shared_ptr<AuxiliaryControllerStatistics>,std::function<void ()()>)
zwift_network::AuxiliaryController::add_pending_command(std::shared_ptr<protobuf::GameToPhoneCommand> const&)
zwift_network::AuxiliaryController::attempt_write_to_tcp_socket(uchar const*,uint32_t)
zwift_network::AuxiliaryController::clear_telemetry()
zwift_network::AuxiliaryController::connect(zwift_network::AuxiliaryControllerAddress const&)
zwift_network::AuxiliaryController::disconnect()
zwift_network::AuxiliaryController::do_receive()
zwift_network::AuxiliaryController::do_tcp_receive_encoded_message(uint32_t)
zwift_network::AuxiliaryController::do_tcp_receive_encoded_message_length()
zwift_network::AuxiliaryController::get_world_id()
zwift_network::AuxiliaryController::handleWorldIdChange(int64_t)
zwift_network::AuxiliaryController::init_encryption(zwift_network::AuxiliaryControllerAddress const&)
zwift_network::AuxiliaryController::keep_io_service_alive()
zwift_network::AuxiliaryController::motion_data(zwift_network::Motion &)
zwift_network::AuxiliaryController::pop_phone_to_game_command(std::shared_ptr<protobuf::PhoneToGameCommand const> &)
zwift_network::AuxiliaryController::process_phone_to_game(protobuf::PhoneToGame &)
zwift_network::AuxiliaryController::reconnect(zwift_network::AuxiliaryControllerAddress const&)
zwift_network::AuxiliaryController::register_bytes_out(uint64_t)
zwift_network::AuxiliaryController::send_activate_power_up_command(int,uint32_t)
zwift_network::AuxiliaryController::send_ble_peripheral_request(protobuf::BLEPeripheralRequest const&)
zwift_network::AuxiliaryController::send_clear_power_up_command()
zwift_network::AuxiliaryController::send_customize_action_button_command(uint32_t,uint32_t,const std::string &,const std::string &,bool)
zwift_network::AuxiliaryController::send_default_activity_name(const std::string &)
zwift_network::AuxiliaryController::send_game_packet(const std::string &,bool)
zwift_network::AuxiliaryController::send_game_to_phone(protobuf::GameToPhone &)
zwift_network::AuxiliaryController::send_image_to_mobile_app(const std::string &,const std::string &)
zwift_network::AuxiliaryController::send_keep_alive_packet()
zwift_network::AuxiliaryController::send_mobile_alert(protobuf::MobileAlert const&)
zwift_network::AuxiliaryController::send_mobile_alert_cancel_command(protobuf::MobileAlert const&)
zwift_network::AuxiliaryController::send_pairing_status(bool)
zwift_network::AuxiliaryController::send_player_profile(protobuf::PlayerProfile const&)
zwift_network::AuxiliaryController::send_player_state(protobuf::PlayerState const&)
zwift_network::AuxiliaryController::send_rider_list_entries(std::list<protobuf::RiderListEntry> const&)
zwift_network::AuxiliaryController::send_set_power_up_command(const std::string &,const std::string &,const std::string &,int)
zwift_network::AuxiliaryController::send_social_player_action(protobuf::SocialPlayerAction const&)
zwift_network::AuxiliaryController::set_client_info_to_telemetry(protobuf::PhoneToGameCommand const&)
zwift_network::AuxiliaryController::set_connection_handlers()
zwift_network::AuxiliaryController::start(zwift_network::AuxiliaryControllerAddress const&,bool,uint32_t,const std::string &)
zwift_network::AuxiliaryController::stop()
zwift_network::AuxiliaryController::tcp_connect(zwift_network::AuxiliaryControllerAddress const&)
zwift_network::AuxiliaryController::~AuxiliaryController()*/
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
    std::future<NetworkResponse<void>> removeFollowee(int64_t playerId, int64_t followeeId) {
        if (!m_restInvoker)
            return makeNetworkResponseFuture<void>(NRO_NOT_INITIALIZED, "Initialize CNL first"s);
        if (playerId <= 0)
            return makeNetworkResponseFuture<void>(NRO_INVALID_ARGUMENT, "Invalid player id"s);
        if (followeeId <= 0)
            return makeNetworkResponseFuture<void>(NRO_INVALID_ARGUMENT, "Invalid followee player id"s);
        return m_restInvoker->removeFollowee(playerId, followeeId);
    }
    std::future<NetworkResponse<protobuf::SocialNetworkStatus>> addFollowee(int64_t playerId, int64_t followeeId, bool a5, protobuf::ProfileFollowStatus pfs) {
        if (!m_restInvoker)
            return makeNetworkResponseFuture<protobuf::SocialNetworkStatus>(NRO_NOT_INITIALIZED, "Initialize CNL first"s);
        if (playerId <= 0)
            return makeNetworkResponseFuture<protobuf::SocialNetworkStatus>(NRO_INVALID_ARGUMENT, "Invalid player id"s);
        if (followeeId <= 0)
            return makeNetworkResponseFuture<protobuf::SocialNetworkStatus>(NRO_INVALID_ARGUMENT, "Invalid followee player id"s);
        return m_restInvoker->addFollowee(playerId, followeeId, a5, pfs);
    }
    /*NetworkClientImpl::NetworkClientImpl(std::shared_ptr<MachineIdProvider>,std::shared_ptr<HttpConnectionFactory>)
NetworkClientImpl::acceptPrivateEventInvitation(int64_t)
NetworkClientImpl::campaignSummary(const std::string &)
NetworkClientImpl::connectLanExerciseDevice(uint32_t,std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>,std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>)
NetworkClientImpl::createActivityRideOn(int64_t,int64_t)
NetworkClientImpl::createRaceResultEntry(protobuf::RaceResultEntrySaveRequest const&)
NetworkClientImpl::createSubgroupRegistration(int64_t)
NetworkClientImpl::createSubgroupSignup(int64_t)
NetworkClientImpl::createUser(const std::string &,const std::string &,const std::string &,const std::string &)
NetworkClientImpl::createWorkout(const std::string &,protobuf::Sport,const std::string &)
NetworkClientImpl::createZFile(protobuf::ZFileProto const&)
NetworkClientImpl::createZFileGzip(const std::string &,const std::string &,const std::string &)
NetworkClientImpl::deleteActivity(int64_t,int64_t)
NetworkClientImpl::deleteActivityImage(int64_t,int64_t,int64_t)
NetworkClientImpl::deletePlayback(const std::string &)
NetworkClientImpl::deleteSubgroupRegistration(int64_t)
NetworkClientImpl::deleteSubgroupSignup(int64_t)
NetworkClientImpl::deleteWorkout(const std::string &)
NetworkClientImpl::deregisterTelemetryJson(const std::string &)
NetworkClientImpl::disconnectLanExerciseDevice(uint32_t)
NetworkClientImpl::downloadZFile(int64_t)
NetworkClientImpl::editWorkout(const std::string &,const std::string &,protobuf::Sport,const std::string &)
NetworkClientImpl::enrollInCampaign(const std::string &)
NetworkClientImpl::enrollInCampaignV2(const std::string &)
NetworkClientImpl::eraseZFile(int64_t)
NetworkClientImpl::fetchAssetSummary(const std::string &)
NetworkClientImpl::fetchCustomWorkouts(zwift_network::Optional<std::string>)
NetworkClientImpl::fetchDropInWorldList()
NetworkClientImpl::fetchUpcomingWorkouts()
NetworkClientImpl::fetchWorkout(zwift_network::model::WorkoutPartnerEnum,const std::string &)
NetworkClientImpl::fetchWorldsCountsAndCapacities()
NetworkClientImpl::fromIso8601(const std::string &)
NetworkClientImpl::generateZFileGzip(protobuf::ZFileProto &,const std::string &,const std::string &,const std::string &)
NetworkClientImpl::get(const std::string &)
NetworkClientImpl::getAchievements()
NetworkClientImpl::getActiveCampaigns()
NetworkClientImpl::getActivities(int64_t,zwift_network::Optional<int64_t>,zwift_network::Optional<int64_t>,bool)
NetworkClientImpl::getActivity(int64_t,int64_t,bool)
NetworkClientImpl::getActivityImage(int64_t,int64_t,int64_t)
NetworkClientImpl::getActivityRecommendations(const std::string &)
NetworkClientImpl::getAllFirmwareReleases(const std::string &)
NetworkClientImpl::getAvailablePowerCurveYears()
NetworkClientImpl::getBestEffortsPowerCurveByDays(zwift_network::PowerCurveAggregationDays)
NetworkClientImpl::getBestEffortsPowerCurveByYear(int)
NetworkClientImpl::getBestEffortsPowerCurveFromAllTime()
NetworkClientImpl::getCampaignsV2()
NetworkClientImpl::getCompletedCampaigns()
NetworkClientImpl::getConnectionMetrics(zwift_network::ConnectivityInfo &)
NetworkClientImpl::getConnectionQuality()
NetworkClientImpl::getEvent(int64_t)
NetworkClientImpl::getEventRaceResult(int64_t,zwift_network::Optional<int>,zwift_network::Optional<int>)
NetworkClientImpl::getEventRaceResultSummary(int64_t)
NetworkClientImpl::getEventSubgroupEntrants(protobuf::EventParticipation,int64_t,uint32_t)
NetworkClientImpl::getEvents(zwift_network::model::EventsSearch const&)
NetworkClientImpl::getEventsInInterval(const std::string &,const std::string &,int)
NetworkClientImpl::getFeatureResponse(protobuf::experimentation::FeatureRequest const&)
NetworkClientImpl::getFeatureResponseByMachineId(protobuf::experimentation::FeatureRequest const&)
NetworkClientImpl::getFirmwareRelease(const std::string &,const std::string &)
NetworkClientImpl::getFirmwareUpdates(std::vector<zwift_network::model::FirmwareRequest> const&)
NetworkClientImpl::getFollowees(int64_t,bool)
NetworkClientImpl::getFollowees(int64_t,int64_t,bool)
NetworkClientImpl::getFollowers(int64_t,bool)
NetworkClientImpl::getFollowers(int64_t,int64_t,bool)
NetworkClientImpl::getGoals(int64_t)
NetworkClientImpl::getLateJoinInformation(int64_t)
NetworkClientImpl::getLibVersion()
NetworkClientImpl::getMyEvents(zwift_network::model::BaseEventsSearch const&)
NetworkClientImpl::getMyPlaybackLatest(int64_t,uint64_t,uint64_t)
NetworkClientImpl::getMyPlaybackPr(int64_t,uint64_t,uint64_t)
NetworkClientImpl::getMyPlaybacks(int64_t)
NetworkClientImpl::getNotifications()
NetworkClientImpl::getPlaybackData(protobuf::playback::PlaybackMetadata const&)
NetworkClientImpl::getPlayerId()
NetworkClientImpl::getPrivateEvent(int64_t)
NetworkClientImpl::getProgressInCampaignV2(const std::string &)
NetworkClientImpl::getRegistrationInCampaignV2(const std::string &)
NetworkClientImpl::getSegmentJerseyLeaders()
NetworkClientImpl::getSegmentResult(int64_t)
NetworkClientImpl::getSessionId()
NetworkClientImpl::getSubgroupRaceResult(int64_t,zwift_network::Optional<int>,zwift_network::Optional<int>)
NetworkClientImpl::getSubgroupRaceResultSummary(int64_t)
NetworkClientImpl::getVersion()
NetworkClientImpl::globalCleanup()
NetworkClientImpl::globalInitialize()
NetworkClientImpl::handleAuxiliaryControllerAddress(zwift_network::AuxiliaryControllerAddress)
NetworkClientImpl::handleDisconnectRequested(bool)
NetworkClientImpl::handleWorldAndMapRevisionChanged(int64_t,uint32_t)
NetworkClientImpl::initialize(const std::string &,const std::string &,std::function<void ()(char *)> const&,const std::string &,zwift_network::NetworkClientOptions const&)
NetworkClientImpl::initializeTelemetry()
NetworkClientImpl::initializeWorkoutManager(protobuf::PlayerProfile const&)
NetworkClientImpl::isLoggedIn()
NetworkClientImpl::isPairedToPhone()
NetworkClientImpl::isPlayerIdInvalid(int64_t)
NetworkClientImpl::latestPlayerState(int64_t,int64_t)
NetworkClientImpl::listMyClubs(zwift_network::Optional<protobuf::club::Membership_Status>,zwift_network::Optional<int>,zwift_network::Optional<int>)
NetworkClientImpl::listPlayerTypes()
NetworkClientImpl::listZFiles(const std::string &)
NetworkClientImpl::logIn(const std::string &,const std::string &,const std::string &,std::vector<std::string> const&,const std::string &)
NetworkClientImpl::logInWithEmailAndPassword(const std::string &,const std::string &,std::vector<std::string> const&,bool,const std::string &)
NetworkClientImpl::logInWithOauth2Credentials(const std::string &,std::vector<std::string> const&,const std::string &)
NetworkClientImpl::logLibSummary()
NetworkClientImpl::logOut()
NetworkClientImpl::machineId()
NetworkClientImpl::motionData(zwift_network::Motion &)
NetworkClientImpl::myActiveClub()
NetworkClientImpl::myProfile()
NetworkClientImpl::myProfileEntitlements()
NetworkClientImpl::networkTime()
NetworkClientImpl::noLogInAttempted()
NetworkClientImpl::onLoggedIn(protobuf::PerSessionInfo const&,const std::string &,GlobalState::EncryptionInfo const&)
NetworkClientImpl::onLoggedOut()
NetworkClientImpl::onMyPlayerProfileReceived(std::shared_ptr<zwift_network::NetworkResponse<protobuf::PlayerProfile> const> const&)
NetworkClientImpl::onSaveActivityReceived(std::shared_ptr<zwift_network::NetworkResponse<int64_t> const> const&)
NetworkClientImpl::onUpdatedPlayerProfileReceived(bool,bool,bool,uint32_t)
NetworkClientImpl::parseValidationErrorMessage(const std::string &)
NetworkClientImpl::popPhoneToGameCommand(std::shared_ptr<protobuf::PhoneToGameCommand const> &)
NetworkClientImpl::popPlayerIdWithUpdatedProfile(int64_t &)
NetworkClientImpl::popServerToClient(std::shared_ptr<protobuf::ServerToClient const> &)
NetworkClientImpl::popWorldAttribute(std::shared_ptr<protobuf::WorldAttribute const> &)
NetworkClientImpl::privateEventFeed(int64_t,int64_t,zwift_network::Optional<protobuf::EventInviteStatusProto>,bool)
NetworkClientImpl::profile(int64_t,bool)
NetworkClientImpl::profile(const std::string &)
NetworkClientImpl::profiles(std::unordered_set<int64_t> const&)
NetworkClientImpl::querySegmentResults(int64_t,int64_t,int64_t,bool)
NetworkClientImpl::querySegmentResults(int64_t,int64_t,int64_t,bool,int64_t)
NetworkClientImpl::querySegmentResults(int64_t,int64_t,const std::string &,const std::string &,bool)
NetworkClientImpl::redeemCoupon(const std::string &)
NetworkClientImpl::registerForEventSubgroup(int64_t)
NetworkClientImpl::registerInCampaign(const std::string &)
NetworkClientImpl::registerInCampaignV2(const std::string &)
NetworkClientImpl::registerLanExerciseDeviceMessageReceivedCallback(std::function<void ()(zwift_network::LanExerciseDeviceInfo const&,const std::vector<uchar> &)> const&)
NetworkClientImpl::registerLanExerciseDeviceStatusCallback(std::function<void ()(zwift_network::LanExerciseDeviceInfo const&)> const&)
NetworkClientImpl::registerLoggingFunction(std::function<void ()(char *)> const&)
NetworkClientImpl::registerTelemetryJson(const std::string &,std::function<Json::Value ()(std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>)> const&)
NetworkClientImpl::rejectPrivateEventInvitation(int64_t)
NetworkClientImpl::remoteLog(zwift_network::LogLevel,char const*,char const*)
NetworkClientImpl::remoteLogAndFlush(zwift_network::LogLevel,char const*,char const*)
NetworkClientImpl::removeFollower(int64_t,int64_t)
NetworkClientImpl::removeGoal(int64_t,int64_t)
NetworkClientImpl::removeRegistrationForEvent(int64_t)
NetworkClientImpl::removeSignupForEvent(int64_t)
NetworkClientImpl::resetCredentials()
NetworkClientImpl::resetMyActiveClub()
NetworkClientImpl::resetPassword(const std::string &)
NetworkClientImpl::resumeSubscription()
NetworkClientImpl::returnToHome()
NetworkClientImpl::roundTripLatencyInMilliseconds()
NetworkClientImpl::saveActivity(protobuf::Activity const&,bool,const std::string &)
NetworkClientImpl::saveActivityImage(int64_t,protobuf::ActivityImage const&,const std::string &)
NetworkClientImpl::saveGoal(protobuf::Goal const&)
NetworkClientImpl::savePlayback(protobuf::playback::PlaybackData const&)
NetworkClientImpl::saveProfileReminders(protobuf::PlayerProfile const&)
NetworkClientImpl::saveRouteResult(protobuf::routeresults::RouteResultSaveRequest const&)
NetworkClientImpl::saveSegmentResult(protobuf::SegmentResult const&)
NetworkClientImpl::saveTimeCrossingStartLine(int64_t,protobuf::CrossingStartingLineProto const&)
NetworkClientImpl::saveWorldAttribute(protobuf::WorldAttribute &)
NetworkClientImpl::sendActivatePowerUpCommand(int,uint32_t)
NetworkClientImpl::sendAnalyticsEvent(const std::string &,std::vector<std::string> const&)
NetworkClientImpl::sendBlePeripheralRequest(protobuf::BLEPeripheralRequest const&)
NetworkClientImpl::sendClearPowerUpCommand()
NetworkClientImpl::sendCustomizeActionButtonCommand(uint32_t,uint32_t,const std::string &,const std::string &,bool)
NetworkClientImpl::sendDefaultActivityNameCommand(const std::string &)
NetworkClientImpl::sendDeviceDiagnostics(const std::string &,const std::string &,const std::vector<uchar> &)
NetworkClientImpl::sendGamePacket(const std::string &,bool)
NetworkClientImpl::sendImageToMobileApp(const std::string &,const std::string &)
NetworkClientImpl::sendMessageToLanExerciseDevice(uint32_t,const std::vector<uchar> &)
NetworkClientImpl::sendMixpanelEvent(const std::string &,std::vector<std::string> const&)
NetworkClientImpl::sendMobileAlert(protobuf::MobileAlert const&)
NetworkClientImpl::sendMobileAlertCancelCommand(protobuf::MobileAlert const&)
NetworkClientImpl::sendPlayerProfile(protobuf::PlayerProfile const&)
NetworkClientImpl::sendPlayerState(int64_t,protobuf::PlayerState const&)
NetworkClientImpl::sendRiderListEntries(std::list<protobuf::RiderListEntry> const&)
NetworkClientImpl::sendSetPowerUpCommand(const std::string &,const std::string &,const std::string &,int)
NetworkClientImpl::sendSocialPlayerAction(protobuf::SocialPlayerAction const&)
NetworkClientImpl::setMyActiveClub(protobuf::club::UUID const&)
NetworkClientImpl::setTeleportingAllowed(bool)
NetworkClientImpl::shouldTryToEnableEncryptionWithZc()
NetworkClientImpl::shutdownAuxiliaryController()
NetworkClientImpl::shutdownServiceEventLoop()
NetworkClientImpl::shutdownTcpClient()
NetworkClientImpl::shutdownUdpClient()
NetworkClientImpl::signUrls(const std::string &,std::vector<std::string> const&)
NetworkClientImpl::signupForEventSubgroup(int64_t)
NetworkClientImpl::startScanningForLanExerciseDevices()
NetworkClientImpl::startTcpClient()
NetworkClientImpl::stopScanningForLanExerciseDevices()
NetworkClientImpl::subscribeToSegmentAndGetLeaderboard(int64_t)
NetworkClientImpl::toIso8601(int64_t)
NetworkClientImpl::unlockAchievements(protobuf::achievement::AchievementUnlockRequest const&)
NetworkClientImpl::unsubscribeFromSegment(int64_t)
NetworkClientImpl::updateFollower(int64_t,int64_t,bool,protobuf::ProfileFollowStatus)
NetworkClientImpl::updateNotificationReadStatus(int64_t,int64_t,bool)
NetworkClientImpl::updateProfile(bool,protobuf::PlayerProfile const&,bool)
NetworkClientImpl::updateTelemetrySampleInterval(std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>)
NetworkClientImpl::uploadReceipt(protobuf::InAppPurchaseReceipt &)
NetworkClientImpl::validateProperty(zwift_network::ProfileProperties,const std::string &)
NetworkClientImpl::withdrawFromCampaign(const std::string &)
NetworkClientImpl::withdrawFromCampaignV2(const std::string &)
NetworkClientImpl::worldTime()
NetworkClientImpl::~NetworkClientImpl()*/
};
/*zwift_network::NetworkClient::NetworkClient(const std::string &)
zwift_network::NetworkClient::acceptPrivateEventInvitation(int64_t)
zwift_network::NetworkClient::campaignSummary(const std::string &)
zwift_network::NetworkClient::connectLanExerciseDevice(uint32_t,std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>,std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>)
zwift_network::NetworkClient::createActivityRideOn(int64_t,int64_t)
zwift_network::NetworkClient::createRaceResultEntry(protobuf::RaceResultEntrySaveRequest const&)
zwift_network::NetworkClient::createSubgroupRegistration(int64_t)
zwift_network::NetworkClient::createSubgroupSignup(int64_t)
zwift_network::NetworkClient::createUser(const std::string &,const std::string &,const std::string &,const std::string &)
zwift_network::NetworkClient::createWorkout(const std::string &,protobuf::Sport,const std::string &)
zwift_network::NetworkClient::createZFile(protobuf::ZFileProto const&)
zwift_network::NetworkClient::createZFileGzip(const std::string &,const std::string &,const std::string &)
zwift_network::NetworkClient::deleteActivity(int64_t,int64_t)
zwift_network::NetworkClient::deleteActivityImage(int64_t,int64_t,int64_t)
zwift_network::NetworkClient::deletePlayback(const std::string &)
zwift_network::NetworkClient::deleteSubgroupRegistration(int64_t)
zwift_network::NetworkClient::deleteSubgroupSignup(int64_t)
zwift_network::NetworkClient::deleteWorkout(const std::string &)
zwift_network::NetworkClient::deregisterTelemetryJson(const std::string &)
zwift_network::NetworkClient::disconnectLanExerciseDevice(uint32_t)
zwift_network::NetworkClient::downloadZFile(int64_t)
zwift_network::NetworkClient::editWorkout(const std::string &,const std::string &,protobuf::Sport,const std::string &)
zwift_network::NetworkClient::enrollInCampaign(const std::string &)
zwift_network::NetworkClient::enrollInCampaignV2(const std::string &)
zwift_network::NetworkClient::eraseZFile(int64_t)
zwift_network::NetworkClient::fetchAssetSummary(const std::string &)
zwift_network::NetworkClient::fetchCustomWorkouts(zwift_network::Optional<std::string>)
zwift_network::NetworkClient::fetchDropInWorldList()
zwift_network::NetworkClient::fetchUpcomingWorkouts()
zwift_network::NetworkClient::fetchWorkout(zwift_network::model::WorkoutPartnerEnum,const std::string &)
zwift_network::NetworkClient::fetchWorldsCountsAndCapacities()
zwift_network::NetworkClient::fromIso8601(const std::string &)
zwift_network::NetworkClient::get(const std::string &)
zwift_network::NetworkClient::getAchievements()
zwift_network::NetworkClient::getActiveCampaigns()
zwift_network::NetworkClient::getActivities(int64_t,zwift_network::Optional<int64_t>,zwift_network::Optional<int64_t>,bool)
zwift_network::NetworkClient::getActivity(int64_t,int64_t,bool)
zwift_network::NetworkClient::getActivityImage(int64_t,int64_t,int64_t)
zwift_network::NetworkClient::getActivityRecommendations(const std::string &)
zwift_network::NetworkClient::getAllFirmwareReleases(const std::string &)
zwift_network::NetworkClient::getAvailablePowerCurveYears()
zwift_network::NetworkClient::getBestEffortsPowerCurveByDays(zwift_network::PowerCurveAggregationDays)
zwift_network::NetworkClient::getBestEffortsPowerCurveByYear(int)
zwift_network::NetworkClient::getBestEffortsPowerCurveFromAllTime()
zwift_network::NetworkClient::getCampaignsV2()
zwift_network::NetworkClient::getCompletedCampaigns()
zwift_network::NetworkClient::getConnectionMetrics(zwift_network::ConnectivityInfo &)
zwift_network::NetworkClient::getConnectionQuality()
zwift_network::NetworkClient::getEvent(int64_t)
zwift_network::NetworkClient::getEventRaceResult(int64_t,zwift_network::Optional<int>,zwift_network::Optional<int>)
zwift_network::NetworkClient::getEventRaceResultSummary(int64_t)
zwift_network::NetworkClient::getEventSubgroupEntrants(protobuf::EventParticipation,int64_t,uint32_t)
zwift_network::NetworkClient::getEvents(zwift_network::model::EventsSearch const&)
zwift_network::NetworkClient::getEventsInInterval(const std::string &,const std::string &,int)
zwift_network::NetworkClient::getFeatureResponse(protobuf::experimentation::FeatureRequest const&)
zwift_network::NetworkClient::getFeatureResponseByMachineId(protobuf::experimentation::FeatureRequest const&)
zwift_network::NetworkClient::getFirmwareRelease(const std::string &,const std::string &)
zwift_network::NetworkClient::getFirmwareUpdates(std::vector<zwift_network::model::FirmwareRequest> const&)
zwift_network::NetworkClient::getFollowees(int64_t,bool)
zwift_network::NetworkClient::getFollowees(int64_t,int64_t,bool)
zwift_network::NetworkClient::getFollowers(int64_t,bool)
zwift_network::NetworkClient::getFollowers(int64_t,int64_t,bool)
zwift_network::NetworkClient::getGoals(int64_t)
zwift_network::NetworkClient::getLateJoinInformation(int64_t)
zwift_network::NetworkClient::getLibVersion()
zwift_network::NetworkClient::getMyEvents(zwift_network::model::BaseEventsSearch const&)
zwift_network::NetworkClient::getMyPlaybackLatest(int64_t,uint64_t,uint64_t)
zwift_network::NetworkClient::getMyPlaybackPr(int64_t,uint64_t,uint64_t)
zwift_network::NetworkClient::getMyPlaybacks(int64_t)
zwift_network::NetworkClient::getNotifications()
zwift_network::NetworkClient::getPlaybackData(protobuf::playback::PlaybackMetadata const&)
zwift_network::NetworkClient::getPlayerId()
zwift_network::NetworkClient::getPrivateEvent(int64_t)
zwift_network::NetworkClient::getProgressInCampaignV2(const std::string &)
zwift_network::NetworkClient::getRegistrationInCampaignV2(const std::string &)
zwift_network::NetworkClient::getSegmentJerseyLeaders()
zwift_network::NetworkClient::getSegmentResult(int64_t)
zwift_network::NetworkClient::getSessionId()
zwift_network::NetworkClient::getSubgroupRaceResult(int64_t,zwift_network::Optional<int>,zwift_network::Optional<int>)
zwift_network::NetworkClient::getSubgroupRaceResultSummary(int64_t)
zwift_network::NetworkClient::getVersion()
zwift_network::NetworkClient::globalCleanup()
zwift_network::NetworkClient::globalInitialize()
zwift_network::NetworkClient::initialize(const std::string &,const std::string &,std::function<void ()(char *)> const&,const std::string &,zwift_network::NetworkClientOptions const&)
zwift_network::NetworkClient::initializeTelemetry()
zwift_network::NetworkClient::isLoggedIn()
zwift_network::NetworkClient::isPairedToPhone()
zwift_network::NetworkClient::latestPlayerState(int64_t,int64_t)
zwift_network::NetworkClient::listMyClubs(zwift_network::Optional<protobuf::club::Membership_Status>,zwift_network::Optional<int>,zwift_network::Optional<int>)
zwift_network::NetworkClient::listPlayerTypes()
zwift_network::NetworkClient::listZFiles(const std::string &)
zwift_network::NetworkClient::logIn(const std::string &,const std::string &,const std::string &,std::vector<std::string> const&,const std::string &)
zwift_network::NetworkClient::logInWithEmailAndPassword(const std::string &,const std::string &,std::vector<std::string> const&,bool,const std::string &)
zwift_network::NetworkClient::logInWithOauth2Credentials(const std::string &,std::vector<std::string> const&,const std::string &)
zwift_network::NetworkClient::logOut()
zwift_network::NetworkClient::machineId()
zwift_network::NetworkClient::motionData(zwift_network::Motion &)
zwift_network::NetworkClient::myActiveClub()
zwift_network::NetworkClient::myProfile()
zwift_network::NetworkClient::myProfileEntitlements()
zwift_network::NetworkClient::networkTime()
zwift_network::NetworkClient::parseValidationErrorMessage(const std::string &)
zwift_network::NetworkClient::popPhoneToGameCommand(std::shared_ptr<protobuf::PhoneToGameCommand const> &)
zwift_network::NetworkClient::popPlayerIdWithUpdatedProfile(int64_t &)
zwift_network::NetworkClient::popServerToClient(std::shared_ptr<protobuf::ServerToClient const> &)
zwift_network::NetworkClient::popWorldAttribute(std::shared_ptr<protobuf::WorldAttribute const> &)
zwift_network::NetworkClient::privateEventFeed(int64_t,int64_t,zwift_network::Optional<protobuf::EventInviteStatusProto>,bool)
zwift_network::NetworkClient::profile(int64_t,bool)
zwift_network::NetworkClient::profile(const std::string &)
zwift_network::NetworkClient::profiles(std::unordered_set<int64_t> const&)
zwift_network::NetworkClient::querySegmentResults(int64_t,int64_t,int64_t,bool)
zwift_network::NetworkClient::querySegmentResults(int64_t,int64_t,int64_t,bool,int64_t)
zwift_network::NetworkClient::querySegmentResults(int64_t,int64_t,const std::string &,const std::string &,bool)
zwift_network::NetworkClient::redeemCoupon(const std::string &)
zwift_network::NetworkClient::registerForEventSubgroup(int64_t)
zwift_network::NetworkClient::registerInCampaign(const std::string &)
zwift_network::NetworkClient::registerInCampaignV2(const std::string &)
zwift_network::NetworkClient::registerLanExerciseDeviceMessageReceivedCallback(std::function<void ()(zwift_network::LanExerciseDeviceInfo const&,const std::vector<uchar> &)> const&)
zwift_network::NetworkClient::registerLanExerciseDeviceStatusCallback(std::function<void ()(zwift_network::LanExerciseDeviceInfo const&)> const&)
zwift_network::NetworkClient::registerLoggingFunction(std::function<void ()(char const*)> const&)
zwift_network::NetworkClient::registerTelemetryJson(const std::string &,std::function<Json::Value ()(std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>)> const&)
zwift_network::NetworkClient::rejectPrivateEventInvitation(int64_t)
zwift_network::NetworkClient::remoteLog(zwift_network::LogLevel,char const*,char const*)
zwift_network::NetworkClient::remoteLogAndFlush(zwift_network::LogLevel,char const*,char const*)
zwift_network::NetworkClient::removeFollower(int64_t,int64_t)
zwift_network::NetworkClient::removeGoal(int64_t,int64_t)
zwift_network::NetworkClient::removeRegistrationForEvent(int64_t)
zwift_network::NetworkClient::removeSignupForEvent(int64_t)
zwift_network::NetworkClient::resetCredentials()
zwift_network::NetworkClient::resetMyActiveClub()
zwift_network::NetworkClient::resetPassword(const std::string &)
zwift_network::NetworkClient::resumeSubscription()
zwift_network::NetworkClient::returnToHome()
zwift_network::NetworkClient::roundTripLatencyInMilliseconds()
zwift_network::NetworkClient::saveActivity(protobuf::Activity const&,bool,const std::string &)
zwift_network::NetworkClient::saveActivityImage(int64_t,protobuf::ActivityImage const&,const std::string &)
zwift_network::NetworkClient::saveGoal(protobuf::Goal const&)
zwift_network::NetworkClient::savePlayback(protobuf::playback::PlaybackData const&)
zwift_network::NetworkClient::saveProfileReminders(protobuf::PlayerProfile const&)
zwift_network::NetworkClient::saveRouteResult(protobuf::routeresults::RouteResultSaveRequest const&)
zwift_network::NetworkClient::saveSegmentResult(protobuf::SegmentResult const&)
zwift_network::NetworkClient::saveTimeCrossingStartLine(int64_t,protobuf::CrossingStartingLineProto const&)
zwift_network::NetworkClient::saveWorldAttribute(protobuf::WorldAttribute &)
zwift_network::NetworkClient::sendActivatePowerUpCommand(int,uint32_t)
zwift_network::NetworkClient::sendAnalyticsEvent(const std::string &,std::vector<std::string> const&)
zwift_network::NetworkClient::sendBlePeripheralRequest(protobuf::BLEPeripheralRequest const&)
zwift_network::NetworkClient::sendClearPowerUpCommand()
zwift_network::NetworkClient::sendCustomizeActionButtonCommand(uint32_t,uint32_t,const std::string &,const std::string &,bool)
zwift_network::NetworkClient::sendDefaultActivityNameCommand(const std::string &)
zwift_network::NetworkClient::sendDeviceDiagnostics(const std::string &,const std::string &,const std::vector<uchar> &)
zwift_network::NetworkClient::sendGamePacket(const std::string &,bool)
zwift_network::NetworkClient::sendImageToMobileApp(const std::string &,const std::string &)
zwift_network::NetworkClient::sendMessageToLanExerciseDevice(uint32_t,const std::vector<uchar> &)
zwift_network::NetworkClient::sendMixpanelEvent(const std::string &,std::vector<std::string> const&)
zwift_network::NetworkClient::sendMobileAlert(protobuf::MobileAlert const&)
zwift_network::NetworkClient::sendMobileAlertCancelCommand(protobuf::MobileAlert const&)
zwift_network::NetworkClient::sendPlayerProfile(protobuf::PlayerProfile const&)
zwift_network::NetworkClient::sendPlayerState(int64_t,protobuf::PlayerState const&)
zwift_network::NetworkClient::sendRiderListEntries(std::list<protobuf::RiderListEntry> const&)
zwift_network::NetworkClient::sendSetPowerUpCommand(const std::string &,const std::string &,const std::string &,int)
zwift_network::NetworkClient::sendSocialPlayerAction(protobuf::SocialPlayerAction const&)
zwift_network::NetworkClient::setMyActiveClub(protobuf::club::UUID const&)
zwift_network::NetworkClient::setTeleportingAllowed(bool)
zwift_network::NetworkClient::signUrls(const std::string &,std::vector<std::string> const&)
zwift_network::NetworkClient::signupForEventSubgroup(int64_t)
zwift_network::NetworkClient::startScanningForLanExerciseDevices()
zwift_network::NetworkClient::stopScanningForLanExerciseDevices()
zwift_network::NetworkClient::subscribeToSegmentAndGetLeaderboard(int64_t)
zwift_network::NetworkClient::toIso8601(int64_t)
zwift_network::NetworkClient::unlockAchievements(protobuf::achievement::AchievementUnlockRequest const&)
zwift_network::NetworkClient::unsubscribeFromSegment(int64_t)
zwift_network::NetworkClient::updateFollower(int64_t,int64_t,bool,protobuf::ProfileFollowStatus)
zwift_network::NetworkClient::updateNotificationReadStatus(int64_t,int64_t,bool)
zwift_network::NetworkClient::updateProfile(bool,protobuf::PlayerProfile const&,bool)
zwift_network::NetworkClient::updateTelemetrySampleInterval(std::chrono::duration<int64_t int64_t,std::ratio<1l,1l>>)
zwift_network::NetworkClient::uploadReceipt(protobuf::InAppPurchaseReceipt &)
zwift_network::NetworkClient::validateProperty(zwift_network::ProfileProperties,const std::string &)
zwift_network::NetworkClient::withdrawFromCampaign(const std::string &)
zwift_network::NetworkClient::withdrawFromCampaignV2(const std::string &)
zwift_network::NetworkClient::worldTime()
zwift_network::NetworkClient::~NetworkClient()*/
NetworkClient::NetworkClient() { m_pImpl = new(calloc(sizeof(NetworkClientImpl), 1)) NetworkClientImpl; }
NetworkClient::~NetworkClient() { m_pImpl->~NetworkClientImpl(); free(m_pImpl); }
std::future<NetworkResponse<void>> NetworkClient::removeFollowee(int64_t playerId, int64_t followeeId) { return m_pImpl->removeFollowee(playerId, followeeId); }
std::future<NetworkResponse<protobuf::SocialNetworkStatus>> NetworkClient::addFollowee(int64_t playerId, int64_t followeeId, bool a5, protobuf::ProfileFollowStatus pfs) { return m_pImpl->addFollowee(playerId, followeeId, a5, pfs); }
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