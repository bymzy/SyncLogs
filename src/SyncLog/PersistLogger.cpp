

#include "include/DirUtil.hpp"
#include "include/Log.hpp"
#include "PersistLogger.hpp"
#include "KVDB.hpp"

#include <iomanip>

int PersistLogger::RecoverFromLog()
{
    int err = 0;
    std::set<std::string> logFiles;
    std::set<std::string>::iterator logIter;

    /* loop dir to get logfile */
    DirUtil dirUtil(mLogDir);

    do {
        err = dirUtil.OpenDir();
        if (0 != err) {
            break;
        }

        dirUtil.GetAllFileWithSuffix(".log", logFiles);

        debug_log("dump log file: ");
        logIter = logFiles.begin();
        for (;logIter != logFiles.end(); ++logIter) {
            debug_log("logFile: " << *logIter);
            RedoLogFile(*logIter);
        }

        debug_log("dump log file end.");

        /* TODO sync wait all log done ??*/

    } while(0);

    return err;
}

int PersistLogger::RedoLogFile(std::string logFileName)
{
    int err = 0;
    Log log(logFileName);
    LogRecord *record = NULL;

    do {
        err = log.OpenFile(false);
        if (0 != err) {
            error_log("open log faile " << logFileName << " failed!, err: " << err)
            break;
        }

        record = new LogRecord;

        while (0 == (err = log.GetNextLogRecord(record))) {
            KVDB::Instance()->GetLogCenter()->AppendLogRecord(record, 
                    LogContext::LOG_recover_log);
        }

    } while(0);

    return err;
}

std::string PersistLogger::GetEpochString(uint32_t epoch)
{
    /* 8 chars hex */
    std::stringstream ss;
    ss.fill(0);
    ss << std::hex << std::setw(8) << epoch;

    return ss.str();
}

std::string PersistLogger::GetLogIndexString(uint32_t index)
{
    /* 8 chars hex */
    std::stringstream ss;
    ss.fill(0);
    ss << std::hex << std::setw(8) << index;

    return ss.str();
}

int PersistLogger::OpenNewLogFile()
{
    if (NULL != mCurrentLog) {
        mCurrentLog->Close();
        delete mCurrentLog;
        mCurrentLog = NULL;
    }

    std::string logFileName = mLogDir + GetEpochString(KVDB::Instance()->GetEpoch())
        + GetLogIndexString(mMaxLogId / 2000);

    mCurrentLog = new Log(logFileName);
    debug_log("new log file name: " << logFileName);
    return mCurrentLog->OpenFile(true);
}

bool PersistLogger::NeedOpenNewLogFile()
{
    if (0 == (mMaxLogId % 2000)) {
        return true;
    } 

    return false;
}

int PersistLogger::WriteLog(LogRecord *record)
{
    int err =0;

    do {
        if (NULL == mCurrentLog || NeedOpenNewLogFile()) {
            err = OpenNewLogFile();
            if (0 != err) {
                error_log("open new log file failed, err: " << err);
                break;
            }
        }

        err = mCurrentLog->AppendLogRecord(record);

    } while(0);

    return err;
}


