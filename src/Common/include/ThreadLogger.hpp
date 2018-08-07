

#ifndef __COMMON_THREAD_LOGGER_HPP__
#define __COMMON_THREAD_LOGGER_HPP__

#include "LogicService.hpp"
#include "Context.hpp"

class ThreadLogger : public LogicService {
public:
    ThreadLogger(std::string filename, std::string logDir):
        LogicService("ThreaLogger"),
        mFileName(filename), mLogDir(logDir), mLogCount(0), mFileFd(-1)
    {
        if (mLogDir[mLogDir.length() - 1] != '/') {
            mLogDir.append("/");
        }
    }

    ~ThreadLogger()
    {
    }

public:
    void WriteLog(std::string log);

private:
    virtual int Init();
    virtual int Finit();
    virtual bool Process(OperContext *ctx);

    void HandleLog(OperContext *ctx);

private:
    std::string mFileName;
    std::string mLogDir;
    uint32_t mLogCount;
    int mFileFd;
};

#endif



