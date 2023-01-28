#pragma once
enum LOG_TYPE {
	LT_NONE = 0, LT_WARN = 1, LT_ERROR = 2, LT_TERMINAL = 3, LT_NONAMED = 4, LT_FILE = 5, LT_NETWORK = 6, LT_ANT1 = 7, LT_ANT2 = 8, LT_BLE = 9, LT_STEERING = 10, LT_VIDEOCAPTURE = 11,
	LT_BRAKING = 12, LT_AUTOBRAKING = 13, LT_AUTOMATION = 14
};
enum LOG_LEVEL { LL_VERBOSE = 0, LL_DEBUG = 1, LL_INFO = 2, LL_WARNING = 3, LL_ERROR = 4, LL_FATAL = 5, LL_NOT_SET_YET = 6 };
enum NoesisLogLevel { NoesisLogLevels_0 = 0, NoesisLogLevels_1 = 1, NoesisLogLevels_2 = 2, NoesisLogLevels_3 = 3, NoesisLogLevels_4 = 4, NoesisLogLevels_5 = 5 };

void LogTyped(LOG_TYPE type, const char *fmt, ...);
void Log(const char *fmt, ...);
void LogLev(LOG_LEVEL level, const char *fmt, ...);
void LogDebug(const char *fmt, ...);
void LogAnt(const char *msg);
void LogNoesis(void *dummy_a1, void *dummy_a2, NoesisLogLevel noesisLevel, void *dummy_a4, const char *msg);

bool ZwiftBeforeAbort(const char *cond, const char *file, unsigned line);
void ZwiftAssert_Abort();

#define zassert(c) if(!(c)) { \
  if (IsDebuggerPresent()) __debugbreak(); \
  if (ZwiftBeforeAbort(#c, __FILE__, __LINE__)) ZwiftAssert_Abort(); }

class GameAssertHandler {
public:
    bool OnBeforeAbort(const char *cond, const char *file, unsigned line, PVOID *BackTrace, int nframes);
    void OnAbort();
    static bool s_disableAbort;
};
extern GameAssertHandler s_instance;

void ZwiftAssert_SetHandler(GameAssertHandler *ptr);