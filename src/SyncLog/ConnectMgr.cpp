

#include "include/Context.hpp"
#include "ConnectMgr.hpp"
#include "KVDB.hpp"

int ConnectMgr::SendMessage(uint64_t connId, Msg *msg)
{
    int err = 0;
    OperContext *ctx = new OperContext(OperContext::OP_SEND);
    ctx->SetMessage(msg);
    ctx->SetConnID(connId);
    if (!mNetService.Enqueue(ctx)) {
        delete msg;
        ctx->SetMessage(NULL);
        err = EAGAIN;
    }
    OperContext::DecRef(ctx);

    return err;
}

int ConnectMgr::SendMessageToSelf(Msg *msg)
{
    int err = 0;
    OperContext *ctx = new OperContext(OperContext::OP_RECV);
    ctx->SetMessage(msg);
    ctx->SetConnID(0);
    if (!KVDB::Instance()->GetRequestCenter()->Enqueue(ctx)) {
        delete msg;
        ctx->SetMessage(NULL);
        err = EAGAIN;
    }
    OperContext::DecRef(ctx);

    return err;
}

int ConnectMgr::SendPeerMessage(uint32_t sid, Msg *msg)
{
    int err = 0;
    std::map<uint32_t, PeerInfo>::iterator iter;
    if (INVALID_SID == sid) {
        iter = mPeers.begin();
        for (;iter != mPeers.end(); ++iter) {
            if (iter->second.sid != mSid) {
                err = SendMessage(iter->second.connId, msg->Dup());
            } else {
                err = SendMessageToSelf(msg->Dup());
            }

            /* should not failed, once program is running for a while */
        }
        delete msg;
        msg = NULL;
    } else {
        if (sid != mSid) {
            iter = mPeers.find(sid);
            err = SendMessage(iter->second.connId, msg);
        } else {
            err = SendMessageToSelf(msg);
        }
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

            debug_log("try re connect!");
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
    //debug_log("Idle Idle Idle");
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


void ConnectMgr::HandleDrop(uint64_t connId, uint64_t *pSid)
{
    std::map<uint32_t, PeerInfo>::iterator iter = mPeers.begin();
    for (;iter != mPeers.end(); ++iter) {
        if (iter->second.connId == connId) {
            iter->second.connected = false;
            iter->second.connId = 0;
            *pSid = iter->second.sid;
            break;
        }
    }
}

uint32_t ConnectMgr::GetOnlinePeerCount()
{
    uint32_t count = 0;
    std::map<uint32_t, PeerInfo>::iterator iter = mPeers.begin();
    for (;iter != mPeers.end(); ++iter) {
        if (iter->second.connected == true) {
            ++count;
        }
    }

    return count;
}

bool ConnectMgr::Process(OperContext * ctx)
{
    bool processed = true;
    switch (ctx->GetType())
    {
        case OperContext::OP_DROP:
            {
                uint64_t *pSid = new uint64_t;
                HandleDrop(ctx->mConnID, pSid);
                ctx->SetArg((void *)pSid);
                KVDB::Instance()->GetRequestCenter()->Enqueue(ctx);
            }
            break;
        case OperContext::OP_RECV:
            KVDB::Instance()->GetRequestCenter()->Enqueue(ctx);
            break;
        default:
            processed = false;
            break;
    }

    return processed;
}


