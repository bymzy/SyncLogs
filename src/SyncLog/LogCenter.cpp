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
            UpdateDataStore((LogRecord*)logCtx->GetArg());
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
            err = KVDB::Instance()->GetDataStore()->Put(body->GetTableName(), body->GetKey(), body->GetValue());
            break;
        case KVRequest::OP_add:
            err = KVDB::Instance()->GetDataStore()->Add(body->GetTableName(), body->GetKey(), body->GetValue());
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

    /* record body is deleted in ~record() */
    delete record;

    return err;
}

int LogCenter::AppendLogRecord(LogRecord *record)
{
    OperContext *ctx = new OperContext(OperContext::OP_LOCAL);
    LogContext *logCtx = new LogContext(LogContext::LOG_recover_log);
    logCtx->SetArg(record);
    ctx->SetArg((void *)logCtx);
    assert(Enqueue(ctx));
    OperContext::DecRef(ctx);
    return 0;
}


