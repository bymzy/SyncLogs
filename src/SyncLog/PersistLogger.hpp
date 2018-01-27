

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
        if (mLogDir[mLogDir.length() - 1] != '/') {
            mLogDir.append("/");
        }
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
    int FlushLog();

private:
    int RedoLogFile(std::string logFileName);
    int OpenNewLogFile(uint64_t id);
    bool NeedOpenNewLogFile(uint64_t id);
    std::string Dec2HexString(uint32_t val, uint32_t bitCount);
    uint64_t ParseEpochFromLogName(std::string name);

private:
    Log *mCurrentLog;
    std::string mLogDir;
    uint32_t mMaxLogId;
};

#endif


