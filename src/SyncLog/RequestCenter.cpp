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

void RequestCenter::ReceiveKVResponse(uint64_t requestId, uint32_t err,
        const std::string& value)
{
    OperContext *ctx = new OperContext(OperContext::OP_LOCAL);
    LogResponse *resp = new LogResponse;
    resp->mRequestId = requestId;
    resp->mValue = value;
    resp->mErr = err;

    LogContext *logCtx = new LogContext(LogContext::REQUEST_finish_data_request);
    logCtx->SetArg(resp);
    ctx->SetArg(logCtx);

    Enqueue(ctx);
    OperContext::DecRef(ctx);
}

void RequestCenter::HandleNewKVRequest(KVRequest *request)
{
    ++mRequestID;
    mRequests.insert(std::make_pair(mRequestID, request));

    /* send request to logCenter
     * notice that read request is also haneld in logcenter
     * */

    KVDB::Instance()->GetLogCenter()->ReceiveClientRequest(mRequestID, request->GetOpType(),
            request->GetTableName(), request->GetKey(), request->GetValue());
}

void RequestCenter::HandleKVResponse(LogResponse *resp)
{
    std::map<uint64_t, KVRequest*>::iterator iter;
    KVRequest *request = NULL;

    iter = mRequests.find(resp->mRequestId);
    if (iter != mRequests.end()) {
        request = iter->second;
        request->mErr = resp->mErr;
        request->mValue = resp->mValue;
        request->Signal();
        mRequests.erase(resp->mRequestId);
    } else {
        error_log("Receive Log Response from LogCenter, but can not find request");
        assert(0);
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

