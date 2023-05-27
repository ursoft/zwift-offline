#include "ZwiftApp.h"
#include "AK/SoundEngine/Common/AkSoundEngine.h"
#include "AK/SoundEngine/Common/AkStreamMgrModule.h"
#include "AkDefaultIOHookBlocking.h"
#include "AkFilePackageLowLevelIO.h"
bool g_soundInitialized;
AkInitSettings g_initSettings;
AkPlatformInitSettings g_platformInitSettings;
AkDeviceSettings g_deviceSettings;
AkStreamMgrSettings g_streamMgrSettings;
class CAkFilePackageLowLevelIOBlocking : public CAkFilePackageLowLevelIO<CAkDefaultIOHookBlocking> {
public:
    CAkFilePackageLowLevelIOBlocking() {}
    virtual ~CAkFilePackageLowLevelIOBlocking() {}
} *g_pLowLevelIO;

void AUDIO_SetVariable(const char *name, float val) {
    if (g_soundInitialized && AK::SoundEngine::SetRTPCValue(name, val) != AK_Success) {
        assert(false);
#if 0 //TODO
        if (dword_7FF6D151CD60 > *(_DWORD *)(*(_QWORD *)NtCurrentTeb()->ThreadLocalStoragePointer + 192i64)) {
            Init_thread_header(&dword_7FF6D151CD60);
            if (dword_7FF6D151CD60 == -1) {
                v14 = operator new(0x28ui64);
                *v14 = v14;
                v14[1] = v14;
                v14[2] = v14;
                *((_WORD *)v14 + 12) = 257;
                qword_7FF6D151E9D0 = (__int64)v14;
                atexit(sub_7FF6D0C81A60);
                Init_thread_footer(&dword_7FF6D151CD60);
            }
        }
        v3 = SIG_CalcCaseInsensitiveSignature(name);
        v4 = qword_7FF6D151E9D0;
        v5 = *(_QWORD *)(qword_7FF6D151E9D0 + 8);
        *(_QWORD *)&v18 = v5;
        DWORD2(v18) = 0;
        v6 = qword_7FF6D151E9D0;
        while (!*(_BYTE *)(v5 + 25))
        {
            *(_QWORD *)&v18 = v5;
            if (*(_DWORD *)(v5 + 28) >= v3)
            {
                DWORD2(v18) = 1;
                v6 = v5;
                v5 = *(_QWORD *)v5;
            } else
            {
                DWORD2(v18) = 0;
                v5 = *(_QWORD *)(v5 + 16);
            }
        }
        if (*(_BYTE *)(v6 + 25) || v3 < *(_DWORD *)(v6 + 28))
        {
            if (qword_7FF6D151E9D8 == 0x666666666666666i64)
                std::vector<void *>::_Xlen();
            v7 = operator new(0x28ui64);
            v7[7] = v3;
            *((_BYTE *)v7 + 32) = 0;
            *(_QWORD *)v7 = v4;
            *((_QWORD *)v7 + 1) = v4;
            *((_QWORD *)v7 + 2) = v4;
            *((_WORD *)v7 + 12) = 0;
            v15 = v18;
            v6 = sub_7FF6CFCBA910((__int64 **)&qword_7FF6D151E9D0, (__int64)&v15, (__int64)v7);
        }
        if (!*(_BYTE *)(v6 + 32))
        {
            v8 = SIG_CalcCaseInsensitiveSignature(name);
            v10 = qword_7FF6D151E9D0;
            v11 = *(_QWORD *)(qword_7FF6D151E9D0 + 8);
            *(_QWORD *)&v19 = v11;
            DWORD2(v19) = 0;
            v12 = qword_7FF6D151E9D0;
            while (!*(_BYTE *)(v11 + 25))
            {
                *(_QWORD *)&v19 = v11;
                if (*(_DWORD *)(v11 + 28) >= v8)
                {
                    DWORD2(v19) = 1;
                    v12 = v11;
                    v11 = *(_QWORD *)v11;
                } else
                {
                    DWORD2(v19) = 0;
                    v11 = *(_QWORD *)(v11 + 16);
                }
            }
            if (*(_BYTE *)(v12 + 25) || v8 < *(_DWORD *)(v12 + 28))
            {
                if (qword_7FF6D151E9D8 == 0x666666666666666i64)
                    std::vector<void *>::_Xlen();
                v16 = &qword_7FF6D151E9D0;
                v17 = 0i64;
                v13 = operator new(0x28ui64);
                v13[7] = v8;
                *((_BYTE *)v13 + 32) = 0;
                *(_QWORD *)v13 = v10;
                *((_QWORD *)v13 + 1) = v10;
                *((_QWORD *)v13 + 2) = v10;
                *((_WORD *)v13 + 12) = 0;
                v18 = v19;
                v12 = sub_7FF6CFCBA910((__int64 **)&qword_7FF6D151E9D0, (__int64)&v18, (__int64)v13);
            }
            *(_BYTE *)(v12 + 32) = 1;
            Log("AUDIO_SetVariable error:  for  %s at val %f", (int)name, COERCE_UNSIGNED_INT64(val), v9);
        }
#endif
    }
}
void AUDIO_LoadData() {
    static AkBankID initBankID, mainBankID, musicBankID, soundMediaBankID, globals_01_MediaBankID, globals_02_MediaBankID, globals_03_MediaBankID;
    AKRESULT r;
    if ((r = AK::SoundEngine::LoadBank("Init.bnk", initBankID)) != AK_Success)
        Log("AK::LoadBank(Main.bnk) failed: %d", r);
    if ((r = AK::SoundEngine::LoadBank("Main.bnk", mainBankID)) != AK_Success)
        Log("AK::LoadBank(Main.bnk) failed: %d", r);
    if ((r = AK::SoundEngine::LoadBank("Music.bnk", musicBankID)) != AK_Success)
        Log("AK::LoadBank(Music.bnk) failed: %d", r);
    if ((r = AK::SoundEngine::LoadBank("soundmedia.bnk", soundMediaBankID)) != AK_Success)
        Log("AK::LoadBank(soundmedia.bnk) failed: %d", r);
    if ((r = AK::SoundEngine::LoadBank("globals_01.bnk", globals_01_MediaBankID)) != AK_Success)
        Log("AK::LoadBank(globals_01.bnk) failed: %d", r);
    if ((r = AK::SoundEngine::LoadBank("globals_02.bnk", globals_02_MediaBankID)) != AK_Success)
        Log("AK::LoadBank(globals_02.bnk) failed: %d", r);
    if ((r = AK::SoundEngine::LoadBank("globals_03.bnk", globals_03_MediaBankID)) != AK_Success)
        Log("AK::LoadBank(globals_03.bnk) failed: %d", r);
    auto sfx = g_UserConfigDoc.GetF32("ZWIFT\\CONFIG\\SFX_VOL", 1.0f, true);
    AUDIO_SetVariable("sfx_volume", sfx * 100.0f);
    auto amb = g_UserConfigDoc.GetF32("ZWIFT\\CONFIG\\AMBIENT_VOL", 1.0f, true);
    AUDIO_SetVariable("ambient_volume", amb * 100.0f);
    Log("Loaded audio banks");
}
void AUDIO_Init() {
    g_NumSoundObjects = 0;
    AK::MemoryMgr::GetDefaultSettings(g_memSettings);
    AK::SoundEngine::GetDefaultInitSettings(g_initSettings);
    AK::SoundEngine::GetDefaultPlatformInitSettings(g_platformInitSettings);
    AK::SoundEngine::StopProfilerCapture();
    if (!AK::MemoryMgr::Init(&g_memSettings))
        return Log("FAILED TO INIT AUDIO: MemoryMgr");
    AK::StreamMgr::GetDefaultDeviceSettings(g_deviceSettings);
    g_deviceSettings.uIOMemorySize = 0x100000;
    if (!AK::StreamMgr::Create(g_streamMgrSettings))
        return Log("FAILED TO INIT AUDIO: StreamMgr");
    g_pLowLevelIO = new CAkFilePackageLowLevelIOBlocking();
    if (AK_Fail == g_pLowLevelIO->Init(g_deviceSettings))
        return Log("FAILED TO INIT AUDIO: lowlevel io");
    g_platformInitSettings.hWnd = glfwGetWin32Window(g_mainWindow);
    if (AK_Success != AK::SoundEngine::Init(&g_initSettings, &g_platformInitSettings))
        return Log("FAILED TO INIT AUDIO: SoundEngine");
    auto bpr = g_pLowLevelIO->AddBasePath(L"data/Audio/PC/");
    LogDebug("AK %s: setting basePath data/Audio/PC/", (bpr == AK_Success) ? "SUCCESS" : "ERROR");
    if (!AK::StreamMgr::SetCurrentLanguage(L"English(US)"))
        return Log("FAILED TO INIT AUDIO: SetCurrentLanguage");
    CONSOLE_AddCommand("play_wem", CMD_PlayWem, nullptr, nullptr, nullptr);
    CONSOLE_AddCommand("play_local_wem", CMD_PlayWemLocal, nullptr, nullptr, nullptr);
    Log("Successful audio init!");
    g_soundInitialized = true;
    AUDIO_LoadData();
}
void AUDIO_Shutdown() {
    if (g_soundInitialized) {
        g_pLowLevelIO->AddBasePath(L"data/Audio/PC/");
        AK::SoundEngine::StopAll();
        AK::SoundEngine::UnregisterAllGameObj();
        g_NumSoundObjects = 0;
        if (AK::SoundEngine::IsInitialized())
            AK::SoundEngine::Term();
        if (AK::IAkStreamMgr::Get()) {
            g_pLowLevelIO->Term();
            AK::IAkStreamMgr::Get()->Destroy();
        }
        char broadcastsPath[1024];
        sprintf_s(broadcastsPath, "%s/Zwift/AudioBroadcasts/", OS_GetUserPath());
        WIN32_FIND_DATAA FindFileData;
        auto hFindFile = FindFirstFileA(broadcastsPath, &FindFileData);
        auto bcPathLen = strlen(broadcastsPath);
        if (hFindFile != INVALID_HANDLE_VALUE) {
            do {
                if (FindFileData.cFileName[0] != '.' && (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                    strcpy_s(broadcastsPath + bcPathLen, _countof(broadcastsPath) - bcPathLen, FindFileData.cFileName);
                    DeleteFileA(broadcastsPath);
                }
            } while (FindNextFileA(hFindFile, &FindFileData));
            FindClose(hFindFile);
        }
        if (AK::MemoryMgr::IsInitialized())
            AK::MemoryMgr::Term();
        delete g_pLowLevelIO;
        g_pLowLevelIO = nullptr;
        g_soundInitialized = false;
    }
}
void GAME_AudioUpdate(GameWorld *, Camera *camera, float a3) {
    if (camera && g_pGameWorld) {
        //TODO
    }
}
struct audioQueueEntry { //0x88 bytes - 2x pointers (prev, next)
    uint32_t m_ebId;
    float m_time;
    int field_18;
    int field_1C;
    std::string m_eventName;
    std::string m_flatName;
    bool m_hasData;
    char field_61;
    char field_62;
    char field_63;
    int field_64;
    char m_data[32];
};
std::list<audioQueueEntry> g_audioQueue;
void AUDIO_Queue(float tm, const char *eventName, int a3 /*AUDIO_PlayFlatFile:1, AUDIO_Event:a2 */, uint32_t ebId /*counter, 0*/, void *data/*,0*/, const char *flatName /*,""*/) {
    if (g_soundInitialized) {
        auto &v13 = g_audioQueue.emplace_back(audioQueueEntry{ ebId, tm, a3, 0, eventName, flatName, data != nullptr });
        if (data)
            memmove(v13.m_data, data, sizeof(v13.m_data));
    }
}
void AUDIO_Event(const char *name, int a2, bool vital) {
    //TODO
}
wchar_t *FileName2AkName(const char *name) {
    const char *end = name + std::max(1023, (int)strlen(name));
    while (--end >= name)
        if (*end == '/' || *end == '\\')
            break;
    UChar buf[1024];
    if (end > name) {
        u_uastrncpy(buf, name, int32_t(end - name));
        buf[end - name] = 0;
        g_pLowLevelIO->AddBasePath((AkOSChar *)buf);
    }
    static UChar sbuf[1024];
    return (wchar_t *)u_uastrncpy(sbuf, end + 1, _countof(sbuf) - 1);
}
int g_pffCounter;
struct AUDIO_LoadedExternalSourceBuffers {
    uint32_t m_id;
    uint32_t m_playId;
    uint64_t field_8;
    void *field_10;
    void *m_pCookie;
    void *m_cbFunc;
};
struct UI_AudioControl { //256 (0x100) bytes
    UI_AudioControl(uint32_t playId) {
        //TODO
    }
    virtual ~UI_AudioControl() {
        //TODO
    }
};
std::unique_ptr<UI_AudioControl> g_pAudioController;
void HandleEventAudioCB(uint32_t id, uint32_t playId, int state) {
    float vol = 50.0f;
    switch (state) {
    case 0:
        g_pAudioController.reset(new UI_AudioControl(playId));
        vol = fminf(g_UserConfigDoc.GetF32("ZWIFT\\CONFIG\\AMBIENT_VOL", 1.0f, true) * 100.0f, vol);
        break;
    case 3:
        g_pAudioController.reset();
        vol = g_UserConfigDoc.GetF32("ZWIFT\\CONFIG\\AMBIENT_VOL", 1.0f, true) * 100.0f;
        break;
    default:
        return;
    }
    AUDIO_SetVariable("ambient_volume", vol);
}
std::list<AUDIO_LoadedExternalSourceBuffers> g_audioEBs;
void AUDIO_ExternalCB(AkCallbackType in_eType, AkCallbackInfo *in_pCallbackInfo) {
    if (AK_EndOfEvent == in_eType)
        g_audioEBs.remove_if([in_pCallbackInfo](auto &i) { return i.m_pCookie == in_pCallbackInfo->pCookie; });
}
bool AUDIO_PlayFlatFile(const char *name, float tm) {
    if (!g_soundInitialized)
        return false;
    g_audio_queue_failsafe++;
    if (g_audio_queue_failsafe > 11 && g_audio_queue_failsafe <= 40)
        AK::SoundEngine::RenderAudio(true);
    AkExternalSourceInfo v48(FileName2AkName(name), AK::SoundEngine::GetIDFromString("External_Source"), AKCODECID_VORBIS);
    AUDIO_LoadedExternalSourceBuffers v51{};
    v51.m_id = g_pffCounter++;
    v51.m_pCookie = (void *)(intptr_t)v48.iExternalSrcCookie;
    v51.m_cbFunc = HandleEventAudioCB;
    if (tm >= 0.01f) {
        if (tm > 0.01f) {
            Log("AUDIO: queueing external source at %fs", tm);
            AUDIO_Queue(tm, "ExternalEventTemplate", 1, v51.m_id, &v48, name);
        }
    } else {
        Log("AUDIO: playing external source at %fs", tm);
        v51.m_playId = AK::SoundEngine::PostEvent(
            "ExternalEventTemplate" /*const char *in_pszEventName */,
            1, /*in_gameObjectID*/
            AK_EndOfEvent, /*in_uFlags Bitmask: AkCallbackType*/
            AUDIO_ExternalCB, /*AkCallbackFunc function*/
            (void *)(intptr_t)v51.m_id, /*void *in_pCookie Callback cookie that will be sent to the callback function along with additional information*/
            1, /*in_cExternals Optional count of external source structures*/
            &v48 /*AkExternalSourceInfo *in_pExternalSources Optional array of external source resolution information*/
            );
        HandleEventAudioCB(v51.m_id, v51.m_playId, 0);
        AK::SoundEngine::SeekOnEvent("ExternalEventTemplate", 1, int(tm * -1000.0f), false, v51.m_playId);
    }
    g_audioEBs.push_back(v51);
    return true;
}

//Unit tests
TEST(SmokeTest, DISABLED_AUDIO_PlayFF) {
    AUDIO_Init();
    AK::Monitor::SetLocalOutput(AK::Monitor::ErrorLevel_All);
    AkGameObjectID id = 1;
    auto er = AK::SoundEngine::RegisterGameObj(id);
    er = AK::SoundEngine::SetDefaultListeners(&id, 1);
    EXPECT_TRUE(1 == er);
    EXPECT_TRUE(CMD_PlayWemLocal("Audio\\PC\\963639793.wem"));
    for (int i = 1; i < 1000; i++) {
        Sleep(10);
        er = AK::SoundEngine::RenderAudio(true);
    }
    AK::SoundEngine::UnregisterAllGameObj();
    AUDIO_Shutdown();
}