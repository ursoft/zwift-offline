#include "ZwiftApp.h"
void ParseXMLMapSchedule() {
    //TODO
}
void GAME_onFinishedDownloadingMapSchedule(const std::string &filename, int err) {
    auto st = g_mDownloader.PrintState();
    Log("Game::GAME_onFinishedDownloadingMapSchedule filename=%s err=%d state: %s", filename.c_str(), err, st.c_str());
    //CrashReporting::AddBreadcrumb(log);
    if (err) {
        g_bMapSchedule_DLFailed = true;
        Log("ERROR: #%d Downloading \"%s\"!", err, filename.c_str());
    } else {
        g_mDownloader.ForgetCompleted(filename);
        g_bMapSchedule_DLFailed = false;
    }
    ParseXMLMapSchedule();
    g_bMapSchedule_Ready = 1;
}
void GAME_GetSuppressedLogTypes() {
    if (g_useLogLevelSettings) {
        auto str = g_UserConfigDoc.GetCStr("ZWIFT\\GAME_LOGS_SUPPRESSED", nullptr, true);
        if (str)
            LogSetSuppressedLogTypes(ParseSuppressedLogs(str));
    }
}
void GAME_SetUseErgModeInWorkouts(bool val) {
    g_UseErgModeInWorkouts = val;
    g_UserConfigDoc.SetBool("ZWIFT\\WORKOUTS\\USE_ERG", val, true);
}
void GAME_Initialize() {
    //TODO
}
void GAME_SetTrainerSlopeModifier(float val) {
    //TODO
}