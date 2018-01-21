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
    void HandleNewKVRequest(KVRequest *request);

private:
    void HandleLocalRequest(LogContext *logCtx);

private:
    uint64_t mRequestID;
    std::map<uint64_t, KVRequest*> mRequests;
};


#endif //PROJECT_REQUESTCENTER_HPP
