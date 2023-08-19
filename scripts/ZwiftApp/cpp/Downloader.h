#pragma once //READY for testing
struct Downloader {
    std::string m_locp, m_urlp;
    struct CurrentFile;
    struct PendingFile {
        PendingFile() {}
        PendingFile(const CurrentFile &);
        std::string m_name, m_locp, m_urlp;
        FILETIME m_fileTime{ (uint32_t)-1, (uint32_t)-1};
        uint64_t m_expectedLength = 0;
        uint32_t m_prbCounter = 0, m_checksumWant = (uint32_t)-1;
        std::vector<std::function<void(const char *)>> m_succCallbacks;
        std::vector<void (*)(const std::string &, int)> m_failCallbacks;
    };
    struct CompletedFile {
        CompletedFile(const std::string &name, bool succ, int refs) : m_name(name), m_succ(succ), m_refs(refs) {}
        std::string m_name;
        bool m_succ = false;
        int m_refs = 0;
    };
    struct CurrentFile {
        CurrentFile() {}
        CurrentFile(const PendingFile &pend_src);
        std::string m_name, m_urlp, m_locp;
        CURL *m_curl = nullptr;
        uint64_t m_actualLength = 0, m_expectedLength = 0;
        FILE *m_FILE = nullptr;
        size_t m_tickLastRead = GetTickCount64();
        FILETIME m_fileTime{ (DWORD)-1, (DWORD)-1 };
        uint32_t m_checksumWant = (uint32_t)-1, m_checksumGot = (uint32_t)-1, m_prbCounter = 0;
        bool m_failed = false, m_timeout = false;
        std::vector<std::function<void(const char *)>> m_succCallbacks;
        std::vector<void (*)(const std::string &, int)> m_failCallbacks;
        ~CurrentFile();
    };
    static inline const int64_t m_noFileTime = -1;
    static inline char m_curlErrorStr[CURL_ERROR_SIZE];
    CURL *m_curlEasy = nullptr;
    CURLM *m_curlMulti = nullptr;
    int64_t m_pendingTotalLength = 0, m_completedTotalLength = 0, m_highLoadStartTime = 0, m_highLoadStartLength = 0;
    std::string m_field_50, m_field_70, m_field_90;
    std::deque<PendingFile> m_filesPending;
    std::deque<CompletedFile> m_filesCompleted;
    std::vector<CurrentFile> m_filesCurrent;
    int m_curlStillRunning = 0, m_curlmLastCode = 0;
    int m_lastErrorCode = 0, m_limitCurrents = 5;
    CURLcode m_curlLastCode = CURLE_OK;
    float m_maxSpeed = 0.0f;
    bool m_constructed = false, m_error = false;
    Downloader();
    ~Downloader();
    void SetLocalPath(const char *lp) { m_locp = lp; }
    void SetServerURLPath(const char *up) { m_urlp = up; }
    void DownloadCStr(const char *name, std::function<void(const char *)>);
    void DownloadFptr(const std::string &name, uint64_t, int64_t, uint32_t, void (*)(const std::string &, int));
    bool DownloadStr(const std::string &name, std::function<void(const char *)> f, void (*cb)(const std::string &, int));
    //not found void DownloadVec(std::vector<std::string>, std::vector<uint64_t>, std::vector<int64_t>, std::vector<uint32_t>);
    bool EnsurePath(std::string path);
    static size_t CurlWriteData(char *ptr, size_t size, size_t nmemb, void *userdata);
    size_t GetUserDownloadsPath(char *downDir);
    std::deque<CompletedFile>::iterator FindCompleted(const std::string &path);
    void ForgetCompleted(const std::string &);
    void ForgetAllCompleted();
    std::string PrintState();
    void Update();
    int Completed(const std::string &path);
    bool CompletedSuccessfully(const std::string &);
    //inlined void GetCompletedFileByName(const std::string &);
    void CancelDownload(const std::string &file);
    void ForgetCompletedWorkout(const std::string &);
    bool FullyCompleted();
    //not found bool FullyCompletedSuccessfully();
    //not found void GetCurrentByFile(FILE *, uint64_t *);
    //not found void GetCurrentByHandle(void *, uint64_t *);
    //not found void GetCurrentFileByName(const std::string &);
    //not found void GetFileCurLengthCompleted();
    uint64_t GetFileCurLengthTotal();
    //not found void GetPendingFileByName(const std::string &);
    //not found bool InternetConnected();
    //inlined bool IsInProgress(const std::string &, std::function<void(const char *)>, void (*)(std::string, int));
    //not found void SetDebugMessage(char *);
    //inlined void SetLastModDate(FILE *, int64_t, const std::string &);
    static Downloader *Instance() {
        static Downloader g_mDownloader;
        return &g_mDownloader;
    }
};