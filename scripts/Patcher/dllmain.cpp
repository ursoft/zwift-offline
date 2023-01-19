// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "stdio.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>
#include "Shlobj.h"
#include "Shlobj_core.h"

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void Report(const char *funcName, const char *param = nullptr);

extern "C" {
    __declspec(dllexport) void Launcher_Log(const char *msg) {
        auto Time = _time64(0i64);
        struct tm Tm;
        _localtime64_s(&Tm, &Time);
        char Buffer[1024] = {};
        if (msg) sprintf_s(Buffer, "[%d:%02d:%02d %d-%02d-%02d] %s", Tm.tm_hour, Tm.tm_min, Tm.tm_sec, Tm.tm_year + 1900, Tm.tm_mon + 1, Tm.tm_mday, msg); else return;
        char PathName[256];
        if (S_OK == SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, PathName)) {
            sprintf_s(PathName, "%s\\Zwift", PathName);
            CreateDirectoryA(PathName, nullptr);
            sprintf_s(PathName, "%s\\Logs", PathName);
            char Destination[256];
            strcpy_s(Destination, 248, PathName);
            CreateDirectoryA(PathName, nullptr);
            sprintf_s(PathName, "%s\\Launcher_log.txt", PathName);
            static FILE *Stream = nullptr;
            if (nullptr == Stream) {
                char OldFilename[256];
                CHAR FileName[256];
                strcpy_s(OldFilename, 248, Destination);
                for (int i = 9; i > 0; --i)
                {
                    sprintf_s(OldFilename, "%s\\Launcher_log (old %d).txt", Destination, i);
                    sprintf_s(FileName, "%s\\Launcher_log (old %d).txt", Destination, i + 1);
                    rename(OldFilename, FileName);
                }
                sprintf_s(FileName, "%s\\Launcher_log (old %d).txt", Destination, 10);
                DeleteFileA(FileName);
                rename(PathName, OldFilename);
            }
            fopen_s(&Stream, PathName, "a");
            if (Stream) {
                //fseek(Stream, 0, SEEK_END);
                fwrite(Buffer, strlen(Buffer), 1, Stream);
                fflush(Stream);
                fclose(Stream);
            }
        }
        OutputDebugStringA(Buffer);
    }
    __declspec(dllexport) bool patcher_AvailabilityWasChecked() { Report(__FUNCTION__); return true; }
    __declspec(dllexport) bool patcher_DelFile(const char *name) { Report(__FUNCTION__, name); return DeleteFileA(name); }
    __declspec(dllexport) bool patcher_GetFailed() { Report(__FUNCTION__); return false; }
    __declspec(dllexport) int patcher_GetFailed_Code() { Report(__FUNCTION__); return 0; }
    __declspec(dllexport) const char *patcher_GetFailed_File() { Report(__FUNCTION__); return "Failed_File"; }
    __declspec(dllexport) int patcher_GetFailed_Line() { Report(__FUNCTION__); return 0; }
    __declspec(dllexport) const char *patcher_GetLauncherUpdateURL() { Report(__FUNCTION__); return "LauncherUpdateURL"; }
    __declspec(dllexport) const char *patcher_GetLocalDownloadPath() { Report(__FUNCTION__); return "LocalDownloadPath"; }
    __declspec(dllexport) const char *patcher_GetLocalUpdatePath() { Report(__FUNCTION__); return "LocalUpdatePath"; }
    __declspec(dllexport) int patcher_GetSoftwareUpdateBytesCompleted() { Report(__FUNCTION__); return 0; }
    __declspec(dllexport) int patcher_GetSoftwareUpdateBytesRemaining() { Report(__FUNCTION__); return 0; }
    __declspec(dllexport) int patcher_GetSoftwareUpdateBytesTotal() { Report(__FUNCTION__); return 0; }
    __declspec(dllexport) int patcher_GetSoftwareUpdateCurFileLengthCompleted() { Report(__FUNCTION__); return 0; }
    __declspec(dllexport) int patcher_GetSoftwareUpdateCurFileLengthTotal() { Report(__FUNCTION__); return 0; }
    __declspec(dllexport) const char *patcher_GetSoftwareUpdateCurFileName() { Report(__FUNCTION__); return "CurFileName"; }
    __declspec(dllexport) int patcher_GetSoftwareUpdateFilesCompleted() { Report(__FUNCTION__); return 0; }
    __declspec(dllexport) int patcher_GetSoftwareUpdateFilesRemaining() { Report(__FUNCTION__); return 0; }
    __declspec(dllexport) int patcher_GetSoftwareUpdateFilesTotal() { Report(__FUNCTION__); return 0; }
    __declspec(dllexport) void patcher_GetVerCur(const char *p) { Report(__FUNCTION__, p); }
    __declspec(dllexport) void patcher_GetVerNew(const char *p) { Report(__FUNCTION__, p); }
    __declspec(dllexport) void patcher_Init() { Report(__FUNCTION__); }
    __declspec(dllexport) bool patcher_InternetConnected() { Report(__FUNCTION__); return true; }
    __declspec(dllexport) bool patcher_IsLauncherUpdateAvailable() { Report(__FUNCTION__); return false; }
    __declspec(dllexport) bool patcher_LoadFile(const char *name, void *dest) { Report(__FUNCTION__, name); return true; }
    __declspec(dllexport) bool patcher_SaveFile(const char *name, const void *src, size_t length) { Report(__FUNCTION__, name); return true; }
    __declspec(dllexport) size_t patcher_SetSoftwareUpdateProcessLocalPath(const char *p) { Report(__FUNCTION__, p); return 0; }
    __declspec(dllexport) void patcher_SetSoftwareUpdateProcessServerDownloadURL(const char *p) { Report(__FUNCTION__, p); return ; }
    __declspec(dllexport) void patcher_SetSoftwareUpdateProcessServerDownloadVerCur(const char *p) { Report(__FUNCTION__, p); return ; }
    __declspec(dllexport) bool patcher_SoftwareUpdateProcessAvail() { Report(__FUNCTION__); return false; }
    __declspec(dllexport) bool patcher_SoftwareUpdateProcessCheck() { Report(__FUNCTION__); return true; }
    __declspec(dllexport) bool patcher_SoftwareUpdateProcessDownload() { Report(__FUNCTION__); return true; }
    __declspec(dllexport) bool patcher_SoftwareUpdateProcessIsMandatoryUpdate() { Report(__FUNCTION__); return false; }
    __declspec(dllexport) bool patcher_SoftwareUpdateProcessIsPatching() { Report(__FUNCTION__); return false; }
    __declspec(dllexport) bool patcher_SoftwareUpdateProcessPatch() { Report(__FUNCTION__); return true; }
    __declspec(dllexport) bool patcher_SoftwareUpdateProcessPrep() { Report(__FUNCTION__); return true; }
    __declspec(dllexport) void patcher_SoftwareUpdateProcessReset() { Report(__FUNCTION__); }
    __declspec(dllexport) bool patcher_VerifyLocalSoftwareFiles() { Report(__FUNCTION__); return true; }

    __declspec(dllexport) bool patcher_IsUpdateAvailable() { Report(__FUNCTION__); return false; }
}

void Report(const char *funcName, const char *param /*= nullptr*/) {
    char buf[1024] = {};
    sprintf_s(buf, "Patcher.dll: %s(%s)\n", funcName, param ? param : "");
    Launcher_Log(buf);
}
