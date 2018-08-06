

#ifndef __PAXOS_HPP__
#define __PAXOS_HPP__

/* Paxoser 依附于RequestCenter并不单独起线程
 *
 * 因此这就要求RequestCenter不仅要处理网络上的请求，还要处理内部request。
 * 但大都是将其分配给Paxoser进行处理
 *
 * */

class Proposer
{

};

class Acceptor
{

};

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
    Proposer * mProposer;
    Acceptor * mAcceptor;
};

#endif


