//
// Created by root on 1/21/18.
//


#include "LogCenter.hpp"

int LogCenter::AppendLogRecord(LogRecord *record)
{
    OperContext *ctx = new OperContext(OperContext::OP_LOCAL);
    ctx->SetArg((void *)record);
    assert(Enqueue(ctx));
    OperContext::DecRef(ctx);
    return 0;
}

bool LogCenter::Process(OperContext *ctx)
{
    bool processed = true;
    switch (ctx->GetType()) {
        case OperContext::OP_LOCAL:
            HandleLog(ctx);
            break;
        default:
            assert(0 && "invalid OperContext");
            processed = false;
            break;
    }

    return processed;
}

int LogCenter::HandleLog(OperContext *ctx)
{
    LogRecord *record = (LogRecord *)ctx->GetArg();
    LogRecordBody *body = record->GetRecordBody();
    int err = 0;

    switch (record->GetOpType()) {
        case LogRecord::OP_put:
            err = mDataStore.Put(body->GetTableName(), body->GetKey(), body->GetValue());
            break;
        case LogRecord::OP_add:
            err = mDataStore.Add(body->GetTableName(), body->GetKey(), body->GetValue());
            break;
        case LogRecord::OP_del:
            err = mDataStore.Del(body->GetTableName(), body->GetKey());
            break;
        case LogRecord::OP_drop_table:
            err = mDataStore.DropTable(body->GetTableName());
            break;
        case LogRecord::OP_add_table:
            err = mDataStore.CreateTable(body->GetTableName());
            break;
        default:
            error_log("invalid LogRecord type: " << record->GetOpType());
            assert(0);
    }

    /* record body is deleted in ~record() */
    delete record;
    ctx->SetArg(NULL);

    return err;
}

