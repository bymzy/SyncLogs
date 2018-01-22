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
    KVRequest():mErr(0), mOpType(OP_null)
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
        OP_null
    }OpType;

    void Signal()
    {
        mSync.Signal();
    }

    void Wait()
    {
        mSync.Wait();
    }

    std::string GetTableName()
    {
        return mTableName;
    }

    std::string GetKey()
    {
        return mKey;
    }

    std::string GetValue()
    {
        return mValue;
    }

    uint32_t GetOpType()
    {
        return mOpType;
    }

public:
    uint32_t mErr;
    uint32_t mOpType;
    std::string mTableName;
    std::string mKey;
    std::string mValue;
    SyncClosure mSync;
};


#endif //PROJECT_KVREQUEST_HPP
