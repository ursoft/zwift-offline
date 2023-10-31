#pragma once //READY for testing
inline const int LOGC_LINE_BUF = 0x400, LOGC_LINES = 1000;
enum LOG_TYPE {
    LOG_GENERAL = 0, LOG_WARNING = 1, LOG_ERROR = 2, LOG_COMMAND = 3, LOG_COMMAND_OUTPUT = 4, LOG_ASSET = 5, LOG_NETWORK = 6, LOG_ZNETWORK_INTERNAL = 7, LOG_ANT = 8, LOG_ANT_IMPORTANT = 9, LOG_BLE = 10, LOG_STEERING = 11,
    LOG_VIDEO_CAPTURE = 12, LOG_BRAKING = 13, LOG_AUTOBRAKING = 14, LOG_AUTOMATION = 15, LOG_CNT = 16
};
enum LOG_LEVEL { LL_VERBOSE = 0, LL_DEBUG = 1, LL_INFO = 2, LL_WARNING = 3, LL_ERROR = 4, LL_FATAL = 5, LL_CNT = 6 };
enum NoesisLogLevel { NLL_TRACE = 0, NLL_DEBUG = 1, NLL_INFO = 2, NLL_WARNING = 3, NLL_ERROR = 4, NLL_CNT = 5 };
enum NetworkLogLevel { NL_UNK = 0, NL_ERROR = 1, NL_WARN = 2, NL_INFO = 3, NL_DEBUG = 4 };
#define NetworkingLogError(...) LogTyped(LOG_ZNETWORK_INTERNAL, "[ERROR] " __VA_ARGS__)
#define NetworkingLogWarn(...)  LogTyped(LOG_ZNETWORK_INTERNAL, "[WARN] " __VA_ARGS__)
#define NetworkingLogInfo(...)  LogTyped(LOG_ZNETWORK_INTERNAL, "[INFO] " __VA_ARGS__)
#define NetworkingLogDebug(...) LogTyped(LOG_ZNETWORK_INTERNAL, "[DEBUG] " __VA_ARGS__)
void LogTyped(LOG_TYPE type, const char *fmt, ...);
void Log(const char *fmt, ...);
void LogLev(LOG_LEVEL level, const char *fmt, ...);
void LogDebug(const char *fmt, ...);
void LogNoesis(void *dummy_a1, void *dummy_a2, NoesisLogLevel noesisLevel, void *dummy_a4, const char *msg);
void SaveLog();
void LogInitialize();
typedef void (*LogWriteHandler)(LOG_LEVEL level, LOG_TYPE ty, const char *msg);
void SetLogWriteHandler(LogWriteHandler h);
struct GameAssertHandler {
    static inline bool s_disableAbort;
    static void Initialize();
    static void Shutdown();
    static void DisableAbort() { s_disableAbort = true; }
    bool BeforeAbort(const char *cond, const char *file, unsigned line, PVOID *BackTrace, int nframes);
    void Abort();
    //static void CheckOnceFlag(const char *, int);
    //static void ComputeAssertHash(const char *, int);
    //static void GetOnceInfo(const char *, int);
    //static void NotifyCrashReporting(const char *, const char *, int, void **, uint);
    //static void PopContext();
    //static void PushContext(const char *, zu::Value &&);
    //static void SetOnceFlag(const char *, int);
};
struct ZwiftAssert {
    static inline std::mutex        g_abortMutex;
    static inline thread_local bool g_abortProcessing;
    static inline GameAssertHandler *g_abortListener;
    static void SetHandler(GameAssertHandler *ptr) { if (!g_abortProcessing) g_abortListener = ptr; }
    static void Abort();
    static bool BeforeAbort(const char *cond, const char *file, unsigned line);
    static bool IsBeingDebugged() { return IsDebuggerPresent(); }
    //static void Test_(bool, const char *, const char *, const char *, int, bool);
};
#define zassert(c) if (!(c)) { \
        if (ZwiftAssert::IsBeingDebugged()) __debugbreak(); \
        if (ZwiftAssert::BeforeAbort(#c, __FILE__, __LINE__)) ZwiftAssert::Abort(); }
std::vector<std::string> ParseSuppressedLogs(const char *ls);
void LogSetSuppressedLogTypes(const std::vector<std::string> &supprLogs);
void glfwZwiftErrorCallback(int code, const char *msg);
struct ConsoleRenderer;
void CONSOLE_DrawCmdline(const ConsoleRenderer &cr, const char *line, int *lineNo, int lineCount, int lineType);
void CONSOLE_DrawPar(const ConsoleRenderer &cr, const char *str, int *a3, int a4, int a5);
void ScrollLog(int dir);
int LogGetLineCount();
LOG_TYPE LogGetLineType(int a1);
const char *LogGetLine(int a1);

inline bool g_alwaysScrollToEnd;
inline int  g_nLogLines, g_curLogLine, g_scrollLogPos;
