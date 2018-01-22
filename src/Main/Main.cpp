

#include "FileLog.hpp"
#include "include/Log.hpp"
#include "include/CRC.h"
#include "include/ThreadLogger.hpp"
#include "PersistLogger.hpp"
#include "KVDB.hpp"

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

    std::cout << req.mErr << std::endl;

    KVDB::Instance()->Stop();
    g_logger->Stop();
    return 0;

    {
        Log l("ddddd");
        err = l.OpenFile(true);
        if (0 != err) {
            return err;
        }

        {
            FileLogRecordBody *body = new FileLogRecordBody;
            body->SetTableName("Table");
            body->SetKey("key");
            body->SetValue("value");
            LogRecord rec;
            rec.SetLogId(1);
            rec.SetOpType(1);
            rec.SetRecordBody(body);

            err = l.AppendLogRecord(&rec);
            if (err != 0) {
                return 1;
            }
        }

        {
            FileLogRecordBody *body = new FileLogRecordBody;
            body->SetTableName("Table");
            body->SetKey("key");
            body->SetValue("value");
            LogRecord rec;
            rec.SetLogId(2);
            rec.SetOpType(1);
            rec.SetRecordBody(body);

            err = l.AppendLogRecord(&rec);
            if (err != 0) {
                return 1;
            }
        }
    }

    {
        Log l2("ddddd");
        err = l2.OpenFile(false);

        {
            LogRecord rec;
            l2.GetNextLogRecord(&rec);
            rec.Dump();
        }

        {
            LogRecord rec;
            l2.GetNextLogRecord(&rec);
            rec.Dump();
        }
    }

    /* create PersistLogger and parse log */
    g_logger->Stop();

    return err;
}


