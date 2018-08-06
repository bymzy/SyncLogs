

#ifndef __MSG_TYPE_HPP__
#define __MSG_TYPE_HPP__

class MsgType {
public:
    enum _type {
        /* 4 bytes msg type
         * 4 bytes sid
         * 8 bytes max logid 
         * 8 bytes epoch 
         * */
        p2p_elect_leader = 1,
    };
};

#endif


