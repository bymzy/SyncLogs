//
// Created by root on 1/21/18.
//

#include <sstream>
#include <string>

#include "Err.hpp"
#include "KVDB.hpp"
#include "include/Cmdline.hpp"
#include "include/Log.hpp"
#include "include/Common.hpp"

KVDB *KVDB::instance = NULL;
pthread_mutex_t KVDB::mMutex;

int KVDB::Parse(int argc, char *argv[])
{
    int err = 0;
    cmdline::parser parser;

    parser.add<std::string>("peers", 'p', "db peers", true);
    parser.add<uint32_t>("sid", 's', "my sid", true);
    parser.add<std::string>("logdir", 'l', "log dir", true);
    parser.add<std::string>("datadir", 'a', "log dir", true);
    parser.add<std::string>("workdir", 'w', "work dir", false);
    parser.add("daemon", 'd', "run as a daemon");

    std::stringstream ss;
    ss << "SyncLog is distributed kv db server with Paxos" <<std::endl;

    parser.SetDesc(ss.str());
    parser.parse_check(argc, argv);

    size_t posStart = 0;
    size_t posEnd = std::string::npos;

    /* parse peers */
    std::string peers = parser.get<std::string>("peers");
    std::map<uint32_t, PeerInfo> peerMap;

    while (1) {
        bool needBreak = false;
        std::string peer;
        size_t pos1, pos2;
        std::string ip;
        short port;
        uint32_t sid;
        PeerInfo info;

        posEnd = peers.find(',', posStart);
        if (posEnd == std::string::npos) {
            peer = peers.substr(posStart, std::string::npos);
            needBreak = true;
            std::cout << "need break" << std::endl;
        } else {
            peer = peers.substr(posStart, posEnd - posStart);
        }

        /* parse single peer info */
        pos1 =  peer.find(':');
        pos2 =  peer.find(':', pos1 + 1);

        if (pos1 == std::string::npos || pos2 == std::string::npos) {
            err = Err::E_INVALID_CMD_LINE;
            break;
        }

        ip = peer.substr(0, pos1);
        port = s2i(peer.substr(pos1 + 1, pos2 - pos1 - 1));
        sid = s2i(peer.substr(pos2 + 1));

        std::cout << "peer: " << peer
            << ", ip: " << ip
            << ", port: " << port
            << ", sid: " << sid << std::endl;

        /*
        if (InvalidIP(ip) || InvalidPort(port)) {
            err = Err::E_INVALID_CMD_LINE;
            break;
        }
        */

        info.ip = ip;
        info.port = port;
        info.sid = sid;

        if (peerMap.find(sid) != peerMap.end()) {
            err = Err::E_INVALID_CMD_LINE;
            break;
        }
        peerMap.insert(std::make_pair(sid, info));

        if (needBreak) {
            break;
        }

        posStart = posEnd + 1;
    }

    do {
        if (err != 0) {
            break;
        }

        /* parse sid */
        uint32_t sid = parser.get<uint32_t>("sid");
        if (peerMap.find(sid) == peerMap.end()) {
            err = Err::E_INVALID_CMD_LINE_SID;
            break;
        }

        mConnectMgr.SetPeerInfo(peerMap);
        mConnectMgr.SetSelfSid(sid);

        mDataDir = parser.get<std::string>("datadir");
        mLogDir = parser.get<std::string>("logdir");
        mWorkDir = parser.get<std::string>("workdir");
        if (mWorkDir.empty()) {
            mWorkDir = "/";
        }

        std::cout << mDataDir << std::endl
            << mLogDir << std::endl
            << mWorkDir << std::endl;

        /* Daemon */
        if (parser.exist("daemon")) {
            std::cout << "run as a daemon!!!" << std::endl;
            daemon(0, 0);
        }

        chdir(mWorkDir.c_str());

    } while(0);

    return err;
}


