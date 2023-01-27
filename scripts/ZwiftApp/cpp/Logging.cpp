#include "ZwiftApp.h"
#include <ctime>

LOG_LEVEL g_MinLogLevel = LL_NOT_SET_YET;
bool g_useLogLevelSettings;
FILE *g_logFile;
LOG_LEVEL g_noesisLogLevels[5] = { LL_DEBUG, LL_DEBUG, LL_INFO, LL_WARNING, LL_ERROR };
int g_LogMutexIdx = -1;
bool g_canUseLogging;
const char *g_logLevelNames[] = { "", "DEBUG LEVEL: ", "INFO LEVEL: ", "WARNING LEVEL: ", "ERROR LEVEL: ", "FATAL LEVEL: " /*TODO: SetupLogTypes*/};
const char *g_logTypeNames[] = { "", "WARN : ", "ERROR: ", "> ", "", "FILE : ", "NETWORK:", "NETCLIENT:", "ANT  : ", "ANT  : ", "BLE  : ", "STEERING : ",
    "VIDEO_CAPTURE : ", "BRAKING: ", "AUTOBRAKING: ", "AUTOMATION: " };

void LoadLogLevelSettings() {
    if (g_useLogLevelSettings) {
#if 0 //TODO
    v1 = XMLDoc::SetCStr((__int64)&g_UserConfigDoc, "ZWIFT\\GAME_LOG_LEVEL", 0);
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
    if (g_MinLogLevel == LL_NOT_SET_YET)
        LoadLogLevelSettings();
    return level >= g_MinLogLevel;
}
void execLogInternal(LOG_LEVEL level, LOG_TYPE ty, const char *msg, size_t msg_len) {
    if (g_canUseLogging) {
        if (g_LogMutexIdx != -1 && /*TODO g_typedLogMetafata[40 * ty] &&*/ ZwiftEnterCriticalSection(g_LogMutexIdx)) {
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
            //TODO: notification
            //if (qword_7FF6D5DF6328)
            //    qword_7FF6D5DF6328(level, ty, msg);
        }
    }
}
void doLogInternal(LOG_LEVEL level, LOG_TYPE ty, const char *fmt, va_list args) {
    char buf[1024] = {};
    if (g_canUseLogging && g_LogMutexIdx != -1 /*TODO && g_typedLogMetafata[40 * ty]*/) {
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
    LogInternal(LT_NONE, fmt, va);
}
//LogDebug, LogAnt, LogNoesis stored as pointers somewhere
void LogDebug(const char *fmt, ...) {
    if (!ShouldLog(LL_DEBUG)) return;
    va_list va;
    va_start(va, fmt);
    doLogInternal(LL_DEBUG, LT_NONE, fmt, va);
}
void LogAnt(const char *msg) {
    if (msg && ShouldLog(LL_DEBUG))
        execLogInternal(LL_DEBUG, LT_ANT1, msg, strnlen_s(msg, 16384));
}
void LogNoesis(void *dummy_a1, void *dummy_a2, NoesisLogLevel noesisLevel, void *dummy_a4, const char *msg) {
    LOG_LEVEL l = LL_ERROR;
    if (noesisLevel < NoesisLogLevels_5)
        l = g_noesisLogLevels[noesisLevel];
    LogLev(l, "[NOESIS] %s", msg ? msg : "(null)");
}
//TODO: one more xref from Noesis
void LogLev(LOG_LEVEL level, const char *fmt, ...) {
    if (ShouldLog(level)) {
        va_list va;
        va_start(va, fmt);
        doLogInternal(level, LT_NONE, fmt, va);
    }
}

bool ZwiftBeforeAbort(const char *cond, const char *file, unsigned line, char a4) {
    //TODO
    return true;
}
void ZwiftAssert_Abort() {
    //TODO
}
