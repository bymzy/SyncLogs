//
// Created by root on 1/21/18.
//

#ifndef PROJECT_KVDB_HPP
#define PROJECT_KVDB_HPP

#include <map>

#include "LogCenter.hpp"
#include "PersistLogger.hpp"
#include "RequestCenter.hpp"
#include "ConnectMgr.hpp"

class KVDB
{
public:
    KVDB(std::string dbDir):mPersistLogger(dbDir),mDataStore(),
        mLogCenter("LogCenter"), mRequestCenter("request center"),
        mConnectMgr("Connect Mgr")
    {}
    ~KVDB()
    {}

public:
    int Start() 
    {
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

            err = mPersistLogger.RecoverFromLog();
            if (0 != err) {
                break;
            }

            err = mConnectMgr.Start();
            if (0 != err) {
                break;
            }

            NotifyNewEpoch();

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
    /* TODO fix this singleton */
    static KVDB *Instance()
    {
        static KVDB kvdb("/tmp/log/");
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

    void SetEpoch(uint64_t epoch)
    {
        mDBEpoch = epoch;
    }

    void NotifyNewEpoch()
    {
        ++mDBEpoch;
    }

private:
    PersistLogger mPersistLogger;
    DataStore mDataStore;
    LogCenter mLogCenter;
    RequestCenter mRequestCenter;
    ConnectMgr mConnectMgr;
    uint64_t mDBEpoch;
};

#endif //PROJECT_KVDB_HPP


