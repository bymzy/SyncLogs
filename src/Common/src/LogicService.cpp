

#include <vector>

#include <time.h>
#include "LogicService.hpp"
#include "Log.hpp"

void
LogicService::Run()
{
    int err = 0;
    OperContext *ctx= NULL;
    std::vector<OperContext *> ctxVec;

    std::cout << "befor init " << std::endl;
    err = Init();
    if (0 != err) {
        std::cerr << "LogicService init failed, name: " << mName
                << ", error: " << err;
        return;
    }
    std::cout << "after init " << std::endl;

    while (mRunning)
    {
        if (mSleepNanosec == 0) {
            pthread_mutex_lock(&mMutex);
            while (mQueue.size() <= 0
                    && mRunning) {
                pthread_cond_wait(&mCond, &mMutex);
            }

            /* now there is something in queure try process it */
            ctxVec.assign(mQueue.begin(), mQueue.end());
            mQueue.clear();

            /* io driver will not be blocked when received msg */
            pthread_mutex_unlock(&mMutex);
        } else {
            struct timespec ts;
            //clock_gettime(CLOCK_MONOTONIC, &ts);
            clock_gettime(CLOCK_REALTIME, &ts);

            ts.tv_sec += mSleepNanosec / 1000000000;
            ts.tv_nsec += mSleepNanosec % 1000000000;

#if 0
            std::cerr << mSleepNanosec / 1000000000
                << "@@" << mSleepNanosec % 1000000000 << std::endl;
            std::cerr << ts.tv_sec << "-"<< ts.tv_nsec << "==" << ts2.tv_sec << "-" << ts2.tv_nsec << std::endl;
#endif

#if 0
            if (ts.tv_nsec > 1000000000) {
                ts.tv_sec += 1;
                ts.tv_nsec -= 1000000000;
            }
#endif

            pthread_mutex_lock(&mMutex);
            pthread_cond_timedwait(&mCond, &mMutex, &ts);
            ctxVec.assign(mQueue.begin(), mQueue.end());
            mQueue.clear();
            pthread_mutex_unlock(&mMutex);
        }

        if (ctxVec.size() > 0) {
            for (size_t i = 0;i < ctxVec.size();++i) {
                ctx = ctxVec[i];
                if (mRunning) {
                    if (!LogicService::Process(ctx)) {
                        assert(Process(ctx));
                    }
                } else {
                    ctx->SetMessage(NULL);
                }

                OperContext::DecRef(ctx);
                ctx = NULL;
            }
        }

        if (mSleepNanosec > 0) {
           Idle(); 
        }

        ctxVec.clear();
    }

    Finit();
    //std::cout << "logic service exit! vec size: " <<  ctxVec.size() <<",cap: "<< ctxVec.capacity() << std::endl;
}

/* Start only return */
void
LogicService::ProcessStart(OperContext *ctx)
{
    /* stop logicservice thread*/
    SyncClosure *sync = ctx->GetClosure();
    mRunning = true;
    sync->Signal();
    ctx->SetClosure(NULL);
}

void
LogicService::ProcessStop(OperContext *ctx)
{
    /* stop logicservice thread*/
    SyncClosure *sync = ctx->GetClosure();
    mRunning = false;
    mThread->Join();
    sync->Signal();
}

void
LogicService::RecvMsg(OperContext *ctx) {
    Msg *msg = ctx->GetMessage();
    std::string data;
    (*msg) >> (data);
    std::cout << "receive data from client, conn id:  " << ctx->GetConnID() << data << std::endl;
    delete msg;
    ctx->SetMessage(NULL);
}

bool
LogicService::Process(OperContext *ctx)
{
    bool processed = true;
    switch (ctx->GetType())
    {
        case OperContext::OP_STOP:
            ProcessStop(ctx);
            break;
        case OperContext::OP_START:
            ProcessStart(ctx);
            break;
        default:
            processed = false;
            break;
    }

    return processed;
}

bool
LogicService::Enqueue(OperContext *ctx)
{
    bool pushed = false;
    pthread_mutex_lock(&mMutex);
    if (mRunning) {
        OperContext::IncRef(ctx);
        mQueue.push_back(ctx);
        pushed = true;
    }
    pthread_cond_signal(&mCond);
    pthread_mutex_unlock(&mMutex);

    return pushed;
}






