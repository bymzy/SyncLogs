

#ifndef __PAXOS_HPP__
#define __PAXOS_HPP__

/* Paxoser 依附于RequestCenter并不单独起线程
 *
 * 因此这就要求RequestCenter不仅要处理网络上的请求，还要处理内部request。
 * 但大都是将其分配给Paxoser进行处理
 *
 * */

class Msg;

typedef struct _Ele{
    uint32_t sid;
    uint64_t epoch;
    uint64_t logId;
} Leader;

enum _Paxos_Role
{
    PAXOS_LEADER = 0,
    PAXOS_FOLLOWER,
    PAXOS_NONE
};

class Paxoser
{
public:
    Paxoser();
    ~Paxoser();

public:
    int StartElection();
    void ReceiveElectionMessage(Msg *msg);
    int HandleElection(uint32_t sid, uint64_t epoch, uint64_t logId);
    void ReceiveElectionMessageRes(Msg *msg);

    void SendElectionAccept();
    void ReceiveLeaderAcceptMessage(Msg *msg);
    void ReceiveLeaderAccpetMessageRes(Msg *msg);

    void PublishLeaderInfo();
    void SendLeaderInfo(uint32_t sid);
    void ReceiveLeaderPublishMessage(Msg *msg);
    void ReceiveLeaderPublishMessageRes(Msg *msg);

    void ResetCounter()
    {
        mElectionResCount = 0;
        mElectionAccCount = 0;
        mLeaderAcceptResCount = 0;
        mLeaderAcceptSuccCount = 0;
        mLeaderAcceptSent = false;
        mLeaderPublished = false;
    }

public:
    bool IsLeader()
    {
        return (mPaxosRole == PAXOS_LEADER);
    }

    bool NeedElection()
    {
        return (mPaxosRole == PAXOS_NONE);
    }

public:
    int mPaxosRole;
    time_t mLastElectionSentTime;

    /* for leader announcement */
    uint32_t mElectionResCount;
    uint32_t mElectionAccCount;

    /* for leader accept */
    uint32_t mLeaderAcceptResCount;
    uint32_t mLeaderAcceptSuccCount;

    bool mLeaderAcceptSent;
    bool mLeaderPublished;

    /* leader proposed by other peer */
    Leader mLeaderSeen;
    /* leader accepted */
    Leader mLeaderAccepted;
    /* leader proposed by myself */
    Leader mLeaderProposed;

    Leader mLeaderElected;
};

#endif


