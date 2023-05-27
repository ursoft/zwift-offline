#pragma once
typedef void (*LogHandler)(const char *fmt, ...);
struct tHigFile {
    inline static LogHandler m_logHandler;
    static void SetLogHandler(LogHandler);
};
