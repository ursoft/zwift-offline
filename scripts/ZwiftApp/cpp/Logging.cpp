#include "ZwiftApp.h"

LOG_LEVEL g_MinLogLevel = LL_CNT; // NOT_SET_YET;
bool g_useLogLevelSettings;
FILE *g_logFile;
LOG_LEVEL g_noesisLogLevels[NLL_CNT] = { LL_DEBUG /*NLL_TRACE*/, LL_DEBUG, LL_INFO, LL_WARNING, LL_ERROR};
int g_LogMutexIdx = -1;
bool g_canUseLogging;
const char *g_logLevelNames[LL_CNT] = { "", "DEBUG LEVEL: ", "INFO LEVEL: ", "WARNING LEVEL: ", "ERROR LEVEL: ", "FATAL LEVEL: " };
const char *g_logTypeNames[LOG_CNT] = { "", "WARN : ", "ERROR: ", "> ", "", "FILE : ", "NETWORK:", "NETCLIENT:", "ANT  : ", "ANT  : ", "BLE  : ", "STEERING : ",
    "VIDEO_CAPTURE : ", "BRAKING: ", "AUTOBRAKING: ", "AUTOMATION: " };
struct LogType { //40 bytes
    std::string m_name;
    bool m_enabled, m_bool2 /* TODO: find where used */;
};
LogType g_LogTypes[LOG_CNT] = {
    { "LOG_GENERAL",            true, true },  //00
    { "LOG_WARNING",            true, true },  //01
    { "LOG_ERROR",              true, true },  //02
    { "LOG_COMMAND",            true, true },  //03
    { "LOG_COMMAND_OUTPUT",     true, true },  //04
    { "LOG_ASSET",              true, false }, //05
    { "LOG_NETWORK",            true, true },  //06
    { "LOG_ZNETWORK_INTERNAL",  true, false }, //07
    { "LOG_ANT",                true, false }, //08
    { "LOG_ANT_IMPORTANT",      true, true },  //09
    { "LOG_BLE",                true, true },  //10
    { "LOG_STEERING",           true, false }, //11
    { "LOG_VIDEO_CAPTURE",      true, false }, //12
    { "LOG_BRAKING",            true, false }, //13
    { "LOG_AUTOBRAKING",        true, false }, //14
    { "LOG_AUTOMATION",         true, false }, //15
};

