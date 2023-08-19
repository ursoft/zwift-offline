#pragma once //OMITted
typedef void (*LogHandler)(const char *fmt, ...);
struct tHigFile {
    inline static LogHandler m_logHandler;
    static void SetLogHandler(LogHandler);
//Close(bool)
//GetSize(void)
//IsReading(void)
//Open(char const*,uint,tHigFile::tCreateMode)
//Read(void *,uint)
//ReadAsync(void *,uint)
//WaitAsync(void)
//tHigFile(void)
//~tHigFile()
};
