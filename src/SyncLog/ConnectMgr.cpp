

#include "include/Context.hpp"
#include "ConnectMgr.hpp"

void ConnectMgr::SendMessage(uint64_t connId, Msg *msg)
{
    OperContext *ctx = new OperContext(OperContext::OP_SEND);
    ctx->SetMessage(msg);
    ctx->SetConnID(connId);
    mNetService.Enqueue(ctx);
    OperContext::DecRef(ctx);
}

int ConnectMgr::SendPeerMessage(uint32_t sid, Msg *msg)
{
    int err = 0;
    std::map<uint32_t, PeerInfo>::iterator iter;
    if (INVALID_SID == sid) {
        iter = mPeers.begin();
        for (;iter != mPeers.end(); ++iter) {
            SendMessage(iter->second.connId, msg->Dup());
        }
        delete msg;
        msg = NULL;
    } else {
        iter = mPeers.find(sid);
        SendMessage(iter->second.connId, msg);
    }

    return err;
}

void ConnectMgr::CheckConnection()
{
    int err = 0;
    uint64_t connId = 0;
    std::map<uint32_t, PeerInfo>::iterator iter = mPeers.begin();
    for (;iter != mPeers.end(); ++iter) {
        if (((!iter->second.connected) || 
                (iter->second.connId == 0)) &&
                (iter->second.sid != mSid)) {
            /* re connect */
            /* do not connect myself */

            err = mNetService.StartConnectRemote(iter->second.ip, iter->second.port,
                    connId);
            if (0 == err) {
                iter->second.connected = true;
                iter->second.connId = connId;
            } else {
                iter->second.connected = false;
                iter->second.connId = 0;
            }
        }
    }
}

void ConnectMgr::Idle()
{
    CheckConnection();
}

int ConnectMgr::Init()
{
    int err = 0;

    /* add listener */
    if (mSid != 0) {
        do {
            std::map<uint32_t, PeerInfo>::iterator iter;
            iter = mPeers.find(mSid);
            if (iter != mPeers.end()) {
                assert(0 == mNetService.AddListener(iter->second.ip, iter->second.port));
            }

            /* start NetService */
            err = mNetService.Start();
            if (0 != err) {
                break;
            }

            /* check conn */
            CheckConnection();
        } while(0);
    }

    return err;
}

int ConnectMgr::Finit()
{
    int err = 0;

    mNetService.Stop();

    return err;
}

bool ConnectMgr::Process(OperContext * ctx)
{
    bool processed = true;
    switch (ctx->GetType())
    {
        case OperContext::OP_DROP:
            /* TODO */
            break;
        default:
            processed = false;
            break;
    }

    return processed;
}