std::vector<std::string> ParseSuppressedLogs(const char *ls) {
    return ZStringUtil::Split(ls, ',');
}
void InitLogging(const std::vector<std::string> &supprLogs) {
    for (const auto &sup : supprLogs) {
        int logId = -1;
        if (1 != sscanf(sup.c_str(), "%d", &logId)) {
            int cnt = 0;
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
    if (g_useLogLevelSettings) {
        auto gll = g_UserConfigDoc.FindElement("ZWIFT\\GAME_LOG_LEVEL", false);
#if 0 //TODO
    v2 = v1;
    if (v1
        && ((v3 = *(_QWORD *)(v1 + 48)) == 0 || !(*(__int64(__fastcall **)(__int64))(*(_QWORD *)v3 + 16i64))(v3)
            ? (v5 = 0i64)
            : (v4 = (*(__int64(__fastcall **)(_QWORD))(**(_QWORD **)(v2 + 48) + 16i64))(*(_QWORD *)(v2 + 48)),
                v5 = (char *)sub_7FF6D4AB1F10(v4 + 24)),
            (unsigned int)sub_7FF6D485AF40(v5, "%d") == 1))
    {
        v6 = v8;
        if (v8 > 6)
        {
            result = 0;
            g_MinLogLevel = LL_VERBOSE;
            return;
        }
    }
    else
    {
        v6 = LL_VERBOSE;
    }
    v0 = v6;
    g_MinLogLevel = v6;
    return;
#endif
    }
    g_MinLogLevel = LL_VERBOSE;
}
bool ShouldLog(LOG_LEVEL level) {
    if (g_MinLogLevel == LL_CNT /*NOT_SET_YET*/)
        LoadLogLevelSettings();
    return level >= g_MinLogLevel;
}
LogWriteHandler g_logWriteHandler;
void SetLogWriteHandler(LogWriteHandler h) { g_logWriteHandler = h; }

void execLogInternal(LOG_LEVEL level, LOG_TYPE ty, const char *msg, size_t msg_len) {
    if (g_canUseLogging) {
        if (g_LogMutexIdx != -1 && g_LogTypes[ty].m_enabled && ZwiftEnterCriticalSection(g_LogMutexIdx)) {
            __time64_t now = _time64(nullptr);
            tm t;
            _localtime64_s(&t, &now);
            if (g_logFile) {
                char buf[1024];
                int cnt = sprintf_s(buf, "[%d:%02d:%02d] ", t.tm_hour, t.tm_min, t.tm_sec);
                if (cnt > 0) fwrite(buf, cnt, 1, g_logFile);
                fwrite(msg, msg_len, 1, g_logFile);
                fwrite("\r\n", 2, 1, g_logFile);
                fflush(g_logFile);
            }
            ZwiftLeaveCriticalSection(g_LogMutexIdx);
            if (g_logWriteHandler)
                g_logWriteHandler(level, ty, msg);
        }
    }
}
void doLogInternal(LOG_LEVEL level, LOG_TYPE ty, const char *fmt, va_list args) {
    char buf[1024];
    if (g_canUseLogging && g_LogMutexIdx != -1 && g_LogTypes[ty].m_enabled) {
        int cnt1 = sprintf_s(buf, "%s%s", g_logLevelNames[level], g_logTypeNames[ty]);
        if (cnt1 < 0) cnt1 = 0;
        int cnt2 = sprintf_s(&buf[cnt1], 1024 - cnt1, fmt, 0, args);
        if (cnt2 >= 0)
            execLogInternal(level, ty, buf, cnt1 + cnt2);
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
//LogDebug, LogNetInt, LogNoesis stored as pointers somewhere
void LogDebug(const char *fmt, ...) {
    if (!ShouldLog(LL_DEBUG)) return;
    va_list va;
    va_start(va, fmt);
    doLogInternal(LL_DEBUG, LOG_GENERAL, fmt, va);
}
void LogNetInt(const char *msg) {
    if (msg && ShouldLog(LL_DEBUG))
        execLogInternal(LL_DEBUG, LOG_ZNETWORK_INTERNAL, msg, strnlen_s(msg, 16384));
}
void LogNoesis(void *dummy_a1, void *dummy_a2, NoesisLogLevel noesisLevel, void *dummy_a4, const char *msg) {
    LOG_LEVEL l = LL_ERROR;
    if (noesisLevel < NLL_CNT)
        l = g_noesisLogLevels[noesisLevel];
    LogLev(l, "[NOESIS] %s", msg ? msg : "(null)");
}
//TODO: one more xref from Noesis
void LogLev(LOG_LEVEL level, const char *fmt, ...) {
    if (ShouldLog(level)) {
        va_list va;
        va_start(va, fmt);
        doLogInternal(level, LOG_GENERAL, fmt, va);
    }
}
bool GameAssertHandler::OnBeforeAbort(const char *cond, const char *file, unsigned line, PVOID *BackTrace, int nframes) {
    //IMPROVE не записывать падение одного места много раз
    Log("ASSERT: \"%s\", file=%s, line=%d", cond, file, line);
    //CrashReporting_stuffAbort(cond, file, (unsigned int)line, BackTrace, nframes);
    return !GameAssertHandler::s_disableAbort;
}
void GameAssertHandler::OnAbort() {
    if (!GameAssertHandler::s_disableAbort)
        ZwiftExit(-1);
}
bool GameAssertHandler::s_disableAbort;

thread_local bool g_abortProcessing;
std::mutex g_abortMutex;
GameAssertHandler *g_abortListener;

void ZwiftAssert_SetHandler(GameAssertHandler *ptr) {
    if (!g_abortProcessing)
        g_abortListener = ptr;
}

bool ZwiftBeforeAbort(const char *cond, const char *file, unsigned line) {
    if (g_abortProcessing) return false;
    bool ret = true;
    g_abortProcessing = true;
    g_abortMutex.lock();
    if (g_abortListener) {
        PVOID BackTrace[32] = {};
        auto frames = RtlCaptureStackBackTrace(1u, sizeof(BackTrace)/sizeof(PVOID), BackTrace, nullptr);
        ret = g_abortListener->OnBeforeAbort(cond, file, line, BackTrace, frames);
        if (!ret) {
            g_abortMutex.unlock();
            g_abortProcessing = false;
        }
    }
    return ret;
}
void ZwiftAssert_Abort() {
    if (g_abortProcessing) {
        if (g_abortListener)
            g_abortListener->OnAbort();
        g_abortMutex.unlock();
        g_abortProcessing = false;
    }
}
