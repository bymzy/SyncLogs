//
// Created by root on 1/21/18.
//

#ifndef PROJECT_LOGCENTER_HPP
#define PROJECT_LOGCENTER_HPP

#include "include/LogicService.hpp"
#include "FileLog.hpp"
#include "DataStore.hpp"
#include "LogContext.hpp"

/* read/write log and update datastore */

class LogCenter : public LogicService
{
public:
    LogCenter(std::string name):LogicService(name)
    {
    }
    ~LogCenter()
    {
    }

public:
    virtual bool Process(OperContext *ctx);

public:
    int AppendLogRecord(LogRecord *record);
    int UpdateDataStore(LogRecord *record);
    void HandleLocalContext(LogContext *logCtx);
};

#endif //PROJECT_LOGCENTER_HPP
