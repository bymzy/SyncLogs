//
// Created by root on 1/21/18.
//

#ifndef PROJECT_LOGCENTER_HPP
#define PROJECT_LOGCENTER_HPP

#include "include/LogicService.hpp"
#include "FileLog.hpp"
#include "DataStore.hpp"
#include "LogContext.hpp"

class LogCenter : public LogicService
{
public:
    LogCenter(std::string name):LogicService(name, 10000)
    {
    }
    ~LogCenter()
    {
    }

public:
    virtual bool Process(OperContext *ctx);

public:
    int ReceiveClientRequest(uint64_t requestId, uint32_t opType, std::string tableName, 
            std::string key, std::string value);
    void AppendLogRecord(LogRecord *record, uint32_t logContextType);
    void HandleLocalContext(LogContext *logCtx);
    uint64_t GenerateLogId();
    uint64_t GetMaxLogId()
    {
        return mMaxLogId;
    }
    void SetMaxLogId(uint64_t id)
    {
        mMaxLogId = id;
    }

    int UpdateDataStore(LogRecord *record);
    void DumpStats();

private:
    int HandleWriteOper(LogRecord *record);
    int FlushLog();
    void HandleRecoverLog(LogRecord *record);
    void Idle();

private:
    /* logid to requestid */
    std::map<uint64_t, uint64_t> mLogId2RequestId;

    /* logid 2 log record*/
    std::map<uint64_t, LogRecord*> mToFlushLog;
    uint64_t mMaxLogId;
    uint32_t mFlushCount[50];
};

#endif //PROJECT_LOGCENTER_HPP


