#include "ZwiftApp.h"
void SetupConsoleCommands() {
    CONSOLE_Init();
    CONSOLE_AddCommand("loadconfig", CMD_LoadConfig);
    CONSOLE_AddCommand("time", CMD_Time);
    CONSOLE_AddCommand("pairhr", CMD_PairHr);
    CONSOLE_AddCommand("pairpower", CMD_PairPower);
    CONSOLE_AddCommand("antstartsearch", CMD_AntStartSearch);
    CONSOLE_AddCommand("antstopsearch", CMD_AntStopSearch);
    CONSOLE_AddCommand("trainersetsimmode", CMD_TrainerSetSimMode);
    CONSOLE_AddCommand("trainersetsimgrade", CMD_TrainerSetSimGrade);
    CONSOLE_AddCommand("listdevices", CMD_ListDevices);
    CONSOLE_AddCommand("settrainerdelay", CMD_SetTrainerDelay);
    CONSOLE_AddCommand("focus", CMD_Focus);
    CONSOLE_AddCommand("raceresults", CMD_RaceResults);
    CONSOLE_AddCommand("time_to_tp_workout", CMD_Time_to_tp_workout);
    CONSOLE_AddCommand("evfin", CMD_EvFin);
    CONSOLE_AddCommand("set_object_visible", CMD_SetObjectVisible);
    CONSOLE_AddCommand("benchmark", CMD_Benchmark);
    CONSOLE_AddCommand("enroll_in_trainingplan", CMD_EnrollInTrainingPlan);
    CONSOLE_AddCommand("show_ui", CMD_ShowUI);
}
void CONSOLE_Init() {
    CONSOLE_AddCommand("help", CMD_Help);
    CONSOLE_AddCommand("togglelog", CMD_ToggleLog);
    CONSOLE_AddCommand("set", CMD_Set, nullptr, CMD_Set3, CMD_Set4);
    CONSOLE_AddCommand("listvars", CMD_ListVars, nullptr, nullptr, CMD_ListVars4);
}
void CONSOLE_AddCommand(const char *name, CMD_bool f1, CMD_static_str f2, CMD_ac_search f3, CMD_string f4) {
    zassert(g_knownCommandsCounter < sizeof(g_knownCommands) / sizeof(g_knownCommands[0]));
    ConsoleCommandFuncs newObj { name, f1, f2, f3, f4 };
    for (int i = 0; i < g_knownCommandsCounter; i++)
        if (g_knownCommands[i] == newObj)
            return;
    g_knownCommands[g_knownCommandsCounter++] = newObj;
}
const char *GAMEPATH(const char *path) {
    zassert(g_MainThread == GetCurrentThreadId());
    return path;
}
bool COMMAND_RunCommandsFromFile(const char *name) {
    char buf[1024 + 4];
    sprintf_s(buf, "data/configs/%s.txt", name);
    FILE *f = nullptr;
    fopen_s(&f, buf, "r");
    if (nullptr == f)
        return false;
    while (!feof(f)) {
        buf[0] = 0;
        fgets(buf, sizeof(buf) - 1, f);
        if (*buf)
            COMMAND_RunCommand(buf);
    }
    fclose(f);
    return true;
}
void StripPaddedSpaces(std::string *dest, const std::string &src) {
    auto srcSize = (int)src.size();
    if (srcSize) {
        int i = 0, j = srcSize - 1;
        for (; i < srcSize; ++i)
            if (!std::isspace(src[i]))
                break;
        for (; j > i; --j)
            if (!std::isspace(src[j]))
                break;
        dest->assign(src, i, j - i + 1);
    } else {
        dest->clear();
    }
}
void SplitCommand(const std::string &cmd, std::string *name, std::string *params, char delim) {
    int i = 0;
    for (; i < cmd.size(); ++i)
        if (cmd[i] == delim || cmd[i] < ' ' /*CR etc*/)
            break;
    name->assign(cmd, 0, i);
    if (i >= cmd.size())
        params->clear();
    else
        StripPaddedSpaces(params, std::string(cmd, i + 1, cmd.size() - i - 1));
}
bool findStringIC(const std::string &strHaystack, const std::string &strNeedle) {
    auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(), strNeedle.end(),
        [](unsigned char ch1, unsigned char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
        );
    return it != strHaystack.end();
}
bool iequals(const std::string &a, const std::string &b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                      [](char a, char b) { return tolower(a) == tolower(b); });
}
void FindCommands(std::vector<ConsoleCommandFuncs *> *dest, const std::string &name) {
    for (int i = 0; i < g_knownCommandsCounter; ++i) {
        auto &curCmd = g_knownCommands[i];
        auto curCmdSize = curCmd.m_name.size();
        auto nameSize = name.size();
        if (curCmdSize == nameSize && iequals(curCmd.m_name, name)) {
            dest->push_back(&curCmd);
        } else if (nameSize <= curCmdSize) {
            if (nameSize) {
                if (findStringIC(curCmd.m_name, name))
                    dest->push_back(&curCmd);
            } else {
                dest->push_back(&curCmd);
            }
        }
    }
}
bool COMMAND_RunCommand(const char *cmd) {
    while (isspace(*cmd)) ++cmd;
    auto i = strlen(cmd);
    while (i > 0 && isspace(cmd[i - 1])) i--;
    std::string scmd(cmd, i), name, params;
    SplitCommand(scmd, &name, &params, ' ');
    std::vector<ConsoleCommandFuncs *> foundCommands;
    FindCommands(&foundCommands, name);
    ConsoleCommandFuncs *selFunc = nullptr;
    if (foundCommands.size() == 1) {
        selFunc = foundCommands[0];
    } else if (foundCommands.size() > 1) {
        int bestDiff = -1;
        for (auto f : foundCommands) {
            auto diff = abs(int(f->m_name.size() - name.size()));
            if (diff < bestDiff) {
                bestDiff = diff;
                selFunc = f;
            }
        }
    }
    if (!selFunc || foundCommands.empty()) {
        CMD_Set(cmd);
        LogTyped(LOG_COMMAND_OUTPUT, "Unknown command \"%s\"", scmd.c_str());
    } else {
        LogTyped(LOG_COMMAND, "%s", cmd);
        if (selFunc->m_bool)
            selFunc->m_bool(params.c_str());
    }
    return false;
}
bool CMD_LoadConfig(const char *par) {
    return COMMAND_RunCommandsFromFile(par);
}
bool CMD_Time(const char *) {
    return true; //TODO
}
bool CMD_PairHr(const char *) {
    return true; //TODO
}
bool CMD_PairPower(const char *) {
    return true; //TODO
}
bool CMD_AntStartSearch(const char *) {
    return true; //TODO
}
bool CMD_AntStopSearch(const char *) {
    return true; //TODO
}
bool CMD_TrainerSetSimMode(const char *) {
    return true; //TODO
}
bool CMD_ChangeRes(const char *par) {
    int w = 0, h = 0, ms = 0;
    if (strstr(par, "x")) {
        if (sscanf(par, "%dx%d(%dx)", &w, &h, &ms) == 3 && w >= 320 && h >= 240 && w <= 5120 && h <= 2880) {
            BACKBUFFER_WIDTH = w;
            if (ms <= 64)
                g_nMultiSamples = ms;
            BACKBUFFER_HEIGHT = h;
            VRAM_CreateAllRenderTargets();
            if (g_nMultiSamples)
                LogTyped(LOG_COMMAND_OUTPUT, "Changed resolution to %d x %d  (%dxMSAA)\n", w, h, g_nMultiSamples);
            else
                LogTyped(LOG_COMMAND_OUTPUT, "Changed resolution to %d x %d  ( NO MSAA )\n", w, h);
        } else if (sscanf(par, "%dx%d", &w, &h) == 2 && w >= 320 && h >= 240 && w <= 5120 && h <= 2880) {
            BACKBUFFER_WIDTH = w;
            BACKBUFFER_HEIGHT = h;
            VRAM_CreateAllRenderTargets();
            LogTyped(LOG_COMMAND_OUTPUT, "Changed resolution to %d x %d\n", w, h);
        } else {
            if (sscanf(par, "(%dx)", &ms) != 1) {
                LogTyped(LOG_COMMAND_OUTPUT, "Error with resolution values (%d x %d)\n", w, h);
                return false;
            }
            if (ms <= 64)
                g_nMultiSamples = ms;
            VRAM_CreateAllRenderTargets();
            LogTyped(LOG_COMMAND_OUTPUT, "Changed MSAA to %dx\n", g_nMultiSamples);
        }
    } else {
        if (g_nMultiSamples)
            LogTyped(LOG_COMMAND_OUTPUT, "Current resolution is %d x %d with %dx MSAA", BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT, g_nMultiSamples);
        else
            LogTyped(LOG_COMMAND_OUTPUT, "Current resolution is %d x %d with NO MSAA", BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT);
        LogTyped(LOG_COMMAND_OUTPUT, "To Change use: res WIDTHxHEIGHT   or   res WIDTHxHEIGHT(4x) for 4xMSAA");
    }
    return true;
}
bool CMD_ChangeShadowRes(const char *par) {
    int w, h;
    if (strstr(par, "x")) {
        sscanf(par, "%dx%d", &w, &h);
        if (sscanf(par, "%dx%d", &w, &h) != 2 || w < 0x100 || h < 0x100 || w > 0x1000 || h > 0x1000) {
            LogTyped(LOG_COMMAND_OUTPUT, "Error with resolution values (%d x %d)\n", w, h);
            return 0;
        }
        SHADOWMAP_WIDTH = w;
        SHADOWMAP_HEIGHT = h;
        VRAM_CreateAllRenderTargets();
        LogTyped(LOG_COMMAND_OUTPUT, "Changed shadow resolution to %d x %d\n", w, h);
    } else {
        LogTyped(
            LOG_COMMAND_OUTPUT,
            "Current SHADOW resolution is %d x %d",
            SHADOWMAP_WIDTH,
            SHADOWMAP_HEIGHT);
        LogTyped(LOG_COMMAND_OUTPUT, "To Change use: sres WIDTHxHEIGHT");
    }
    return true;
}
bool CMD_TrainerSetSimGrade(const char *arg) {
    float v = 0.0;
    sscanf_s(arg, "%f", &v);
    if (v < -1.0 || v >= 1.0)
        return false;
    FitnessDeviceManager::TrainerSetSimGrade(v);
    return true;
}
bool CMD_ListDevices(const char *) {
    return true; //TODO
}
bool CMD_SetTrainerDelay(const char *arg) {
    int v;
    if (sscanf_s(arg, "%d", &v) != 1)
        return false;
    g_trainerDelay = v;
    return true;
}
bool CMD_Focus(const char *) {
    return true; //TODO
}
bool CMD_RaceResults(const char *) {
    return true; //TODO
}
bool CMD_Time_to_tp_workout(const char *) {
    return true; //TODO
}
bool CMD_EvFin(const char *) {
    return true; //TODO
}
bool CMD_SetObjectVisible(const char *) {
    return true; //TODO
}
bool CMD_Benchmark(const char *) {
    return true; //TODO
}
bool CMD_EnrollInTrainingPlan(const char *) {
    return true; //TODO
}
bool CMD_ShowUI(const char *) {
    return true; //TODO
}
bool CMD_Help(const char *) {
    LogTyped(LOG_COMMAND_OUTPUT, "Known Commands: ");
    for (int i = 0; i < g_knownCommandsCounter; i++)
        LogTyped(LOG_COMMAND_OUTPUT, "%s", g_knownCommands[i].m_name.c_str());
    return true;
}
bool CMD_ToggleLog(const char *) {
    g_Console.m_logVisible = !g_Console.m_logVisible;
    return true;
}
consteval uint32_t f2u(float f) { return std::bit_cast<uint32_t, float>(f); }
static_assert(f2u(300.0f) == 0x43960000);
TweakInfo g_tweakArray[] = {
    //m_valueUnion,  fMin, fMax, um, uM,        im,   iM,   1C,20,      name,                                        file, line                          dataType
    { 1u,            0.0f, 0.0f, 0u, 1u,        0,    0,    0, nullptr, "s_autoBrakingMode",                         "AutoBrakingModule.cpp",        23, TWD_UINT },
    { 0u,            0.0f, 0.0f, 0u, 2u,        0,    0,    0, nullptr, "s_AB_onRoadVisualsMode",                    "AutoBrakingModule.cpp",        36, TWD_UINT },
    { f2u(300.0f),   FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_StrideDistance",                          "BikeComputer.cpp",             60, TWD_FLOAT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "g_TestUpdateRealTime",                      "BikeComputer.cpp",             61, TWD_BOOL },
    { f2u(20.0f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_minPowerPauseThreshold",                  "BikeComputer.cpp",             65, TWD_FLOAT },
    { f2u(10000.0f), FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_BikeSoundDistCutoffCentemeters",          "BikeEntity.cpp",              195, TWD_FLOAT },
    { f2u(0.86f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_bikeRainParticleForwardOffset",           "BikeEntity.cpp",              197, TWD_FLOAT },
    { f2u(0.8f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_bikeDustParticleForwardOffset",           "BikeEntity.cpp",              199, TWD_FLOAT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "bShowEventPlacement",                       "BikeEntity.cpp",              250, TWD_BOOL },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "bShowSpeed",                                "BikeEntity.cpp",              251, TWD_BOOL },
    { f2u(0.75f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "k_MaxPacketAgeInSeconds",                   "BikeEntity.cpp",              260, TWD_FLOAT },
    { f2u(100.0f),   FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "k_MaxSideProjectionInCm",                   "BikeEntity.cpp",              261, TWD_FLOAT },
    { f2u(3.0f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "k_AimForceMultiplier",                      "BikeEntity.cpp",              262, TWD_FLOAT },
    { f2u(3.0f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "k_SideForceMultiplier",                     "BikeEntity.cpp",              263, TWD_FLOAT },
    { f2u(3.0f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "k_DirectionForceMultiplier",                "BikeEntity.cpp",              264, TWD_FLOAT },
    { f2u(1.25f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "k_NetworkQualityAmplifier",                 "BikeEntity.cpp",              265, TWD_FLOAT },
    { f2u(5.0f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "k_ProjectionTimeMultiplier_AutoSteering",   "BikeEntity.cpp",              266, TWD_FLOAT },
    { f2u(50.0f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "k_ProjectionTimeMultiplier_ManualSteering", "BikeEntity.cpp",              267, TWD_FLOAT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gDebugRoadWidth",                           "BikeEntity.cpp",              355, TWD_BOOL },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gShowWorldCoordinate",                      "BikeEntity.cpp",              356, TWD_BOOL },
    { uint32_t(-1),  0.0f, 0.0f, 0u, 0u,        IMIN, IMAX, 0, nullptr, "g_DEBUG_logSplineDistNetworkID",            "BikeEntity.cpp",             1615, TWD_INT },
    { 0u,            0.0f, 0.0f, 0u, 999u,      0,    0,    0, nullptr, "arch_dir_male",                             "BikeEntity.cpp",             2875, TWD_UINT },
    { f2u(20.0f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "f3DEffectStrength",                         "BikeManager.cpp",              53, TWD_FLOAT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gPositionAccuracyOverNetwork",              "BikeManager.cpp",              93, TWD_BOOL },
    { 50u,           0.0f, 0.0f, 0u, 1000u,     0,    0,    0, nullptr, "gSpawnGap",                                 "BikeManager.cpp",             147, TWD_UINT },
    { 0u,            0.0f, 0.0f, 0u, 10u,       0,    0,    0, nullptr, "gAIgroupingTest",                           "BikeManager.cpp",            2722, TWD_UINT },
    { 1u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "g_bAllowAvatarsToOverflow",                 "BikeManager.cpp",            3613, TWD_BOOL },
    { f2u(50.0f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "mass",                                      "Camera.cpp",                  404, TWD_FLOAT },
    { f2u(600.0f),   FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "maxDamp",                                   "Camera.cpp",                  405, TWD_FLOAT },
    { f2u(120.0f),   FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "minDamp",                                   "Camera.cpp",                  406, TWD_FLOAT },
    { f2u(1.0f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "lerpTime",                                  "Camera.cpp",                  407, TWD_FLOAT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gAllowPlayersOnGrass",                      "CollisionResolver.cpp",         6, TWD_BOOL },
    { 300u,          0.0f, 0.0f, 0u, 10000u,    0,    0,    0, nullptr, "g_defaultButtonDelay_MS",                   "ControllerActionManager.cpp",  10, TWD_UINT },
    { f2u(0.0f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_cullDist",                                "Instancing.cpp",               22, TWD_FLOAT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gShowHistogram",                            "PostEffects.cpp",               8, TWD_BOOL },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gShowLuminance",                            "PostEffects.cpp",               9, TWD_BOOL },
    { f2u(0.4f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "gDefaultLuminanceMean",                     "PostEffects.cpp",              11, TWD_FLOAT },
    { f2u(0.8f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "gDefaultLuminanceMax",                      "PostEffects.cpp",              12, TWD_FLOAT },
    { f2u(1.4f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "gHistogramLuminanceCutoff",                 "PostEffects.cpp",              15, TWD_FLOAT },
    { 2u,            0.0f, 0.0f, 1u, 8u,        0,    0,    0, nullptr, "gLuminanceSubRectCountX",                   "PostEffects.cpp",              18, TWD_UINT },
    { 2u,            0.0f, 0.0f, 1u, 8u,        0,    0,    0, nullptr, "gLuminanceSubRectCountY",                   "PostEffects.cpp",              19, TWD_UINT },
    { f2u(1.0f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "gMaxBloomScale",                            "PostEffects.cpp",              22, TWD_FLOAT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gGetTrigger",                               "AnimatedProp.cpp",           1545, TWD_BOOL },
    { 200u,          0.0f, 0.0f, 0u, 2000u,     0,    0,    0, nullptr, "g_ShiftingInputDelay_MS",                   "DeviceManager.cpp",            66, TWD_UINT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "g_FTMS_EnableSimBikeTuning",                "FTMS_Control_v3.cpp",          22, TWD_BOOL },
    { f2u(0.75f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_FTMS_GradeLookaheadTime",                 "FTMS_Control_v3.cpp",          23, TWD_FLOAT },
    { 250u,          0.0f, 0.0f, 0u, 0u,        IMIN, IMAX, 0, nullptr, "g_ComputrainerSendTime",                    "SerialTrainerReceiver.cpp",    92, TWD_INT },
    { 250u,          0.0f, 0.0f, 0u, 0u,        IMIN, IMAX, 0, nullptr, "g_EliteSendTime",                           "SerialTrainerReceiver.cpp",    93, TWD_INT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "g_bShowSteeringOutro",                      "GameplayEventsManager.cpp",    75, TWD_BOOL },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "g_bAwardMTB",                               "GameplayEventsManager.cpp",    76, TWD_BOOL },
    { f2u(0.15f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_IdealBikesPerMeter",                      "Rubberbanding.cpp",           359, TWD_FLOAT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "g_bForceSteeringUI",                        "UI_PauseScreen.cpp",         2588, TWD_BOOL },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "g_bForceQuitEventButton",                   "UI_PauseScreen.cpp",         2599, TWD_BOOL },
    { 1000u,         0.0f, 0.0f, 0u, 0u,        IMIN, IMAX, 0, nullptr, "gPostRideGraphDisplayLimit",                "UI_PostRideStats.cpp",       1166, TWD_INT },
    { f2u(3.0f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_workoutAutoPauseTime",                    "workout.cpp",                  50, TWD_FLOAT },
    { f2u(10.0f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_rampUpTime",                              "workout.cpp",                  51, TWD_FLOAT },
    { f2u(5.0f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_minERGModeDisableTime",                   "workout.cpp",                  52, TWD_FLOAT },
    { f2u(10.0f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_reenableERGPowerThreshold",               "workout.cpp",                  53, TWD_FLOAT },
    { f2u(0.1f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_steadyStateFailureThreshold",             "workout.cpp",                  54, TWD_FLOAT },
    { f2u(0.1f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_intervalOnFailureThreshold",              "workout.cpp",                  55, TWD_FLOAT },
    { f2u(0.25f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g_intervalOffFailureThreshold",             "workout.cpp",                  56, TWD_FLOAT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "g_bEnableOculus",                           "ZwiftApp.cpp",               1082, TWD_BOOL },
    { 1u,            0.0f, 0.0f, 0u, 0u,        IMIN, IMAX, 0, nullptr, "g_BikeMaxShadowCascade",                    "ZwiftApp.cpp",               1085, TWD_INT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "g_3DTVEnabled",                             "ZwiftApp.cpp",               1083, TWD_BOOL },
    { 1u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "g_MinimalUI",                               "ZwiftApp.cpp",               1092, TWD_BOOL },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gSSAO",                                     "ZwiftApp.cpp",               1115, TWD_BOOL },
    { 1u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gFXAA",                                     "ZwiftApp.cpp",               1116, TWD_BOOL },
    { 1u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gBloom",                                    "ZwiftApp.cpp",               1118, TWD_BOOL },
    { 1u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gRadialBlur",                               "ZwiftApp.cpp",               1119, TWD_BOOL },
    { 1u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gVignette",                                 "ZwiftApp.cpp",               1120, TWD_BOOL },
    { 1u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gColorCorrection",                          "ZwiftApp.cpp",               1121, TWD_BOOL },
    { 1u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gAutoExposure",                             "ZwiftApp.cpp",               1122, TWD_BOOL },
    { 1u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gTonemap",                                  "ZwiftApp.cpp",               1123, TWD_BOOL },
    { 1u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gSSR",                                      "ZwiftApp.cpp",               1124, TWD_BOOL },
    { 1u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gDistortion",                               "ZwiftApp.cpp",               1125, TWD_BOOL },
    { 1u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gHeadlight",                                "ZwiftApp.cpp",               1126, TWD_BOOL },
    { f2u(0.5f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "gHeadlightIntensity",                       "ZwiftApp.cpp",               1127, TWD_FLOAT },
    { 1u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gSunRays",                                  "ZwiftApp.cpp",               1132, TWD_BOOL },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gSimpleReflections",                        "ZwiftApp.cpp",               1135, TWD_BOOL },
    { f2u(0.5f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "gBloomStrength",                            "ZwiftApp.cpp",               1137, TWD_FLOAT },
    { f2u(0.99f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "gBloomThreshold",                           "ZwiftApp.cpp",               1138, TWD_FLOAT },
    { f2u(2.0f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "gRainExposureMultiplier",                   "ZwiftApp.cpp",               1140, TWD_FLOAT },
    { f2u(2.0f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "gBlurMultiplier",                           "ZwiftApp.cpp",               1141, TWD_FLOAT },
    { f2u(0.6f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "gTonemapExponentBias",                      "ZwiftApp.cpp",               1143, TWD_FLOAT },
    { f2u(1.3f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "gTonemapExponentScale",                     "ZwiftApp.cpp",               1144, TWD_FLOAT },
    { f2u(0.75f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "gExposureRate",                             "ZwiftApp.cpp",               1145, TWD_FLOAT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "gShowFPS",                                  "ZwiftApp.cpp",               1154, TWD_BOOL },
    { 0u,            0.0f, 0.0f, 0u, 0u,        IMIN, IMAX, 0, nullptr, "gLODBias",                                  "ZwiftApp.cpp",               1160, TWD_INT },
    { f2u(8.0f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g3DTVScreenDistance",                       "ZwiftApp.cpp",               1183, TWD_FLOAT },
    { f2u(0.18f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g3DTVEffect",                               "ZwiftApp.cpp",               1184, TWD_FLOAT },
    { f2u(50.0f),    FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g3DTVSizeInches",                           "ZwiftApp.cpp",               1185, TWD_FLOAT },
    { f2u(6.4f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "g3DTVEyeSpacingCM",                         "ZwiftApp.cpp",               1186, TWD_FLOAT },
    { f2u(1.0f),     FMIN, FMAX, 0u, 0u,        0,    0,    0, nullptr, "gFoliagePercent",                           "ZwiftApp.cpp",               1188, TWD_FLOAT },
    { 0u,            0.0f, 0.0f, 0u, 0u,        0,    0,    0, nullptr, "g_bShowPacketDelay",                        "ZwiftApp.cpp",               7964, TWD_BOOL },
};
static_assert(TWI_CNT == _countof(g_tweakArray));
void CMD_Set3_Populate(std::vector<std::string> *dest, const std::string &par) {
    std::string lowPar;
    lowPar.reserve(par.size());
    for (auto c : par)
        lowPar.push_back(std::tolower(c));
    for (auto &tw : g_tweakArray) {
        std::string lowTw;
        auto        *pName = tw.m_name;
        while (*pName)
            lowTw.push_back(std::tolower(*pName++));
        if (lowPar.size() <= lowTw.size()) {
            auto f = lowTw.find(lowPar);
            if (f != -1)
                dest->push_back("set " + std::string(tw.m_name) + '=');
        }
    }
}
bool CMD_Set(const char *par) {
    std::string spar(par), name, value;
    SplitCommand(spar, &name, &value, '=');
    if (value.empty()) {
        LogTyped(LOG_COMMAND_OUTPUT, "Syntax Error.  USAGE: set VARNAME=VALUE");
        LogTyped(LOG_COMMAND_OUTPUT, "Type 'listvars' for list of tweakable variables");
    } else {
        std::string lowPar;
        lowPar.reserve(name.size());
        for (auto c : name)
            lowPar.push_back(std::tolower(c));
        TweakInfo *pFound = nullptr;
        for (auto &tw : g_tweakArray) {
            std::string lowTw;
            auto        *pName = tw.m_name;
            while (*pName)
                lowTw.push_back(std::tolower(*pName++));
            if (lowPar == lowTw) {
                pFound = &tw;
                break;
            }
        }
        if (pFound) {
            if (pFound->SetValue(value))
                CMD_ListVars(name.c_str());
            else
                LogTyped(LOG_COMMAND_OUTPUT, "Error setting %s. Possible error with value \"%s\".", name.c_str(), value.c_str());
        } else {
            LogTyped(LOG_COMMAND_OUTPUT, "Error: Variable \"%s\" not found! ", name.c_str());
        }
    }
    return true;
}
void CMD_Set3(CMD_AutoCompleteParamSearchResults *dest, const char *par) {
    if (par && *par && 0 != strcmp(par, "set")) {
        dest->m_descr.assign(par);
        CMD_Set3_Populate(&dest->m_field_20, dest->m_descr);
        dest->m_field_38 = true;
        return;
    }
    dest->m_descr.clear();
    dest->m_field_20.clear();
    dest->m_field_38 = false;
}
std::string CMD_Set4(const char *) {
    return "set <TweakableID> [parameters] {sets TweakableID to parameters; use 'listvars' to show all tweakable IDs}";
}
bool CMD_ListVars(const char *par) { //TweakMaster_DumpMatchedToLog
    std::string lowPar, spar(par);
    lowPar.reserve(spar.size());
    for (auto c : spar)
        lowPar.push_back(std::tolower(c));
    for (auto &tw : g_tweakArray) {
        std::string lowTw;
        auto        *pName = tw.m_name;
        while (*pName)
            lowTw.push_back(std::tolower(*pName++));
        if (lowPar.size() <= lowTw.size()) {
            auto f = lowTw.find(lowPar);
            if (f != -1) {
                switch (tw.m_dataType) {
                    case TWD_STRING:
                        LogTyped(LOG_COMMAND_OUTPUT, "%s = %s", tw.m_name, tw.m_str.c_str());
                        break;
                    case TWD_BOOL:
                        LogTyped(LOG_COMMAND_OUTPUT, "%s = %s", tw.m_name, tw.m_valueUnion ? "TRUE" : "FALSE");
                        break;
                    case TWD_INT:
                        LogTyped(LOG_COMMAND_OUTPUT, "%s = %d (valid: %d...%d)", tw.m_name, tw.IntValue(), tw.m_intMin, tw.m_intMax);
                        break;
                    case TWD_UINT:
                        LogTyped(LOG_COMMAND_OUTPUT, "%s = %uu (valid: %d...%d)", tw.m_name, tw.m_valueUnion, tw.m_uintMin, tw.m_uintMax);
                        break;
                    case TWD_FLOAT:
                        LogTyped(LOG_COMMAND_OUTPUT, "%s = %3.2f (valid: %g..%g)", tw.m_name, tw.FloatValue(), tw.m_floatMin, tw.m_floatMax);
                        break;
                }
            }
        }
    }
    return true;
}
std::string CMD_ListVars4(const char *) {
    return "listvars {prints all Tweakable IDs to console log; use 'set' to modify a Tweakable}";
}
//non-zwift: console redirection (useful for debugging and unit testing)
//https://stackoverflow.com/questions/191842/how-do-i-get-console-output-in-c-with-a-windows-program
namespace non_zwift {
bool RedirectConsoleIO() {
    bool result = true;
    FILE *fp;
    // Redirect STDIN if the console has an input handle
    if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE)
        if (freopen_s(&fp, "CONIN$", "r", stdin) != 0)
            result = false;
        else
            setvbuf(stdin, NULL, _IONBF, 0);
    // Redirect STDOUT if the console has an output handle
    if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE)
        if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0)
            result = false;
        else
            setvbuf(stdout, NULL, _IONBF, 0);
    // Redirect STDERR if the console has an error handle
    if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE)
        if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0)
            result = false;
        else
            setvbuf(stderr, NULL, _IONBF, 0);
    // Make C++ standard streams point to console as well.
    std::ios::sync_with_stdio(true);
    // Clear the error state for each of the C++ standard streams.
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();
    return result;
}
bool ReleaseConsole() {
    bool result = true;
    FILE *fp;
    // Just to be safe, redirect standard IO to NUL before releasing.
    // Redirect STDIN to NUL
    if (freopen_s(&fp, "NUL:", "r", stdin) != 0)
        result = false;
    else
        setvbuf(stdin, NULL, _IONBF, 0);
    // Redirect STDOUT to NUL
    if (freopen_s(&fp, "NUL:", "w", stdout) != 0)
        result = false;
    else
        setvbuf(stdout, NULL, _IONBF, 0);
    // Redirect STDERR to NUL
    if (freopen_s(&fp, "NUL:", "w", stderr) != 0)
        result = false;
    else
        setvbuf(stderr, NULL, _IONBF, 0);
    // Detach from console
    if (!FreeConsole())
        result = false;
    return result;
}
void AdjustConsoleBuffer(int16_t minLength) {
    // Set the screen buffer to be big enough to scroll some text
    CONSOLE_SCREEN_BUFFER_INFO conInfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
    if (conInfo.dwSize.Y < minLength)
        conInfo.dwSize.Y = minLength;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), conInfo.dwSize);
}
bool CreateNewConsole(int16_t minLength) {
    bool result = false;
    // Release any current console and redirect IO to NUL
    ReleaseConsole();
    // Attempt to create new console
    if (AllocConsole()) {
        AdjustConsoleBuffer(minLength);
        result = RedirectConsoleIO();
    }
    return result;
}
bool AttachParentConsole(int16_t minLength) {
    bool result = false;
    // Release any current console and redirect IO to NUL
    ReleaseConsole();
    // Attempt to attach to parent process's console
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        AdjustConsoleBuffer(minLength);
        result = RedirectConsoleIO();
    }
    return result;
}
ConsoleHandler::ConsoleHandler(int16_t minLength) {
    m_releaseNeed = AttachParentConsole(minLength);
    if (!m_releaseNeed) {
        m_releaseNeed = CreateNewConsole(minLength);
        OutputDebugStringA("non_zwift: ConsoleHandler: AttachParentConsole failed");
    }
    if (!m_releaseNeed)
        OutputDebugStringA("non_zwift: ConsoleHandler: CreateNewConsole failed");
}
bool ConsoleHandler::LaunchUnitTests(int argc, char **argv) {
    int cnt = 0, cntMax = 1;
    bool ret = false;
    ::testing::InitGoogleTest(&argc, argv);
    do {
        ret = RUN_ALL_TESTS() == 0;
    } while (ret && ++cnt < cntMax);
    return ret;
}
ConsoleHandler::~ConsoleHandler() {
    if (m_releaseNeed)
        ReleaseConsole();
}
//for google tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#include <crtdbg.h>
#pragma warning(push)
#pragma warning(disable:4073)
#pragma init_seg(lib)
struct memLeakAtExit {
    _CrtMemState m_initial_state;
    memLeakAtExit() {
        g_MainThread = GetCurrentThreadId();
        ZMUTEX_SystemInitialize();
        LogInitialize();
#if defined(_DEBUG)
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
        OutputDebugStringA("ZA memLeakAtExit: ctr\n");
        _CrtMemCheckpoint(&m_initial_state);
#endif
    }
    void Update() {
#if defined(_DEBUG)
        OutputDebugStringA("ZA memLeakAtExit: Update requested\n");
        _CrtMemCheckpoint(&m_initial_state);
#endif
    }
    void Terminate() {
        LogShutdown();
        ZMUTEX_SystemShutdown();
        google::protobuf::ShutdownProtobufLibrary();
        u_cleanup();
#if defined(_DEBUG)
        _CrtMemState finish_state, diff_state;
        _CrtMemCheckpoint(&finish_state);
        _CrtMemDifference(&diff_state, &m_initial_state, &finish_state);
        _set_error_mode(_OUT_TO_STDERR);
        int crtReportMode = _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG;
        _CrtSetReportMode(_CRT_ASSERT, crtReportMode);
        _CrtSetReportMode(_CRT_WARN, crtReportMode);
        _CrtSetReportMode(_CRT_ERROR, crtReportMode);
        int d_n = (int)diff_state.lCounts[_NORMAL_BLOCK];
        int d_c = (int)diff_state.lCounts[_CLIENT_BLOCK];
        int f_n = (int)finish_state.lCounts[_NORMAL_BLOCK];
        int f_c = (int)finish_state.lCounts[_CLIENT_BLOCK];
        if (d_n > 1000 || d_c > 1000 || f_n > 1000 || f_c > 1000) {
            char stat[1024];
            sprintf_s(stat, "ZA memLeakAtExit: more than 1000 blocks (f %d/%d, d %d/%d), dump skipped.", f_n, f_c, d_n, d_c);
            if (IsDebuggerPresent())
                MessageBoxA(nullptr, stat, "ZA MLK", MB_OK | MB_ICONWARNING | MB_TOPMOST);
            else
                OutputDebugStringA(stat);
            int tmp = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
            tmp &= ~_CRTDBG_LEAK_CHECK_DF;
            _CrtSetDbgFlag(tmp);
        }
#endif
    }
    ~memLeakAtExit() {
        Terminate();
    }
} gMLK;
#pragma warning(pop)
} // namespace non_zwift
bool CMD_SetLanguage(const char *lang) {
    auto l = LOC_GetLanguageFromString(lang);
    if (l == LOC_CNT)
        return false;
    LOC_SetLanguageFromEnum(l, true);
    return true;
}
void CONSOLE_DrawCmdline(const ConsoleRenderer &cr, const char *line) {
    g_LargeFontW.RenderWString(cr.m_cmdX1, cr.m_cmdY, ">", 0xAA00CCFF, 0, cr.m_cmdScale, true, false);
    g_LargeFontW.RenderWString(cr.m_cmdX2, cr.m_cmdY, line, 0xAA00CCFF, 0, cr.m_cmdScale, true, false);
}
void CONSOLE_DrawPar(const ConsoleRenderer &cr, const char *str, int *lineNo, int lineCount, LOG_TYPE lineType) {
    auto ustr = ToUTF8_ib(str);
    auto w = cr.m_width - 6.0f /*URSOFT scrollbar*/;
    auto mea = g_LargeFontW.GetParagraphLineCountW(w, ustr, g_Console.LargeFontScale, 0.0f, false);
    int  lines = g_LargeFontW.RenderParagraphW(15.0f, cr.m_atY + 16.0f * (lineCount - mea + 1 - *lineNo),
                                               w, cr.m_height, ustr, ConsoleRenderer::TYPE_COLORS[lineType],
                                               0,
                                               g_Console.LargeFontScale,
                                               true,
                                               0.886f, //URSOFT FIX (was 1.0)
                                               0.0f,
                                               cr.m_atY + 16.0f /*URSOFT FIX, otherwise multiline overwrites top line*/);
    if (lines > 1)
        *lineNo += lines - 1;
}
void ScrollLog(int dir) {
    int maxScroll = LogGetLineCount() - (LOGC_PAGE + 1);
    g_scrollLogPos = std::clamp(g_scrollLogPos + (dir <= 0 ? 1 : -1), 0, maxScroll);
    g_alwaysScrollToEnd = (g_scrollLogPos >= maxScroll);
}
float     g_blinkTime;
const int CONSOLE_CMD_BUF = 1024, CONSOLE_CMDS_HISTORY = 16;
char      g_consoleCommand[CONSOLE_CMD_BUF], g_consoleCmdHistory[CONSOLE_CMDS_HISTORY][CONSOLE_CMD_BUF], g_consoleCmdHistoryIdx;
void ConsoleRenderer::Update(float atY) {
    if (m_mirrorY)
        atY = -atY;
    m_atY = atY;
    m_height = 1280.0f / VRAM_GetUIAspectRatio();
    m_cmdY = m_atY + 608.0f + 5.0f;
    m_freeHeight = fminf(m_height - m_cmdY, 150.0f);
    m_top = m_cmdY;
    if (m_logVisible)
        m_top = m_atY;
    m_delimHeight = m_height;
    if (!m_logVisible)
        m_delimHeight -= m_freeHeight;
    m_field_40 = m_height - 32.0f + m_atY;
}
CircularVector g_autoComplete;
CircularVector::CircularVector() { clear(); }
void CircularVector::clear() { m_vec.clear(); m_iter = m_vec.begin(); }
CircularVector::~CircularVector() { clear(); }
CircularVectorData *CircularVector::Current() const {
    if (m_vec.empty())
        return nullptr;
    if (m_iter != m_vec.end())
        return m_iter.operator->();
    zassert(m_iter != m_vec.end() && "Invalid invariant for CircularVector: iter == end()");
    return m_iter.operator->();
}
void CONSOLE_Paste(int cmdLen) {
    auto cs = glfwGetClipboardString(g_mainWindow);
    if (cs) {
        auto pDest = g_consoleCommand + cmdLen, pDestMax = g_consoleCommand + sizeof(g_consoleCommand) - 1;
        while (*cs >= ' ' && pDest < pDestMax) {
            if (*cs == '\x7f')
                break;
            *pDest++ = *cs++;
        }
        *pDest = 0;
    }
}
void CONSOLE_DefaultKey(int codePoint, int keyMods, unsigned int promptLen) {
    if (codePoint < 255) {
        if (keyMods & GLFW_MOD_CONTROL) {
            if (codePoint == 'C') {
                glfwSetClipboardString(g_mainWindow, g_consoleCommand);
            } else if (codePoint == 'V') {
                CONSOLE_Paste(promptLen);
            }
        } else {
            if (promptLen < 0x3FF) {
                g_consoleCommand[promptLen] = codePoint;
                g_consoleCommand[promptLen + 1] = 0;
            } else {
                zassert(0);
            }
            g_autoComplete.clear();
        }
    }
}
void CircularVectorData::toString(std::string *dest) {
    dest->reserve(m_descr.size() + 1 + m_params.size());
    dest->assign(m_descr);
    if (m_params.size()) {
        *dest += ' ';
        *dest += m_params;
    }
}
void CONSOLE_PrepareAutocompleteItem(ConsoleCommandFuncs *funcs, const char *params, bool isParams) {
    CMD_AutoCompleteParamSearchResults v30;
    if (funcs->m_string)
        v30.m_descr = funcs->m_string(params);
    if (funcs->m_ac_search) {
        funcs->m_ac_search(&v30, params);
        if (v30.m_field_38) {
            for (auto &pars : v30.m_field_20) {
                g_autoComplete.m_vec.push_back(CircularVectorData{ pars, pars, v30.m_descr });
                g_autoComplete.m_iter = g_autoComplete.m_vec.begin();
            }
            return;
        }
        if (isParams) {
            g_autoComplete.m_vec.push_back(CircularVectorData{ funcs->m_name, params, v30.m_descr });
            g_autoComplete.m_iter = g_autoComplete.m_vec.begin();
        }
        return;
    }
    if (funcs->m_static_str) {
        auto v27 = funcs->m_static_str(params);
        if (v27) {
            g_autoComplete.m_vec.push_back(CircularVectorData{ funcs->m_name, v27, v30.m_descr });
            g_autoComplete.m_iter = g_autoComplete.m_vec.begin();
            return;
        }
    }
    if (isParams) {
        g_autoComplete.m_vec.push_back(CircularVectorData{ funcs->m_name, params, v30.m_descr });
        g_autoComplete.m_iter = g_autoComplete.m_vec.begin();
    }
}
void CONSOLE_PrepareAutocomplete(const char *cmd) {
    if (cmd == nullptr || *cmd == 0)
        cmd = "help";
    const char *startCmd = cmd, *endCmd = startCmd + strlen(cmd);
    while (*startCmd && *startCmd == ' ') ++startCmd;
    while (endCmd >= startCmd && endCmd[-1] == ' ')
        --endCmd;
    std::string scmd(startCmd, endCmd), name, params;
    SplitCommand(scmd, &name, &params, ' ');
    if (name.size() || params.size()) {
        g_Console.m_logBanner.swap(scmd);
        g_autoComplete.clear();
        g_autoComplete.m_vec.push_back(CircularVectorData{ name, params, scmd });
        g_autoComplete.m_iter = g_autoComplete.m_vec.begin();
        std::vector<ConsoleCommandFuncs *> foundCommands;
        FindCommands(&foundCommands, name);
        for (auto j : foundCommands)
            CONSOLE_PrepareAutocompleteItem(j, params.c_str(), 1);
        if (!params.size()) {
            for (int k = 0; k < g_knownCommandsCounter; ++k)
                CONSOLE_PrepareAutocompleteItem(&g_knownCommands[k], name.c_str(), 0);
        }
    }
}
void CONSOLE_KeyPress(int codePoint, int keyModifiers) {
    auto cmdLen = (int)strlen(g_consoleCommand);
    int  scrollDelta = 0;
    switch (codePoint) {
        case GLFW_KEY_ESCAPE:
            if (g_autoComplete.m_vec.empty()) {
                g_consoleCommand[0] = 0;
            } else {
                g_Console.m_logBanner.clear();
                g_autoComplete.m_iter = g_autoComplete.m_vec.begin();
                auto v29 = g_autoComplete.CircularVector::Current();
                if (v29) {
                    std::string cts;
                    v29->toString(&cts);
                    strncpy(g_consoleCommand, cts.c_str(), sizeof(g_consoleCommand));
                }
            }
            break;
        case GLFW_KEY_ENTER:
            if (!cmdLen)
                return;
            g_alwaysScrollToEnd = true;
            COMMAND_RunCommand(g_consoleCommand);
            memmove(g_consoleCmdHistory[g_consoleCmdHistoryIdx % CONSOLE_CMDS_HISTORY], g_consoleCommand, cmdLen + 1);
            g_consoleCmdHistoryIdx++;
            g_consoleCommand[0] = 0;
            g_autoComplete.clear();
            return;
        case GLFW_KEY_TAB:
            if (g_autoComplete.m_vec.empty())
                CONSOLE_PrepareAutocomplete(g_consoleCommand);
            if (!g_autoComplete.m_vec.empty()) {
                if (keyModifiers & GLFW_MOD_SHIFT) {
                    if (g_autoComplete.m_iter == g_autoComplete.m_vec.begin())
                        g_autoComplete.m_iter = g_autoComplete.m_vec.end();
                    g_autoComplete.m_iter--;
                } else {
                    if (g_autoComplete.m_iter == g_autoComplete.m_vec.end())
                        g_autoComplete.m_iter = g_autoComplete.m_vec.begin();
                    g_autoComplete.m_iter++;
                    if (g_autoComplete.m_iter == g_autoComplete.m_vec.end())
                        g_autoComplete.m_iter = g_autoComplete.m_vec.begin();
                }
                auto v16 = g_autoComplete.Current();
                if (v16) {
                    v16->toString(&g_Console.m_logBanner);
                    strncpy(g_consoleCommand, v16->m_name.c_str(), sizeof(g_consoleCommand));
                    return;
                }
            }
            break;
        case GLFW_KEY_BACKSPACE:
            if (cmdLen) {
                g_consoleCommand[cmdLen - 1] = 0;
                g_autoComplete.clear();
            }
            return;
        case GLFW_KEY_INSERT:
            if (keyModifiers & GLFW_MOD_SHIFT)
                CONSOLE_Paste(cmdLen);
            break;
        case GLFW_KEY_F12:
            if (keyModifiers & GLFW_MOD_CONTROL)
                g_Console.m_logVisible = !g_Console.m_logVisible;
            break;
        //URSOFT ADDITIONS:
        case GLFW_KEY_UP:
            scrollDelta = -1;
            break;
        case GLFW_KEY_DOWN:
            scrollDelta = 1;
            break;
        case GLFW_KEY_PAGE_UP:
            scrollDelta = -LOGC_PAGE;
            break;
        case GLFW_KEY_PAGE_DOWN:
            scrollDelta = LOGC_PAGE;
            break;
        case GLFW_KEY_HOME:
            if (keyModifiers & GLFW_MOD_CONTROL)
                scrollDelta = -g_scrollLogPos;
            break;
        case GLFW_KEY_END:
            if (keyModifiers & GLFW_MOD_CONTROL)
                scrollDelta = LogGetLineCount();
            break;
        default:
            CONSOLE_DefaultKey(codePoint, keyModifiers, cmdLen);
    }
    if (scrollDelta) {
        int maxScroll = LogGetLineCount() - (LOGC_PAGE + 1);
        g_scrollLogPos = std::clamp(g_scrollLogPos + scrollDelta, 0, maxScroll);
        g_alwaysScrollToEnd = (g_scrollLogPos >= maxScroll);
    }
}
void CONSOLE_KeyFilter(uint32_t codePoint, int keyModifiers) {
    int v3 = GLFW_KEY_ENTER;
    if (codePoint != GLFW_KEY_KP_ENTER)
        v3 = codePoint;
    if (v3 == '`' && !g_bShowConsole && (keyModifiers & (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT)) == 0) { //URSOFT FIX
        g_bShowConsole = true;
        return;
    }
    if (g_bShowConsole) {
        if (v3 == '`')
            g_bShowConsole = false;
        else
            CONSOLE_KeyPress(v3, keyModifiers);
    } else {
        ZwiftAppKeyProcessorManager::Instance()->m_stack.ProcessKey(v3, keyModifiers);
    }
}
void CONSOLE_Draw(float atY, float dt) {
    int lc = std::min(LOGC_PAGE, LogGetLineCount());
    int scrollLogPos = g_scrollLogPos;
    if (g_alwaysScrollToEnd) {
        scrollLogPos = LogGetLineCount() - LOGC_PAGE - 1;
        if (scrollLogPos < 0)
            scrollLogPos = 0;
    }
    g_scrollLogPos = scrollLogPos;
    const char *cursor = "";
    g_blinkTime += dt;
    if (((int)(g_blinkTime + g_blinkTime) & 1) == 0)
        cursor = "_";
    char buf[sizeof(g_consoleCommand) + 4];
    sprintf(buf, "%s%s", g_consoleCommand, cursor);
    g_Console.Update(atY);
    GFX_Draw2DQuad(0.0f, g_Console.m_top, g_Console.m_width, g_Console.m_delimHeight, ConsoleRenderer::LogBGColor, true);
    if (g_Console.m_logVisible) {
        float lastLinePrinted = lc + scrollLogPos;
        for (int v6 = 0; lc > v6; --lc) {
            //sprintf(v11, "linenum = %d\n", lc);
            auto str = LogGetLine(lc + scrollLogPos);
            if (str)
                CONSOLE_DrawPar(g_Console, str, &v6, lc, LogGetLineType(lc + scrollLogPos));
        }
        float firstLinePrinted = lc + scrollLogPos;
        GFX_Draw2DQuad(0.0f, g_Console.m_cmdY, 1280.0f, 1.0f, (uint32_t)-1, false);
        //scrollBarTop, scrollBarHeight=0..g_Console.m_cmdY
        float scrollBarTop = firstLinePrinted / LogGetLineCount() * g_Console.m_cmdY, scrollBarHeight = (lastLinePrinted - firstLinePrinted + 1) / LogGetLineCount() * g_Console.m_cmdY;
        GFX_Draw2DQuad(1274.0f, scrollBarTop, 6.0f, scrollBarHeight, 0xFFFFFF00, false);
        GFX_Draw2DQuad(1276.0f, scrollBarTop + 2.0, 2.0f, scrollBarHeight - 4.0, g_alwaysScrollToEnd ? 0xFF0000FF : 0xFFFF0000, false);
    }
    CONSOLE_DrawCmdline(g_Console, buf);
    auto banner = "Zwift Debug Console (toggle with ` key and Ctrl+F12; <tab> to autocomplete)";
    if (!g_Console.m_logBanner.empty())
        banner = g_Console.m_logBanner.c_str();
    g_LargeFontW.RenderWString(15.0f, 0.0f, banner, 0xFFFFFF00, 0, 0.35f /*URSOFT FIX : was 0.4*/, true, false);
}
bool CMD_PlayWem(const char *) {
    //TODO
    return true;
}
char g_cmdPlayFileName[1024];
bool CMD_PlayWemLocal(const char *par) {
    char cd[MAX_PATH + 1];
    cd[0] = 0;
    if (GetCurrentDirectoryA(sizeof(cd) - 1, cd)) {
        sprintf(g_cmdPlayFileName, "%s/data/%s", cd, par);
        for (auto &s : g_cmdPlayFileName)
            if (!s) break; else if (s == '/') s = '\\';
        AUDIO_PlayFlatFile(g_cmdPlayFileName, 0.0f);
    }
    return true;
}
