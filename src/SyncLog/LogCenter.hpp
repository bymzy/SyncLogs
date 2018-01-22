//
// Created by root on 1/21/18.
//

#ifndef PROJECT_LOGCENTER_HPP
#define PROJECT_LOGCENTER_HPP

#include "include/LogicService.hpp"
#include "FileLog.hpp"
#include "DataStore.hpp"
#include "LogContext.hpp"

/* read/write log and update datastore */
typedef struct _GetReq
{
    std::string mTableName;
    std::string mKey;
    uint64_t mRequestId;
}GetReq;

class LogCenter : public LogicService
{
public:
    LogCenter(std::string name):LogicService(name)
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

private:
    int UpdateDataStore(LogRecord *record);
    int HandleWriteOper(LogRecord *record);
    void HandleReadData(GetReq *req);
    void AppendGetRequest(GetReq *req);

private:
    std::map<uint64_t, uint64_t> mLogId2RequestId;
    uint64_t mMaxLogId;
};

#endif //PROJECT_LOGCENTER_HPP


