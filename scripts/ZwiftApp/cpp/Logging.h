#pragma once
enum LOG_TYPE {
    LOG_GENERAL = 0, LOG_WARNING = 1, LOG_ERROR = 2, LOG_COMMAND = 3, LOG_COMMAND_OUTPUT = 4, LOG_ASSET = 5, LOG_NETWORK = 6, LOG_ZNETWORK_INTERNAL = 7, LOG_ANT = 8, LOG_ANT_IMPORTANT = 9, LOG_BLE = 10, LOG_STEERING = 11, 
    LOG_VIDEO_CAPTURE = 12, LOG_BRAKING = 13, LOG_AUTOBRAKING = 14, LOG_AUTOMATION = 15, LOG_CNT = 16
};
enum LOG_LEVEL { LL_VERBOSE = 0, LL_DEBUG = 1, LL_INFO = 2, LL_WARNING = 3, LL_ERROR = 4, LL_FATAL = 5, LL_CNT = 6 };
enum NoesisLogLevel { NLL_TRACE = 0, NLL_DEBUG = 1, NLL_INFO = 2, NLL_WARNING = 3, NLL_ERROR = 4, NLL_CNT = 5 };

void LogTyped(LOG_TYPE type, const char *fmt, ...);
void Log(const char *fmt, ...);
void LogLev(LOG_LEVEL level, const char *fmt, ...);
void LogDebug(const char *fmt, ...);
void LogNetInt(const char *msg);
void LogNoesis(void *dummy_a1, void *dummy_a2, NoesisLogLevel noesisLevel, void *dummy_a4, const char *msg);

typedef void (*LogWriteHandler)(LOG_LEVEL level, LOG_TYPE ty, const char *msg);
void SetLogWriteHandler(LogWriteHandler h);

bool ZwiftBeforeAbort(const char *cond, const char *file, unsigned line);
void ZwiftAssert_Abort();

#define zassert(c) if(!(c)) { \
  if (IsDebuggerPresent()) __debugbreak(); \
  if (ZwiftBeforeAbort(#c, __FILE__, __LINE__)) ZwiftAssert_Abort(); }

class GameAssertHandler {
public:
    bool OnBeforeAbort(const char *cond, const char *file, unsigned line, PVOID *BackTrace, int nframes);
    void OnAbort();
    static void Initialize();
    static bool s_disableAbort;
    static GameAssertHandler s_instance;
};

void ZwiftAssert_SetHandler(GameAssertHandler *ptr);
std::vector<std::string> ParseSuppressedLogs(const char *ls);
void InitLogging(const std::vector<std::string> &supprLogs);
