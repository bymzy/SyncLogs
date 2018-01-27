

#ifndef __FILE_LOG_HPP__
#define __FILE_LOG_HPP__

#include <stdint.h>
#include <string>
#include <sstream>

#include "include/Util.hpp"
#include "include/Log.hpp"
#include "FileHandler.hpp"

#define LOG_MAGIC "LOGS"
#define LOG_VERSION 1

class LogRecordBody
{
public:
    LogRecordBody()
    {}
    virtual ~LogRecordBody()
    {}

public:
    virtual int Encode(std::string& str) = 0;
    virtual int Decode(const std::string& str) = 0;

    void SetTableName(std::string val)
    {
        mTableName = val;
    }

    std::string GetTableName()
    {
        return mTableName;
    }

    void SetKey(std::string val)
    {
        mKey = val;
    }

    std::string GetKey()
    {
        return mKey;
    }

    void SetValue(std::string val)
    {
        mValue = val;
    }

    std::string GetValue()
    {
        return mValue;
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
    virtual ~FileLogRecordBody()
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
    {
        if (NULL != mBody) {
            delete mBody;
            mBody = NULL;
        }
    }

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

    LogRecordBody *GetRecordBody()
    {
        return mBody;
    }

    void SetLogId(uint32_t id)
    {
        mLogId = id;
    }

    uint32_t GetLogId()
    {
        return mLogId;
    }

    void SetOpType(uint32_t type)
    {
        mOpType = type;
    }

    uint32_t GetOpType()
    {
        return mOpType;
    }

    std::string GetDumpString()
    {
        std::stringstream ss;
        ss << "dump log record body, logid: " << mLogId
                << ", optype: " << mOpType
                << ", " << mBody->GetDumpString();
        return ss.str();
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
    int GetNextLogRecord(LogRecord** ppLogRecord);
    int OpenFile(bool create);
    int AppendLogRecord(LogRecord *pLogRecord);
    int Close();
    int Flush();

private:
    int ValidateHeader();
    int WriteHeader();

private:
    FileHandler mFileHandler;
    std::string mMagic;
    uint32_t mVersion;
};

#endif


