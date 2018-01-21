//
// Created by root on 1/21/18.
//

#ifndef PROJECT_KVREQUEST_HPP
#define PROJECT_KVREQUEST_HPP

#include <stdint.h>
#include <string>
#include "include/Closure.hpp"

class KVRequest
{
public:
    KVRequest()
    {}
    ~KVRequest()
    {}

public:
    enum _OpType {
        OP_put,
        OP_add,
        OP_del,
        OP_get,
        OP_drop_table,
        OP_add_table,
    }OpType;

public:
    uint32_t mOpType;
    std::string mTableName;
    std::string mKey;
    std::string mValue;
    SyncClosure mSync;
};


#endif //PROJECT_KVREQUEST_HPP
