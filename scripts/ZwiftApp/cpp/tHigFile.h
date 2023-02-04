#pragma once
typedef void (*LogHandler)(const char *fmt, ...);
class tHigFile {
public:
    inline static LogHandler m_logHandler;
    static void SetLogHandler(LogHandler);
};
