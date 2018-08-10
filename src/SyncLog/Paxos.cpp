

#include "include/Msg.hpp"
#include "include/Log.hpp"
#include "KVDB.hpp"
#include "Paxos.hpp"
#include "ConnectMgr.hpp"
#include "MsgType.hpp"

#define LEADER_ELECTION_TIME (5)

Paxoser::Paxoser():mPaxosRole(PAXOS_NONE)
{
    mLastElectionSentTime = 0;
    mElectionResCount = 0;
    mElectionAccCount = 0;

    mLeaderAcceptResCount = 0;
    mLeaderAcceptSuccCount = 0;

    mLeaderAcceptSent = false;
    mLeaderPublished = true;

    mLeader.sid = 0;
    mLeader.epoch = 0;
    mLeader.logId = 0;

    mProposedLeader.sid = 0;
    mProposedLeader.epoch = 0;
    mProposedLeader.logId = 0;
}

Paxoser::~Paxoser()
{
}

/* 如果么有recover 完毕，那么就不应该发送Election消息 */
int Paxoser::StartElection()
{
    int err = 0;
    //debug_log("try start election");

    time_t now = time(NULL);

    if (now < mLastElectionSentTime) {
        mLastElectionSentTime = now;
    }

    /* 只有处于选举状态的peer才可能会发送选举消息 */
    if (mPaxosRole == PAXOS_NONE
            && (now >= (mLastElectionSentTime + LEADER_ELECTION_TIME) 
                || mLastElectionSentTime == 0)) {

        /* 如果在 LEADER_ELECTION_TIME 时间内任然没有收到Leader 被选出来的消息,
         * 那么就重新发送Election 消息，可能之前被接收的Leader还没来得急发送Leader publish消息就挂了
         * */

        ResetCounter();

        mLastElectionSentTime = time(NULL);
        mLeader.sid = 0;
        mLeader.epoch = 0;
        mLeader.logId = 0;

        mProposedLeader.sid = KVDB::Instance()->GetSelfSid();
        mProposedLeader.epoch = KVDB::Instance()->GetEpoch();
        mProposedLeader.logId = KVDB::Instance()->GetMaxLogId();

        Msg *msg = new Msg;
        (*msg) << MsgType::p2p_elect_leader;
        (*msg) << KVDB::Instance()->GetSelfSid();
        (*msg) << KVDB::Instance()->GetEpoch();
        (*msg) << KVDB::Instance()->GetMaxLogId();

        msg->SetLen();

        info_log("send leader election message to all peers!");
        err = KVDB::Instance()->GetConnectMgr()->SendPeerMessage(INVALID_SID, msg);
        if (err != 0) {
            error_log("SendPeerMessage failed, err: " << err);
        }
    }

    return err;
}

void Paxoser::ReceiveElectionMessage(Msg *msg)
{
    uint32_t remoteSid = 0;
    uint64_t remoteEpoch = 0;
    uint64_t remoteMaxLogId = 0;

    (*msg) >> remoteSid;
    (*msg) >> remoteEpoch;
    (*msg) >> remoteMaxLogId;

    /* if leader already selected */
    if (mPaxosRole == PAXOS_NONE) {
        HandleElection(remoteSid, remoteEpoch, remoteMaxLogId);
    }
}

int Paxoser::HandleElection(uint32_t sid, uint64_t epoch, uint64_t logId)
{
    int err = 0;
    
    debug_log("receive election message from remote sid: " << sid
            << ", epoch: " << epoch
            << ", maxlogid: " << logId
            << ", local sid: " << KVDB::Instance()->GetSelfSid()
            << ", epoch: " << KVDB::Instance()->GetEpoch()
            << ", maxlogid: " << KVDB::Instance()->GetMaxLogId());

    Msg msg;
    if (epoch > mLeader.epoch) {
        /* success */
        mLeader.epoch = epoch;
        mLeader.sid = sid;
        mLeader.logId = logId;
    } else if (epoch == mLeader.epoch) {
        if (logId > mLeader.logId) {
            /* success */
            mLeader.epoch = epoch;
            mLeader.sid = sid;
            mLeader.logId = logId;
        } else if (logId == mLeader.logId) {
            if (sid >= mLeader.sid) {
                /* success */
                mLeader.epoch = epoch;
                mLeader.sid = sid;
                mLeader.logId = logId;
            } else {
                err = 1;
            }
        } else {
            err = 1;
        }
    } else {
        /* fail */
        err = 1;
    }

    msg << MsgType::p2p_elect_leader_res;
    msg << err;
    msg << sid;
    msg.SetLen();

    /* election message only sent from self and only publish self info */
    KVDB::Instance()->GetConnectMgr()->SendPeerMessage(sid, msg.Dup());

    return 0;
}

