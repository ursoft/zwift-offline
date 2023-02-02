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
bool OS_GetUserPath(char *dest) {
    static bool pathOK;
    if (pathOK == false) {
        if (SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, pszPath) < 0 || !CheckDataPath(pszPath)) {
            if (SHGetFolderPathA(nullptr, CSIDL_DESKTOP, nullptr, 0, pszPath) < 0 || !CheckDataPath(pszPath)) {
                MessageBoxA(nullptr, "There was an error accessing your Documents folder. There was also an error accessing your Desktop as a fal"
                    "lback. This means Zwift can't save any files. Please contact Zwift Support.", "Error accessing writable storage", MB_OK);
                return false;
            }
        }
        pathOK = true;
    }
    return true;
}