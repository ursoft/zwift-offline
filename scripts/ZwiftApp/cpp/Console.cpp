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
    g_knownCommands[g_knownCommandsCounter++];
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
        if(*buf)
            COMMAND_RunCommand(buf);
    }
    fclose(f);
    return true;
}
void StripPaddedSpaces(std::string *dest, const std::string &src) {
    auto srcSize = src.size();
    if (srcSize) {
        int i = 0, j = srcSize - 1;
        for (; i < srcSize; ++i)
            if (src[i] != ' ')
                break;
        for (; j > i; --j)
            if (src[j] != ' ')
                break;
        dest->assign(src, i, j - i + 1);
    } else {
        dest->clear();
    }
}
void SplitCommand(const std::string &cmd, std::string *name, std::string *params, char delim) {
    int i = 0;
    for (; i < cmd.size(); ++i)
        if (cmd[i] == delim)
            break;
    name->assign(cmd, 0, i);
    if (i >= cmd.size())
        params->clear();
    else
        StripPaddedSpaces(params, std::string(cmd, i, cmd.size() - i));
}
bool findStringIC(const std::string &strHaystack, const std::string &strNeedle)
{
    auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(), strNeedle.end(),
        [](unsigned char ch1, unsigned char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
    );
    return (it != strHaystack.end());
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
    std::string scmd(cmd), name, params;
    SplitCommand(scmd, &name, &params, ' ');
    std::vector<ConsoleCommandFuncs *> foundCommands;
    FindCommands(&foundCommands, name);
    ConsoleCommandFuncs *selFunc = nullptr;
    if (foundCommands.size() == 1) {
        selFunc = foundCommands[0];
    } else if (foundCommands.size() > 1) {
        int bestDiff = -1;
        for (auto f : foundCommands) {
            auto diff = f->m_name.size() - name.size();
            if (diff < bestDiff) {
                bestDiff = diff;
                selFunc = f;
            }
        }
    }
    if (!selFunc || foundCommands.empty()) {
        CMD_Set(cmd);
        LogTyped(LOG_COMMAND_OUTPUT, "Unknown command \"%s\"", cmd);
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
bool CMD_ChangeShadowRes(const char *) {
    return true; //TODO
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
        LogTyped(LOG_COMMAND_OUTPUT, "%s", g_knownCommands[i].m_name);
    return true;
}
bool CMD_ToggleLog(const char *) {
    g_Console.m_visible = !g_Console.m_visible;
    return true;
}
bool CMD_Set(const char *) {
    return true; //TODO
}
CMD_AutoCompleteParamSearchResults CMD_Set3(const char *) {
    return TODO; //TODO
}
std::string CMD_Set4(const char *) {
    return ""; //TODO
}
bool CMD_ListVars(const char *) {
    return true; //TODO
}
std::string CMD_ListVars4(const char *) {
    return ""; //TODO
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
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS() == 0;
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
}
bool CMD_SetLanguage(const char *lang) {
    auto l = LOC_GetLanguageFromString(lang);
    if (l == LOC_CNT)
        return false;
    LOC_SetLanguageFromEnum(l, true);
    return true;
}
