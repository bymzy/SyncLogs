//
// Created by root on 1/21/18.
//

#include "RequestCenter.hpp"
#include "LogContext.hpp"
#include "KVDB.hpp"

void RequestCenter::EnqueueKVRequest(KVRequest *request)
{
    OperContext *ctx = new OperContext(OperContext::OP_LOCAL);
    LogContext *logCtx = new LogContext(LogContext::REQUEST_recv_data_request);
    logCtx->SetArg(request);
    ctx->SetArg((void *)logCtx);

    Enqueue(ctx);
    OperContext::DecRef(ctx);

    /* wait kv request done */
    request->Wait();
}

void RequestCenter::ReceiveKVResponse(LogResponse *resp)
{
    OperContext *ctx = new OperContext(OperContext::OP_LOCAL);
    LogContext *logCtx = new LogContext(LogContext::REQUEST_finish_data_request);
    logCtx->SetArg(resp);
    ctx->SetArg(logCtx);

    Enqueue(ctx);
    OperContext::DecRef(ctx);
}

void RequestCenter::AppendGetRequest(GetReq *req)
{
    OperContext *ctx = new OperContext(OperContext::OP_LOCAL);
    LogContext *logCtx = new LogContext(LogContext::LOG_read_data);
    logCtx->SetArg((void*)req);
    ctx->SetArg((void*)logCtx);
    Enqueue(ctx);
    OperContext::DecRef(ctx);
}


void RequestCenter::HandleNewKVRequest(KVRequest *request)
{

    /* send request to logCenter
     * notice that read request is also haneld in logcenter
     * */

    if (request->GetOpType() == KVRequest::OP_get) {
        request->mErr = KVDB::Instance()->GetDataStore()->Get(request->GetTableName(), request->GetKey(), request->mValue);
        request->Signal();
    } else {
        ++mRequestID;
        mRequests.insert(std::make_pair(mRequestID, request));
        KVDB::Instance()->GetLogCenter()->ReceiveClientRequest(mRequestID, request->GetOpType(),
                request->GetTableName(), request->GetKey(), request->GetValue());
    }
}

void RequestCenter::HandleKVResponse(LogResponse *resp)
{
    std::map<uint64_t, KVRequest*>::iterator iter;
    KVRequest *request = NULL;
    std::set<RequestIdLogRecord*>::iterator setIter;
    RequestIdLogRecord *pair = NULL;
    LogRecord * record = NULL;
    int err = 0;

    for (setIter = resp->pairs.begin(); setIter != resp->pairs.end(); ++setIter) {
        pair = (RequestIdLogRecord*)(*setIter);

        iter = mRequests.find(pair->id);
        if (iter != mRequests.end()) {
            request = iter->second;
            record = (LogRecord*)pair->param;

            err = KVDB::Instance()->GetLogCenter()->UpdateDataStore(record);
            request->mErr = err;
            request->Signal();

            mRequests.erase(pair->id);
        } else {
            error_log("Receive Log Response from LogCenter, but can not find request");
            assert(0);
        }
    }


    delete resp;
}

void RequestCenter::HandleLocalRequest(LogContext *logCtx)
{
    switch (logCtx->GetCtxType()) {
        case LogContext::REQUEST_recv_data_request:
            HandleNewKVRequest((KVRequest *)logCtx->GetArg());
            break;
        case LogContext::REQUEST_finish_data_request:
            HandleKVResponse((LogResponse*)logCtx->GetArg());
            break;
        default:
            assert(0);
            break;
    }

    logCtx->SetArg(NULL);
    delete logCtx;
}

bool RequestCenter::Process(OperContext *ctx)
{
    bool processed = true;
    switch (ctx->GetType()) {
        case OperContext::OP_LOCAL:
            HandleLocalRequest((LogContext*)ctx->GetArg());
            break;
        default:
            assert(0 && "invalid OperContext");
            processed = false;
            break;
    }

    ctx->SetArg(NULL);
    return processed;
}

void RequestCenter::Idle()
{
    /* 判断Paxoser当前是否处于稳定状态，如果不是就开始进行选举 */
    //debug_log("RequestCenter Idle");
    if (mPaxoser.NeedElection()) {
        mPaxoser.StartElection();
    }
}


