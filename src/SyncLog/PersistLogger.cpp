

#include "include/DirUtil.hpp"
#include "PersistLogger.hpp"

uint32_t PersistLogger::GenerateLogId()
{
    return 0;
}

int PersistLogger::RecoverFromLog()
{
    int err = 0;
    std::vector<std::string> logFiles;

    /* loop dir to get logfile */
    DirUtil dirUtil(mLogDir);

    do {
        err = dirUtil.OpenDir();
        if (0 != err) {
            break;
        }

        dirUtil.GetAllFileWithSuffix(".log", logFiles);
        
        /* TODO sort logFiles */

    } while(0);

    return err;
}

int PersistLogger::AppendLogRecord(LogRecord *record)
{
    return 0;

}


