

#include "Log.hpp"
#include "CRC.h"

int main()
{
    int err = 0;
    mycrc32_init();
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

    return err;
}


