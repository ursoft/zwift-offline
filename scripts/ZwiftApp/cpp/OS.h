#pragma once
const char *const OS_GetUserPath();
void OS_Initialize();
bool OS_IsOnBattery();
void OS_Shutdown();
const char *OS_GetLanguage();
inline bool OS_GetPicturesPath(LPSTR pszPath) { return SHGetFolderPathA(nullptr, CSIDL_MYPICTURES, nullptr, 0, pszPath) >= 0; }