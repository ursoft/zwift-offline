#include "ZwiftApp.h"
std::deque<Downloader::CompletedFile>::iterator Downloader::FindCompleted(const std::string &path) {
    std::deque<CompletedFile>::iterator it = m_filesCompleted.begin();
    for (; it != m_filesCompleted.end(); it++)
        if (it->m_name == path) 
            break;
    return it;
}
bool Downloader::CompletedSuccessfully(const std::string &path) {
    auto c = FindCompleted(path);
    if (c == m_filesCompleted.end())
        return false;
    else
        return c->m_succ;
}
bool Downloader::Download/*IDA: DownloadStr*/(const std::string &path, std::function<void(const char *)> fsucc, void (*cbFail)(const std::string &, int)) {
    for (auto &it : m_filesPending)
        if (it.m_name == path) {
            if (fsucc) it.m_succCallbacks.push_back(fsucc);
            if (cbFail) it.m_failCallbacks.push_back(cbFail);
            return true;
        }
    for (auto &it : m_filesCurrent) {
        if (it.m_name == path) {
            if (fsucc) it.m_succCallbacks.push_back(fsucc);
            if (cbFail) it.m_failCallbacks.push_back(cbFail);
            return true;
        }
    }
    auto ci = FindCompleted(path);
    if (ci != m_filesCompleted.end()) {
        if (cbFail)
            cbFail(path, 0);
        if (fsucc)
            fsucc((m_locp + path).c_str());
        return true;
    }
    return false;
}
void Downloader::Download/*IDA: DownloadCStr*/(const char *name, std::function<void(const char *)> fsucc) {
    std::string sname(name);
    if (!Download(sname, fsucc, nullptr)) {
        PendingFile newPf;
        newPf.m_name = sname;
        newPf.m_locp = m_locp;
        newPf.m_urlp = m_urlp;
        newPf.m_succCallbacks.push_back(fsucc);
        m_filesPending.push_back(newPf);
        m_pendingTotalLength++;
    }
}
Downloader::Downloader() {
    char path[MAX_PATH];
    m_curlLastCode = curl_global_init(CURL_GLOBAL_DEFAULT);
    m_curlEasy = curl_easy_init();
    m_curlMulti = curl_multi_init();
    if (SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, path) == S_OK) {
        sprintf_s(path, "%s\\Zwift", path);
        CreateDirectoryA(path, nullptr);
        sprintf_s(path, "%s\\Logs", path);
        CreateDirectoryA(path, nullptr);
        //sprintf_s(path, "%s\\Libcurl_log.txt", path);
        g_pDownloader = &g_mDownloader;
    }
    m_constructed = true;
}
void Downloader::Download/*IDA: DownloadFptr*/(const std::string &name, uint64_t expectedLength, int64_t fileTime, uint32_t checksumWant, void (*cbFail)(const std::string &, int)) {
    auto f = FindCompleted(name);
    if (f != m_filesCompleted.end() && f->m_succ == false) {
        m_filesCompleted.erase(f);
    }
    std::function<void(const char *)> empty;
    if (!Download(name, empty, cbFail)) {
        PendingFile newPf;
        newPf.m_name = name;
        newPf.m_locp = m_locp;
        newPf.m_urlp = m_urlp;
        newPf.m_expectedLength = expectedLength;
        newPf.m_failCallbacks.push_back(cbFail);
        memcpy(&newPf.m_fileTime, &fileTime, sizeof(FILETIME));
        newPf.m_checksumWant = checksumWant;
        m_filesPending.push_back(newPf);
        m_pendingTotalLength += (expectedLength ? expectedLength : 1);
    }
}
void Downloader::ForgetAllCompleted() {
    Log("Downloader::ForgetAllCompleted m_filesCompleted.size=%d", (int)m_filesCompleted.size());
    m_filesCompleted.clear();
    m_completedTotalLength = 0;
}
void Downloader::ForgetCompleted(const std::string &name) {
    Log("Downloader::ForgetCompleted path=%s", name.c_str());
    auto f = FindCompleted(name);
    if (f != m_filesCompleted.end() && --f->m_refs == 0) {
        m_filesCompleted.erase(f);
    }
}
std::string Downloader::PrintState() {
    return "TODO";
}
char debugDestination[0x800];
bool strstr_s(const char *haystack, size_t haystack_len, const char *needle, size_t needle_len) {
    for (auto h = haystack; haystack_len >= needle_len; ++h, --haystack_len) {
        if (!memcmp(h, needle, needle_len)) {
            return true;
        }
    }
    return false;
}
size_t Downloader::CurlWriteData(char *ptr, size_t size, size_t nmemb, void *userdata) {
    if (!ptr || !userdata || !g_pDownloader) {
        strcpy_s(debugDestination, "Downloader::CurlWriteData(): buffer/file/downloader is null\n");
        return 0;
    }
    char Source[1024];
    sprintf_s(Source, "Downloader::CurlWriteData(): 1st of buffer=%d, size=%d, count=%d, file=%p\n", *ptr, (int)size, (int)nmemb, userdata);
    strcpy_s(debugDestination, Source);
    auto total_size = size * nmemb;
    size_t i = 0;
    if (total_size >= 6 && !memcmp(ptr, "<html>", 6) && strstr_s(ptr, std::min(52ULL, total_size), "404 Not Found", 13))
        return 0;
    FILE *f = (FILE *)userdata;
    auto written = fwrite(ptr, size, nmemb, f);
    if (written != total_size) {
        sprintf_s(Source, "Downloader::CurlWriteData(): ERROR: \"fwrite\" returned incorrect written bytes! passed=%d, written=%d\n",
            (int)total_size,
            (int)written);
        strcpy_s(debugDestination, Source);
        return 0;
    }
    for (auto &i : g_pDownloader->m_filesCurrent) {
        if (i.m_FILE == f) {
            i.m_tickLastRead = GetTickCount64();
            i.m_actualLength += written;
            break;
        }
    }
    return written;
}
size_t Downloader::GetUserDownloadsPath(char *downDir) {
    PWSTR pszPath;
    if (SHGetKnownFolderPath(FOLDERID_Downloads, KF_FLAG_CREATE, nullptr, &pszPath) == S_OK) {
        int wr = WideCharToMultiByte(CP_THREAD_ACP, 0, pszPath, -1, downDir, MAX_PATH - 2, 0, nullptr);
        CoTaskMemFree(pszPath);
        if (wr > 0) {
            downDir[wr - 1] = '\\'; downDir[wr] = 0;
            return wr;
        } else {
            downDir[0] = 0;
            return 0;
        }
    } else {
        GetModuleFileNameA(nullptr, downDir, sizeof(downDir));
        int last = strlen(downDir) - 1;
        while (last >= 0 && downDir[last] != '\\') last--;
        downDir[last + 1] = 0; //slash included or null-term
        return last + 1;
    }
}
size_t SubstringPos(const char *path, size_t path_len, size_t offset, const char *down, size_t downDirLen) {
    if (downDirLen > path_len || offset > path_len - downDirLen)
        return -1;
    if (!downDirLen)
        return offset;
    auto chd = *down;
    auto fnd = (char *)memchr(&path[offset], chd, path_len - downDirLen + 1 - offset);
    if (!fnd)
        return -1;
    auto v9 = &path[path_len - downDirLen];
    while (memcmp(fnd, down, downDirLen)) {
        fnd = (char *)memchr(fnd + 1, chd, v9 - fnd);
        if (!fnd)
            return -1;
    }
    return fnd - path;
}
bool Downloader::EnsurePath(std::string path) {
    std::error_code ec;
    std::replace(path.begin(), path.end(), '/', '\\');
    std::filesystem::path fpath(path);
    if (!std::filesystem::create_directories(fpath.parent_path(), ec)) {
        auto msg = ec.message();
        Log("Downloader: Failed to create_directories for %s. Error msg: %s.\n", path.c_str(), msg.c_str());
        m_error = true;
        m_lastErrorCode = 117;
        return false;
    }
    /* ну и бред!!!
    char cd[MAX_PATH], downDir[MAX_PATH];
    if (!GetCurrentDirectoryA(sizeof(cd) - 1, cd)) {
        Log("Downloader: Failed to get current directory. Error code: %d.\n", GetLastError());
        m_field_1 = true;
        m_field_4 = 117;
        return false;
    }
    auto downDirLen = GetUserDownloadsPath(downDir);
    if (0 == SubstringPos(path.c_str(), path.length(), 0, downDir, downDirLen)) {
        if (!SetCurrentDirectoryA(downDir)) {
            Log("Downloader: Failed to set current directory to \"%s\". Error code: %d.\n", downDir, GetLastError());
            m_field_1 = true;
            m_field_4 = 117;
            return false;
        }
    }
    while (true) {
        //auto sl_pos = path.find
        //for every part of path: cd && mkdir
    }
    if (!SetCurrentDirectoryA(cd)) {
        Log("Downloader: Failed to set current directory to \"%s\". Error code: %d.\n", cd, GetLastError());
        m_field_1 = true;
        m_field_4 = 117;
        return false;
    }*/
    return true;
}
void Downloader::Update() {
    if (!m_constructed) {
        m_error = 1;
        m_lastErrorCode = 101;
        return;
    }
    do {
        m_curlmLastCode = curl_multi_perform(m_curlMulti, &m_curlStillRunning);
    } while (m_curlmLastCode == -1); //deprecated
    int msgs_in_queue;
    int cur_idx = 0;
    CurrentFile *cur = nullptr;
    std::vector<int> idx_del_cur;
    do {
        m_error = 0;
        m_lastErrorCode = 100;
        CURLMsg *msg = curl_multi_info_read(m_curlMulti, &msgs_in_queue);
        if (msg && msg->msg == CURLMSG_DONE) {
            for (; cur_idx < m_filesCurrent.size(); cur_idx++) {
                if (m_filesCurrent[cur_idx].m_curl == msg->easy_handle) {
                    cur = &m_filesCurrent[cur_idx];
                    idx_del_cur.push_back(cur_idx);
                    break;
                }
            }
            if (cur) {
                int length = 1;
                if (cur->m_expectedLength)
                    length = cur->m_expectedLength;
                m_completedTotalLength += length;
                m_pendingTotalLength -= length;
                if (msg->data.result) {
                    if (cur->m_FILE) {
                        fclose(cur->m_FILE);
                        cur->m_FILE = nullptr;
                    }
                    cur->m_failed = cur->m_timeout = m_error = true;
                    m_lastErrorCode = msg->data.result;
                    Log("\nDownloader: ERROR: \"%s\" failed with 'CURLcode'=%d!\n\n", cur->m_name.c_str(), m_lastErrorCode);
                    for (auto cbFail : cur->m_failCallbacks)
                        cbFail(cur->m_name, m_lastErrorCode);
                } else {
                    if (cur->m_fileTime.dwHighDateTime != (DWORD)-1 && cur->m_fileTime.dwLowDateTime != (DWORD)-1) {
                        if (!SetFileTime((HANDLE)_get_osfhandle(_fileno(cur->m_FILE)), nullptr, nullptr, &cur->m_fileTime)) {
                            m_error = 1;
                            m_lastErrorCode = 109;
                        }
                    }
                    fclose(cur->m_FILE);
                    cur->m_FILE = 0i64;
                    auto fullname = m_locp + cur->m_name;
                    if (cur->m_checksumWant == -1) {
                        cur->m_checksumGot = -1;
                        Log("Downloader: \"%s\" downloaded successfully (manifest checksum not provided, i.e. -1).\n", fullname.c_str());
                    } else {
                        cur->m_checksumGot = -1;
                        uint32_t tmpcrc;
                        if (CCRC32::FileCRC(fullname.c_str(), &tmpcrc, cur->m_expectedLength))
                            cur->m_checksumGot = tmpcrc;
                        Log("Downloader: \"%s\" downloaded successfully (local checksum=%d, manifest checksum=%d).\n", fullname.c_str(), cur->m_checksumGot, cur->m_checksumWant);
                    }
                    if (!g_mDownloader.m_error) 
                        for (auto cbSuccess : cur->m_succCallbacks)
                            cbSuccess(fullname.c_str());
                }
            }
        }
    } while (msgs_in_queue);
    auto now = GetTickCount64();
    for (cur_idx = 0; cur_idx < m_filesCurrent.size(); cur_idx++) {
        cur = &m_filesCurrent[cur_idx];
        if (cur->m_FILE && now - cur->m_tickLastRead > 30000) {
            cur->m_timeout = true;
            idx_del_cur.push_back(cur_idx);
        }
    }
    for (auto ri = idx_del_cur.rbegin(); ri != idx_del_cur.rend(); ri++) {
        cur = &m_filesCurrent[*ri];
        m_curlmLastCode = curl_multi_remove_handle(m_curlMulti, cur->m_curl);
        bool hasProblems = true, will_repeat = false, len_mismatch = false;
        if (cur->m_timeout) {
            Log("\nDownloader: ERROR: \"%s\" timed out!\n\n", cur->m_name.c_str());
        } else {
            if (!cur->m_expectedLength || cur->m_actualLength == cur->m_expectedLength) {
                hasProblems = (g_mDownloader.m_curlmLastCode != 0);
            } else {
                len_mismatch = true;
                Log("\nDownloader: ERROR: \"%s\" completed download successfully, but failed the file length test (manifest=%d, written=%d)!\n\n",
                    cur->m_name.c_str(), cur->m_expectedLength, cur->m_actualLength);
            }
            if (cur->m_checksumGot != cur->m_checksumWant) {
                Log("\nDownloader: ERROR: \"%s\" completed download successfully, but failed the file checksum test (manifest=%d, local=%d)!\n\n",
                    cur->m_name.c_str(), cur->m_checksumWant, cur->m_checksumGot);
                DeleteFileA((m_locp + cur->m_name).c_str());
                hasProblems = true;
            }
        }
        if (hasProblems) {
            ++cur->m_prbCounter;
            Log("\nDownloader: %s: \"%s\" failed to download with code: %d!\n\n", (cur->m_prbCounter > 10) ? "ERROR" : "WARNING", cur->m_name.c_str(), m_curlmLastCode);
            if (cur->m_prbCounter > 10) {
                m_error = true;
                if (cur->m_timeout) {
                    m_lastErrorCode = 103;
                } else if (len_mismatch) {
                    g_mDownloader.m_lastErrorCode = 112;
                } else {
                    m_lastErrorCode = (cur->m_checksumGot != cur->m_checksumWant) ? 113 : 106;
                }
                return;
            }
            Log("Downloader: Attempting to try again downloading \"%s\" \n", cur->m_name.c_str());
            will_repeat = true;
            Sleep(0x3E8u); // TODO: FIXME
            m_filesPending.emplace_back(*cur);
        }
        if (cur->m_FILE) {
            fclose(cur->m_FILE);
            cur->m_FILE = nullptr;
        }
        if (!will_repeat) {
            std::string fullPath(m_locp + cur->m_name);
            std::replace(fullPath.begin(), fullPath.end(), '/', '\\');
            bool succ = (cur->m_failed == 0);
            m_filesCompleted.emplace_back(fullPath, succ, 1);
            Log("Downloader::Update m_filesCompleted.push_back path=%s success=%d", fullPath.c_str(), succ);
            for (auto cbFail : cur->m_failCallbacks)
                cbFail(cur->m_name, m_error ? m_lastErrorCode : 0);
            for (auto sc : cur->m_succCallbacks)
                sc(fullPath.c_str());
        }
        m_filesCurrent.erase(m_filesCurrent.begin() + *ri); //call ~CurrentFile
    }
    if (m_limitCurrents < 50 && m_filesCurrent.size() == m_limitCurrents) {
        if (m_highLoadStartTime) {
            auto xnow = _Xtime_get_ticks();
            auto dt = xnow / 10000 - m_highLoadStartTime;
            if (dt > 50) {
                auto now_clt = GetFileCurLengthTotal();
                uint64_t delta = now_clt - m_highLoadStartLength;
                float fdelta;
                if ((int64_t)delta < 0)
                    fdelta = (float)(int)(delta & 1 | (delta >> 1)) + (float)(int)(delta & 1 | (delta >> 1));
                else
                    fdelta = (float)(int)delta;
                float speed = delta / (float)dt;
                if (speed > m_maxSpeed) {
                    m_maxSpeed = speed;
                    ++m_limitCurrents;
                }
                m_highLoadStartTime = xnow;
                m_highLoadStartLength = now_clt;
            }
        } else {
            m_highLoadStartTime = _Xtime_get_ticks() / 10000;
            m_highLoadStartLength = GetFileCurLengthTotal();
        }
    }
    while (m_filesCurrent.size() < m_limitCurrents && !m_filesPending.empty()) {
        m_error = 0;
        m_lastErrorCode = 100;
        auto pend_src = m_filesPending.front();
        std::string fileName(pend_src.m_locp + pend_src.m_name);
        std::replace(fileName.begin(), fileName.end(), '/', '\\');
        if (!EnsurePath(fileName)) {
            m_lastErrorCode = 117;
            m_error = 1;
            return;
        }
        FILE *hFile = nullptr;
        fopen_s(&hFile, fileName.c_str(), "wb");
        if (!hFile) {
            Log("Downloader: Failed to create file handle for \"%s\". Error code: %d. \n", fileName.c_str(), GetLastError());
            m_lastErrorCode = 117;
            m_error = 1;
            return;
        }
        Log("Downloader: Created file handle for \"%s\".\n", fileName.c_str());
        auto curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_CAPATH, "./");
        curl_easy_setopt(curl, CURLOPT_CAINFO, "data/cacert.pem");
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
        std::string url(pend_src.m_urlp + pend_src.m_name);
        std::replace(url.begin(), url.end(), '\\', '/');
        m_curlLastCode = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        bool bad_news = m_curlLastCode != 0;
        m_curlLastCode = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Downloader::CurlWriteData);
        bad_news = bad_news || (m_curlLastCode != 0);
        m_curlLastCode = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, Downloader::m_curlErrorStr);
        if (m_curlLastCode)
            bad_news = true;
        m_curlLastCode = curl_easy_setopt(curl, CURLOPT_WRITEDATA, hFile);
        if ((m_curlLastCode != 0) || bad_news) {
            m_lastErrorCode = 104;
            m_error = 1;
            fclose(hFile);
            curl_easy_cleanup(curl);
            return;
        }
        m_curlmLastCode = curl_multi_add_handle(m_curlMulti, curl);
        if (m_curlLastCode) {
            m_lastErrorCode = 105;
            m_error = 1;
            fclose(hFile);
            curl_easy_cleanup(curl);
            return;
        }
        auto new_cur = m_filesCurrent.emplace_back(pend_src);
        new_cur.m_FILE = hFile;
        new_cur.m_curl = curl;
        m_filesPending.pop_front();
    }
}
uint64_t Downloader::GetFileCurLengthTotal() {
    uint64_t ret = m_completedTotalLength;
    for (auto &i : m_filesCurrent)
        ret += i.m_actualLength;
    return ret;
}
Downloader::CurrentFile::CurrentFile(const PendingFile &pend_src) :
    m_name(pend_src.m_name), m_expectedLength(pend_src.m_expectedLength), m_fileTime(pend_src.m_fileTime), m_succCallbacks(pend_src.m_succCallbacks), m_failCallbacks(pend_src.m_failCallbacks),
    m_locp(pend_src.m_locp), m_urlp(pend_src.m_urlp), m_prbCounter(pend_src.m_prbCounter) {}
Downloader::PendingFile::PendingFile(const CurrentFile &cur_src) :
    m_name(cur_src.m_name), m_expectedLength(cur_src.m_expectedLength), m_fileTime(cur_src.m_fileTime), m_succCallbacks(cur_src.m_succCallbacks), m_failCallbacks(cur_src.m_failCallbacks),
    m_locp(cur_src.m_locp), m_urlp(cur_src.m_urlp), m_prbCounter(0) {}
Downloader::CurrentFile::~CurrentFile() {
    if (m_curl)
        curl_easy_cleanup(m_curl);
    m_curl = nullptr;
}