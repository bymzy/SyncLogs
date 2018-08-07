

#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>

#include "ThreadLogger.hpp"

int
ThreadLogger::Init()
{
    int err = 0;
    char outstr[20];
    memset(outstr, 0, 100);
    struct tm *temp;
    time_t now = time(NULL);

    temp = localtime(&now);
    strftime(outstr, sizeof(outstr), "%Y-%m-%d_%H-%M-%S", temp);

    std::string logFile = mLogDir + mFileName  + "-" + outstr + ".log";

    /* open log file */
    mFileFd = open(logFile.c_str(), O_WRONLY | O_CREAT, 0777);

    if (mFileFd < 0) {
        err = errno; 
    }

    return err;
}

int
ThreadLogger::Finit()
{
    /* close log file and flush */ 
    if (mFileFd != -1) {
        close(mFileFd);
    }

    return 0;
}

bool
ThreadLogger::Process(OperContext *ctx)
{
    bool processed = true;
    switch (ctx->GetType()) {
        case OperContext::OP_RECV:
            HandleLog(ctx);
            break;
        default:
            assert(0 && "invalid OperContext for logger");
            processed = false; 
            break;
    }

    return processed;
}

void
ThreadLogger::WriteLog(std::string log)
{
    char outstr[20];
    char usec[9];
    memset(outstr, 0, 100);
    struct tm *temp;
    time_t now = time(NULL);
    temp = localtime(&now);
    strftime(outstr, sizeof(outstr), "%Y-%m-%d %H:%M:%S", temp);

    /* get usec */
    struct timeval now2;
    gettimeofday(&now2, NULL);
    memset(usec, 0, 9);
    sprintf(usec, ".%ld ", now2.tv_usec);

    /* get thread LWP id */
    pid_t pid = syscall(SYS_gettid);
    std::string threadStr = "- LWP: " + i2s(pid) + " ";

    log = std::string(outstr) + std::string(usec) + threadStr + log;

    OperContext *ctx = new OperContext(OperContext::OP_RECV);
    Msg * msg = new Msg;
    (*msg) << log;
    ctx->SetMessage(msg);
    assert(Enqueue(ctx));
    OperContext::DecRef(ctx);
}

void
ThreadLogger::HandleLog(OperContext *ctx)
{
    Msg * msg = ctx->GetMessage();
    std::string log;

    (*msg) >> log;
    ssize_t writed = 0;
    size_t toWrite = log.size(); 
    size_t hasWrited = 0;

    while (toWrite > 0)  {
        writed = write(mFileFd, 
                log.c_str() + hasWrited,
                toWrite);
        if (writed < 0) {
            fprintf(stderr, "Write Log File Failed!!!, err:%d\n", errno);
            break;
        } else {
            hasWrited += (size_t)writed;
            toWrite -= (size_t)writed;
        }
    }

    ++mLogCount;
    /* TODO  new file if file size is very big */

    delete msg;
    ctx->SetMessage(NULL);
}

