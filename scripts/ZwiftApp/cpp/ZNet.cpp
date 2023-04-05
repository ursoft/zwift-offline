#include "ZwiftApp.h"
#include "readerwriterqueue/readerwriterqueue.h"
#include "concurrentqueue/concurrentqueue.h"
#include "openssl/md5.h"
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
struct ContentTypeHeader { std::string m_hdr; };
struct NetworkResponseBase {
    const char *m_errMsg = nullptr;
    int m_errCode = 0;
    void storeError(int code, const char *errMsg) { m_errMsg = errMsg; m_errCode = code; }
    void storeError(const NetworkResponseBase &src) { m_errMsg = src.m_errMsg; m_errCode = src.m_errCode; }
    //void storeError(int code, std::string &&errMsg) { m_errMsg = std::move(errMsg); m_errCode = code; }
    //void storeError(int code, const std::string &errMsg) { m_errMsg = errMsg; m_errCode = code; }
    bool ok(NetworkResponseBase *errDest = nullptr) const {
        if (errDest)
            errDest->storeError(*this);
        return m_errCode == 0;
    }
};
template<class T> struct NetworkResponse : public NetworkResponseBase, T {};
using QueryResult = NetworkResponse<std::vector<char>>;
struct CurlHttpConnection {
    CURL *m_curl = nullptr;
    curl_slist *m_headers = nullptr;
    void *m_cbData;
    std::string m_authHeader, m_sessionState, m_subject, m_sidHeader, m_requestIdHdr, m_cainfo, m_someHeader, m_someHeader1, m_someHeader2;
    std::string m_field_138;
    uint64_t m_requestId = 0, m_headersSize = 0;
    int m_timeout = 0, m_uplTimeout = 0, m_field_58 = 0;
    bool m_http2 = false, m_sslNoVerify = false;
    CurlHttpConnection(const std::string &certs, bool ncoSkipCertCheck, bool a5, int ncoTimeoutSec, int ncoUploadTimeoutSec,
        const std::string &curlVersion, const std::string &zaVersion, const std::string &machineId, bool *pKilled, HttpRequestMode hrm) {
        //TODO
    }
    void appendHeader(const std::string_view &h) {
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
    QueryResult performGet(const std::string &url, const AcceptHeader &ach, const std::string &descr) /*vptr[10]*/ {
        reset(false, 0, true);
        curl_easy_setopt(m_curl, CURLOPT_HTTPGET, 1);
        appendHeader(ach.m_hdr);
        const std::string_view op("GET");
        return performRequest(op, url, descr, 0);
    }
    QueryResult performDelete(const std::string &url, const AcceptHeader &ach, const std::string &descr) /*vptr[11]*/ {
        reset(false, 0, true);
        const std::string_view op("DELETE");
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, op.data());
        appendHeader(ach.m_hdr);
        return performRequest(op, url, descr, 0);
    }
    const static inline std::string_view POST = "POST";
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
    const static inline std::string_view PUT = "PUT";
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
        userdata->insert(userdata->end(), ptr, ptr + size * nmemb);
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
        const std::string_view srcHeader("SOURCE: Game Client");
        appendHeader(srcHeader);
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
    QueryResult performRequest(const std::string_view &op, const std::string &url, const std::string &descr, int payloadSize) {
        QueryResult ret;
        appendHeader(m_sidHeader);
        appendHeader(m_requestIdHdr);
        curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_headers);
#if 0 //TODO
        v14 = operator new(0x28ui64);
        v15 = v14;
        if (v14)
        {
            *(_OWORD *)v14 = 0i64;
            v14[2] = 1;
            v14[3] = 1;
            *(_QWORD *)v14 = &std::_Ref_count_obj2<std::vector<char>>::`vftable';
                * ((_QWORD *)v14 + 2) = 0i64;
            *((_QWORD *)v14 + 3) = 0i64;
            *((_QWORD *)v14 + 4) = 0i64;
        } else
        {
            v15 = 0i64;
        }
        v16 = (char **)(v15 + 4);
        v64 = (char **)(v15 + 4);
        v65 = v15;
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, v15 + 4);
        if (this->m_field_58 == 1)                  // CurlHttpConnection::performConcurrentRequest
        {
            p_m_field_138 = &this->m_field_138;
            if (this->m_field_138._Mysize)
            {
                if (this->m_field_138._Myres >= 0x10ui64)
                    p_m_field_138 = (std::string *)p_m_field_138->_Bx._Ptr;
                this->m_headers = curl_slist_append((_QWORD *)this->m_headers, p_m_field_138->_Bx._Buf);
                this->m_headersSize += this->m_field_138._Mysize;
            }
            Stopwatch::Stopwatch(v62);
            v50 = url;
            if (url->_Myres >= 0x10ui64)
                v50 = (std::string *)url->_Bx._Ptr;
            v46 = op;
            if (op->_Myres >= 0x10ui64)
                v46 = (std::string *)op->_Bx._Ptr;
            Logger = Logger::getLogger();
            NetworkingLog(
                NL_DEBUG,
                &Logger->m_debugPrefix,
                "Performing concurrent request: %s %s (payload: %li bytes)",
                v46->_Bx._Buf,
                v50->_Bx._Buf,
                payloadSize);
            Stopwatch::start(v62);
            v47 = sub_7FF6B4D6ED30((_QWORD *)this->m_curl);
            Stopwatch::stop(v62);
            v51 = Stopwatch::elapsedInMilliseconds(v62);
            v19 = v62;
        } else                                          // CurlHttpConnection::performSequentialRequest
        {
            Stopwatch::Stopwatch(v66);
            Stopwatch::Stopwatch(v62);
            Stopwatch::start(v66);
            v55 = &CurlHttpConnection::libcryptoMutex;
            v20 = Mtx_lock(&CurlHttpConnection::libcryptoMutex);
            if (v20)
                std::_Throw_C_error(v20);
            Stopwatch::stop(v66);
            v52 = url;
            if (url->_Myres >= 0x10ui64)
                v52 = (std::string *)url->_Bx._Ptr;
            v48 = op;
            if (op->_Myres >= 0x10ui64)
                v48 = (std::string *)op->_Bx._Ptr;
            v21 = Logger::getLogger();
            NetworkingLog(
                NL_DEBUG,
                &v21->m_debugPrefix,
                "Performing request: %s %s (payload: %li bytes)",
                v48->_Bx._Buf,
                v52->_Bx._Buf,
                payloadSize);
            Stopwatch::start(v62);
            v47 = sub_7FF6B4D6ED30((_QWORD *)this->m_curl);
            Stopwatch::stop(v62);
            Mtx_unlock(&CurlHttpConnection::libcryptoMutex);
            v22 = sub_7FF6B4978010(v66, &v56);
            HttpStatistics::addRequestLockTime(m_stat->field_0, *v22);
            v51 = Stopwatch::elapsedInMilliseconds(v62);
            SteadyClock_getvtbl(v62);
            v19 = v66;
        }
        SteadyClock_getvtbl(v19);
        if (v47)
        {
            //OMIT HttpStatistics::enqueueEndpointEvent(m_stat, descr, this->m_requestId, 0, v47, v51);
            if (v47 == 28)
            {
                if (url->_Myres >= 0x10ui64)
                    url = (std::string *)url->_Bx._Ptr;
                if (op->_Myres >= 0x10ui64)
                    op = (std::string *)op->_Bx._Ptr;
                v23 = Logger::getLogger();
                NetworkingLog(NL_WARN, &v23->m_warnPrefix, "Request timed out for: %s %s", op->_Bx._Buf, url->_Bx._Buf);
                v53._Bx._Ptr = 0i64;
                v53._Mysize = 0i64;
                v53._Myres = 15i64;
                string_assign(&v53, "Request timed out", 0x11ui64);
                v24 = (__int128 *)&v53;
                v25 = 31;
                goto LABEL_70;
            }
            v26 = (char *)sub_7FF6B4D6FDE0(v47);
            v60._Bx._Ptr = 0i64;
            v60._Mysize = 0i64;
            v60._Myres = 15i64;
            v27 = -1i64;
            do
                ++v27;
            while (v26[v27]);
            string_assign(&v60, v26, v27);
            if (url->_Myres >= 0x10ui64)
                url = (std::string *)url->_Bx._Ptr;
            if (op->_Myres >= 0x10ui64)
                op = (std::string *)op->_Bx._Ptr;
            v28 = &v60;
            if (v60._Myres >= 0x10ui64)
                v28 = (std::string *)v60._Bx._Ptr;
            v29 = Logger::getLogger();
            NetworkingLog(
                NL_ERROR,
                &v29->m_errorPrefix,
                "Curl error: [%d] '%s' for: %s %s",
                v47,
                v28->_Bx._Buf,
                op->_Bx._Buf,
                url->_Bx._Buf);
            v57 = v60;
            v60._Mysize = 0i64;
            v60._Myres = 15i64;
            v60._Bx._Buf[0] = 0;
            sub_7FF6B48C36F0(ret, 30, (__int128 *)v57._Bx._Buf);
            if (v15)
                return ret;
        } else
        {
            v63 = 0i64;
            sub_7FF6B4D6EC40(m_curl, 0x20000Cu, &v63);
            HttpStatistics::addRequestSize(m_stat->field_0, (__int64)v63);
            HttpStatistics::addResponseSize(m_stat->field_0, *((_QWORD *)v15 + 3) - (_QWORD)*v16);
            LODWORD(v59) = 0;
            sub_7FF6B4D6EC40(m_curl, 0x200002u, &v59);
            HttpStatistics::enqueueEndpointEvent(m_stat->field_0, descr, this->m_requestId, (int)v59, 0, v51);
            v49 = (int)v59;
            v31 = 503;
            if ((unsigned __int16)v59 > 0x1F7u)
            {
                v31 = 509;
                if ((unsigned __int16)v59 != 509)
                {
LABEL_87:
                    v37 = (char *)*((_QWORD *)v15 + 3);
                    v38 = *v16;
                    v61._Bx._Ptr = 0i64;
                    v61._Mysize = 0i64;
                    Myres = 15i64;
                    v61._Myres = 15i64;
                    if (v38 == v37)
                    {
                        v40 = (unsigned int)v59;
                    } else
                    {
                        string_assign(&v61, v38, v37 - v38);
                        v40 = (unsigned int)v59;
                        Myres = v61._Myres;
                    }
                    if (url->_Myres >= 0x10ui64)
                        url = (std::string *)url->_Bx._Ptr;
                    if (op->_Myres >= 0x10ui64)
                        op = (std::string *)op->_Bx._Ptr;
                    v41 = &v61;
                    if (Myres >= 0x10)
                        v41 = (std::string *)v61._Bx._Ptr;
                    v42 = Logger::getLogger();
                    NetworkingLog(
                        NL_WARN,
                        &v42->m_warnPrefix,
                        "Unexpected HTTP response: [%d] '%s' for: %s %s",
                        v40,
                        v41->_Bx._Buf,
                        op->_Bx._Buf,
                        url->_Bx._Buf);
                    v58 = v61;
                    v61._Mysize = 0i64;
                    v61._Myres = 15i64;
                    v61._Bx._Buf[0] = 0;
                    sub_7FF6B48C36F0(ret, 32, (__int128 *)v58._Bx._Buf);
                    v61._Mysize = 0i64;
                    v61._Myres = 15i64;
                    v61._Bx._Buf[0] = 0;
                    if (!v15)
                        return v15;
                    return ret;
                }
            } else if ((unsigned __int16)v59 != 503)
            {
                switch ((__int16)v59)
                {
                case 200:
                case 201:
                case 202:
                case 203:
                case 204:
                case 205:
                case 206:
                    v31 = 0;
                    break;
                case 400:
                    v31 = 400;
                    break;
                case 401:
                    v31 = 401;
                    break;
                case 403:
                    v31 = 403;
                    break;
                case 404:
                    v31 = 404;
                    break;
                case 409:
                    v31 = 409;
                    break;
                case 410:
                    v31 = 410;
                    break;
                default:
                    goto LABEL_87;
                }
            }
            if (url->_Myres >= 0x10ui64)
                url = (std::string *)url->_Bx._Ptr;
            if (op->_Myres >= 0x10ui64)
                op = (std::string *)op->_Bx._Ptr;
            v32 = Logger::getLogger();
            LODWORD(v45) = v49;
            NetworkingLog(
                NL_DEBUG,
                &v32->m_debugPrefix,
                "Completed request: %s %s (status: %d, elapsed: %lims)",
                op->_Bx._Buf,
                url->_Bx._Buf,
                v45,
                v51);
            if (v31)
            {
                v33 = (char *)*((_QWORD *)v15 + 3);
                v34 = *v16;
                v54._Bx._Ptr = 0i64;
                v54._Mysize = 0i64;
                v54._Myres = 15i64;
                if (v34 != v33)
                    string_assign(&v54, v34, v33 - v34);
                v24 = (__int128 *)&v54;
                v25 = v31;
LABEL_70:
                sub_7FF6B48C36F0(ret, v25, v24);
                if (!v15)
                    return ret;
                return v15;
            }
            if (v15)
            {
                _InterlockedIncrement(v15 + 2);
                v16 = v64;
            }
            v35 = operator new(0x48ui64);
            v36 = v35;
            if (v35)
            {
                *(_OWORD *)v35 = 0i64;
                v35[2] = 1;
                v35[3] = 1;
                *(_QWORD *)v35 = &std::_Ref_count_obj2<zwift_network::NetworkResponse<std::vector<char>> const>::`vftable';
                    v35[4] = 0;
                *((_QWORD *)v35 + 3) = 0i64;
                *((_QWORD *)v35 + 5) = 0i64;
                *((_QWORD *)v35 + 6) = 15i64;
                *((_QWORD *)v35 + 7) = 0i64;
                *((_QWORD *)v35 + 8) = 0i64;
                if (v15)
                {
                    _InterlockedIncrement(v15 + 2);
                    v16 = v64;
                }
                *((_QWORD *)v35 + 7) = v16;
                *((_QWORD *)v35 + 8) = v15;
            } else
            {
                v36 = 0i64;
            }
            *(_OWORD *)ret = 0i64;
            *(_QWORD *)ret = v36 + 4;
            *(_QWORD *)(ret + 8) = v36;
        }
