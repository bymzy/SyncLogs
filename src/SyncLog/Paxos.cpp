

#include "include/Msg.hpp"
#include "include/Log.hpp"
#include "KVDB.hpp"
#include "Paxos.hpp"
#include "ConnectMgr.hpp"
#include "MsgType.hpp"

#define LEADER_ELECTION_TIME (5)

bool operator < (const Leader &left, const Leader &right)
{
    bool ret = false;

    if (left.epoch < right.epoch) {
        ret = true;
    } else if (left.epoch == right.epoch) {
        if (left.logId < right.logId) {
            ret = true;
        } else if (left.logId == right.logId) {
            if (left.sid < right.sid) {
                ret = true;
            }
        } else {
            ret = false;
        }
    } else {
        ret = false;
    }

    return ret;
}

void ResetLeader(Leader& l)
{
    l.sid = 0;
    l.epoch = 0;
    l.logId = 0;
}

void Paxoser::ResetElection()
{
    mElectionResCount = 0;
    mElectionAccCount = 0;
    mLeaderAcceptResCount = 0;
    mLeaderAcceptSuccCount = 0;
    mLeaderAcceptSent = false;
    mLeaderPublished = false;

    ResetLeader(mLeaderSeen);
    ResetLeader(mLeaderAccepted);
}

Paxoser::Paxoser():mPaxosRole(PAXOS_NONE)
{
    mLastElectionSentTime = 0;
    mElectionResCount = 0;
    mElectionAccCount = 0;

    mLeaderAcceptResCount = 0;
    mLeaderAcceptSuccCount = 0;

    mLeaderAcceptSent = false;
    mLeaderPublished = true;

    ResetLeader(mLeaderSeen);
    ResetLeader(mLeaderProposed);
    ResetLeader(mLeaderAccepted);
    ResetLeader(mLeaderElected);

    mLeaderElected.sid = 0;
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

        ResetElection();
        mLastElectionSentTime = time(NULL);
        ResetLeader(mLeaderElected);

        mLeaderProposed.sid = KVDB::Instance()->GetSelfSid();
        mLeaderProposed.epoch = KVDB::Instance()->GetEpoch();
        mLeaderProposed.logId = KVDB::Instance()->GetMaxLogId();

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
    } else if (mPaxosRole == PAXOS_LEADER){
        /* send him current leaer */
        SendLeaderInfo(remoteSid);
    } else {
        /* follower will ignore this message */
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

    Leader remote;
    remote.sid = sid;
    remote.epoch = epoch;
    remote.logId = logId;

    if (remote < mLeaderSeen) {
        err = 1;
    } else {
        err = 0;
        mLeaderSeen = remote;
    }

    Msg * msg = new Msg;
    (*msg) << MsgType::p2p_elect_leader_res;
    (*msg) << err;
    (*msg) << KVDB::Instance()->GetConnectMgr()->GetSelfSid();
    (*msg) << mLeaderAccepted.sid;
    (*msg) << mLeaderAccepted.epoch;
    (*msg) << mLeaderAccepted.logId;

    msg->SetLen();

    /* election message only sent from self and only publish self info */
    KVDB::Instance()->GetConnectMgr()->SendPeerMessage(sid, msg);

    return 0;
}

