//
// Created by root on 1/21/18.
//

#ifndef PROJECT_LOGCENTER_HPP
#define PROJECT_LOGCENTER_HPP

#include "include/LogicService.hpp"
#include "FileLog.hpp"
#include "DataStore.hpp"

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
    int HandleLog(OperContext *ctx);

private:
    DataStore mDataStore;
};

#endif //PROJECT_LOGCENTER_HPP
