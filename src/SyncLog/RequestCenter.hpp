//
// Created by root on 1/21/18.
//

#ifndef PROJECT_REQUESTCENTER_HPP
#define PROJECT_REQUESTCENTER_HPP

#include <map>
#include <string>
#include <set>

#include "include/LogicService.hpp"
#include "include/Closure.hpp"
#include "KVRequest.hpp"
#include "LogContext.hpp"
#include "Paxos.hpp"

/* handle kv request
 * hold request context
 * */
typedef struct _LogIdRecordPair {
    uint64_t id;
    void *param;
}RequestIdLogRecord;

typedef struct _LogResponse
{
    std::set<RequestIdLogRecord*> pairs;
}LogResponse;

/* read/write log and update datastore */
typedef struct _GetReq
{
    std::string mTableName;
    std::string mKey;
    uint64_t mRequestId;
}GetReq;

class RequestCenter : public LogicService
{
public:
    RequestCenter(std::string name): LogicService(name, 200000000), mRequestID(0)
    {
    }
    ~RequestCenter()
    {
    }

public:
    virtual bool Process(OperContext *ctx);
    virtual void Idle();
    void EnqueueKVRequest(KVRequest *request);
    void ReceiveKVResponse(LogResponse *resp);

private:
    void HandleNewKVRequest(KVRequest *request);
    void HandleLocalRequest(LogContext *logCtx);
    void HandleKVResponse(LogResponse *resp);
    void AppendGetRequest(GetReq *req);

private:
    uint64_t mRequestID;
    /* request id to request */
    std::map<uint64_t, KVRequest*> mRequests;
    Paxoser mPaxoser;   
};

#endif