void Paxoser::ReceiveElectionMessageRes(Msg *msg)
{
    int err = 0;
    Leader remoteAccepted;
    uint32_t remoteSid;

    (*msg) >> err;
    (*msg) >> remoteSid;
    (*msg) >> remoteAccepted.sid;
    (*msg) >> remoteAccepted.epoch;
    (*msg) >> remoteAccepted.logId;

    //assert(sid == KVDB::Instance()->GetConnectMgr()->GetSelfSid());

    if (err == 0) {
        ++mElectionAccCount;
        if (mLeaderProposed < remoteAccepted) {
            mLeaderProposed = remoteAccepted;
        }
    }
    ++mElectionResCount;

    info_log("receive election res, err: " << err 
            << ", remote sid: " << remoteSid
            << ", remote max accepted, sid: " << remoteAccepted.sid
            << ", epoch: " << remoteAccepted.epoch
            << ", logId: " << remoteAccepted.logId);

    if (mElectionAccCount >= KVDB::Instance()->GetConnectMgr()->GetQuorum()) {
        /* TODO should we wait to see all 3 msg feed back ?? */

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
    msg << mLeaderProposed.sid;
    msg << mLeaderProposed.epoch;
    msg << mLeaderProposed.logId;
    msg.SetLen();

    debug_log("going to send accept to all peer, sid: " << mLeaderProposed.sid
            << ", epoch: " << mLeaderProposed.epoch
            << ", logId: " << mLeaderProposed.logId);

    KVDB::Instance()->GetConnectMgr()->SendPeerMessage(INVALID_SID, msg.Dup());
}

void Paxoser::ReceiveLeaderAcceptMessage(Msg *msg)
{
    int err = 0;
    Leader accept;

    (*msg) >> accept.sid;
    (*msg) >> accept.epoch;
    (*msg) >> accept.logId;

    if (accept < mLeaderSeen) {
        err = 1;
    } else {
        err = 0;
        mLeaderAccepted = accept;
    }

    warn_log("receive accpet leader,err: " << err
            << ", local leader seen, sid: " << mLeaderSeen.sid
            << ", epoch: " << mLeaderSeen.epoch
            << ", logId: " << mLeaderSeen.logId
            << ", remote leader, sid: " << accept.sid
            << ", epoch: " << accept.epoch
            << ", logId: " << accept.logId);

    Msg *resmsg = new Msg;
    (*resmsg) << MsgType::p2p_elect_accept_leader_res;
    (*resmsg) << err;
    (*resmsg) << KVDB::Instance()->GetSelfSid();
    resmsg->SetLen();

    KVDB::Instance()->GetConnectMgr()->SendPeerMessage(accept.sid, resmsg);
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

    info_log("peer " << sid << ", accept res: " << err);

    ++mLeaderAcceptResCount;

    if (mLeaderAcceptSuccCount >= KVDB::Instance()->GetConnectMgr()->GetQuorum()) {
        if (mLeaderProposed.sid == KVDB::Instance()->GetConnectMgr()->GetSelfSid()) {
            PublishLeaderInfo();
        }
    }
}

/* only leader will do this */
void Paxoser::PublishLeaderInfo()
{
    if (!mLeaderPublished &&
            mLeaderProposed.sid == KVDB::Instance()->GetConnectMgr()->GetSelfSid()) {

        mLeaderPublished = true;
        mLeaderElected = mLeaderProposed;
        info_log("i am leader!");
        ResetElection();

        mPaxosRole = PAXOS_LEADER;
        KVDB::Instance()->ChangeToLeader();

        Msg msg;
        msg << MsgType::p2p_elect_leader_publish;
        msg << mLeaderProposed.sid;
        msg << mLeaderProposed.epoch;
        msg << mLeaderProposed.logId;
        msg.SetLen();

        KVDB::Instance()->GetConnectMgr()->SendPeerMessage(INVALID_SID, msg.Dup());
    }
}

void Paxoser::SendLeaderInfo(uint32_t sid)
{
    Msg *msg = new Msg;
    (*msg) << MsgType::p2p_elect_leader_publish;
    (*msg) << mLeaderElected.sid;
    (*msg) << mLeaderElected.epoch;
    (*msg) << mLeaderElected.logId;
    msg->SetLen();

    KVDB::Instance()->GetConnectMgr()->SendPeerMessage(sid, msg);
}

void Paxoser::ReceiveLeaderPublishMessage(Msg *msg)
{
   Leader leader;
   (*msg) >> leader.sid;
   (*msg) >> leader.epoch;
   (*msg) >> leader.logId;


   if (leader.sid != KVDB::Instance()->GetConnectMgr()->GetSelfSid()) {
       mPaxosRole = PAXOS_FOLLOWER;
       debug_log("i am follower!!");
       ResetElection();
       mLeaderElected = leader;
   }

   Msg *resmsg = new Msg;
   (*resmsg) << MsgType::p2p_elect_leader_publish_res;
   (*resmsg) << KVDB::Instance()->GetConnectMgr()->GetSelfSid();

   resmsg->SetLen();
   
   KVDB::Instance()->GetConnectMgr()->SendPeerMessage(leader.sid, resmsg);
}

void Paxoser::ReceiveLeaderPublishMessageRes(Msg *msg)
{
    uint32_t sid; 
    (*msg) >> sid;

    info_log("receive leader publish res msg , peer " << sid);
}

void Paxoser::HandlePeerDrop(uint64_t sid)
{
    /* TODO, we may need to inform upper that leader is failed */
    if (!NeedElection()) {
        if (mLeaderElected.sid == sid) {
            /* leader drop */
            mPaxosRole = PAXOS_NONE;
        }

        if (mPaxosRole == PAXOS_LEADER) {
            /* if not qurom peer alive, we need to leader LEADER state */
            if (KVDB::Instance()->GetConnectMgr()->GetOnlinePeerCount() <
                    KVDB::Instance()->GetConnectMgr()->GetQuorum()) {
                mPaxosRole = PAXOS_NONE;
                KVDB::Instance()->ChangeToNone();
            }
        }
    }
}


