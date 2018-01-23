

#include "include/DirUtil.hpp"
#include "include/Log.hpp"
#include "PersistLogger.hpp"
#include "KVDB.hpp"

#include <iomanip>

#define LOG_PER_FILE 5000

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

        while (0 == (err = log.GetNextLogRecord(&record))) {
            KVDB::Instance()->GetLogCenter()->AppendLogRecord(record, 
                    LogContext::LOG_recover_log);

            /* update max log id */
            KVDB::Instance()->GetLogCenter()->SetMaxLogId(record->GetLogId());
        }

        KVDB::Instance()->SetEpoch(ParseEpochFromLogName(logFileName));

        /* TODO EIO means the end of file */
        if (EIO == err) {
            err = 0;
            break;
        }
    } while(0);

    return err;
}

std::string PersistLogger::Dec2HexString(uint32_t val, uint32_t bitCount)
{
    /* 8 chars hex */
    std::stringstream ss;
    ss << std::hex << val;
    std::string temp, temp2;
    temp = ss.str();
    temp2.assign(bitCount - temp.length(), '0');
    temp2 += temp;

    return temp2;
}

int PersistLogger::OpenNewLogFile()
{
    if (NULL != mCurrentLog) {
        mCurrentLog->Close();
        delete mCurrentLog;
        mCurrentLog = NULL;
    }

    std::string logFileName = mLogDir 
        + Dec2HexString(KVDB::Instance()->GetEpoch(), 8)
        + Dec2HexString(KVDB::Instance()->GetLogCenter()->GetMaxLogId() / LOG_PER_FILE, 8)
        + ".log";

    mCurrentLog = new Log(logFileName);
    debug_log("new log file name: " << logFileName);
    return mCurrentLog->OpenFile(true);
}

uint64_t PersistLogger::ParseEpochFromLogName(std::string name)
{
    uint64_t epoch;
    std::string baseName(basename(name.c_str()));
    std::string epochString = baseName.substr(0, 8);
    std::stringstream ss;
    ss << std::hex << epochString;
    ss >> epoch;

    return epoch;
}

bool PersistLogger::NeedOpenNewLogFile()
{
    if (0 == (KVDB::Instance()->GetLogCenter()->GetMaxLogId() % LOG_PER_FILE)) {
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
        //trace_log("write log, record: " << record->GetDumpString());

    } while(0);

    return err;
}


