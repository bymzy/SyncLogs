//
// Created by root on 1/21/18.
//


#include "LogCenter.hpp"
#include "KVRequest.hpp"
#include "LogContext.hpp"
#include "KVDB.hpp"

void LogCenter::HandleLocalContext(LogContext *logCtx)
{
    switch (logCtx->GetCtxType()) {
        case LogContext::LOG_recover_log:
            HandleRecoverLog((LogRecord*)logCtx->GetArg());
            break;
        case LogContext::LOG_write_log:
            HandleWriteOper((LogRecord*)logCtx->GetArg());
            break;
        case LogContext::LOG_read_data:
            HandleReadData((GetReq*)logCtx->GetArg());
            break;
        default:
            assert(0);
            break;
    }

    logCtx->SetArg(NULL);
    delete logCtx;
}

bool LogCenter::Process(OperContext *ctx)
{
    bool processed = true;
    switch (ctx->GetType()) {
        case OperContext::OP_LOCAL:
            HandleLocalContext((LogContext*)ctx->GetArg());
            break;
        default:
            assert(0 && "invalid OperContext");
            processed = false;
            break;
    }

    if (NULL != ctx->GetArg()) {
        ctx->SetArg(NULL);
    }

    return processed;
}

int LogCenter::UpdateDataStore(LogRecord *record)
{
    LogRecordBody *body = record->GetRecordBody();
    int err = 0;

    switch (record->GetOpType()) {
        case KVRequest::OP_put:
            err = KVDB::Instance()->GetDataStore()->Put(body->GetTableName(),
                    body->GetKey(), body->GetValue());
            break;
        case KVRequest::OP_add:
            err = KVDB::Instance()->GetDataStore()->Add(body->GetTableName(),
                    body->GetKey(), body->GetValue());
            break;
        case KVRequest::OP_del:
            err = KVDB::Instance()->GetDataStore()->Del(body->GetTableName(), body->GetKey());
            break;
        case KVRequest::OP_drop_table:
            err = KVDB::Instance()->GetDataStore()->DropTable(body->GetTableName());
            break;
        case KVRequest::OP_add_table:
            err = KVDB::Instance()->GetDataStore()->CreateTable(body->GetTableName());
            break;
        default:
            error_log("invalid LogRecord type: " << record->GetOpType());
            assert(0);
    }

    return err;
}

void LogCenter::HandleRecoverLog(LogRecord *record)
{
    UpdateDataStore(record);

    /* record body is deleted in ~record() */
    delete record;
}

int LogCenter::HandleWriteOper(LogRecord *record)
{
    int err = 0;

    do {
        /* step1 
         * write log
         * */
        err = KVDB::Instance()->GetPersistLogger()->WriteLog(record);
        if (0 != err) {
            error_log("Write Log Failed, err: " << err
                    << ", logRecord, opType:  " << record->GetOpType()
                    << ", logId: " << record->GetLogId()
                    << ", body: " << record->GetRecordBody()->GetDumpString());
            break;
        }

        /* step2
         * update data store
         * */
        err = UpdateDataStore(record);
        if (0 != err) {
            break;
        }


    } while(0);

    /* step3
     * send response
     * */
    std::map<uint64_t, uint64_t>::iterator iter;
    iter = mLogId2RequestId.find(record->GetLogId());
    assert(iter != mLogId2RequestId.end());

    KVDB::Instance()->GetRequestCenter()->ReceiveKVResponse(iter->second,
            err, "");

    delete record;
    return err;
}

void LogCenter::AppendLogRecord(LogRecord *record, uint32_t logContexType)
{
    OperContext *ctx = new OperContext(OperContext::OP_LOCAL);
    LogContext *logCtx = new LogContext(logContexType);
    logCtx->SetArg(record);
    ctx->SetArg((void *)logCtx);
    assert(Enqueue(ctx));
    OperContext::DecRef(ctx);
}

uint64_t LogCenter::GenerateLogId()
{
    /* TODO mutex */
    return ++mMaxLogId;
}

void LogCenter::AppendGetRequest(GetReq *req)
{
    OperContext *ctx = new OperContext(OperContext::OP_LOCAL);
    LogContext *logCtx = new LogContext(LogContext::LOG_read_data);
    logCtx->SetArg((void*)req);
    ctx->SetArg((void*)logCtx);
    assert(Enqueue(ctx));
    OperContext::DecRef(ctx);
}

int LogCenter::ReceiveClientRequest(uint64_t requestId, uint32_t opType,
        std::string tableName, std::string key, std::string value)
{
    if (opType == KVRequest::OP_get) {
        GetReq *req = new GetReq;
        req->mTableName = tableName;
        req->mKey = key;
        req->mRequestId = requestId;

        AppendGetRequest(req);
    } else {
        uint64_t logId = GenerateLogId();
        FileLogRecordBody *body = new FileLogRecordBody;
        body->SetTableName(tableName);
        body->SetKey(key);
        body->SetValue(value);

        LogRecord *record = new LogRecord;
        record->SetLogId(logId);
        record->SetOpType(opType);
        record->SetRecordBody(body);

        /* TODO this need lock ?? */
        mLogId2RequestId.insert(std::make_pair(logId, requestId));

        AppendLogRecord(record, LogContext::LOG_write_log);
    }

    return 0;
}

void LogCenter::HandleReadData(GetReq *req)
{
    int err = 0;
    std::string value;
    err = KVDB::Instance()->GetDataStore()->Get(req->mTableName,
            req->mKey, value);
    if (0 != err) {
        error_log("KVDB get failed, err: " << err);
    }

    KVDB::Instance()->GetRequestCenter()->ReceiveKVResponse(req->mRequestId,
            err, value);
}


