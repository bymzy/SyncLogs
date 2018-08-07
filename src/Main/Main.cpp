

#include "FileLog.hpp"
#include "include/Log.hpp"
#include "include/CRC.h"
#include "include/ThreadLogger.hpp"
#include "include/Util.hpp"

#include <iostream>
#include <vector>

#include "KVDB.hpp"
#include "PersistLogger.hpp"

ThreadLogger *g_logger = NULL;

class TestWorker : public LogicService
{
public:
    TestWorker(uint32_t begin, uint32_t end):LogicService("test_worker"),
        mBegin(begin), mEnd(end)
    {
    }
    ~TestWorker()
    {
    }

public:
    virtual int Start()
    {
        mThread->Start(LogicService::thread_func, this);
        return 0;
    }

    virtual void Run()
    {
        std::cout << mBegin << " - " << mEnd << std::endl;
        std::string key;
        std::string value = "value";
        uint64_t begin = 0;
        uint64_t end = 0;
        KVRequest req;

        begin = time_now();
        for (uint32_t i = mBegin; i < mEnd; ++i) {
            req.mOpType = KVRequest::OP_put;
            req.mTableName = "testtable";
            req.mKey = i2s(i);
            req.mValue = value;

            KVDB::Instance()->PostKVRequest(&req);
            assert(req.mErr == 0);
        }
        end = time_now();
        std::cout <<  mBegin << "-" << mEnd << "10000 write costs " << (end - begin) << " usec " << std::endl;
    }

public:
    uint32_t mBegin;
    uint32_t mEnd;
};

int main(int argc, char *argv[])
{
    int err = 0;
    mycrc32_init();

    uint64_t begin = 0;
    uint64_t end = 0;

    begin = time_now();

    KVDB::Instance()->Parse(argc, argv);
    g_logger = new ThreadLogger("SyncLog", KVDB::Instance()->GetLogDir());
    std::cout << "aaa" << std::endl;
    std::cout << KVDB::Instance()->GetLogDir() << std::endl
        << KVDB::Instance()->GetDataDir() << std::endl;
    g_logger->Start();

#if 0
    /* make peer info */
    std::map<uint32_t , PeerInfo> peerMap;
    PeerInfo info;
    info.ip = "192.168.76.21";
    info.port = 1001;
    info.sid = 1;
    info.connected = false;
    info.connId = 0;

    peerMap.insert(std::make_pair(1, info));

    info.port = 1002;
    info.sid = 2;
    peerMap.insert(std::make_pair(2, info));

    info.port = 1003;
    info.sid = 3;
    peerMap.insert(std::make_pair(3, info));

    KVDB::Instance()->SetPeerInfo(peerMap);
    KVDB::Instance()->SetSelfSid(1);
#endif

    KVDB::Instance()->Start();
    end = time_now();
    std::cout <<  "recover 100000 write costs " << (end - begin) << " usec " << std::endl;

    KVRequest req;
    req.mOpType = KVRequest::OP_add_table;
    req.mTableName = "testtable";
    KVDB::Instance()->PostKVRequest(&req);
    std::cout << "create table " << req.mErr << std::endl;

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
    std::cout << " get request should succeed!" << std::endl;
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

#if 0
    /* perf test */
    /* 1. 10000 write */
    std::string key;
    std::string value = "value";
    uint64_t begin = 0;
    uint64_t end = 0;

    begin = time_now();
    for (uint32_t i = 0; i < 10000; ++i) {
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
    for (uint32_t i = 0; i < 10000; ++i) {
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
    for (uint32_t i = 0; i < 10000; ++i) {
        req.mOpType = KVRequest::OP_del;
        req.mTableName = "testtable";
        req.mKey = i2s(i);

        KVDB::Instance()->PostKVRequest(&req);
        assert(req.mErr == 0);
    }
    end = time_now();
    std::cout << "100000 del costs " << (end - begin) << " usec "<< std::endl;
#endif

#if 0
    std::vector<TestWorker*> vec;
    TestWorker * worker = NULL;
    for (uint32_t i = 0; i < 1; ++i) {
        worker = new TestWorker(i * 100000, (i + 1)* 100000);
        worker->Start();
        vec.push_back(worker);
    }

    for (uint32_t i = 0; i < vec.size(); ++i) {
        vec[i]->Join();
        delete vec[i];
    }

    end = time_now();
    std::cout << std::endl << "total 100000 write costs " << (end - begin) << " usec " << std::endl;
#endif
    sleep(100000);

    KVDB::Instance()->Stop();
    g_logger->Stop();

    return err;
}


