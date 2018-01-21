

#include "include/DirUtil.hpp"
#include "PersistLogger.hpp"
#include "KVDB.hpp"

uint32_t PersistLogger::GenerateLogId()
{
    return 0;
}

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

        debug_log("dump log file: ")
        logIter = logFiles.begin();
        for (;logIter != logFiles.end(); ++logIter) {
            debug_log("logFile: " << *logIter);
            RedoLogFile(*logIter);
        }

        debug_log("dump log file end.")

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
            KVDB::Instance()->GetLogCenter()->AppendLogRecord(record);
        }

    } while(0);

    return err;
}


