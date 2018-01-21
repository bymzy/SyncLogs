//
// Created by root on 1/21/18.
//

#include "RequestCenter.hpp"
#include "LogContext.hpp"

void RequestCenter::EnqueueKVRequest(KVRequest *request)
{
    OperContext *ctx = new OperContext(OperContext::OP_LOCAL);
    LogContext *logCtx = new LogContext(LogContext::REQUEST_recv_data_request);
    logCtx->SetArg(request);
    ctx->SetArg((void *)request);

    Enqueue(ctx);
    OperContext::DecRef(ctx);
}

void RequestCenter::HandleNewKVRequest(KVRequest *request)
{
    ++mRequestID;
    mRequests.insert(std::make_pair(mRequestID, request));

    /* TODO add request to logCenter */
}

void RequestCenter::HandleLocalRequest(LogContext *logCtx)
{
    switch (logCtx->GetCtxType()) {
        case LogContext::REQUEST_recv_data_request:
            HandleNewKVRequest((KVRequest *)logCtx->GetArg());
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

