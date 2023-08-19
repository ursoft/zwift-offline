#include "OS.h" //READY for testing
#include "Logging.h"
bool CheckDataPath(const char *path) {
    char Buffer[MAX_PATH];
    sprintf_s(Buffer, "%s\\Zwift\\prefs.xml", path);
    auto f = fopen(Buffer, "a");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}
char pszPath[MAX_PATH];
const char *const OS_GetUserPath() {
    if (pszPath[0] == 0) {
        if (SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, pszPath) < 0 || !CheckDataPath(pszPath)) {
            if (SHGetFolderPathA(nullptr, CSIDL_DESKTOP, nullptr, 0, pszPath) < 0 || !CheckDataPath(pszPath)) {
                MessageBoxA(nullptr, "There was an error accessing your Documents folder. There was also an error accessing your Desktop as a fal"
                    "lback. This means Zwift can't save any files. Please contact Zwift Support.", "Error accessing writable storage", MB_OK);
                return nullptr;
            }
        }
    }
    return pszPath;
}
bool OS_IsOnBattery() {
    SYSTEM_POWER_STATUS SystemPowerStatus;
    auto v0 = GetSystemPowerStatus(&SystemPowerStatus);
    if (!v0 || SystemPowerStatus.ACLineStatus == 1)
        return false;
    return v0;
}
UINT g_timeResolution;
void OS_Initialize() {
    UINT uPeriod = OS_IsOnBattery() ? 0 : 1;
    if (!timeBeginPeriod(uPeriod)) {
        g_timeResolution = uPeriod;
        Log("[OS]: Time resolution(%llu)", g_timeResolution * 1000);
    }
    Log("[OS]: Initialized");
}
void OS_Shutdown() {
    if (g_timeResolution) {
        timeEndPeriod(g_timeResolution);
        g_timeResolution = 0;
    }
    Log("[OS]: Shutdown");
}
const char *OS_GetLanguage() {
    static char ret[32]{};
    if (ret[0] == 0) {
        WCHAR ln[32];
        if (!GetUserDefaultLocaleName(ln, _countof(ln)))
            return "en";

        wctomb(ret, ln[0]);
        wctomb(ret + 1, ln[1]);
    }
    return ret;
}
bool OS_FileExists(const char *name) { //QUEST: why not wchar_t
    auto dwAttrib = GetFileAttributesA(name);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
bool g_dateFormatKnown;
OS_DateFormat g_dateFormat;
OS_DateFormat OS_GetDateFormat() {
    if (g_dateFormatKnown)
        return g_dateFormat;
    char geoData[3];
    if (GetGeoInfoA(244, GEO_ISO2, nullptr, 0, 0) > 0 && GetGeoInfoA(244, GEO_ISO2, geoData, sizeof(geoData), 0) > 0) {
        if (0 == strcmp(geoData, "CN") || 0 == strcmp(geoData, "KR") || 0 == strcmp(geoData, "JP") || 0 == strcmp(geoData, "TW")) g_dateFormat = DF_YMD;
        else if (0 == strcmp(geoData, "US") || 0 == strcmp(geoData, "CA")) g_dateFormat = DF_MDY;
    }
    g_dateFormatKnown = true;
    return g_dateFormat;
}
char g_FormatDate[128];
const char *OS_FormatDate(time_t dt) {
    auto v1 = _localtime64(&dt);
    if (!v1)
        return "0/0/0";
    g_FormatDate[0] = 0;
    auto p2 = v1->tm_mon + 1;
    int p3 = v1->tm_year + 1900, p1;
    switch (OS_GetDateFormat()) {
    case DF_DMY:
        p1 = v1->tm_mday;
        break;
    case DF_YMD:
        p1 = p3;
        p3 = v1->tm_mday;
        break;
    default: //DF_MDY
        p1 = p2;
        p2 = v1->tm_mday;
        break;
    }
    sprintf_s(g_FormatDate, "%d/%d/%d", p1, p2, p3);
    return g_FormatDate;
}
bool OS_GetCurrentWorkingDirectory(wchar_t *dest) {
    GetCurrentDirectoryW(MAX_PATH, dest);
    return true;
}
int OS_GetUTCOffsetInMinutes() {
    struct _TIME_ZONE_INFORMATION TimeZoneInformation;
    if (GetTimeZoneInformation(&TimeZoneInformation) == TIME_ZONE_ID_STANDARD)
        return -TimeZoneInformation.Bias;
    return -TimeZoneInformation.Bias - TimeZoneInformation.DaylightBias;
}
bool OS_MachineHasBattery() {
    SYSTEM_POWER_STATUS SystemPowerStatus;
    auto v0 = GetSystemPowerStatus(&SystemPowerStatus);
    if (!v0 || (SystemPowerStatus.BatteryFlag & 0x80u) != 0 || SystemPowerStatus.ACLineStatus == 0xFF)
        return false;
    return true;
}
void OS_MakeDirectoryPath(const char *path) {
    namespace fs = std::filesystem;
    auto ret = fs::create_directories(path);
    zassert(ret);
}
//#include <commdlg.h> //does not compile
typedef UINT_PTR(CALLBACK *LPCCHOOKPROC) (HWND, UINT, WPARAM, LPARAM);
typedef struct tagCHOOSECOLORA {
    DWORD        lStructSize;
    HWND         hwndOwner;
    HWND         hInstance;
    COLORREF     rgbResult;
    COLORREF *lpCustColors;
    DWORD        Flags;
    LPARAM       lCustData;
    LPCCHOOKPROC lpfnHook;
    LPCSTR       lpTemplateName;
} CHOOSECOLORA, *LPCHOOSECOLORA;
extern "C" BOOL WINAPI ChooseColorA(LPCHOOSECOLORA);
COLORREF g_customColors[16];
bool OS_UI_ShowColorPicker(COLORREF *dest) {
    CHOOSECOLORA v4{};
    v4.lStructSize = sizeof(CHOOSECOLORA);
    v4.Flags = 131 /*CC_RGBINIT | CC_FULLOPEN | CC_SOLIDCOLOR*/;
    v4.hwndOwner = GetDesktopWindow();
    v4.rgbResult = *dest & 0xFFFFFF;
    v4.lpCustColors = g_customColors;
    if (!ChooseColorA(&v4))
        return false;
    *dest = v4.rgbResult;
    return true;
}
void OS_UI_NormalizePaths(std::wstring *path, bool killData) {
    auto offset = killData ? 5 : 0;
    for (auto &i : *path)
        if (i == L'\\')
            i = L'/';
    if (path->find(L"ZwiftApp/", 0) != -1) {
        auto v22 = path->find(L"data/", 0);
        if (v22 != -1)
            path->erase(0, v22 + offset);
    }
}
typedef UINT_PTR(CALLBACK *LPOFNHOOKPROC) (HWND, UINT, WPARAM, LPARAM);
typedef struct tagOFNW {
    DWORD        lStructSize;
    HWND         hwndOwner;
    HINSTANCE    hInstance;
    LPCWSTR      lpstrFilter;
    LPWSTR       lpstrCustomFilter;
    DWORD        nMaxCustFilter;
    DWORD        nFilterIndex;
    LPWSTR       lpstrFile;
    DWORD        nMaxFile;
    LPWSTR       lpstrFileTitle;
    DWORD        nMaxFileTitle;
    LPCWSTR      lpstrInitialDir;
    LPCWSTR      lpstrTitle;
    DWORD        Flags;
    WORD         nFileOffset;
    WORD         nFileExtension;
    LPCWSTR      lpstrDefExt;
    LPARAM       lCustData;
    LPOFNHOOKPROC lpfnHook;
    LPCWSTR      lpTemplateName;
} OPENFILENAMEW, *LPOPENFILENAMEW;
extern "C" BOOL WINAPI GetSaveFileNameW(LPOPENFILENAMEW);
extern "C" BOOL WINAPI GetOpenFileNameW(LPOPENFILENAMEW);
bool OS_UI_ShowFileBrowseBox(std::wstring *ioPath, OS_UI_EFileType ty, bool save, bool multiSelect) {
    wchar_t curDir[MAX_PATH + 1], fileBuf[MAX_PATH + 1];
    OS_GetCurrentWorkingDirectory(curDir);
    std::wstring wcurDir(curDir), wcurDird = wcurDir + L"\\data", *v97 = ((ty == BB_TGA) ? &wcurDird : &wcurDir);
    if (ioPath->length()) {
        for (auto &i : *ioPath)
            if (i == L'/')
                i = L'\\';
        std::wstring fullPath = *v97 + L"\\" + *ioPath;
        auto v58 = _wfopen(fullPath.c_str(), L"rb");
        if (v58)
            fclose(v58);
        else
            v97 = (ty == BB_FBX_NU) ? &wcurDir : &wcurDird;
    }
    if (ioPath->length() >= MAX_PATH)
        ioPath->erase(MAX_PATH - 1);
    OPENFILENAMEW v110{};
    wcscpy_s(fileBuf, ioPath->c_str());
    wcscpy_s(curDir, v97->c_str());
    v110.lStructSize = sizeof(OPENFILENAMEW);
    v110.lpstrFile = fileBuf;
    v110.hwndOwner = GetDesktopWindow();
    v110.nMaxFile = MAX_PATH;
    v110.nFilterIndex = 1;
    v110.lpstrInitialDir = curDir;
    v110.lpstrFileTitle = nullptr;
    v110.Flags = multiSelect ? 0x81A08 : 0x81808; //OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | (multiSelect ? OFN_ALLOWMULTISELECT)
    switch (ty) {
    case BB_TGA:
        v110.lpstrFilter = L"TGA(*.tga)";
        break;
    case BB_GDE:
        v110.lpstrFilter = L"GDE(*.gde)";
        break;
    case BB_ANIM:
        v110.lpstrFilter = L"ANIM(*.anim)";
        break;
    default:
        v110.lpstrFilter = L"All(*.*)";
        break;
    }
    if (save) {
        if (!GetSaveFileNameW(&v110))
            return false;
    } else {
        if (!GetOpenFileNameW(&v110))
            return false;
    }
    *ioPath = v110.lpstrFile;
    if (save && ioPath->find(L'.') == -1) {
        switch (ty) {
        case BB_TGA:
            *ioPath += L".tga";
            break;
        case BB_GDE:
            *ioPath += L".gde";
            break;
        case BB_ANIM:
            *ioPath += L".anim";
            break;
        case BB_FBX_NU:
            *ioPath += L".fbx";
            break;
        }
    }
    OS_UI_NormalizePaths(ioPath, ty == BB_TGA);
    return true;
}