#include "ZwiftApp.h"
void ParseXMLMapSchedule() {
    //TODO
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
            if (g_AudioTime > g_audioNextTime) {
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
void GAME_onFinishedDownloadingMapSchedule(const std::string &filename, int err) {
    auto st = Downloader::Instance()->PrintState();
    Log("Game::GAME_onFinishedDownloadingMapSchedule filename=%s err=%d state: %s", filename.c_str(), err, st.c_str());
    //CrashReporting::AddBreadcrumb(log);
    if (err) {
        g_bMapSchedule_DLFailed = true;
        Log("ERROR: #%d Downloading \"%s\"!", err, filename.c_str());
    } else {
        Downloader::Instance()->ForgetCompleted(filename);
        g_bMapSchedule_DLFailed = false;
    }
    ParseXMLMapSchedule();
    g_bMapSchedule_Ready = 1;
}
void GAME_GetSuppressedLogTypes() {
    if (g_UserConfigDoc.m_loadResult) {
        auto str = g_UserConfigDoc.GetCStr("ZWIFT\\GAME_LOGS_SUPPRESSED", nullptr, true);
        if (str)
            LogSetSuppressedLogTypes(ParseSuppressedLogs(str));
    }
}
void GAME_SetUseErgModeInWorkouts(bool val) {
    g_UseErgModeInWorkouts = val;
    g_UserConfigDoc.SetBool("ZWIFT\\WORKOUTS\\USE_ERG", val);
}
void GAME_Initialize() {
    //TODO
}
void GAME_SetTrainerSlopeModifier(float val) {
    //TODO
}
bool GAME_IsWorldIDAvailableViaPrefsFile(int) {
    //TODO
    return true;
}
void GAME_GetMapForTime(time_t t) {
    //TODO
}
void GAME_GetMapForTime_New(time_t t) {
    //TODO
}
void GAME_LoadLevel(int) {
    //TODO
}
void GAME_ResetScreenshotsForActivity() {
    //TODO
}
void GAME_IncreaseFlatRoadTrainerResistance() {
    //TODO
}
void GAME_DecreaseFlatRoadTrainerResistance() {
    //TODO
}
void TriggerLocalPlayerAction(BikeEntity::USER_ACTION) {
    //TODO
}
int GAME_PrepareScreenshotFilename(GAME_Screenshot *scr, const GAME_ScreenshotParams &parms) {
    scr->m_isJpg = parms.m_isJpg;
    scr->m_ss = parms.m_ss;
    scr->m_useContentKinds = parms.m_useContentKinds;
    scr->m_field_570 = false;
    scr->m_field_519 = parms.m_field_3;
    scr->m_soundName = parms.m_soundName;
    scr->m_field_528 = parms.m_field_88;
    scr->m_func =  parms.m_func;
    auto imgprefs = g_UserConfigDoc.GetS32("ZWIFT\\CONFIG\\IMGPREFS", 0, true);
    scr->m_field_518 = (imgprefs == 2) ? false : parms.m_field_4;
    char buf[64]{}, pathName[0x400]{'.'};
    auto t = _time64(nullptr);
    auto v11 = _localtime64(&t);
    if (!OS_GetPicturesPath(pathName))
        return -1;
    sprintf_s(pathName, 0x400ui64, "%s\\Zwift", pathName);
    CreateDirectoryA(pathName, nullptr);
    sprintf_s(buf, sizeof(buf), "%d-%02d-%02d_%02d-%02d-%02d_%d", v11->tm_year + 1900, v11->tm_mon + 1, v11->tm_mday, v11->tm_hour, v11->tm_min, v11->tm_sec, g_screenShotCounter);
    uint8_t mask = 1;
    for (auto &m_contents : scr->m_contents) {
        if (mask & scr->m_useContentKinds) {
            const char *p_m_activityName = (parms.m_activityName.length()) ? parms.m_activityName.c_str() : buf;
            sprintf_s(m_contents.m_pathName, sizeof(m_contents.m_imageName), (mask == 1) ? "%s_clean.%s" : "%s.%s", p_m_activityName, scr->m_isJpg ? "jpg" : "tga");
            sprintf_s(m_contents.m_imageName, sizeof(m_contents.m_imageName), "%s\\%s", pathName, m_contents.m_pathName);
        }
        mask <<= 1;
    }
    return 0;
}
int GAME_QueueScreenshot(const GAME_ScreenshotParams &ssp) {
    //if (qword_7FF77A2E52B0)                     // not used?
    //    return 1;
    if (g_PostFX_GetScreenFlashTime > 0.0f)
        return -1;
    if (!ssp.m_useContentKinds)
        return 1;
    if (g_screenshotInProgress)
        return -1;
    auto gs_ = ssp.m_gameScreenshot;
    if (!gs_) {
        if (g_screenShotCounter >= GAME_MaxStoredScreenshots())
            return -1;
        gs_ = &g_screenshotPool[g_screenShotCounter];
    }
    auto result = GAME_PrepareScreenshotFilename(gs_, ssp);
    if (result < 0)
        return result;
    ++g_screenShotCounter;
    g_screenshotInProgress = gs_;
    g_lastScreenshotSource = ssp.m_ss;
    if (gs_->m_soundName)
        AUDIO_Event(gs_->m_soundName, 1, false);
    return 0;
}
