#include "ZwiftApp.h" //READY for testing
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
bool AUDIO_SetObjectPosition(int objId, const VEC3 &a2, VEC3 a3) {
    if (!g_soundInitialized || objId == -1)
        return false;
    static_assert(sizeof(AkSoundPosition) == sizeof(VEC3[3]));
    union { AkSoundPosition v46; float f[9]; } par;
    a3.Normalize();
    par.f[6] = -a2.m_data[0] * 0.01f;
    par.f[7] = a2.m_data[1] * 0.01f;
    par.f[8] = a2.m_data[2] * 0.01f;
    VEC3 tmp{ -a3.m_data[1] * a3.m_data[0], a3.m_data[0] * a3.m_data[0] + a3.m_data[2] * a3.m_data[2], -a3.m_data[1] * a3.m_data[2] };
    tmp.Normalize();
    par.f[0] = -a3.m_data[0];
    par.f[1] = a3.m_data[1];
    par.f[2] = a3.m_data[2];
    par.f[3] = -tmp.m_data[0];
    par.f[4] = tmp.m_data[1];
    par.f[5] = tmp.m_data[2];
    if (AK_Success == AK::SoundEngine::SetPosition(objId, par.v46))
        return true;
    static std::set<int> gmap_AUDIO_SetObjectPosition;
    if (!gmap_AUDIO_SetObjectPosition.contains(objId)) {
        gmap_AUDIO_SetObjectPosition.insert(objId);
        Log("AK::SoundEngine::SetPosition(objectID:%d): Failed...", objId);
    }
    return false;
}
void AUDIO_SetListenerPosition(int id, const VEC3 &p1, const VEC3 &p2) {
    if (g_soundInitialized) {
        AUDIO_HandleFailsafe();
        AUDIO_SetObjectPosition(id, p1, p2);
    }
}
struct audioQueueEntry { //0x88 bytes - 2x pointers (prev, next)
    uint32_t m_ebId;
    float m_time;
    int m_objId;
    int field_1C;
    std::string m_eventName;
    std::string m_flatName;
    bool m_hasInfo;
    char field_61;
    char field_62;
    char field_63;
    int field_64;
    AkExternalSourceInfo m_info;
};
std::list<audioQueueEntry> g_audioQueue;
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
struct AUDIO_LoadedExternalSourceBuffers {
    uint32_t m_id;
    uint32_t m_playId;
    uint64_t field_8;
    void *field_10;
    void *m_pCookie;
    void (*m_cbFunc)(uint32_t, uint32_t, int);
};
std::list<AUDIO_LoadedExternalSourceBuffers> g_audioEBs;
void AUDIO_ExternalCB(AkCallbackType in_eType, AkCallbackInfo *in_pCallbackInfo) {
    if (AK_EndOfEvent == in_eType)
        g_audioEBs.remove_if([in_pCallbackInfo](auto &i) { return i.m_pCookie == in_pCallbackInfo->pCookie; });
}
void AUDIO_Update(float dt) {
    if (g_soundInitialized) {
        g_audio_queue_failsafe = 0;
        if (!g_audioQueue.empty()) {
            auto &i = g_audioQueue.front();
            i.m_time -= dt;
            if (i.m_time <= 0.0f) {
                if (i.m_hasInfo) {
                    i.m_info.szFile = FileName2AkName(i.m_flatName.c_str());
                    static_assert(sizeof(AkExternalSourceInfo) == 32);
                    auto v30 = AK::SoundEngine::PostEvent(i.m_eventName.c_str(), i.m_objId, AK_EndOfEvent, AUDIO_ExternalCB, (void *)(intptr_t)i.m_ebId, 1, &i.m_info);
                    for (auto &j : g_audioEBs) {
                        if (i.m_ebId == j.m_id) {
                            j.m_playId = v30;
                            if (j.m_cbFunc)
                                j.m_cbFunc(j.m_id, v30, 0);
                            break;
                        }
                    }
                } else {
                    AUDIO_Event(i.m_eventName.c_str(), i.m_objId, false);
                }
                g_audioQueue.pop_front();
            }
        }
        AK::SoundEngine::RenderAudio(true);
    }
}
void AUDIO_UnregisterObject(int oid) {
    if (g_soundInitialized && oid >= 2) {
        AK::SoundEngine::StopAll(oid);
        AK::SoundEngine::UnregisterGameObj(oid);
        --g_NumSoundObjects;
    }
}
void AUDIO_UnloadLevel(int wid) {
    if (g_soundInitialized)  {
        AK::SoundEngine::RenderAudio(true);
        g_audioQueue.clear();
        AK::SoundEngine::StopAll();
        AK::SoundEngine::UnregisterAllGameObj();
        g_NumSoundObjects = 0;
        AK::SoundEngine::RenderAudio(true);
        g_TitleMusic = -1;
        char s[1024];
        sprintf_s(s, "World%d.bnk", wid);
        AK::SoundEngine::UnloadBank(s, nullptr);
        AK::SoundEngine::RenderAudio(true);
    }
}
void AUDIO_StopEvent(const char *name, int oid, uint32_t plId) { //Name:"ExternalEventTemplate"
    if (g_soundInitialized)
        AK::SoundEngine::ExecuteActionOnEvent(name, AK::SoundEngine::AkActionOnEventType_Stop, oid, 0, AkCurveInterpolation_Linear, plId);
}
void AUDIO_SetVolume(int oid, float vol) {
    AUDIO_SetVariableOnObject("sfx_volume", vol, oid);
}
bool AUDIO_SetVariableOnObject(const char *name, float val, uint32_t oid) {
    if (!g_soundInitialized)
        return false;
    return AK::SoundEngine::SetRTPCValue(AK::SoundEngine::GetIDFromString(name), val, oid) == AK_Success;
}
int g_nextObjectID = 2;
int AUDIO_RegisterNewObject(char *name) {
    if (g_soundInitialized) {
        auto v3 = g_nextObjectID++;
        AUDIO_HandleFailsafe();
        if (AK::SoundEngine::RegisterGameObj(v3) == AK_Success) {
            ++g_NumSoundObjects;
            return v3;
        }
        Log("AUDIO: Failed to register GameObj ID %d (%s)", v3, name ? name : "(NULL)");
    }
    return -1;
}
AkBankID g_curWorldBank;
void AUDIO_LoadLevel(int wid) {
    if (g_soundInitialized) {
        char s[1024];
        g_pLowLevelIO->AddBasePath(L"data/Audio/PC/");
        sprintf_s(s, "World%d.bnk", wid);
        auto bank = AK::SoundEngine::LoadBank(s, g_curWorldBank);
        AK::SoundEngine::RenderAudio(true);
        if (bank != AK_Success)
            Log("AK::LoadBank(%s) failed: %d", s, bank);
        if (AK::SoundEngine::RegisterGameObj(1) != AK_Success)
            Log("AK::SoundEngine::RegisterGameObj failed");
        AkGameObjectID oid = 1;
        AK::SoundEngine::SetDefaultListeners(&oid, 1u);
        sprintf_s(s, "world_%d_init", wid);
        AUDIO_Event(s, 1, false);
        AK::SoundEngine::RenderAudio(true);
    }
}
int AUDIO_GetNumObjects() {
    return g_NumSoundObjects;
}
void AUDIO_Event(uint32_t eid, int oid) {
    if (!g_soundInitialized || oid == -1) {
        static std::set<uint32_t> gmap_AUDIO_Event1;
        if (!gmap_AUDIO_Event1.contains(eid)) {
            gmap_AUDIO_Event1.insert(eid);
            Log("AUDIO_Event() early return! eventID = %d", eid);
        }
    } else {
        AUDIO_HandleFailsafe();
        if (AK_Success != AK::SoundEngine::PostEvent(eid, oid)) {
            static std::set<uint32_t> gmap_AUDIO_Event2;
            if (!gmap_AUDIO_Event2.contains(eid)) {
                gmap_AUDIO_Event2.insert(eid);
                Log("AUDIO_Event(eventID:%d) failed!", eid);
            }
        }
    }
}
void AUDIO_SetVariable(const char *name, float val) {
    if (g_soundInitialized && AK::SoundEngine::SetRTPCValue(name, val) != AK_Success) {
        static std::set<uint32_t> gmap_AUDIO_SetVariable;
        auto sig = SIG_CalcCaseInsensitiveSignature(name);
        if (!gmap_AUDIO_SetVariable.contains(sig)) {
            gmap_AUDIO_SetVariable.insert(sig);
            Log("AUDIO_SetVariable error:  for  %s at val %f", name, val);
        }
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
float g_AudioTime, g_audioRand = 352637.94f, g_audioNextTime = 25.0f;
void GAME_AudioUpdate(GameWorld *, Camera *camera, float dtime) {
    if (camera && g_pGameWorld) {
        auto wd = g_pGameWorld->GetWorldDef();
        auto v7 = camera->m_pos.m_data[1] - (wd ? wd->m_ws.m_seaLevel : 0.0f);
        float v14, v15;
        AUDIO_SetVariable("player_altitude", v7);
        auto v9 = IsUnderWater(camera->m_pos);
        AUDIO_SetVariable("underwater", v9);
        AUDIO_SetVariable("rain_intensity", fmaxf((Weather::GetRainEffect() - 0.3333f) * 150.0f, 0.0f));
        g_AudioTime += dtime;
        switch (wd->m_WorldID) {
        case WID_WATOPIA:
            v14 = (VEC3{ -41478.875f, 12043.274f, 535491.38f } - camera->m_pos).len() / g_audioRand;
            AUDIO_SetVariable("DesertScalar", v14);
            if (g_AudioTime <= g_audioNextTime)
                return;
            g_AudioTime = 0.0f;
            g_seed = 214013 * g_seed + 2531011;
            v15 = (float)(HIWORD(g_seed) & 0x7FFF);
            if (v14 < 1.0f) {
                g_audioNextTime = v15 * 0.0051881466f + 30.0f;
            } else {
                g_audioNextTime = v15 * 0.0027466659f + 30.0f;
                if (v9 <= 0.5f) {
                    if (v7 < 1400.0f)
                        AUDIO_Event("Play_Ambient_Coastal_Oneshots", 1, false);
                    else if (v7 < 3500.0)
                        AUDIO_Event("Play_Ambient_Forest_Oneshots", 1, true);
                }
            }
            break;
        case WID_RICHMOND:
            if(g_AudioTime > g_audioNextTime) {
                g_AudioTime = 0.0f;
                g_seed = 214013 * g_seed + 2531011;
                g_audioNextTime = (HIWORD(g_seed) & 0x7FFF) * 0.00061037019f + 15.0f;
                if (v7 < 1400.0f)
                    AUDIO_Event("Play_Ambient_Coastal_Oneshots", 1, false);
                else if (v7 < 3500.0f)
                    AUDIO_Event("Play_Ambient_Forest_Oneshots", 1, true);
            }
            break;
        default:
            break;
        }
    }
}
void AUDIO_Queue(float tm, const char *eventName, int objId /*AUDIO_PlayFlatFile:1, AUDIO_Event:a2 */, uint32_t ebId /*counter, 0*/, void *data/*,0*/, const char *flatName /*,""*/) {
    if (g_soundInitialized) {
        auto &v13 = g_audioQueue.emplace_back(audioQueueEntry{ ebId, tm, objId, 0, eventName, flatName, data != nullptr });
        if (data)
            memmove(&v13.m_info, data, sizeof(v13.m_info));
    }
}
void AUDIO_Event(const char *name, int objId, bool vital) {
    static std::set<uint32_t> gmap_AUDIO_Event;
    if (objId != -1 && g_soundInitialized) {
        if (AUDIO_HandleFailsafe()) {
            if (vital)
                AUDIO_Queue(g_audio_queue_failsafe * 0.0025f, name, objId, 0, nullptr, "");
            return;
        }
        if (!AK::SoundEngine::PostEvent(name, objId)) {
            auto sig = SIG_CalcCaseInsensitiveSignature(name);
            if (!gmap_AUDIO_Event.contains(sig)) {
                gmap_AUDIO_Event.insert(sig);
                Log("AUDIO_Event(eventName:%s) failed!", name);
            }
        }
    }
}
int g_pffCounter;
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
bool AUDIO_HandleFailsafe() {
    if (g_audio_queue_failsafe++ >= 11) {
        if (g_audio_queue_failsafe > 40)
            return true;
        AK::SoundEngine::RenderAudio(true);
    }
    return false;
}
bool AUDIO_PlayFlatFile(const char *name, float tm) {
    if (!g_soundInitialized)
        return false;
    AUDIO_HandleFailsafe();
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