

#ifndef __MSG_TYPE_HPP__
#define __MSG_TYPE_HPP__

class MsgType {
public:
    enum _type {
        /* 4 bytes msg type
         * 4 bytes sid
         * 8 bytes epoch 
         * 8 bytes max logid 
         *
         * one peer try announce that he is the leader
         * */
        p2p_elect_leader = 1,

        /* 4 bytes msg type
         * 4 bytes err
         * 
         * answer to leader announcement
         * */
        p2p_elect_leader_res,

        /* 4 bytes msg type
         * 4 bytes sid
         * 8 bytes epoch 
         * 8 bytes max logid 
         *
         * one peer ask others to accept that he is the leader
         * */
        p2p_elect_accept_leader,

        /* 4 bytes msg type
         * 4 bytes err
         * 4 bytes sid of msg sender
         * answer to leader accept
         * */
        p2p_elect_accept_leader_res,

        /* 4 bytes msg type
         * 4 bytes sid
         * 8 bytes epoch 
         * 8 bytes max logid 
         *
         * leader publish that himself
         * */
        p2p_elect_leader_publish,

        /* 4 bytes msg type
         * 4 bytes err
         *
         * TODO should send local info back for sync???
         * */
        p2p_elect_leader_publish_res
    };
};

#endif


