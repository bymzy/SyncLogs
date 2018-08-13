

#ifndef __CONNECT_MGR_HPP__
#define __CONNECT_MGR_HPP__

#include <map>

#include "include/LogicService.hpp"
#include "include/NetService.hpp"
#include "include/Msg.hpp"

#define INVALID_SID 0xFFFFFFFF

typedef struct _PeerInfo {
    std::string ip;
    uint16_t port;
    uint32_t sid;
    bool connected; /* should be initialized with false */
    uint64_t connId; /* should be initialized with 0 */
}PeerInfo;

class ConnectMgr : public LogicService
{
public:
    ConnectMgr(std::string name):
        LogicService(name, 500000000), mNetService(this, "NetService"),
        mSid(0)
    {
    }
    ~ConnectMgr()
    {
    }

public:
    /* Check Peer Connection */
    virtual void Idle();
    virtual int Init();
    virtual int Finit();
    virtual bool Process(OperContext *ctx);

public:
    void SetPeerInfo(std::map<uint32_t, PeerInfo> peerMap)
    {
        assert(peerMap.size() % 2 == 1);
        mPeers = peerMap;
    }

    void SetSelfSid(uint32_t sid)
    {
        mSid = sid;
    }

    uint32_t GetSelfSid()
    {
        return mSid;
    }

    uint32_t GetQuorum()
    {
        return mPeers.size() / 2 + 1;
    }

    void CheckConnection();
    void HandleDrop(uint64_t connId, uint64_t *pSid);
    int SendPeerMessage(uint32_t sid, Msg *msg);
    uint32_t GetOnlinePeerCount();

private:
    int SendMessage(uint64_t connId, Msg *msg);
    int SendMessageToSelf(Msg *msg);

public:
    NetService mNetService;
    std::map<uint32_t, PeerInfo> mPeers;
    uint32_t mSid;
};

#endif


