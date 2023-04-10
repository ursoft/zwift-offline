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
const char *g_CNL_VER = "3.27.4";
enum HttpRequestMode { HRM_0, HRM_1 };
struct AcceptHeader { std::string m_hdr; };
enum ContentType { CTH_UNK, CTH_JSON, CTH_PB, CTH_PBv2, CTH_URLENC, CTH_OCTET, CTH_CNT };
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
using QueryResult = NetworkResponse<std::vector<char>>;
struct CurlHttpConnection {
    static inline std::mutex g_libcryptoMutex;
    CURL *m_curl = nullptr;
    curl_slist *m_headers = nullptr;
    void *m_cbData;
    std::string m_authHeader, m_sessionState, m_subject, m_sidHeader, m_requestIdHdr, m_cainfo, m_someHeader, m_someHeader1, m_someHeader2;
    std::string m_field_138;
    uint64_t m_requestId = 0, m_headersSize = 0;
    int m_timeout = 0, m_uplTimeout = 0, m_reqIsConcurrent = 0;
    bool m_http2 = false, m_sslNoVerify = false;
    CurlHttpConnection(const std::string &certs, bool ncoSkipCertCheck, bool a5, int ncoTimeoutSec, int ncoUploadTimeoutSec,
        const std::string &curlVersion, const std::string &zaVersion, const std::string &machineId, bool *pKilled, HttpRequestMode hrm) {
        //TODO
    }
    void appendHeader(const std::string &h) {
        if (h.size()) {
            m_headers = curl_slist_append(m_headers, h.data());
            m_headersSize += h.size();
        }
    }
    ~CurlHttpConnection() /*vptr[0]*/ {
        //TODO
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
        if (m_sslNoVerify) {
            curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0);
        }
        appendHeader(m_someHeader);
        appendHeader("SOURCE: Game Client"s);
        appendHeader(m_someHeader1);
        appendHeader(m_someHeader2);
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
        std::vector<char> v16;
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &v16);
        CURLcode v47;
        uint64_t v51;
        if (m_reqIsConcurrent == 1) { // CurlHttpConnection::performConcurrentRequest
            appendHeader(m_field_138);
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
                ret.storeError(31, "Request timed out"s);
                //QUEST: why was empty ret here
                return ret;
            }
            auto v60 = curl_easy_strerror(v47);
            NetworkingLogError("Curl error: [%d] '%s' for: %s %s", v47, v60, op.c_str(), url.c_str());
            ret.storeError(30, v60);
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
            //v65 = v14 = v15 = operator new(0x28ui64), std::_Ref_count_obj2<std::vector<char>>::`vftable'
            //v64 = v16 = v15.vec
            int v31 = 503;
            switch (v59) {
            case 200: case 201: case 202: case 203: case 204: case 205: case 206:
                v31 = 0;
                break;
            case 400: case 401: case 403: case 404: case 409: case 410:
            case 503: case 509:
                v31 = v59;
                break;
            default:
                if (v59 > 503) {
                    std::string v61;
                    if (v16.size())
                        v61.assign(v16.data(), v16.size());
                    NetworkingLogWarn("Unexpected HTTP response: [%d] '%s' for: %s %s", v59, v61.c_str(), op.c_str(), url.c_str());
                    ret.storeError(32, v61);
                    //QUEST: why was empty ret here
                    return ret;
                }
            }
            NetworkingLogDebug("Completed request: %s %s (status: %d, elapsed: %lims)", op.c_str(), url.c_str(), v59, v51);
            if (v31) {
                std::string v61;
                if (v16.size())
                    v61.assign(v16.data(), v16.size());
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
            m_pool.emplace_back([&]() { worker(i); });
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
            m_pool.emplace_back([&]() { worker(oldVal); });
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
        auto conn = m_curlf->instance(m_certs, m_ncoSkipCertCheck, true, m_ncoTimeoutSec, m_ncoUploadTimeoutSec, HRM_0);
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
    std::future<NetworkResponseBase> pushRequestTask(const std::function<NetworkResponseBase(CurlHttpConnection *)> &f) {
        if (m_nThreads == 0)
            startWorkers();
        task_type task(f);
        auto res = task.get_future();
        { std::lock_guard l(m_mutex); m_ptq.push(std::move(task)); }
        m_conditionVar.notify_one();
        return res;
    }
/* TODO:
pushRequestTask<std::multiset<zwift_network::model::Workout>>(std::function<std::shared_ptr<zwift_network::NetworkResponse<std::multiset<zwift_network::model::Workout>> const> ()(HttpConnection &)> const&)
pushComposableRequestTask<std::vector<zwift_network::model::WorkoutsFromPartner>,std::multiset<zwift_network::model::Workout>>(RequestTaskComposer<std::vector<zwift_network::model::WorkoutsFromPartner>,std::multiset<zwift_network::model::Workout>>::Composable,std::function<std::shared_ptr<zwift_network::NetworkResponse<std::multiset<zwift_network::model::Workout>> const> ()(HttpConnection &)> const&)*/
};
struct UdpClient;
struct EventLoop;
struct EncryptionInfo;
struct GlobalState { //0x530 bytes
    std::string m_sessionInfo;
    bool m_shouldUseEncryption;
    GlobalState(EventLoop *, const protobuf::PerSessionInfo &, const std::string &, const EncryptionInfo &);
    bool shouldUseEncryption() { return m_shouldUseEncryption; }
    void registerUdpConfigListener(UdpClient *cli);
    void registerEncryptionListener(UdpClient *cli);
    std::string getSessionInfo() { return m_sessionInfo; }
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
            ret.storeError(23, "Error parsing json"s);
        }
        return ret;
    }
    static NetworkResponse<Json::Value> parseJson(const std::vector<char> &src) {
        NetworkResponse<Json::Value> ret;
        Json::Reader r;
        if (!r.parse(&src.front(), &src.back(), ret, false)) {
            NetworkingLogError("Error parsing JSON: %s\nJSON: %s", r.getFormattedErrorMessages().c_str(), &src.front());
            ret.storeError(23, "Error parsing json"s);
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
    void protobufToCharVector(std::vector<char> *dest, google::protobuf::MessageLite &src) {
        auto size = src.ByteSize();
        dest->resize(size);
        if (!src.SerializeToArray(dest->data(), size))
            LogDebug("Failed to encode protobuf.");
    }
    template <class RET>
    static void convertToResultResponse(NetworkResponse<RET> *ret, const QueryResult &queryResult) {
        if (queryResult.m_errCode) {
            protobuf::ZErrorMessageProtobuf v40;
            ret->m_errCode = queryResult.m_errCode;
            if (v40.ParseFromString(queryResult.m_msg) && v40.message().length())
                ret->m_msg = v40.message();
            else
                ret->m_msg = queryResult.m_msg;
        } else {
            if (!ret->m_T.ParseFromArray(queryResult.m_T.data(), queryResult.m_T.size()))
                ret->storeError(26, "Failed to decode protobuf"s);
        }
    }
    static NetworkResponse<Json::Value> convertToJsonResponse(const QueryResult &src) {
        if(src.m_errCode)
            return NetworkResponse<Json::Value>(src);
        return parseJson(src);
    }
    static NetworkResponse<std::string> convertToStringResponse(const NetworkResponse<std::vector<char>> &src) {
        NetworkResponse<std::string> ret(src);
        if (src.m_errCode == 0)
            ret.m_T.assign(src.m_T.begin(), src.m_T.end());
        return ret;
    }
    static NetworkResponseBase convertToVoidResponse(const NetworkResponse<std::vector<char>> &src) {
        return NetworkResponseBase(src);
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
            storeError(24, "First separator not found"s);
            return false;
        }
        auto secondSep = jwt.find('.', firstSep + 1);
        if (secondSep == -1) {
            storeError(24, "Second separator not found"s);
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
                    m_errCode = 0;
                    m_msg.clear();
                } else {
                    refreshAccessToken(conn);
                    return (m_errCode) ? acquireAccessToken(conn) : *this;
                }
            } else {
                return acquireAccessToken(conn);
            }
        } else {
            m_errCode = 429;
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
        m_errCode = 0;
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
            m_accessTokenDeathTime = g_steadyClock.now() + 1000000000ull * m_oauth2.m_exp - 30000000000ull;
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
            auto resp = conn->performPost(m_authUrl,
                ContentTypeHeader(CTH_URLENC),
                qsb.getString(false),
                AcceptHeader{ "Accept: application/json"s },
                "Acquire Access Token"s,
                false);
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
                m_oauth2.m_errCode = 401;
            }
            return m_oauth2;
        }
        return m_oauth2.storeError(27, "Could not acquire access token because credentials are missing"s);
    }
    const NetworkResponseBase &refreshAccessToken(CurlHttpConnection *conn) {
        if (m_accessTokenInvalid || _Xtime_get_ticks() >= getRefreshToken().m_exp)
            return m_oauth2.storeError(28, "Refresh token expired"s);
        if (m_authUrl.empty() && !findAuthenticationServer(conn))
            return m_oauth2;
        QueryStringBuilder qsb;
        qsb.add("grant_type"s, "refresh_token"s);
        qsb.add("refresh_token"s, conn->escapeUrl(getRefreshToken().asString()));
        qsb.add("client_id"s, conn->escapeUrl(m_oauthClient));
        conn->setRequestId(++m_reqId);
        auto resp = conn->performPost(m_authUrl,
            ContentTypeHeader(CTH_URLENC),
            qsb.getString(false),
            AcceptHeader{ "Accept: application/json"s },
            "Refresh Access Token"s,
            false);
        parseOauth2Credentials(HttpHelper::convertToStringResponse(resp));
        if (resp.m_errCode == 400 || resp.m_errCode == 401 || resp.m_errCode == 403) {
            NetworkingLogError("Error refreshing access token: [%d] %s", m_oauth2.m_errCode, m_oauth2.m_msg.c_str());
            m_accessToken.clear();
            m_accessTokenDeathTime = 0;
            m_accessTokenInvalid = true;
            m_throttlingLevel = 0;
            m_ratNextTime = 0;
            m_oauth2.m_errCode = 401;
        } else if (resp.m_errCode) {
            NetworkingLogError("Error refreshing access token: [%d] %s", m_oauth2.m_errCode, m_oauth2.m_msg.c_str());
        } else {
            setTokens(true);
        }
        return m_oauth2;
    }
    void clearEmailAndPassword() {
        //TODO
    }
    void clearTokenHeaders() {
        //TODO
    }
    bool findAuthenticationServer(CurlHttpConnection *conn) {
        conn->setRequestId(++m_reqId);
        m_oauth2.storeError(0, nullptr);
        auto v6 = conn->performGet(m_apiUrl, AcceptHeader{ "Accept: application/json"s }, "Find Authentication Server"s);
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
        //TODO
        bool m_secured = false;
    };
    std::queue<RequestTaskContext> m_rtq;
    GlobalState *m_gs = nullptr;
    ZwiftAuthenticationManager *m_authMgr = nullptr;
    volatile int64_t m_requestId = 1;
    bool m_needNewAcToken = false, m_some0;
    void setGlobalState(GlobalState *gs) { m_gs = gs; }
    ZwiftHttpConnectionManager(CurlHttpConnectionFactory *curlf, const std::string &certs, bool ncoSkipCertCheck, ZwiftAuthenticationManager *am, bool some0, int ncoTimeoutSec, int ncoUploadTimeoutSec, HttpRequestMode rm, int nThreads) :
        HttpConnectionManager(curlf, certs, ncoSkipCertCheck, ncoTimeoutSec, ncoUploadTimeoutSec, rm, nThreads), m_authMgr(am), m_some0(some0) {
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
            std::unique_lock<std::mutex> lock(m_mutex);
            m_conditionVar.wait(lock, [this] { return this->m_needNewAcToken && !this->m_rtq.empty(); });
            conn->clearAuthorizationHeader();
            conn->clearTokenInfo();
            bool needNewAcToken = false;
            RequestTaskContext task;
            {
                task = std::move(m_rtq.front());
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
                m_rtq.pop();
                conn->setTimeout(m_ncoTimeoutSec);
                conn->setUploadTimeout(m_ncoUploadTimeoutSec);
                if (m_gs)
                    conn->setSessionIdHeader(m_gs->getSessionInfo());
                else
                    conn->setSessionIdHeader("");
            }
            task(conn, needNewAcToken);
        }
    }
    ~ZwiftHttpConnectionManager() { shutdown(); }
    template<class RET>
    std::future<NetworkResponse<RET>> pushRequestTask(const std::function<NetworkResponse<RET>(CurlHttpConnection *)> &f, bool b1, bool b2) {
        std::future<NetworkResponse<RET>> ret;
        if (b2 || !m_some0) {
#if 0
            v48 = this;
            v50 = 0i64;
            v15 = *(__int64(__fastcall ****)(_QWORD, char *))(func + 56);
            if (v15)
                v50 = (char *)(**v15)(v15, v49);
            v59 = 0i64;
            v16 = (char *)operator new(0x50ui64);
            v55[3] = v16;
            *(_QWORD *)v16 = &std::_Func_impl_no_alloc<_lambda_dea4d45f1c93d919d606ab83b6fd9293_, std::shared_ptr<zwift_network::NetworkResponse<std::string> const>, HttpConnection &, bool>::`vftable';
                * ((_QWORD *)v16 + 1) = v48;
            *((_QWORD *)v16 + 9) = 0i64;
            v17 = v50;
            if (v50)
            {
                *((_QWORD *)v16 + 9) = (**(__int64(__fastcall ***)(char *, __int64))v50)(v50, (__int64)(v16 + 16));
                v17 = v50;
            }
            v59 = v16;
            if (v17)
            {
                v18 = v49;
                LOBYTE(v18) = v17 != v49;
                (*(void(__fastcall **)(char *, char *))(*(_QWORD *)v17 + 32i64))(v17, v18);
                v50 = 0i64;
            }
            v19 = operator new(0x28ui64);
            v21 = v19;
            if (v19)
            {
                *(_OWORD *)v19 = 0i64;
                v19[2] = 1;
                v19[3] = 1;
                *(_QWORD *)v19 = &std::_Ref_count_obj2<std::packaged_task<std::shared_ptr<zwift_network::NetworkResponse<std::string> const>(HttpConnection &, bool)>>::`vftable';
                    v22 = operator new(0x120ui64);
                v23 = v22;
                if (v22)
                {
                    memset(v22, 0, 0x120ui64);
                    sub_7FF703263A00((__int64)v23, 0i64);
                    *v23 = &std::_Packaged_state<std::shared_ptr<zwift_network::NetworkResponse<std::string> const>(HttpConnection &, bool)>::`vftable';
                        v23[35] = 0i64;
                    if (v16 == v58)
                    {
                        v23[35] = (*(__int64(__fastcall **)(char *, _QWORD *))(*(_QWORD *)v16 + 8i64))(v16, v23 + 28);
                        (*(void(__fastcall **)(char *, _QWORD))(*(_QWORD *)v16 + 32i64))(v16, 0i64);
                    } else
                    {
                        v23[35] = v16;
                    }
                    v16 = 0i64;
                    v59 = 0i64;
                    *((_QWORD *)v21 + 2) = v23;
                    *((_BYTE *)v21 + 24) = 0;
                    *((_BYTE *)v21 + 32) = 0;
                } else
                {
                    *((_QWORD *)v21 + 2) = 0i64;
                    *((_BYTE *)v21 + 24) = 0;
                    *((_BYTE *)v21 + 32) = 0;
                }
            } else
            {
                v21 = 0i64;
            }
            v24 = (__int128 *)(v21 + 4);
            v63 = (__int128 *)(v21 + 4);
            v64 = v21;
            if (v16)
            {
                LOBYTE(v20) = v16 != v58;
                (*(void(__fastcall **)(char *, __int64))(*(_QWORD *)v16 + 32i64))(v16, v20);
            }
            v38 = 0i64;
            if (!*(_QWORD *)v24 || *((_BYTE *)v21 + 24) && *(_BYTE *)(*(_QWORD *)v24 + 200i64))
            {
                error_code = (const struct std::error_code *)std::make_error_code(v62, 4i64);
                std::_Throw_future_error(error_code);
            }
            if (*((_BYTE *)v21 + 32))
            {
                v35 = (const struct std::error_code *)std::make_error_code(v61, 2i64);
                std::_Throw_future_error(v35);
            }
            *((_BYTE *)v21 + 32) = 1;
            v25 = 0i64;
            *(_QWORD *)&v38 = 0i64;
            if (&v38 != v24)
            {
                if (*(_QWORD *)v24)
                {
                    _InterlockedIncrement((volatile signed __int32 *)(*(_QWORD *)v24 + 8i64));
                    v25 = *(_QWORD *)v24;
                    v24 = v63;
                } else
                {
                    v25 = 0i64;
                }
                *(_QWORD *)&v38 = v25;
            }
            BYTE8(v38) = 1;
            p_m_mutex = (_Mtx_internal_imp_t *)&this->m_base.m_mutex;
            v26 = Mtx_lock((_Mtx_t)&this->m_base.m_mutex);
            if (v26)
                std::_Throw_C_error(v26);
            if (v21)
            {
                _InterlockedIncrement(v21 + 2);
                v24 = v63;
                v25 = v38;
            }
            *(_QWORD *)&v41 = v24;
            v27 = v21;
            *((_QWORD *)&v41 + 1) = v21;
            v28 = operator new(0x118ui64);
            v29 = v28;
            if (v28)
            {
                memset(v28, 0, 0x118ui64);
                sub_7FF70359C430((__int64)v29, 0i64);
                *v29 = &std::_Packaged_state<std::nullptr_t>::`vftable';
                    v41 = 0i64;
                v29[27] = &std::_Func_impl_no_alloc<_lambda_08f659b348b191e2e4e18e80202026f3_, std::nullptr_t, HttpConnection &, bool>::`vftable';
                    v29[28] = v24;
                v29[29] = v21;
                v29[34] = v29 + 27;
                v27 = (_DWORD *)*((_QWORD *)&v41 + 1);
            } else
            {
                v29 = 0i64;
            }
            v46 = 0;
            v47 = 0;
            v51 = v29;
            v45 = 0i64;
            v52 = 0;
            v53 = 0;
            v54 = b1;
            Stopwatch::Stopwatch(v55);
            v30 = (__int64)this;
            sub_7FF7035D3970(&this->m_rtq.Myproxy, (__int64)&v51);
            SteadyClock_dtr_0(v55);
            future_stuff_0((__int64 *)&v51);
            future_stuff_0(&v45);
            if (v27)
            {
                if (_InterlockedExchangeAdd(v27 + 2, 0xFFFFFFFF) == 1)
                {
                    v31 = *((_QWORD *)&v41 + 1);
                    (***((void(__fastcall ****)(_QWORD)) & v41 + 1))(*((_QWORD *)&v41 + 1));
                    if (_InterlockedExchangeAdd((volatile signed __int32 *)(v31 + 12), 0xFFFFFFFF) == 1)
                        (*(void(__fastcall **)(_QWORD))(**((_QWORD **)&v41 + 1) + 8i64))(*((_QWORD *)&v41 + 1));
                }
                v25 = v38;
            }
            Mtx_unlock(p_m_mutex);
            Cnd_signal((_Cnd_t)(v30 + 184));
            *(_QWORD *)ret = 0i64;
            if ((__int128 *)ret != &v38)
            {
                *(_QWORD *)ret = v25;
                v25 = 0i64;
                *(_QWORD *)&v38 = 0i64;
                *(_BYTE *)(ret + 8) = 1;
            }
            *(_BYTE *)(ret + 8) = 1;
#endif
            //TODO
            RequestTaskContext task([&](CurlHttpConnection *conn, bool needNewAcToken) { //pushRequestTask_lambda_0
                if (needNewAcToken) {
                    auto rett = this->attendToAccessToken(conn);
                    if (rett.m_errCode)
                        return NetworkResponse<RET>(std::move(rett));
                    conn->setAuthorizationHeader(this->m_authMgr->getAccessTokenHeader());
                    conn->setTokenInfo(this->m_authMgr->getSessionStateFromToken(), this->m_authMgr->getSubjectFromToken());
                }
                conn->setRequestId(InterlockedExchangeAdd64(&this->m_requestId, 1));
                auto ret = f(conn);
                if (ret.m_errCode == 401) {
                    NetworkingLogWarn("Request status unauthorized, token invalidated.");
                    this->m_authMgr->setAccessTokenAsExpired();
                }
                return ret;
            });
            auto res = task.get_future();
            { std::lock_guard l(m_mutex); m_rtq.push(std::move(task)); }
            m_conditionVar.notify_one();
        } else {
            std::promise<NetworkResponse<RET>> p;
            ret = p.get_future();
            p.set_value(NetworkResponse<RET>{"Disconnected due to simultaneous log ins"s, 14});
        }
        return ret;
    }
};
struct EncryptionInfo {
    std::string m_sk;
    uint32_t m_relaySessionId = 0, m_expiration = 0;
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
struct AuthServerRestInvoker { //0x60 bytes
    const std::string &m_machineId, &m_server;
    ZwiftAuthenticationManager *m_authMgr;
    ExperimentsRestInvoker *m_expRi;
    ZwiftHttpConnectionManager *m_conn;
    NetworkResponseBase logIn(const EncryptionOptions &encr, const std::vector<std::string> &anEventProps, 
        const std::function<void(const protobuf::PerSessionInfo &, const std::string &, const EncryptionInfo &)> &func) {
        m_conn->pushRequestTask<std::string>([&](CurlHttpConnection *conn) {
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
                return NetworkResponse<std::string>(m_authMgr->getRefreshToken());
            }
            this->m_authMgr->resetCredentials();
            NetworkingLogError("Couldn't obtain a session id from the server.");
            return NetworkResponse<std::string>(ret);
        }, true, false);
        return NetworkResponseBase{};
    }
    AuthServerRestInvoker(const std::string &machineId, ZwiftAuthenticationManager *authMgr, ZwiftHttpConnectionManager *httpConnMgr3, ExperimentsRestInvoker *expRi, const std::string &server) : m_machineId(machineId), m_server(server), m_authMgr(authMgr), m_expRi(expRi), m_conn(httpConnMgr3) {}
    NetworkResponse<protobuf::LoginResponse> doLogIn(const std::string &sk, const std::vector<std::string> &anEventProps, CurlHttpConnection *conn) {
        NetworkResponse<protobuf::LoginResponse> ret;
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
            AcceptHeader{ "Accept: application/x-protobuf-lite"s }, LogInV2, false);
        HttpHelper::convertToResultResponse(&ret, v42);
        return ret;
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
    NetworkResponseBase logOut(const std::function<void()> &) {
        //TODO
    }
    void resetPassword(const std::string &) {
        //TODO
    }
    bool shouldEnableEncryptionBasedOnFeatureFlag(const EncryptionOptions &) {
        //TODO
        return true;
    }
    ~AuthServerRestInvoker() {
        //TODO
    }
};
//template<class RET> <bool>()
//sometype NetworkResponseHelper_makeNetworkResponseFuture(zwift_network::NetworkRequestOutcome, std::string)
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
        boost::asio::post(m_asioCtx, [&]() { 
            m_asioCtx.stop(); });
        if (m_thrd.joinable()) 
            m_thrd.join();
    }
    void post(std::function<void()> &&f) { boost::asio::post(m_asioCtx, f); }
};
struct NetworkClockService;
struct WorldClockService { //0x2120 bytes
    WorldClockService(EventLoop *el, NetworkClockService *ncs) {
        //TODO
    }
};
struct RelayServerRestInvoker { //0x30 bytes
    RelayServerRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &) {
        //TODO
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
struct UdpClient { //0x1400-16 bytes
    //            UdpClient::UdpClient(GlobalState *, WorldClockService *, HashSeedService *, HashSeedService *, UdpStatistics *, RelayServerRestInvoker *, TelemetryService *, UdpClient::Listener &, std::chrono::duration<long long, std::ratio<1l, 1l>>, std::chrono::duration<long long, std::ratio<1l, 1000l>>)
    UdpClient(GlobalState *, WorldClockService *, HashSeedService *, HashSeedService *, UdpStatistics *, RelayServerRestInvoker *, void /*netImpl*/ *) {
        //TODO
    }
};
struct WorldAttributeService { //0x270 bytes
    WorldAttributeService(HashSeedService *hs) {
        //TODO
    }
    void registerListener(UdpClient *cli) {
        //TODO
    }
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
struct ExperimentsRestInvoker { //0x30 bytes
    ExperimentsRestInvoker(ZwiftHttpConnectionManager *mgr, const std::string &server) {
        //TODO
    }
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
struct NetworkClientImpl { //0x400 bytes, calloc
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
    moodycamel::ReaderWriterQueue<const AuxiliaryControllerAddress> m_rwqAux;
    AuxiliaryControllerAddress m_curAux;
    uint32_t m_field_10 = 100;
    bool m_someFunc0 = false, m_initOK = false, m_loginOK = false;
    NetworkClientImpl() : m_rwqAux(1) { //QUEST: why two vtables
        google::protobuf::internal::VerifyVersion(3021000 /* URSOFT FIX: slightly up from 3020000*/, 3020000, __FILE__);
    }
    void somefunc0_0() {
        //TODO
#if 0
        if (!*(_QWORD *)&a1->field_98[656] && !(unsigned __int8)sub_7FF620845640(&a1->m_nco)) {
            v2 = (volatile signed __int32 *)operator new(0x10C80ui64);
            v3 = v2;
            if (v2) {
                *(_OWORD *)v2 = 0i64;
                *((_DWORD *)v2 + 2) = 1;
                *((_DWORD *)v2 + 3) = 1;
                *(_QWORD *)v2 = &std::_Ref_count_obj2<TcpClient>::`vftable';
                    sub_7FF620A3A4F0(
                        (v2 + 16),
                        &a1->field_50,
                        (__int64 *)&a1->field_98[424],
                        (__int64 *)&a1->field_98[440],
                        (__int64 *)&a1->field_98[488],
                        &a1->field_98[640],
                        &a1->field_98[248],
                        &a1->field_98[280],
                        a1);
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
    void shutdownUdpClient() {
        //TODO
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
        //smart shared pointers actively used here, but what for???
        m_httpConnMgr0 = new GenericHttpConnectionManager(&m_curlf, certs, skipCertCheck, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_0);
        m_httpConnMgr1 = new GenericHttpConnectionManager(&m_curlf, certs, skipCertCheck, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_1);
        m_httpConnMgr2 = new GenericHttpConnectionManager(&m_curlf, certs, skipCertCheck, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_0);
        m_authMgr = new ZwiftAuthenticationManager(server);
        m_httpConnMgr3 = new ZwiftHttpConnectionManager(&m_curlf, certs, m_nco.m_skipCertCheck, m_authMgr, m_someFunc0, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_0, 3);
        m_httpConnMgr4 = new ZwiftHttpConnectionManager(&m_curlf, certs, m_nco.m_skipCertCheck, m_authMgr, m_someFunc0, m_nco.m_timeoutSec, m_nco.m_uploadTimeoutSec, HRM_1, 3);
        m_expRi = new ExperimentsRestInvoker(m_httpConnMgr3, server);
        m_arRi = new ActivityRecommendationRestInvoker(m_httpConnMgr4, server);
        m_achRi = new AchievementsRestInvoker(m_httpConnMgr4, server);
        m_authInvoker = new AuthServerRestInvoker(m_machine.m_id, m_authMgr, m_httpConnMgr3, m_expRi, server);
        m_camRi = new CampaignRestInvoker(server);
        m_clubsRi = new ClubsRestInvoker(m_httpConnMgr3, server);
        m_ecRi = new EventCoreRestInvoker(m_httpConnMgr3, server);
        m_efRi = new EventFeedRestInvoker(m_httpConnMgr4, server);
        m_fuRi = new FirmwareUpdateRestInvoker(m_httpConnMgr3, server);
        m_gRi = new GenericRestInvoker(m_httpConnMgr1);
        m_peRi = new PrivateEventsRestInvoker(m_httpConnMgr3, server);
        m_rarRi = new RaceResultRestInvoker(m_httpConnMgr3, server);
        m_rorRi = new RouteResultsRestInvoker(m_httpConnMgr3, server);
        m_ppbRi = new PlayerPlaybackRestInvoker(m_httpConnMgr4, server);
        m_srRi = new SegmentResultsRestInvoker(m_httpConnMgr4, server);
        m_pcRi = new PowerCurveRestInvoker(m_httpConnMgr4, server);
        m_zfRi = new ZFileRestInvoker(m_httpConnMgr3, server);
        m_zwRi = new ZwiftWorkoutsRestInvoker(m_httpConnMgr3, server);
        m_wsRi = new WorkoutServiceRestInvoker(m_httpConnMgr4, server);
        m_udpStat = new UdpStatistics();
        m_tcpStat = new TcpStatistics();
        m_wcStat = new WorldClockStatistics();
        m_lanStat = new LanExerciseDeviceStatistics();
        m_auxStat = new AuxiliaryControllerStatistics();
        m_waStat = new WorldAttributeStatistics();
        //OMIT telemetry
        m_restInvoker = new RestServerRestInvoker(m_machine.m_id, m_httpConnMgr3, server, zaVersion);
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
        m_wat = new WorldAttributeService(m_hashSeed2);
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
    NetworkResponseBase logInWithOauth2Credentials(/*ret a2,*/ const std::string &sOauth, const std::vector<std::string> &anEventProps, const std::string &oauthClient) {
        if (m_authMgr && m_authInvoker) {
            auto ret = m_authMgr->setCredentialsMid(sOauth, oauthClient);
            return ret.m_errCode ? ret :
                m_authInvoker->logIn({ m_nco.m_disableEncr, m_nco.m_disableEncryptionWithServer, m_nco.m_ignoreEncryptionFeatureFlag, m_nco.m_secretKeyBase64 },
                    anEventProps, 
                    [&](const protobuf::PerSessionInfo &psi, const std::string &str, const EncryptionInfo &ei) { onLoggedIn(psi, str, ei); });
        }
        return NetworkResponseBase{ "Initialize CNL first", 2 };
    }
    NetworkResponseBase logInWithEmailAndPassword(const std::string &email, const std::string &pwd, const std::vector<std::string> &anEventProps, bool reserved, const std::string &oauthClient) {
        if (m_authMgr && m_authInvoker) {
            m_authMgr->setCredentialsOld(email, pwd, oauthClient);
            if (reserved)
                return NetworkResponseBase{ "Good luck, soldier", 4 };
            else
                return m_authInvoker->logIn(
                    { m_nco.m_disableEncr, m_nco.m_disableEncryptionWithServer, m_nco.m_ignoreEncryptionFeatureFlag, m_nco.m_secretKeyBase64},
                    anEventProps, [&](const protobuf::PerSessionInfo &psi, const std::string &str, const EncryptionInfo &ei) { onLoggedIn(psi, str, ei); });
        }
        return NetworkResponseBase{ "Initialize CNL first", 2 };
    }
    void shutdownServiceEventLoop() {
        if (m_evLoop) {
            m_evLoop->shutdown();
            m_evLoop = nullptr;
        }
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
void get_goals(int64_t playerId) { 
    //TODO
}
void save_goal(const protobuf::Goal &g) {
    //TODO
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
        g_networkClient->m_pImpl->m_authMgr->getLoggedIn();
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
void GlobalState::registerUdpConfigListener(UdpClient *cli) {
    //TODO
}
void GlobalState::registerEncryptionListener(UdpClient *cli) {
    //TODO
}
GlobalState::GlobalState(EventLoop *, const protobuf::PerSessionInfo &, const std::string &, const EncryptionInfo &) {
    //TODO
}
NetworkResponseBase NetworkClient::logInWithOauth2Credentials(const std::string &sOauth, const std::vector<std::string> &anEventProps, const std::string &oauthClient) { return m_pImpl->logInWithOauth2Credentials(sOauth, anEventProps, oauthClient); }

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
TEST(SmokeTest, DISABLED_LoginTest) {
    auto token = "{\"access_token\":\"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjkiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiQmVhcmVyIiwiYXpwIjoiR2FtZV9MYXVuY2hlciIsImF1dGhfdGltZSI6MTUzNTUwNzI0OSwic2Vzc2lvbl9zdGF0ZSI6IjA4NDZubzluLTc2NXEtNHAzcy1uMjBwLTZwbnA5cjg2cjVzMyIsImFjciI6IjAiLCJhbGxvd2VkLW9yaWdpbnMiOlsiaHR0cHM6Ly9sYXVuY2hlci56d2lmdC5jb20qIiwiaHR0cDovL3p3aWZ0Il0sInJlYWxtX2FjY2VzcyI6eyJyb2xlcyI6WyJldmVyeWJvZHkiLCJ0cmlhbC1zdWJzY3JpYmVyIiwiZXZlcnlvbmUiLCJiZXRhLXRlc3RlciJdfSwicmVzb3VyY2VfYWNjZXNzIjp7Im15LXp3aWZ0Ijp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiR2FtZV9MYXVuY2hlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFJFU1QgQVBJIC0tIHByb2R1Y3Rpb24iOnsicm9sZXMiOlsiYXV0aG9yaXplZC1wbGF5ZXIiLCJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFplbmRlc2siOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSZWxheSBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIl19LCJlY29tLXNlcnZlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sImFjY291bnQiOnsicm9sZXMiOlsibWFuYWdlLWFjY291bnQiLCJtYW5hZ2UtYWNjb3VudC1saW5rcyIsInZpZXctcHJvZmlsZSJdfX0sIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20iLCJzZXNzaW9uX2Nvb2tpZSI6IjZ8YTJjNWM1MWY5ZDA4YzY4NWUyMDRlNzkyOWU0ZmMyMDAyOWI5ODE1OGYwYjdmNzk0MmZiMmYyMzkwYWMzNjExMDMzN2E3YTQyYjVlNTcwNmVhODM0YjQzYzFlNDU1NzJkMTQ2MzIwMTQxOWU5NzZjNTkzZWZjZjE0M2UwNWNiZjgifQ._kPfXO8MdM7j0meG4MVzprSa-3pdQqKyzYMHm4d494w\",\"expires_in\":1000021600,\"id_token\":\"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjciLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiSUQiLCJhenAiOiJHYW1lX0xhdW5jaGVyIiwiYXV0aF90aW1lIjoxNTM1NTA3MjQ5LCJzZXNzaW9uX3N0YXRlIjoiMDg0Nm5vOW4tNzY1cS00cDNzLW4yMHAtNnBucDlyODZyNXMzIiwiYWNyIjoiMCIsIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20ifQ.rWGSvv5TFO-i6LKczHNUUcB87Hfd5ow9IMG9O5EGR4Y\",\"not-before-policy\":1408478984,\"refresh_expires_in\":611975560,\"refresh_token\":\"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjgiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiUmVmcmVzaCIsImF6cCI6IkdhbWVfTGF1bmNoZXIiLCJhdXRoX3RpbWUiOjAsInNlc3Npb25fc3RhdGUiOiIwODQ2bm85bi03NjVxLTRwM3MtbjIwcC02cG5wOXI4NnI1czMiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZXZlcnlib2R5IiwidHJpYWwtc3Vic2NyaWJlciIsImV2ZXJ5b25lIiwiYmV0YS10ZXN0ZXIiXX0sInJlc291cmNlX2FjY2VzcyI6eyJteS16d2lmdCI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIkdhbWVfTGF1bmNoZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIiwiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBaZW5kZXNrIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiWndpZnQgUmVsYXkgUkVTVCBBUEkgLS0gcHJvZHVjdGlvbiI6eyJyb2xlcyI6WyJhdXRob3JpemVkLXBsYXllciJdfSwiZWNvbS1zZXJ2ZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJhY2NvdW50Ijp7InJvbGVzIjpbIm1hbmFnZS1hY2NvdW50IiwibWFuYWdlLWFjY291bnQtbGlua3MiLCJ2aWV3LXByb2ZpbGUiXX19LCJzZXNzaW9uX2Nvb2tpZSI6IjZ8YTJjNWM1MWY5ZDA4YzY4NWUyMDRlNzkyOWU0ZmMyMDAyOWI5ODE1OGYwYjdmNzk0MmZiMmYyMzkwYWMzNjExMDMzN2E3YTQyYjVlNTcwNmVhODM0YjQzYzFlNDU1NzJkMTQ2MzIwMTQxOWU5NzZjNTkzZWZjZjE0M2UwNWNiZjgifQ.5e1X1imPlVfXfhDHE_OGmG9CNGvz7hpPYPXcNkPJ5lw\",\"scope\":\"\",\"session_state\":\"0846ab9a-765d-4c3f-a20c-6cac9e86e5f3\",\"token_type\":\"bearer\"}"s;
    g_MainThread = GetCurrentThreadId();
    ZNETWORK_Initialize();
    EXPECT_FALSE(ZNETWORK_IsLoggedIn());
    std::vector<std::string> v{"OS"s};
    g_networkClient->logInWithOauth2Credentials(token, v, "Game_Launcher"s);
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