

#ifndef __PERSIST_LOGGER_HPP__
#define __PERSIST_LOGGER_HPP__

#include "FileLog.hpp"
#include "LogCenter.hpp"

class PersistLogger
{
public:
    PersistLogger(std::string logDir):
        mCurrentLog(NULL), mLogDir(logDir), mMaxLogId(0)
    {
    }
    ~PersistLogger()
    {
        if (NULL != mCurrentLog) {
            delete mCurrentLog;
            mCurrentLog = NULL;
        }
    }

public:
    int RecoverFromLog();
    int WriteLog(LogRecord *record);

private:
    int RedoLogFile(std::string logFileName);
    int OpenNewLogFile();
    bool NeedOpenNewLogFile();
    std::string GetEpochString(uint32_t epoch);
    std::string GetLogIndexString(uint32_t index);

private:
    Log *mCurrentLog; 
    std::string mLogDir;
    uint32_t mMaxLogId;
};

#endif


