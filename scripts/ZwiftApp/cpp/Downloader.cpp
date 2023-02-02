#include "ZwiftApp.h"
Downloader g_mDownloader, *g_pDownloader;
Downloader::Downloader() {
    char path[MAX_PATH];
    /* TODO android
    *(_OWORD *)&this->field_30 = 0u;
    *(_OWORD *)&this->field_60 = 0u;
    v1 = (_DWORD)this + 96;
    *(_OWORD *)&this->field_90 = 0u;
    v2 = (_DWORD)this + 144;
    *(_OWORD *)&this->field_C0 = 0u;
    v3 = &this->field_C0;
    *(_OWORD *)&this->field_70 = 0u;
    *(_OWORD *)&this->field_80 = 0u;
    *(_OWORD *)&this->field_40 = 0u;
    *(_OWORD *)&this->field_50 = 0u;
    *(_OWORD *)&this->field_D0 = 0u;
    *(_OWORD *)&this->field_E0 = 0u;
    *(_OWORD *)&this->field_F0 = 0u;
    *(_OWORD *)&this->field_100 = 0u;
    *(_OWORD *)&this->field_110 = 0u;
    *(_OWORD *)&this->field_120 = 0u;
    v5 = (_DWORD)this + 72;
    v6 = (_DWORD)this + 120;
    v7 = &this->field_108;
    *(_QWORD *)&this->field_A0 = 0LL;
    *(_QWORD *)&this->field_130 = 0LL;
    *(_WORD *)&this->field_0 = 1;
    *(_QWORD *)&this->field_18 = 0LL;
    *(_QWORD *)&this->field_20 = 0LL;
    *(_QWORD *)&this->field_4 = 100LL;
    *(_DWORD *)&this->field_28 = 0;
    std::string::assign((_DWORD)this + 48, "REQUIRED");
    std::string::assign(v5, "REQUIRED");
    std::string::assign(v1, "REQUIRED");
    std::string::assign(v6, &byte_1F5F62E);
    std::string::assign(v2, "REQUIRED");
    *(_QWORD *)&this->field_138 = 0LL;
    *(_QWORD *)&this->field_140 = 0LL;*/
    m_curlLastCode = curl_global_init(CURL_GLOBAL_DEFAULT);
    m_curlEasy = curl_easy_init();
    m_curlMulti = curl_multi_init();
    /**(_QWORD *)&this->field_B4 = 0LL;
    *(_QWORD *)&this->field_AC = 0LL;
    *(_DWORD *)&this->field_A8 = 5;
    *(_DWORD *)&this->field_BC = 0;*/
    Downloader::m_curlErrorStr = 0;
    if (SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, path) >= 0) {
        sprintf_s(path, "%s\\Zwift", path);
        CreateDirectoryA(path, nullptr);
        sprintf_s(path, "%s\\Logs", path);
        CreateDirectoryA(path, nullptr);
        //sprintf_s(path, "%s\\Libcurl_log.txt", path);
        g_pDownloader = &g_mDownloader;
    }
}
//"MapSchedule_v2.xml", 0LL, Downloader::m_noFileTime, -1, GAME_onFinishedDownloadingMapSchedule
void Downloader::Download(const std::string &name, uint64_t, int64_t fileTime, uint32_t, void (*cb)(const std::string &, int)) {
    //TODO
}
void Downloader::ForgetCompleted(const std::string &name) {
    //TODO
}
std::string Downloader::PrintState() {
    return "TODO";
}
size_t Downloader::CurlWriteData(char *ptr, size_t size, size_t nmemb, void *userdata) {
    //TODO
    return 0;
}
