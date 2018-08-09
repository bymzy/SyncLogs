

#ifndef __MSG_TYPE_HPP__
#define __MSG_TYPE_HPP__

class MsgType {
public:
    enum _type {
        /* 4 bytes msg type
         * 4 bytes sid
         * 8 bytes max logid 
         * 8 bytes epoch 
         *
         * one peer try announcement that he is the leader
         * */
        p2p_elect_leader = 1,

        /* 4 bytes msg type
         * 4 bytes err
         * 
         * answer to leader announcement
         * */
        p2p_elect_leader_res,
    };
};

#endif


