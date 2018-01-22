//
// Created by root on 1/21/18.
//

#ifndef PROJECT_KVDB_HPP
#define PROJECT_KVDB_HPP

#include "LogCenter.hpp"
#include "PersistLogger.hpp"
#include "RequestCenter.hpp"

class KVDB
{
public:
    KVDB(std::string dbDir):mPersistLogger(dbDir),mDataStore(),
        mLogCenter("LogCenter"), mRequestCenter("request center")
    {}
    ~KVDB()
    {}

public:
    int Start() {
        int err = 0;
        do {
            err = mLogCenter.Start();
            if (0 != err) {
                break;
            }

            err = mRequestCenter.Start();
            if (0 != err) {
                break;
            }

        } while(0);

        return err;
    }

    int Stop()
    {
        mLogCenter.Stop();
        mRequestCenter.Stop();

        return 0;
    }

public:
    static KVDB *Instance()
    {
        static KVDB kvdb("/tmp/log");
        return &kvdb;
    }

    int Recover()
    {
        return mPersistLogger.RecoverFromLog();
    }

    void PostKVRequest(KVRequest *request)
    {
        mRequestCenter.EnqueueKVRequest(request);
    }

public:
    DataStore* GetDataStore()
    {
        return &mDataStore;
    }

    PersistLogger* GetPersistLogger()
    {
        return &mPersistLogger;
    }

    LogCenter* GetLogCenter()
    {
        return &mLogCenter;
    }

    RequestCenter* GetRequestCenter()
    {
        return &mRequestCenter;
    }

    uint64_t GetEpoch()
    {
        return mDBEpoch;
    }

private:
    PersistLogger mPersistLogger;
    DataStore mDataStore;
    LogCenter mLogCenter;
    RequestCenter mRequestCenter;
    uint64_t mDBEpoch;
};

#endif //PROJECT_KVDB_HPP