#endif
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
    //global downloader will init/deinit CURL CURLcode globalInitialize() { return curl_global_init(CURL_GLOBAL_DEFAULT); } globalCleanup(void)
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
        /*TODOv2 = this[10];
        for (i = this[11]; i != v2; v2 = (std::thread *)((char *)v2 + 8))
        {
            if (*(_QWORD *)v2)
                std::thread::join(v2);
        }*/
    }
    void setUploadTimeout(uint64_t to) { std::lock_guard l(m_mutex); m_ncoUploadTimeoutSec = to; }
    void setTimeout(uint64_t to) { std::lock_guard l(m_mutex); m_ncoTimeoutSec = to; }
    void setThreadPoolSize(uint64_t val) { 
        //TODO
        //not only m_nThreads = val; 
    }
};
struct GenericHttpConnectionManager : public HttpConnectionManager { //0x128 bytes
    using task_type = std::packaged_task<NetworkResponseBase(CurlHttpConnection &)>;
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
            task(*conn);
        }
    }
    template<class RET>
    std::future<NetworkResponseBase> pushRequestTask(const std::function<NetworkResponse<RET>(CurlHttpConnection &)> &f) {
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
struct Base64Url : public std::string {
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
};
namespace HttpHelper {
    static NetworkResponse<Json::Value> parseJson(const std::string &src) {
        NetworkResponse<Json::Value> ret;
        Json::Reader r;
        if (!r.parse(src, ret, false)) {
            NetworkingLogError("Error parsing JSON: %s\nJSON: %s", r.getFormattedErrorMessages().c_str(), src.c_str());
            ret.storeError(23, "Error parsing json");
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
            m_exp = 10000000ull * pr["exp"].asInt();
            m_sub = pr["sub"].asString();
            m_sessionState = pr["session_state"].asString();
            return true;
        }
        return false;
    }
    bool parse(const std::string &jwt) {
        auto firstSep = jwt.find('.');
        if (firstSep == -1) {
            storeError(24, "First separator not found");
            return false;
        }
        auto secondSep = jwt.find('.', firstSep + 1);
        if (secondSep == -1) {
            storeError(24, "Second separator not found");
            return false;
        }
        Base64Url payload;
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
            auto at = pr["access_token"].asString();
            if (m_acToken.parseOk(at, this)) {
                auto rt = pr["refresh_token"].asString();
                if (m_rfToken.parseOk(rt, this)) {
                    m_exp = pr["expires_in"].asInt();
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
    std::string m_apiUrl, m_accessToken, m_mail, m_password, m_oauthClient, m_field_F8;
    uint64_t m_accessTokenDeathTime = 0, m_reqId = 0;
    Oauth2Credentials m_oauth2;
    bool m_field_80 = true, m_loggedIn = false; //all other data also 0
    ZwiftAuthenticationManager(const std::string &server) { m_apiUrl = server + "/api/auth"; }
    ~ZwiftAuthenticationManager() { //vptr[0]
        //TODO: destroy() inlined
        //looks like field_40 is shared ptr too - destructed; also pure base class (AuthenticationManager) dtr called
    }
    bool isAccessTokenInvalidOrExpired() const { return getRefreshTokenStr().empty() || int64_t(g_steadyClock.now() - m_accessTokenDeathTime) > 0; } //vptr[1]
    const std::string &getAccessTokenHeader() const { return m_accessToken; } //vptr[2]
    const std::string &getRefreshTokenStr() const { return m_oauth2.getRefreshToken().asString(); } //vptr[3]
    const JsonWebToken &getRefreshToken() const { return m_oauth2.getRefreshToken(); } //vptr[4]
    const std::string &getSessionStateFromToken() const { return m_oauth2.getAccessToken().getSessionState(); } //vptr[5]
    const std::string &getSubjectFromToken() const { return m_oauth2.getAccessToken().getSubject(); } //vptr[6]
    const std::string &getOauthClient() const { return m_oauthClient; } //vptr[7]
    void setLoggedIn(bool val) { m_loggedIn = val; } //vptr[8]
    bool getLoggedIn() const { return m_loggedIn; } //vptr[9]
    void attendToAccessToken(CurlHttpConnection *a2) { //vptr[10]
        //TODO
    }
    void setRequestId(uint64_t id) { m_reqId = id; } //vptr[11]
    uint64_t setRequestId() { return m_reqId; } //vptr[12]
    void setAccessTokenAsExpired() { m_accessTokenDeathTime = 0; } //vptr[13]
    bool resetCredentials() { //vptr[14]
        m_accessToken.clear();
        m_mail.clear();
        m_accessTokenDeathTime = 0;
        m_field_80 = true;
        //TODO *(_DWORD *)&this->field_70 = 0; *(_QWORD *)&this->field_78 = 0i64;
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
        m_field_80 = 1;
        //TODO *(_DWORD *)&this->field_70 = 0; *(_QWORD *)&this->field_78 = 0i64;
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
        m_field_80 = false;
        /* TODO (_DWORD *)&this->field_70 = 0;
        *(_QWORD *)&this->field_78 = 0i64;*/
    }
};
struct ZwiftHttpConnectionManager : public HttpConnectionManager { //0x160 bytes
    struct RequestTaskContext {
        //TODO
        bool m_secured = false;
    };
    using task_type = std::packaged_task<NetworkResponseBase(CurlHttpConnection &, bool)>;
    std::queue<task_type> m_ptq;
    std::queue<RequestTaskContext> m_rtq;
    GlobalState *m_gs;
    ZwiftAuthenticationManager *m_authMgr = nullptr;
    uint64_t m_field_158 = 1;
    bool m_needNewAcToken = false, m_some0;
    void setGlobalState(GlobalState *gs) { m_gs = gs; }
    ZwiftHttpConnectionManager(CurlHttpConnectionFactory *curlf, const std::string &certs, bool ncoSkipCertCheck, ZwiftAuthenticationManager *am, bool some0, int ncoTimeoutSec, int ncoUploadTimeoutSec, HttpRequestMode rm, int nThreads) :
        HttpConnectionManager(curlf, certs, ncoSkipCertCheck, ncoTimeoutSec, ncoUploadTimeoutSec, rm, nThreads), m_authMgr(am), m_some0(some0) {
        startWorkers();
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
#if 0 //TODO
                v17 = sub_7FF7BCCB8010(&m_rtq.front()->m_succ, &v49);
                sub_7FF7BCC02900(v6, *v17);
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
#if 0 //TODO
            //sub_7FF7BCC209B0(ptr, conn, needNewAcToken);
            task(*conn, needNewAcToken);
#endif
        }
    }
    ~ZwiftHttpConnectionManager() { shutdown(); }
};
struct EncryptionInfo {};
struct EncryptionOptions {
    bool m_disableEncr, m_disableEncryptionWithServer, m_ignoreEncryptionFeatureFlag;
    const std::string &m_secretKeyBase64;
};
struct ExperimentsRestInvoker;
struct AuthServerRestInvoker { //0x60 bytes
    NetworkResponseBase logIn(const EncryptionOptions &encr, const std::vector<std::string> &a4, const std::function<void(const protobuf::PerSessionInfo &, const std::string &, const EncryptionInfo &)> &a5) {
        //TODO
        return NetworkResponseBase{ "todo", 0 };
    }
    AuthServerRestInvoker(const std::string &machineId, ZwiftAuthenticationManager *authMgr, ZwiftHttpConnectionManager *httpConnMgr3, ExperimentsRestInvoker *expRi, const std::string &server) {
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
    void post(std::function<void(void)> &&f) { boost::asio::post(m_asioCtx, f); }
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
        //OMIT TelemetryService::setGlobalState(this->m_ts, (__int64)&v88);
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
        //OMIT TelemetryService::remoteLogF(*(_DWORD **)v49, 0, 3u, (__int64)"session", "Session ID %s", v58->_Bx._Buf);
    }
    NetworkResponseBase logInWithOauth2Credentials(/*ret a2,*/ const std::string &sOauth, const std::vector<std::string> &a4, const std::string &oauthClient) {
        if (m_authMgr) {
            auto ret = m_authMgr->setCredentialsMid(sOauth, oauthClient);
            return ret.m_errCode ? ret :
                m_authInvoker->logIn({ m_nco.m_disableEncr, m_nco.m_disableEncryptionWithServer, m_nco.m_ignoreEncryptionFeatureFlag, m_nco.m_secretKeyBase64 },
                    a4, 
                    [&](const protobuf::PerSessionInfo &psi, const std::string &str, const EncryptionInfo &ei) { onLoggedIn(psi, str, ei); });
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
    g_NetworkOn = zwift_network::initialize_zwift_network(server, GAMEPATH("data/cacert.pem"), "1.0.106405");
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

//Units
TEST(SmokeTest, JsonWebToken) {
    Oauth2Credentials o;
    //zoffline first
    std::string at("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjkiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiQmVhcmVyIiwiYXpwIjoiR2FtZV9MYXVuY2hlciIsImF1dGhfdGltZSI6MTUzNTUwNzI0OSwic2Vzc2lvbl9zdGF0ZSI6IjA4NDZubzluLTc2NXEtNHAzcy1uMjBwLTZwbnA5cjg2cjVzMyIsImFjciI6IjAiLCJhbGxvd2VkLW9yaWdpbnMiOlsiaHR0cHM6Ly9sYXVuY2hlci56d2lmdC5jb20qIiwiaHR0cDovL3p3aWZ0Il0sInJlYWxtX2FjY2VzcyI6eyJyb2xlcyI6WyJldmVyeWJvZHkiLCJ0cmlhbC1zdWJzY3JpYmVyIiwiZXZlcnlvbmUiLCJiZXRhLXRlc3RlciJdfSwicmVzb3VyY2VfYWNjZXNzIjp7Im15LXp3aWZ0Ijp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiR2FtZV9MYXVuY2hlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFJFU1QgQVBJIC0tIHByb2R1Y3Rpb24iOnsicm9sZXMiOlsiYXV0aG9yaXplZC1wbGF5ZXIiLCJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFplbmRlc2siOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSZWxheSBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIl19LCJlY29tLXNlcnZlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sImFjY291bnQiOnsicm9sZXMiOlsibWFuYWdlLWFjY291bnQiLCJtYW5hZ2UtYWNjb3VudC1saW5rcyIsInZpZXctcHJvZmlsZSJdfX0sIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20iLCJzZXNzaW9uX2Nvb2tpZSI6IjZ8YTJjNWM1MWY5ZDA4YzY4NWUyMDRlNzkyOWU0ZmMyMDAyOWI5ODE1OGYwYjdmNzk0MmZiMmYyMzkwYWMzNjExMDMzN2E3YTQyYjVlNTcwNmVhODM0YjQzYzFlNDU1NzJkMTQ2MzIwMTQxOWU5NzZjNTkzZWZjZjE0M2UwNWNiZjgifQ._kPfXO8MdM7j0meG4MVzprSa-3pdQqKyzYMHm4d494w"),
    rt("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjgiLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiUmVmcmVzaCIsImF6cCI6IkdhbWVfTGF1bmNoZXIiLCJhdXRoX3RpbWUiOjAsInNlc3Npb25fc3RhdGUiOiIwODQ2bm85bi03NjVxLTRwM3MtbjIwcC02cG5wOXI4NnI1czMiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZXZlcnlib2R5IiwidHJpYWwtc3Vic2NyaWJlciIsImV2ZXJ5b25lIiwiYmV0YS10ZXN0ZXIiXX0sInJlc291cmNlX2FjY2VzcyI6eyJteS16d2lmdCI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIkdhbWVfTGF1bmNoZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIiwiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBaZW5kZXNrIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiWndpZnQgUmVsYXkgUkVTVCBBUEkgLS0gcHJvZHVjdGlvbiI6eyJyb2xlcyI6WyJhdXRob3JpemVkLXBsYXllciJdfSwiZWNvbS1zZXJ2ZXIiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJhY2NvdW50Ijp7InJvbGVzIjpbIm1hbmFnZS1hY2NvdW50IiwibWFuYWdlLWFjY291bnQtbGlua3MiLCJ2aWV3LXByb2ZpbGUiXX19LCJzZXNzaW9uX2Nvb2tpZSI6IjZ8YTJjNWM1MWY5ZDA4YzY4NWUyMDRlNzkyOWU0ZmMyMDAyOWI5ODE1OGYwYjdmNzk0MmZiMmYyMzkwYWMzNjExMDMzN2E3YTQyYjVlNTcwNmVhODM0YjQzYzFlNDU1NzJkMTQ2MzIwMTQxOWU5NzZjNTkzZWZjZjE0M2UwNWNiZjgifQ.5e1X1imPlVfXfhDHE_OGmG9CNGvz7hpPYPXcNkPJ5lw"),
        json("{\"access_token\":\"" + at + "\",\"expires_in\":1000021600,\"id_token\":\"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJqdGkiOiJiYjQ4czgyOS03NDgzLTQzbzEtbzg1NC01ZDc5M3E1bjAwbjciLCJleHAiOjIxNDc0ODM2NDcsIm5iZiI6MCwiaWF0IjoxNTM1NTA4MDg3LCJpc3MiOiJodHRwczovL3NlY3VyZS56d2lmdC5jb20vYXV0aC9yZWFsbXMvendpZnQiLCJhdWQiOiJHYW1lX0xhdW5jaGVyIiwic3ViIjoiMDJyM2RlYjUtbnE5cS00NzZzLTlzczAtMDM0cTk3N3NwMnIxIiwidHlwIjoiSUQiLCJhenAiOiJHYW1lX0xhdW5jaGVyIiwiYXV0aF90aW1lIjoxNTM1NTA3MjQ5LCJzZXNzaW9uX3N0YXRlIjoiMDg0Nm5vOW4tNzY1cS00cDNzLW4yMHAtNnBucDlyODZyNXMzIiwiYWNyIjoiMCIsIm5hbWUiOiJad2lmdCBPZmZsaW5lIiwicHJlZmVycmVkX3VzZXJuYW1lIjoiem9mZmxpbmVAdHV0YW5vdGEuY29tIiwiZ2l2ZW5fbmFtZSI6Ilp3aWZ0IiwiZmFtaWx5X25hbWUiOiJPZmZsaW5lIiwiZW1haWwiOiJ6b2ZmbGluZUB0dXRhbm90YS5jb20ifQ.rWGSvv5TFO-i6LKczHNUUcB87Hfd5ow9IMG9O5EGR4Y\",\"not-before-policy\":1408478984,\"refresh_expires_in\":611975560,\"refresh_token\":\"" + rt + "\",\"scope\":\"\",\"session_state\":\"0846ab9a-765d-4c3f-a20c-6cac9e86e5f3\",\"token_type\":\"bearer\"}");
    EXPECT_TRUE(o.parse(json));
    EXPECT_EQ(nullptr, o.m_errMsg);
    EXPECT_EQ(0, o.m_errCode);
    EXPECT_EQ(json, o.m_json);
    EXPECT_EQ(0x000000003b9b1e60, o.m_exp);

    auto &a = o.getAccessToken();
    EXPECT_EQ(nullptr, a.m_errMsg);
    EXPECT_EQ(0, a.m_errCode);
    EXPECT_STREQ("02r3deb5-nq9q-476s-9ss0-034q977sp2r1", a.getSubject().c_str());
    EXPECT_STREQ("0846no9n-765q-4p3s-n20p-6pnp9r86r5s3", a.getSessionState().c_str());
    EXPECT_EQ(at, a.asString());
    EXPECT_EQ(0x004c4b3fff676980, a.m_exp);

    auto &r = o.getRefreshToken();
    EXPECT_EQ(nullptr, r.m_errMsg);
    EXPECT_EQ(0, r.m_errCode);
    EXPECT_STREQ("02r3deb5-nq9q-476s-9ss0-034q977sp2r1", r.getSubject().c_str());
    EXPECT_STREQ("0846no9n-765q-4p3s-n20p-6pnp9r86r5s3", r.getSessionState().c_str());
    EXPECT_EQ(rt, r.asString());
    EXPECT_EQ(0x004c4b3fff676980, r.m_exp);
    
    //retail (zca)
    std::string retail_at("eyJhbGciOiJSUzI1NiIsInR5cCIgOiAiSldUIiwia2lkIiA6ICJPLUVjXzJJNjg5bW9peGJIZzFfNDZDVFlGeEdZMDViaDluYm5Mcjl0RzY4In0.eyJleHAiOjE2MzY5MTAwODksImlhdCI6MTYzNjg4ODQ4OSwianRpIjoiMmI4YzI0OWEtMGU3MS00YWUyLThlY2ItNzgyYjFiYjZkNjlmIiwiaXNzIjoiaHR0cHM6Ly9zZWN1cmUuendpZnQuY29tL2F1dGgvcmVhbG1zL3p3aWZ0IiwiYXVkIjpbImVtYWlsLXByZWZzLXNlcnZpY2UiLCJteS16d2lmdCIsInNzby1nYXRld2F5Iiwic3Vic2NyaXB0aW9uLXNlcnZpY2UiLCJHYW1lX0xhdW5jaGVyIiwiWndpZnQgWmVuZGVzayIsIlp3aWZ0IFJFU1QgQVBJIC0tIHByb2R1Y3Rpb24iLCJad2lmdCBSZWxheSBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIiwiZWNvbS1zZXJ2ZXIiLCJhY2NvdW50Il0sInN1YiI6IjFhNzM2ZWNjLTFjYTYtNGFmZi1hMTc2LWU1NzgzMTk3YTE1NyIsInR5cCI6IkJlYXJlciIsImF6cCI6Ilp3aWZ0X01vYmlsZV9MaW5rIiwic2Vzc2lvbl9zdGF0ZSI6ImVjNzJmYWIyLWQ2NDItNDU5Ny04YmZmLTUwOTM5MjRjNTEyMCIsImFjciI6IjEiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZXZlcnlib2R5IiwidHJpYWwtc3Vic2NyaWJlciIsImV2ZXJ5b25lIiwiYmV0YS10ZXN0ZXIiXX0sInJlc291cmNlX2FjY2VzcyI6eyJlbWFpbC1wcmVmcy1zZXJ2aWNlIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwibXktendpZnQiOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJzc28tZ2F0ZXdheSI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sInN1YnNjcmlwdGlvbi1zZXJ2aWNlIjp7InJvbGVzIjpbImF1dGhlbnRpY2F0ZWQtdXNlciJdfSwiR2FtZV9MYXVuY2hlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sIlp3aWZ0IFplbmRlc2siOnsicm9sZXMiOlsiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIiwiYXV0aGVudGljYXRlZC11c2VyIl19LCJad2lmdCBSZWxheSBSRVNUIEFQSSAtLSBwcm9kdWN0aW9uIjp7InJvbGVzIjpbImF1dGhvcml6ZWQtcGxheWVyIl19LCJlY29tLXNlcnZlciI6eyJyb2xlcyI6WyJhdXRoZW50aWNhdGVkLXVzZXIiXX0sImFjY291bnQiOnsicm9sZXMiOlsibWFuYWdlLWFjY291bnQiLCJtYW5hZ2UtYWNjb3VudC1saW5rcyIsInZpZXctcHJvZmlsZSJdfX0sInNjb3BlIjoiIiwibmFtZSI6IllvdXJ5IFBlcnNoaW4iLCJwcmVmZXJyZWRfdXNlcm5hbWUiOiJzdWxpbW92YTA4QG1haWwucnUiLCJnaXZlbl9uYW1lIjoiWW91cnkiLCJmYW1pbHlfbmFtZSI6IlBlcnNoaW4iLCJlbWFpbCI6InN1bGltb3ZhMDhAbWFpbC5ydSJ9.VfuMKYGZzRCBMk7JOCsEIhVsUTHEBfIY7za8no_YtgNXbjxmnwcxMXRRUz_rCzKQDYvo4aTqThhuVMz9DpAMv4csrmWuST8KS4NlkwMBj-IqrGIr5ZI5mkKfFRDXrD44e5wk-3-6Z2F2oWxd3JoyzyuvIcu6CYEYYl4xtWj3TlN_GhlYyWPLJrcCBOHVtEX5diYyqWbHrpfeQ9dat3N3of0v_PXG4cjAMYV6DR-K9nIpKWGWE3siUIkt7pTY-cyJldRYCWzHjo6bwrZgwN5gB6wO-q3A0_gXgpr2oOriuToP-CAqtM60AdwGkckE6h4r-nFVcHV9j0Mo-I0-mgZbxw");
    JsonWebToken rat;
    EXPECT_TRUE(rat.parse(retail_at));
    EXPECT_EQ(nullptr, rat.m_errMsg);
    EXPECT_EQ(0, rat.m_errCode);
    EXPECT_STREQ("1a736ecc-1ca6-4aff-a176-e5783197a157", rat.getSubject().c_str());
    EXPECT_STREQ("ec72fab2-d642-4597-8bff-5093924c5120", rat.getSessionState().c_str());
    EXPECT_EQ(retail_at, rat.asString());
    EXPECT_EQ(0x003a279c4bd74a80, rat.m_exp);

    std::string retail_rt("eyJhbGciOiJIUzI1NiIsInR5cCIgOiAiSldUIiwia2lkIiA6ICIxYTY0ZDZkNC1iODVhLTQxZjQtOTFiMy01ZmJhNGQ4Y2FhMzMifQ.eyJleHAiOjE2MzgwOTgwODksImlhdCI6MTYzNjg4ODQ4OSwianRpIjoiZjJhMzhiNDgtMjlhMC00ZDZkLTkxYWQtMTg3ZTliMmQ4ZmViIiwiaXNzIjoiaHR0cHM6Ly9zZWN1cmUuendpZnQuY29tL2F1dGgvcmVhbG1zL3p3aWZ0IiwiYXVkIjoiaHR0cHM6Ly9zZWN1cmUuendpZnQuY29tL2F1dGgvcmVhbG1zL3p3aWZ0Iiwic3ViIjoiMWE3MzZlY2MtMWNhNi00YWZmLWExNzYtZTU3ODMxOTdhMTU3IiwidHlwIjoiUmVmcmVzaCIsImF6cCI6Ilp3aWZ0X01vYmlsZV9MaW5rIiwic2Vzc2lvbl9zdGF0ZSI6ImVjNzJmYWIyLWQ2NDItNDU5Ny04YmZmLTUwOTM5MjRjNTEyMCIsInNjb3BlIjoiIn0.vDBIPqDaOaJKcXK0MPJthsK_0nyHA3iKikE9oroPS3A");
    JsonWebToken rrt;
    EXPECT_TRUE(rrt.parse(retail_rt));
    EXPECT_EQ(nullptr, rrt.m_errMsg);
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