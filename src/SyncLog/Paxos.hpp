

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
    int HandleElection(uint32_t sid, uint64_t epoch, uint64_t logId);
    void ReceiveElectionMessage(Msg *msg);
    void ReceiveElectionMessageRes(Msg *msg);

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

    uint32_t mElectionResCount;
    uint32_t mElectionAccCount;
    /* TODO should init to myself after recover */
    Leader mLeader;
};

#endif


