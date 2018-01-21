

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
    uint32_t GenerateLogId();
    int RecoverFromLog();

private:
    int RedoLogFile(std::string logFileName);

private:
    Log *mCurrentLog; 
    std::string mLogDir;
    uint32_t mMaxLogId;
};

#endif


