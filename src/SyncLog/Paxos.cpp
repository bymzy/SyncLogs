

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

    mLeader.sid = 0;
    mLeader.epoch = 0;
    mLeader.logId = 0;
}

Paxoser::~Paxoser()
{
}

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
                || now == 0)) {

        /* 如果在 LEADER_ELECTION_TIME 时间内任然没有收到Leader 被选出来的消息,
         * 那么就重新发送Election 消息，可能之前被接收的Leader还没来得急发送Leader publish消息就挂了
         * */

        mLastElectionSentTime = time(NULL);
        mLeader.sid = 0;
        mLeader.epoch = 0;
        mLeader.logId = 0;

        Msg *msg = new Msg;
        (*msg) << MsgType::p2p_elect_leader;
        (*msg) << KVDB::Instance()->GetSelfSid();
        (*msg) << KVDB::Instance()->GetEpoch();
        (*msg) << KVDB::Instance()->GetMaxLogId();

        msg->SetLen();

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

    HandleElection(remoteSid, remoteEpoch, remoteMaxLogId);
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
            if (sid > mLeader.sid) {
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

    if (mElectionAccCount > KVDB::Instance()->GetConnectMgr()->GetQuorum()) {
        /* now i can send accpet message to peers */
        /* TODO */
    }
}



