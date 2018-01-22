//
// Created by root on 1/21/18.
//

#ifndef PROJECT_LOGCONTEXT_HPP
#define PROJECT_LOGCONTEXT_HPP

#include <assert.h>

class LogContext {
public:
    enum _LogContext {
        /* in LogCenter */
        LOG_recover_log = 0,
        LOG_write_log,
        LOG_read_data,

        /* in RequestCenter */
        REQUEST_finish_data_request,
        REQUEST_read_data_request,
        REQUEST_recv_data_request,
    };

public:
    LogContext(uint32_t ctxType) : mArg(NULL), mCtxType(ctxType)
    {}

    ~LogContext()
    {
        if (NULL != mArg) {
            assert(0);
        }
    }

public:
    uint32_t GetCtxType()
    {
        return mCtxType;
    }
    void SetArg(void *arg)
    {
        mArg = arg;
    }

    void *GetArg()
    {
        return mArg;
    }

public:
    void *mArg;
    uint32_t mCtxType;
};

#endif //PROJECT_LOGCONTEXT_HPP


