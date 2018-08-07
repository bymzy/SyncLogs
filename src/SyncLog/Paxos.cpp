

#include "include/Msg.hpp"
#include "include/Log.hpp"
#include "KVDB.hpp"
#include "Paxos.hpp"
#include "ConnectMgr.hpp"
#include "MsgType.hpp"

Paxoser::Paxoser():mPaxosRole(PAXOS_NONE)
{
}

Paxoser::~Paxoser()
{
}

int Paxoser::StartElection()
{
    int err = 0;
    //debug_log("try start election");

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

    return err;
}