void Paxoser::ReceiveElectionMessageRes(Msg *msg)
{
    int err = 0;
    uint32_t sid = 0;

    (*msg) >> err;
    (*msg) >> sid;

    assert(sid == KVDB::Instance()->GetConnectMgr()->GetSelfSid());

    if (err == 0) {
        ++mElectionAccCount;
    }
    ++mElectionResCount;

    info_log("receive election res, err: " << err);
    if (mElectionAccCount >= KVDB::Instance()->GetConnectMgr()->GetQuorum()) {
        /* now i can send accpet message to peers */
        if (!mLeaderAcceptSent) {
            mLeaderAcceptSent = true;
            SendElectionAccept();
        }
    }
}

void Paxoser::SendElectionAccept()
{
    Msg msg;
    msg << MsgType::p2p_elect_accept_leader;
#if 0
    msg << mLeader.sid;
    msg << mLeader.epoch;
    msg << mLeader.logId;
#endif
    msg << mProposedLeader.sid;
    msg << mProposedLeader.epoch;
    msg << mProposedLeader.logId;
    msg.SetLen();

    debug_log("going to send accept to all peer, sid: " << mLeader.sid
            << ", epoch: " << mLeader.epoch
            << ", logId: " << mLeader.logId);

    KVDB::Instance()->GetConnectMgr()->SendPeerMessage(INVALID_SID, msg.Dup());
}

void Paxoser::ReceiveLeaderAcceptMessage(Msg *msg)
{
    int err = 0;
    uint32_t sid = 0;
    uint64_t epoch = 0;
    uint64_t logId = 0;

    (*msg) >> sid;
    (*msg) >> epoch;
    (*msg) >> logId;

#if 0
    if (sid == mLeader.sid && epoch == mLeader.epoch && logId == mLeader.logId) {
        err = 0;
        info_log("receive accept leader success, leader sid: " << sid
                << ", epoch: " << epoch
                << ", logId: " << logId);
    } else {
        err = 1;
        warn_log("receive accpet leader failed, local mLeader, sid: " << mLeader.sid
                << ", epoch: " << mLeader.epoch
                << ", logId: " << mLeader.logId
                << ", remote leader, sid: " << sid
                << ", epoch: " << epoch
                << ", logId: " << logId);
    }
#endif

    if (epoch == mLeader.epoch) {
        if (logId < mLeader.logId) {
            err = 1;
        } else if (logId == mLeader.logId) {
            if (sid < mLeader.sid) {
                err = 1;
            }
        }
    } else if (epoch < mLeader.epoch) {
        err = 1;
    }

    Msg *resmsg = new Msg;
    (*resmsg) << MsgType::p2p_elect_accept_leader_res;
    (*resmsg) << err;
    (*resmsg) << KVDB::Instance()->GetSelfSid();
    resmsg->SetLen();

    KVDB::Instance()->GetConnectMgr()->SendPeerMessage(sid, resmsg);
}

void Paxoser::ReceiveLeaderAccpetMessageRes(Msg *msg)
{
    int err = 0;
    uint32_t sid = 0;

    (*msg) >> err;
    (*msg) >> sid; /* sid of this message sender */

    if (err == 0) {
        ++mLeaderAcceptSuccCount;
    }

    ++mLeaderAcceptResCount;

    if (mLeaderAcceptSuccCount >= KVDB::Instance()->GetConnectMgr()->GetQuorum()) {
        PublishLeaderInfo();
    }
}

void Paxoser::PublishLeaderInfo()
{
    if (!mLeaderPublished) {
        mLeaderPublished = true;
        info_log("i am leader!");

        mPaxosRole = PAXOS_LEADER;

        Msg msg;
        msg << MsgType::p2p_elect_leader_publish;
        msg << KVDB::Instance()->GetConnectMgr()->GetSelfSid();
        msg << KVDB::Instance()->GetEpoch();
        msg << KVDB::Instance()->GetMaxLogId();

        msg.SetLen();

        KVDB::Instance()->GetConnectMgr()->SendPeerMessage(INVALID_SID, msg.Dup());
    }
}

void Paxoser::ReceiveLeaderPublishMessage(Msg *msg)
{
   (*msg) >> mLeader.sid;
   (*msg) >> mLeader.epoch;
   (*msg) >> mLeader.logId;

   if (mLeader.sid != KVDB::Instance()->GetConnectMgr()->GetSelfSid()) {
       mPaxosRole = PAXOS_FOLLOWER;
       debug_log("i am follower!!");
   }

   Msg *resmsg = new Msg;
   (*resmsg) << MsgType::p2p_elect_leader_publish_res;
   (*resmsg) << KVDB::Instance()->GetConnectMgr()->GetSelfSid();

   resmsg->SetLen();
   
   KVDB::Instance()->GetConnectMgr()->SendPeerMessage(mLeader.sid, resmsg);
}

void Paxoser::ReceiveLeaderPublishMessageRes(Msg *msg)
{
    uint32_t sid; 
    (*msg) >> sid;

    info_log("receive leader publish res msg , peer " << sid);
}


