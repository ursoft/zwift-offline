#include "ZwiftApp.h" //READY for testing
LOG_TYPE   g_LogLineTypes[LOGC_LINES];
char       *g_LogLines[LOGC_LINES];
LOG_LEVEL  g_MinLogLevel = LL_CNT; // NOT_SET_YET;
FILE       *g_logFile /*, *g_fpCalibrationLog not used*/;
LOG_LEVEL  g_noesisLogLevels[NLL_CNT] = { LL_DEBUG /*NLL_TRACE*/, LL_DEBUG, LL_INFO, LL_WARNING, LL_ERROR };
int        g_LogMutexIdx = -1;
bool       g_canUseLogging;
const char *g_logLevelNames[LL_CNT] = { "", "DEBUG LEVEL: ", "INFO LEVEL: ", "WARNING LEVEL: ", "ERROR LEVEL: ", "FATAL LEVEL: " };
const char *g_logTypeNames[LOG_CNT] = { "", "WARN : ", "ERROR: ", "> ", "", "FILE : ", "NETWORK:", "NETCLIENT:", "ANT  : ", "ANT  : ", "BLE  : ", "STEERING : ",
                                        "VIDEO_CAPTURE : ", "BRAKING: ", "AUTOBRAKING: ", "AUTOMATION: " };
struct LogType { //40 bytes
    std::string m_name;
    bool        m_enabled, m_bool2 /* QUEST: find where used */;
};
LogType g_LogTypes[LOG_CNT] = {
    { "LOG_GENERAL", true, true },            //00
    { "LOG_WARNING", true, true },            //01
    { "LOG_ERROR", true, true },              //02
    { "LOG_COMMAND", true, true },            //03
    { "LOG_COMMAND_OUTPUT", true, true },     //04
    { "LOG_ASSET", true, false },             //05
    { "LOG_NETWORK", true, true },            //06
    { "LOG_ZNETWORK_INTERNAL", true, false }, //07
    { "LOG_ANT", true, false },               //08
    { "LOG_ANT_IMPORTANT", true, true },      //09
    { "LOG_BLE", true, true },                //10
    { "LOG_STEERING", true, false },          //11
    { "LOG_VIDEO_CAPTURE", true, false },     //12
    { "LOG_BRAKING", true, false },           //13
    { "LOG_AUTOBRAKING", true, false },       //14
    { "LOG_AUTOMATION", true, false },        //15
};
std::vector<std::string> ParseSuppressedLogs(const char *ls) {
    return ZStringUtil::Split(ls, ',');
}
void LogSetSuppressedLogTypes(const std::vector<std::string> &supprLogs) {
    for (const auto &sup : supprLogs) {
        int logId = -1;
        if (1 != sscanf(sup.c_str(), "%d", &logId)) {
            for (int cnt = 0; cnt < LOG_CNT; cnt++) {
                if (g_LogTypes[cnt].m_name == sup) {
                    logId = cnt;
                    break;
                }
            }
        }
        if (logId >= 0 && logId < LOG_CNT)
            g_LogTypes[logId].m_enabled = false;
    }
}
void LoadLogLevelSettings() {
    g_MinLogLevel = LL_VERBOSE;
    if (g_UserConfigDoc.m_loadResult) {
        auto set = g_UserConfigDoc.GetU32("ZWIFT\\GAME_LOG_LEVEL", LL_VERBOSE);
        if (set >= LL_CNT) // QUEST: was > 6, unknown level
            set = LL_VERBOSE;
        g_MinLogLevel = (LOG_LEVEL)set;
    }
}
bool ShouldLog(LOG_LEVEL level) {
    if (g_MinLogLevel == LL_CNT /*NOT_SET_YET*/)
        LoadLogLevelSettings();
    return level >= g_MinLogLevel;
}
LogWriteHandler g_logWriteHandler;
void SetLogWriteHandler(LogWriteHandler h) { g_logWriteHandler = h; }

