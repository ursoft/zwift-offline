#pragma once //READY for testing
const char *const OS_GetUserPath();
void OS_Initialize();
bool OS_IsOnBattery();
void OS_Shutdown();
const char *OS_GetLanguage();
inline bool OS_GetPicturesPath(LPSTR pszPath) { return SHGetFolderPathA(nullptr, CSIDL_MYPICTURES, nullptr, 0, pszPath) >= 0; }
bool OS_FileExists(const char *);
const char *OS_FormatDate(time_t dt);
enum OS_DateFormat { DF_DMY, DF_YMD, DF_MDY };
OS_DateFormat OS_GetDateFormat();
bool OS_GetCurrentWorkingDirectory(char *dest);
int OS_GetUTCOffsetInMinutes();
bool OS_MachineHasBattery();
void OS_MakeDirectoryPath(const char *);
inline void OS_Sleep(int ms) { ::Sleep(ms); }
void OS_UI_NormalizePaths(std::wstring *path, bool);
enum OS_UI_EFileType { BB_ALL, BB_TGA, BB_GDE, BB_FBX_NU, BB_ANIM };
bool OS_UI_ShowFileBrowseBox(std::wstring *, OS_UI_EFileType, bool, bool);
bool OS_UI_ShowColorPicker(COLORREF *dest);
/* not used or inlined:
OS_CloseLibrary(void *&)
OS_OpenLibrary(void *&,char const*)
OS_CreateDirInternal(std::string const&)
OS_SetUserPath(std::string)
int OS_SetUTCOffsetInSeconds(int);
OS_GetLibrarySymbol(void *&,void *,char const*)
OS_uSleep(int mcs)
OS_GetAppPath(void)
OS_GetIsDeviceUsingMetricSystem(void)
OS_GetCountryCode(char *)
OS_GetFrameCount(void)
OS_SetFrameCount(int)
OS_GetGameVersion(void)
OS_GetLastError(void)
OS_GetLocalUserDataPath(char *)
OS_GetMaxMemory(void)
OS_GetScreenSize(void)
OS_GetSoftKeyboardHeightScale(void)
OS_GetStoreVersion(void)
OS_GetVersionString(void)
OS_SetCountryCode(std::string)
OS_SetDeviceName(std::string)
OS_SetGameVersion(int)
OS_SetLanguage(std::string)
OS_SetMaxMemory(int)
OS_SetPicturesPath(std::string)
OS_SetReachability(bool)
OS_SetScreenSize(float)
OS_SetSoftKeyboardHeightScale(float)
OS_SetStoreVersion(int)
OS_SetVersion(std::string)
*/
