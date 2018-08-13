//
// Created by root on 1/21/18.
//

#ifndef PROJECT_KVDB_HPP
#define PROJECT_KVDB_HPP

#include <map>
#include <pthread.h>

#include "LogCenter.hpp"
#include "PersistLogger.hpp"
#include "RequestCenter.hpp"
#include "ConnectMgr.hpp"

class KVDB
{
public:
    KVDB(std::string dbDir):mPersistLogger(dbDir),mDataStore(),
        mLogCenter("LogCenter"), mRequestCenter("request center"),
        mConnectMgr("Connect Mgr"), mIsLeader(false)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&mMutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    ~KVDB()
    {}

public:
    int Start() 
    {
        std::cout << "in start " << mDataDir << " " << mLogDir << std::endl;
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

            mPersistLogger.SetDir(mDataDir);
            err = mPersistLogger.RecoverFromLog();
            if (0 != err) {
                break;
            }

            err = mConnectMgr.StartWithoutNewThread();
            if (0 != err) {
                break;
            }

            //NotifyNewEpoch();

        } while(0);

        return err;
    }

    void SetPeerInfo(std::map<uint32_t, PeerInfo> peerMap)
    {
        mConnectMgr.SetPeerInfo(peerMap);
    }

    void SetSelfSid(uint32_t sid)
    {
        mConnectMgr.SetSelfSid(sid);
    }

    int Stop()
    {
        mLogCenter.DumpStats();
        mLogCenter.Stop();
        mRequestCenter.Stop();

        /* TODO stop order */
        mConnectMgr.Stop();

        return 0;
    }

public:
    static KVDB *Instance()
    {
        if (instance == NULL) {
            pthread_mutex_lock(&mMutex);
            if (instance == NULL) {
                instance = new KVDB("/tmp/log");
            }
            pthread_mutex_unlock(&mMutex);
        }
        return instance;
    }

    int Recover()
    {
        return mPersistLogger.RecoverFromLog();
    }

    int PostKVRequest(KVRequest *request)
    {
        if (mIsLeader) {
            mRequestCenter.EnqueueKVRequest(request);
            return 0;
        } else {
            return EAGAIN;
        }
    }

    int Parse(int argc, char *argv[]);

    void ChangeToLeader()
    {
        mIsLeader = true;
    }

    void ChangeToNone()
    {
        mIsLeader = false;
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

    ConnectMgr *GetConnectMgr()
    {
        return &mConnectMgr;
    }

    uint64_t GetEpoch()
    {
        return mDBEpoch;
    }
    
    uint32_t GetSelfSid()
    {
        return mConnectMgr.GetSelfSid();
    }

    uint64_t GetMaxLogId()
    {
        return mLogCenter.GetMaxLogId();
    }

    void SetEpoch(uint64_t epoch)
    {
        mDBEpoch = epoch;
    }

    void NotifyNewEpoch()
    {
        ++mDBEpoch;
    }

    std::string GetLogDir()
    {
        return mLogDir;
    }
    
    std::string GetDataDir()
    {
        return mDataDir;
    }

    std::string GetWorkDir()
    {
        return mWorkDir;
    }

private:
    PersistLogger mPersistLogger;
    DataStore mDataStore;
    LogCenter mLogCenter;
    RequestCenter mRequestCenter;
    ConnectMgr mConnectMgr;
    uint64_t mDBEpoch;
    std::string mDataDir;
    std::string mLogDir;
    std::string mWorkDir;
    bool mIsLeader;
    static pthread_mutex_t mMutex;
    static KVDB *instance;
};

#endif //PROJECT_KVDB_HPP


