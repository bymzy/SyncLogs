//
// Created by root on 1/21/18.
//


#include "LogCenter.hpp"
#include "KVRequest.hpp"
#include "LogContext.hpp"
#include "KVDB.hpp"
#include "Pch.hpp"

void LogCenter::HandleLocalContext(LogContext *logCtx)
{
    switch (logCtx->GetCtxType()) {
        case LogContext::LOG_recover_log:
            HandleRecoverLog((LogRecord*)logCtx->GetArg());
            break;
        case LogContext::LOG_write_log:
            HandleWriteOper((LogRecord*)logCtx->GetArg());
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

        assert(0 == err);

        mToFlushLog.insert(std::make_pair(record->GetLogId(), record));

        if (mToFlushLog.size() > MAX_LOG_NUM_TO_FLUSH) {
            err = FlushLog();
            assert(0 == err);

            LogResponse *resp = new LogResponse;
            RequestIdLogRecord *pair = NULL;

            std::map<uint64_t, LogRecord*>::iterator iter;
            for (iter = mToFlushLog.begin(); iter != mToFlushLog.end(); ++iter) {
                pair = new RequestIdLogRecord;
                pair->id = iter->first;
                pair->param = (void *)iter->second;

                resp->pairs.insert(pair);
            }
            /* inform request center to response */
            KVDB::Instance()->GetRequestCenter()->ReceiveKVResponse(resp);

            mToFlushLog.clear();
        }

    } while(0);

    return err;
}

int LogCenter::FlushLog()
{
    int err = KVDB::Instance()->GetPersistLogger()->FlushLog();
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

int LogCenter::ReceiveClientRequest(uint64_t requestId, uint32_t opType,
        std::string tableName, std::string key, std::string value)
{
    if (opType == KVRequest::OP_get) {
        assert(0);
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

void LogCenter::Idle()
{
    int err = 0;
    if (mToFlushLog.size() > 0) {
        err = FlushLog();
        assert(0 == err);
        mFlushCount[mToFlushLog.size()]++;

        LogResponse *resp = new LogResponse;
        RequestIdLogRecord *pair = NULL;
        std::map<uint64_t, uint64_t>::iterator index;

        std::map<uint64_t, LogRecord*>::iterator iter;
        for (iter = mToFlushLog.begin(); iter != mToFlushLog.end(); ++iter) {
            pair = new RequestIdLogRecord;
            index = mLogId2RequestId.find(iter->first);
            assert(index != mLogId2RequestId.end());
            pair->id = index->second;
            pair->param = (void *)iter->second;

            resp->pairs.insert(pair);
        }
        /* inform request center to response */
        KVDB::Instance()->GetRequestCenter()->ReceiveKVResponse(resp);

        mToFlushLog.clear();
    }
}

void LogCenter::DumpStats()
{
    for (uint32_t i = 0 ; i < 50; ++i) {
        debug_log("commit log count: " << mFlushCount[i]);
    }
}


