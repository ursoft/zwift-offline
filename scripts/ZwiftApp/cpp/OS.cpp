#include "ZwiftApp.h"
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
    struct _SYSTEM_POWER_STATUS SystemPowerStatus;
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