

#include "FileLog.hpp"
#include "include/Log.hpp"
#include "include/CRC.h"
#include "include/ThreadLogger.hpp"
#include "PersistLogger.hpp"
#include "KVDB.hpp"
#include "include/Util.hpp"

ThreadLogger *g_logger = NULL;

int main()
{
    int err = 0;
    mycrc32_init();
    g_logger = new ThreadLogger("SyncLog");
    g_logger->Start();

    KVDB::Instance()->Start();

    KVRequest req;
    req.mOpType = KVRequest::OP_add_table;
    req.mTableName = "testtable";
    KVDB::Instance()->PostKVRequest(&req);

#if 0
    req.mOpType = KVRequest::OP_add;
    req.mTableName = "testtable";
    req.mKey = "aaa";
    req.mValue = "bbb";
    KVDB::Instance()->PostKVRequest(&req);
    std::cout << " add request should succeed!" << std::endl;
    std::cout << req.mErr << std::endl;

    req.mOpType = KVRequest::OP_add;
    req.mTableName = "testtable";
    req.mKey = "aaa";
    req.mValue = "bbb";
    KVDB::Instance()->PostKVRequest(&req);
    std::cout << " add request should failed!" << std::endl;
    std::cout << req.mErr << std::endl;

    req.mOpType = KVRequest::OP_get;
    req.mTableName = "testtable";
    req.mKey = "aaa";
    req.mValue = "";
    KVDB::Instance()->PostKVRequest(&req);
    std::cout << " get request should succeed!" << std::endl;
    std::cout << req.mErr << std::endl;
    std::cout << "value:" << req.mValue << std::endl;

    req.mOpType = KVRequest::OP_put;
    req.mTableName = "testtable";
    req.mKey = "aaa";
    req.mValue = "ccc";
    KVDB::Instance()->PostKVRequest(&req);
    std::cout << " put request should succeed!" << std::endl;
    std::cout << req.mErr << std::endl;

    req.mOpType = KVRequest::OP_get;
    req.mTableName = "testtable";
    req.mKey = "aaa";
    req.mValue = "";
    KVDB::Instance()->PostKVRequest(&req);
    std::cout << " get request should succeed! ccc" << std::endl;
    std::cout << req.mErr << std::endl;
    std::cout << "value:" << req.mValue << std::endl;

    req.mOpType = KVRequest::OP_del;
    req.mTableName = "testtable";
    req.mKey = "aaa";
    req.mValue = "";
    KVDB::Instance()->PostKVRequest(&req);
    std::cout << " del request should succeed!" << std::endl;
    std::cout << req.mErr << std::endl;

    req.mOpType = KVRequest::OP_get;
    req.mTableName = "testtable";
    req.mKey = "aaa";
    req.mValue = "";
    KVDB::Instance()->PostKVRequest(&req);
    std::cout << " get request should failed!" << std::endl;
    std::cout << req.mErr << std::endl;

    req.mOpType = KVRequest::OP_del;
    req.mTableName = "testtable";
    req.mKey = "aaa";
    req.mValue = "";
    KVDB::Instance()->PostKVRequest(&req);
    std::cout << " del request should succeed!" << std::endl;
    std::cout << req.mErr << std::endl;

#endif

    /* perf test */
    /* 1. 10000 write */
    std::string key;
    std::string value = "value";
    uint64_t begin = 0;
    uint64_t end = 0;

    begin = time_now();
    for (uint32_t i = 0; i < 100000; ++i) {
        req.mOpType = KVRequest::OP_put;
        req.mTableName = "testtable";
        req.mKey = i2s(i);
        req.mValue = value;

        KVDB::Instance()->PostKVRequest(&req);
        assert(req.mErr == 0);
    }
    end = time_now();
    std::cout << "100000 write costs " << (end - begin) << " usec " << std::endl;

    /* 2. 10000 read */
    begin = time_now();
    for (uint32_t i = 0; i < 100000; ++i) {
        req.mOpType = KVRequest::OP_get;
        req.mTableName = "testtable";
        req.mKey = i2s(i);

        KVDB::Instance()->PostKVRequest(&req);
        assert(req.mErr == 0);
    }
    end = time_now();
    std::cout << "100000 read costs " << (end - begin) << " usec " << std::endl;

    /* 3. 10000 del */
    begin = time_now();
    for (uint32_t i = 0; i < 100000; ++i) {
        req.mOpType = KVRequest::OP_del;
        req.mTableName = "testtable";
        req.mKey = i2s(i);

        KVDB::Instance()->PostKVRequest(&req);
        assert(req.mErr == 0);
    }
    end = time_now();
    std::cout << "100000 del costs " << (end - begin) << " usec "<< std::endl;

    KVDB::Instance()->Stop();
    g_logger->Stop();

    return err;
}


