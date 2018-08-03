

#ifndef __PAXOS_HPP__
#define __PAXOS_HPP__

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
    int Prepare();

public:
    int mPaxosRole;
    Proposer * mProposer;
    Acceptor * mAcceptor;
};

#endif


