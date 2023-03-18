#include "ZwiftApp.h"
const int LOGC_LINE_BUF = 0x400, LOGC_LINES = 1000, LOGC_PAGE = 37;
int g_nLogLines, g_curLogLine, g_scrollLogPos;
LOG_TYPE g_LogLineTypes[LOGC_LINES];
bool g_overflowScroll;
char *g_LogLines[LOGC_LINES];
LOG_LEVEL g_MinLogLevel = LL_CNT; // NOT_SET_YET;
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
void LogSetSuppressedLogTypes(const std::vector<std::string> &supprLogs) {
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
    g_MinLogLevel = LL_VERBOSE;
    if (g_useLogLevelSettings) {
        auto set = g_UserConfigDoc.GetU32("ZWIFT\\GAME_LOG_LEVEL", LL_VERBOSE, true);
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
        int cnt2 = vsnprintf_s(&buf[cnt1], 1024 - cnt1, _TRUNCATE, fmt, args);
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
    //IMPROVE не записывать падение одного места много раз
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
        if(val == EXP_ENABLED) {
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
        auto frames = RtlCaptureStackBackTrace(1u, sizeof(BackTrace)/sizeof(PVOID), BackTrace, nullptr);
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
void LogInitialize() {
    char PathName[MAX_PATH]{'.'}, FileName[MAX_PATH], v18[MAX_PATH], Buffer[MAX_PATH];
    auto userPath = OS_GetUserPath();
    if (userPath) {
        sprintf_s(PathName, "%s\\Zwift", userPath);
        CreateDirectoryA(PathName, nullptr);
        sprintf_s(PathName, "%s\\Logs", PathName);
        CreateDirectoryA(PathName, nullptr);
        for (auto &i : g_LogLines) {
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
        tHigFile::SetLogHandler(LogDebug);
    }
}
int LogGetLineCount() { return std::min(LOGC_LINES, g_nLogLines); }
LOG_TYPE LogGetLineType(int a1) { return LOG_TYPE(g_LogLineTypes[(a1 + g_curLogLine) % LOGC_LINES] % LOG_CNT); }
const char *LogGetLine(int a1) { return g_LogLines[(a1 + g_curLogLine) % LOGC_LINES]; }
void CONSOLE_DrawCmdline(const ConsoleRenderer &cr, const char *line) {
    g_LargeFontW.RenderWString(cr.m_cmdX1, cr.m_cmdY, ">", 0xAA00CCFF, 0, cr.m_cmdScale, true, false);
    g_LargeFontW.RenderWString(cr.m_cmdX2, cr.m_cmdY, line, 0xAA00CCFF, 0, cr.m_cmdScale, true, false);
}
void CONSOLE_DrawPar(const ConsoleRenderer &cr, const char *str, int *lineNo, int lineCount, LOG_TYPE lineType) {
    auto ustr = ToUTF8_ib(str);
    auto mea = g_LargeFontW.GetParagraphLineCountW(cr.m_width, ustr, 0.4f, 0.0f, false);
    int lines = g_LargeFontW.RenderParagraphW(15.0f, 16.0f * (lineCount - mea + 1) + cr.m_atY - 16.0f * (*lineNo),
        cr.m_width, cr.m_height, ustr, ConsoleRenderer::TYPE_COLORS[lineType],
        0, 0.4f, 1, 1.0f, 0.0f);
    if (lines > 1)
        *lineNo += lines - 1;
}
void ScrollLog(int dir) {
    int scrollLogPos;
    int maxScroll = LogGetLineCount() - (LOGC_PAGE - 1);
    if (dir <= 0) {
        scrollLogPos = std::clamp(g_scrollLogPos + 1, 0, maxScroll);
        g_overflowScroll = (g_scrollLogPos >= maxScroll);
    } else {
        scrollLogPos = std::clamp((int)g_scrollLogPos - 1, 0, maxScroll);
        g_overflowScroll = false;
    }
    g_scrollLogPos = scrollLogPos;
}
float g_blinkTime;
char g_consolePrompt[1024];
void ConsoleRenderer::Update(float atY) {
    if (m_mirrorY)
        atY = -atY;
    m_atY = atY;
    m_height = 1280.0f / VRAM_GetUIAspectRatio();
    m_cmdY = m_atY + 608.0f + 5.0f;
    m_freeHeight = fminf(m_height - m_cmdY, 150.0f);
    m_top = m_cmdY;
    if (m_visible)
        m_top = m_atY;
    m_delimHeight = m_height;
    if (!m_visible)
        m_delimHeight -= m_freeHeight;
    m_field_40 = m_height - 32.0f + m_atY;
}
void CONSOLE_Draw(float atY, float dt) {
    int lc = std::min(LOGC_PAGE, LogGetLineCount());
    if (g_overflowScroll) {
        g_scrollLogPos = LogGetLineCount() - LOGC_PAGE;
        if (g_scrollLogPos < 0)
            g_scrollLogPos = 0;
    }
    const char *cursor = "";
    g_blinkTime += dt;
    if (((int)(g_blinkTime + g_blinkTime) & 1) == 0)
        cursor = "_";
    char buf[4];
    sprintf(buf, "%s%s", g_consolePrompt, cursor);
    g_Console.Update(atY);
    GFX_Draw2DQuad(0.0f, g_Console.m_top, g_Console.m_width, g_Console.m_delimHeight, ConsoleRenderer::LogBGColor, true);
    if (g_Console.m_visible) {
        for(int v6 = 0; lc > v6; --lc) {
            //sprintf(v11, "linenum = %d\n", lc);
            auto str = LogGetLine(lc + g_scrollLogPos);
            if (str)
                CONSOLE_DrawPar(g_Console, str, &v6, lc, LogGetLineType(lc + g_scrollLogPos));
        }
        if (g_Console.m_visible)
            GFX_Draw2DQuad(0.0f, 0.0f, 1280.0f, 1.0f, -1, false);
    }
    CONSOLE_DrawCmdline(g_Console, buf);
    if (!g_Console.m_logBanner.empty())
        g_LargeFontW.RenderWString(15.0f, 0.0f, g_Console.m_logBanner.c_str(), 0xFFFFFF00, 0, 0.4f, true, false);
}