#define LOG_DEBUG
void LogWrite(LOG_LEVEL level, LOG_TYPE ty, const char *msg, size_t msg_len) {
    if (g_canUseLogging) {
        if (g_LogMutexIdx != -1 && g_LogTypes[ty].m_enabled && ZMUTEX_TryLock(g_LogMutexIdx)) {
            __time64_t now = _time64(nullptr);
            tm         t;
            _localtime64_s(&t, &now);
            char       buf[1024];
            int        cnt = sprintf_s(buf, "[%d:%02d:%02d] ", t.tm_hour, t.tm_min, t.tm_sec);
            if (g_logFile) {
                if (cnt > 0) fwrite(buf, cnt, 1, g_logFile);
                fwrite(msg, msg_len, 1, g_logFile);
#ifdef LOG_DEBUG
                OutputDebugStringA(msg); OutputDebugStringA("\n");
#endif
                fwrite("\r\n", 2, 1, g_logFile);
                fflush(g_logFile);
            }
            auto dest = g_LogLines[g_nLogLines % LOGC_LINES], destMax = dest + LOGC_LINE_BUF - 1;
            auto src = msg, srcMax = src + msg_len;
            strncpy(dest, buf, cnt);
            dest += cnt;
            //URSOFT fix: if there are non-ASCII symbols (1251) -> transliterate them (need new fonts though)
            while (dest < destMax && src < srcMax) {
                auto ch = *src++;
                switch (ch) {
                    default: *dest++ = ch; break;
                    case '�': *dest++ = 'J'; if (dest < destMax) *dest++ = 'O'; break;
                    case '�': *dest++ = 'j'; if (dest < destMax) *dest++ = 'o'; break;
                    case '�': *dest++ = 'J'; break; case '�': *dest++ = 'j'; break;
                    case '�': *dest++ = 'C'; break; case '�': *dest++ = 'c'; break;
                    case '�': *dest++ = 'U'; break; case '�': *dest++ = 'u'; break;
                    case '�': *dest++ = 'K'; break; case '�': *dest++ = 'k'; break;
                    case '�': *dest++ = 'E'; break; case '�': *dest++ = 'e'; break;
                    case '�': *dest++ = 'N'; break; case '�': *dest++ = 'n'; break;
                    case '�': *dest++ = 'G'; break; case '�': *dest++ = 'g'; break;
                    case '�': *dest++ = 'S'; if (dest < destMax) *dest++ = 'H'; break;
                    case '�': *dest++ = 's'; if (dest < destMax) *dest++ = 'h'; break;
                    case '�': *dest++ = 'S'; if (dest < destMax) *dest++ = 'C'; break;
                    case '�': *dest++ = 's'; if (dest < destMax) *dest++ = 'c'; break;
                    case '�': *dest++ = 'Z'; break; case '�': *dest++ = 'z'; break;
                    case '�': *dest++ = 'K'; if (dest < destMax) *dest++ = 'H'; break;
                    case '�': *dest++ = 'k'; if (dest < destMax) *dest++ = 'h'; break;
                    case '�': *dest++ = 'F'; break; case '�': *dest++ = 'f'; break;
                    case '�': *dest++ = 'Y'; break; case '�': *dest++ = 'y'; break;
                    case '�': *dest++ = 'V'; break; case '�': *dest++ = 'v'; break;
                    case '�': *dest++ = 'A'; break; case '�': *dest++ = 'a'; break;
                    case '�': *dest++ = 'P'; break; case '�': *dest++ = 'p'; break;
                    case '�': *dest++ = 'R'; break; case '�': *dest++ = 'r'; break;
                    case '�': *dest++ = 'O'; break; case '�': *dest++ = 'o'; break;
                    case '�': *dest++ = 'L'; break; case '�': *dest++ = 'l'; break;
                    case '�': *dest++ = 'D'; break; case '�': *dest++ = 'd'; break;
                    case '�': *dest++ = 'Z'; if (dest < destMax) *dest++ = 'H'; break;
                    case '�': *dest++ = 'z'; if (dest < destMax) *dest++ = 'h'; break;
                    case '�': *dest++ = 'E'; break; case '�': *dest++ = 'e'; break;
                    case '�': *dest++ = 'J'; if (dest < destMax) *dest++ = 'A'; break;
                    case '�': *dest++ = 'j'; if (dest < destMax) *dest++ = 'a'; break;
                    case '�': *dest++ = 'C'; if (dest < destMax) *dest++ = 'H'; break;
                    case '�': *dest++ = 'c'; if (dest < destMax) *dest++ = 'h'; break;
                    case '�': *dest++ = 'S'; break; case '�': *dest++ = 's'; break;
                    case '�': *dest++ = 'M'; break; case '�': *dest++ = 'm'; break;
                    case '�': *dest++ = 'I'; break; case '�': *dest++ = 'i'; break;
                    case '�': *dest++ = 'T'; break; case '�': *dest++ = 't'; break;
                    case '�': *dest++ = '\''; break; case '�': *dest++ = '\''; break;
                    case '�': *dest++ = 'B'; break; case '�': *dest++ = 'b'; break;
                    case '�': *dest++ = 'J'; if (dest < destMax) *dest++ = 'U'; break;
                    case '�': *dest++ = 'j'; if (dest < destMax) *dest++ = 'u'; break;
                }
            }
            *dest = 0;
            g_LogLineTypes[g_nLogLines % LOGC_LINES] = ty;
            g_nLogLines++;
            ZMUTEX_Unlock(g_LogMutexIdx);
            if (g_logWriteHandler)
                g_logWriteHandler(level, ty, msg);
        }
    }
}
void doLogInternal(LOG_LEVEL level, LOG_TYPE ty, const char *fmt, va_list args) {
    char buf[16384];
    if (g_canUseLogging && g_LogMutexIdx != -1 && g_LogTypes[ty].m_enabled) {
        int cnt1 = sprintf_s(buf, "%s%s", g_logLevelNames[level], g_logTypeNames[ty]);
        if (cnt1 < 0) cnt1 = 0;
        int cnt2 = vsnprintf_s(&buf[cnt1], 1024 - cnt1, _TRUNCATE, fmt, args);
        if (cnt2 >= 0)
            LogWrite(level, ty, buf, cnt1 + cnt2);
    }
}
void LogInternal(LOG_TYPE ty, const char *fmt, va_list list) { doLogInternal(LL_VERBOSE, ty, fmt, list); }
void LogTyped(LOG_TYPE type, const char *fmt, ...) {
    if (!ShouldLog(LL_DEBUG)) return;
    va_list va;
    va_start(va, fmt);
    LogInternal(type, fmt, va);
}
void Log(const char *fmt, ...) {
    if (!ShouldLog(LL_DEBUG)) return;
    va_list va;
    va_start(va, fmt);
    LogInternal(LOG_GENERAL, fmt, va);
}
//LogDebug, LogNoesis stored as pointers somewhere
void LogDebug(const char *fmt, ...) {
    if (!ShouldLog(LL_DEBUG)) return;
    va_list va;
    va_start(va, fmt);
    doLogInternal(LL_DEBUG, LOG_GENERAL, fmt, va);
}
void LogNoesis(void *dummy_a1, void *dummy_a2, NoesisLogLevel noesisLevel, void *dummy_a4, const char *msg) {
    LOG_LEVEL l = LL_ERROR;
    if (noesisLevel < NLL_CNT)
        l = g_noesisLogLevels[noesisLevel];
    LogLev(l, "[NOESIS] %s", msg ? msg : "(null)");
}
void glfwZwiftErrorCallback(int code, const char *msg) {
    Log("GLFW ERROR %d: %s", code, msg);
}
void LogLev(LOG_LEVEL level, const char *fmt, ...) {
    if (ShouldLog(level)) {
        va_list va;
        va_start(va, fmt);
        doLogInternal(level, LOG_GENERAL, fmt, va);
    }
}
bool GameAssertHandler::BeforeAbort(const char *cond, const char *file, unsigned line, PVOID *BackTrace, int nframes) {
    //IMPROVE �� ���������� ������� ������ ����� ����� ���
    Log("ASSERT: \"%s\", file=%s, line=%d", cond, file, line);
    //CrashReporting_stuffAbort(cond, file, (unsigned int)line, BackTrace, nframes);
    return !GameAssertHandler::s_disableAbort;
}
void GameAssertHandler::Shutdown() { ZwiftAssert::SetHandler(nullptr); }
void GameAssertHandler::Abort() {
    if (!GameAssertHandler::s_disableAbort)
        ZwiftExit(-1);
}
void GameAssertHandler::Initialize() {
    Experimentation::Instance()->IsEnabled(FID_ASSERTD, [](ExpVariant val) {
        if (val == EXP_ENABLED) {
            GameAssertHandler::s_disableAbort = 1;
            Log("Experiment service disabled assert abort");
        }
    });
    static GameAssertHandler s_instance;
    ZwiftAssert::SetHandler(&s_instance);
    GameAssertHandler::DisableAbort();
}
bool ZwiftAssert::BeforeAbort(const char *cond, const char *file, unsigned line) {
    if (g_abortProcessing) return false;
    bool ret = true;
    g_abortProcessing = true;
    g_abortMutex.lock();
    if (g_abortListener) {
        PVOID BackTrace[32] = {};
        auto  frames = RtlCaptureStackBackTrace(1u, sizeof(BackTrace) / sizeof(PVOID), BackTrace, nullptr);
        ret = g_abortListener->BeforeAbort(cond, file, line, BackTrace, frames);
        if (!ret) {
            g_abortMutex.unlock();
            g_abortProcessing = false;
        }
    }
    return ret;
}
void ZwiftAssert::Abort() {
    if (g_abortProcessing) {
        if (g_abortListener)
            g_abortListener->Abort();
        g_abortMutex.unlock();
        g_abortProcessing = false;
    }
}
void SaveLog() {
    if (ZMUTEX_TryLock(g_LogMutexIdx)) {
        if (g_logFile) {
            fclose(g_logFile);
            g_logFile = nullptr;
        }
        /* not used if (g_fpCalibrationLog) {
            fclose(g_fpCalibrationLog);
            g_fpCalibrationLog = nullptr;
        }*/
        for (auto &i : g_LogLines) {
            if (i) {
                free(i);
                i = nullptr;
            }
        }
        ZMUTEX_Unlock(g_LogMutexIdx);
    }
}
void LogInitialize() {
    if (g_logFile) {
        fclose(g_logFile);
        g_logFile = nullptr;
    }
    char PathName[MAX_PATH]{ '.' }, FileName[MAX_PATH], v18[MAX_PATH], Buffer[MAX_PATH];
    auto userPath = OS_GetUserPath();
    if (userPath) {
        sprintf_s(PathName, "%s\\Zwift", userPath);
        CreateDirectoryA(PathName, nullptr);
        sprintf_s(PathName, "%s\\Logs", PathName);
        CreateDirectoryA(PathName, nullptr);
        for (auto &i : g_LogLines) {
            if (!i)
                i = (char *)malloc(LOGC_LINE_BUF);
            i[0] = 0;
        }
        g_canUseLogging = true;
        //memset(g_LogLineTypes, 0, sizeof(g_LogLineTypes)); //not need for static data
        g_LogMutexIdx = ZMUTEX_Create("log");
        strcpy(v18, PathName);
        sprintf(PathName, "%s/%s.txt", PathName, "Log");
        strcpy(Buffer, v18);
        for (int j = 9; j > 0; --j) {
            sprintf_s(Buffer, "%s/%s (old %d).txt", v18, "Log", j);
            sprintf_s(FileName, "%s/%s (old %d).txt", v18, "Log", j + 1);
            rename(Buffer, FileName);
        }
        sprintf_s(FileName, "%s/%s (old %d).txt", v18, "Log", 10);
        DeleteFileA(FileName);
        rename(PathName, Buffer);
        g_logFile = fopen(PathName, "w");
        auto Time = _time64(nullptr);
        auto lt = _localtime64(&Time);
        Log("Log Time: %d:%02d:%02d %d-%02d-%02d", lt->tm_hour, lt->tm_min, lt->tm_sec, lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
        Log("Game Version: 1.32.1(106405) Ursoft fake/1.32.1");
        Log("Config:       Shipping");
        Log("Device:       PC");
        //OMIT tHigFile::SetLogHandler(LogDebug);
    }
}
int LogGetLineCount() { return std::min(LOGC_LINES, g_nLogLines); }
LOG_TYPE LogGetLineType(int a1) { return LOG_TYPE(g_LogLineTypes[(a1 + g_curLogLine) % LOGC_LINES] % LOG_CNT); }
const char *LogGetLine(int a1) { return g_LogLines[(a1 + g_curLogLine) % LOGC_LINES]; }
