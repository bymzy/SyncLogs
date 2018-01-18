

#ifndef __LOG_HPP__
#define __LOG_HPP__

/* A single process key-value database
 * */

#include <stdint.h>
#include <string>
#include <sstream>

#include "FileHandler.hpp"
#include "Util.hpp"

#define LOG_MAGIC "LOGS"
#define LOG_VERSION 1

class LogRecordBody
{
public:
    LogRecordBody()
    {}
    ~LogRecordBody()
    {}

public:
    virtual int Encode(std::string& str) = 0;
    virtual int Decode(const std::string& str) = 0;

    void SetTableName(std::string val)
    {
        mTableName = val;
    }

    void GetTableName(std::string *val)
    {
        *val = mTableName;
    }

    void SetKey(std::string val)
    {
        mKey = val;
    }

    void GetKey(std::string *val)
    {
        *val = mKey;
    }

    void SetValue(std::string val)
    {
        mValue = val;
    }

    void GetValue(std::string *val)
    {
        *val = mValue;
    }

    std::string GetDumpString()
    {
        std::stringstream ss;
        ss << "log record, table name: " << mTableName
                << ", key: " << mKey
                << ", value: " << mValue;
        return ss.str();
    }

protected:
    std::string mTableName;
    std::string mKey;
    std::string mValue;
};

class FileLogRecordBody : public LogRecordBody
{
public:
    FileLogRecordBody():LogRecordBody()
    {}
    ~FileLogRecordBody()
    {}

public:
    virtual int Encode(std::string& str);
    virtual int Decode(const std::string& str);
};

class LogRecord
{
public:
    LogRecord()
    {
        mBody = NULL;
    }
    ~LogRecord()
    {}

public:
    int ReadFromFile(FileHandler *fh);
    int WriteToFile(FileHandler *fh);

public:
    void SetRecordBody(LogRecordBody *body)
    {
        if (NULL != mBody) {
            delete mBody;
        }
        mBody = body;
    }

    void SetLogId(uint32_t id)
    {
        mLogId = id;
    }

    void GetLogId(uint32_t* id)
    {
        *id = mLogId;
    }

    void SetOpType(uint32_t type)
    {
        mOpType = type;
    }

    void GetOpType(uint32_t *type)
    {
        *type = mOpType;
    }

    void Dump()
    {
        DEBUG_LOG("dump log record body, logid: " << mLogId
                << ", optype: " << mOpType
                << ", " << mBody->GetDumpString());
    }

private:
    uint32_t mBodyLength;
    uint32_t mCRC;
    uint32_t mLogId;
    uint32_t mOpType;
    LogRecordBody *mBody;
};

class Log
{
public:
    Log(std::string name, std::string magic = "", uint32_t version = 0):
        mFileHandler(name), mMagic(magic), mVersion(version)
    {
    }
    ~Log()
    {
    }

public:
    int GetNextLogRecord(LogRecord* pLogRecord);
    int OpenFile(bool create);
    int AppendLogRecord(LogRecord *pLogRecord);

private:
    int ValidateHeader();
    int WriteHeader();

private:
    FileHandler mFileHandler;
    std::string mMagic;
    uint32_t mVersion;
};

#endif


