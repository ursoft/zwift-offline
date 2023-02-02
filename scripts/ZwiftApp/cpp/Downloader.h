#pragma once
class Downloader {
    std::string m_lp, m_up;
public:
    static inline const int64_t m_noFileTime = -1;
    static inline const char *m_curlErrorStr = nullptr;
    struct PendingFile {
        std::string m_name;
        PendingFile(const PendingFile &) {}
        PendingFile() {}
        ~PendingFile() {}
    };
    struct CompletedFile {
        CompletedFile() {}
    };
    struct CurrentFile {
        uint64_t m_length = 0;
        CurrentFile() {}
        ~CurrentFile() {}
    };
    std::deque<PendingFile> m_filesPending;
    std::deque<CompletedFile> m_filesCompleted;
    std::vector<CurrentFile> m_filesCurrent;
    CURLcode m_curlLastCode = CURLE_OK;
    CURL *m_curlEasy = nullptr;
    CURLM *m_curlMulti = nullptr;
    Downloader();
    void SetLocalPath(const char *lp) { m_lp = lp; }
    void SetServerURLPath(const char *up) { m_up = up; }
    void Download(const std::string &, uint64_t, int64_t, uint32_t, void (*)(const std::string &, int));

    void CancelDownload(const char *);
    void Completed(const std::string &);
    void CompletedSuccessfully(const std::string &);
    static size_t CurlWriteData(char *ptr, size_t size, size_t nmemb, void *userdata);
    void Download(const char *, std::function<void(const char *)>);
    void Download(const std::string &, uint64_t, int64_t, uint32_t, void (*)(std::string, int));
    void Download(std::vector<std::string>, std::vector<uint64_t>, std::vector<int64_t>, std::vector<uint32_t>);
    void EnsurePath(std::string);
    void FindCompleted(const std::string &);
    void ForgetAllCompleted();
    void ForgetCompleted(const std::string &);
    void ForgetCompletedWorkout(const std::string &);
    void FullyCompleted();
    void FullyCompletedSuccessfully();
    void GetCompletedFileByName(const std::string &);
    void GetCurrentByFile(FILE *, uint64_t *);
    void GetCurrentByHandle(void *, uint64_t *);
    void GetCurrentFileByName(const std::string &);
    void GetFileCurLengthCompleted();
    void GetFileCurLengthTotal();
    void GetPendingFileByName(const std::string &);
    void GetUserDownloadsPath();
    bool InternetConnected();
    bool IsInProgress(const std::string &, std::function<void(const char *)>, void (*)(std::string, int));
    std::string PrintState();
    void SetDebugMessage(char *);
    void SetLastModDate(FILE *, int64_t, const std::string &);
    void Update();
};
extern Downloader g_mDownloader, *g_pDownloader;