//
// Created by root on 1/21/18.
//

#ifndef PROJECT_REQUESTCENTER_HPP
#define PROJECT_REQUESTCENTER_HPP

#include <map>
#include <string>

#include "include/LogicService.hpp"
#include "include/Closure.hpp"
#include "KVRequest.hpp"
#include "LogContext.hpp"

/* handle kv request
 * hold request context
 * */
class LogResponse
{
public:
    LogResponse():mErr(0),mRequestId(0)
    {}
    ~LogResponse()
    {}

public:
    uint32_t mErr;
    uint64_t mRequestId;
    std::string mValue;
};

class RequestCenter : public LogicService
{
public:
    RequestCenter(std::string name): LogicService(name), mRequestID(0)
    {
    }
    ~RequestCenter()
    {
    }

public:
    virtual bool Process(OperContext *ctx);
    void EnqueueKVRequest(KVRequest *request);
    void ReceiveKVResponse(uint64_t requestId, uint32_t err,  const std::string& value);

private:
    void HandleNewKVRequest(KVRequest *request);
    void HandleLocalRequest(LogContext *logCtx);
    void HandleKVResponse(LogResponse *resp);

private:
    uint64_t mRequestID;
    std::map<uint64_t, KVRequest*> mRequests;
};

#endif


