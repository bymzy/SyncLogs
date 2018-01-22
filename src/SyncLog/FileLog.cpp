

#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>

#include "FileLog.hpp"
#include "include/Base64.hpp"
#include "include/CRC.h"
#include "include/Common.hpp"
#include "Err.hpp"


int Log::OpenFile(bool create)
{
    int err = 0;

    do {
        err = mFileHandler.OpenFile(create);
        if (0 != err) {
            break;
        }

        if (create) {
            err = WriteHeader();
        } else {
            err = ValidateHeader();
        }

    } while(0);

    return err;
}

int Log::Close()
{
    return mFileHandler.Close();
}

int Log::ValidateHeader()
{
    int err = 0;
    char magic[5];
    uint32_t version;
    memset(magic, 0, 5);

    do {
        err = mFileHandler.ReadNBytes(magic, 4);
        if (0 != err) {
            break;
        }

        if (0 != strcmp(LOG_MAGIC, magic)) {
            err = EINVAL;
            break;
        }

        err = mFileHandler.ReadNBytes((char *)&version, 4);
        if (0 != err) {
            break;
        }
        version = ntohl(version);

        if (LOG_VERSION != version) {
            err = EINVAL;
            break;
        }

        debug_log(magic << ", " << version);
    } while(0);

    return err;
}

int Log::WriteHeader()
{
    int err = 0;
    int temp = 0;

    do {
        err = mFileHandler.WriteNBytes(LOG_MAGIC, 4);
        if (0 != err) {
            break;
        }

        temp = htonl(LOG_VERSION);
        err = mFileHandler.WriteNBytes((const char *)&temp, 4);
        if (0 != err) {
            break;
        }

    } while(0);

    return err;
}

int Log::GetNextLogRecord(LogRecord *pLogRecord)
{
    int err = 0;
    err = pLogRecord->ReadFromFile(&mFileHandler);
    return err;
}

int Log::AppendLogRecord(LogRecord *pLogRecord)
{
    int err = 0;
    err = pLogRecord->WriteToFile(&mFileHandler);
    return err;
}

int LogRecord::ReadFromFile(FileHandler *fh)
{
    int err = 0;
    char *bodyBuf = NULL;
    std::string base64Str;
    std::string pureStr;
    off_t tempOffset = fh->GetOffset();

    do {
        err = fh->ReadNBytes((char *)&mBodyLength, 4);
        if (0 != err) {
            break;
        }
        mBodyLength = ntohl(mBodyLength);

        err = fh->ReadNBytes((char *)&mCRC, 4);
        if (0 != err) {
            break;
        }
        mCRC = ntohl(mCRC);

        err = fh->ReadNBytes((char *)&mLogId, 4);
        if (0 != err) {
            break;
        }
        mLogId = ntohl(mLogId);

        err = fh->ReadNBytes((char *)&mOpType, 4);
        if (0 != err) {
            break;
        }
        mOpType = ntohl(mOpType);

        bodyBuf = (char *)malloc(mBodyLength);
        assert(NULL != bodyBuf);
        err = fh->ReadNBytes(bodyBuf, mBodyLength);
        if (0 != err) {
            break;
        }

        /* check crc */
        if (mCRC != mycrc32(0, (const uint8_t *)bodyBuf, mBodyLength)) {
            err = Err::E_CRC_CHECK_FAILED;
            error_log("crc check failed!");
            break;
        }

        base64Str.assign(bodyBuf, mBodyLength);
        if (!Base64::Decode(base64Str, &pureStr)) {
            err = Err::E_BASE64_DECODE_FAILED;
            error_log("base64 decode failed!");
            break;
        }

        mBody = new FileLogRecordBody;
        err = mBody->Decode(pureStr);
        if (0 != err) {
            break;
        }

        debug_log("parse log record done, log id: " << mLogId);
    } while(0);

    if (NULL != bodyBuf) {
        free(bodyBuf);
        bodyBuf = NULL;
    }

    if (EIO == err) {
        debug_log("reach end of file, try truncate it!");
        fh->SetOffset(tempOffset);
        err = fh->Truncate(tempOffset);
    }

    return err;
}

int LogRecord::WriteToFile(FileHandler *fh)
{
    int err = 0;
    uint32_t tempValue = 0;
    std::string base64Str;
    std::string pureStr;
    
    do {
        mBody->Encode(pureStr);
        if (!Base64::Encode(pureStr, &base64Str)) {
            error_log("base64 encode failed!");
            break;
        }
        mBodyLength = base64Str.length();
        mCRC = mycrc32(0, (const uint8_t *)base64Str.c_str(), mBodyLength);

        tempValue = htonl(mBodyLength);
        err = fh->WriteNBytes((const char *)&tempValue, 4);
        if (0 != err) {
            break;
        }

        tempValue = htonl(mCRC);
        err = fh->WriteNBytes((const char *)&tempValue, 4);
        if (0 != err) {
            break;
        }

        tempValue = htonl(mLogId);
        err = fh->WriteNBytes((const char *)&tempValue, 4);
        if (0 != err) {
            break;
        }

        tempValue = htonl(mOpType);
        err = fh->WriteNBytes((const char *)&tempValue, 4);
        if (0 != err) {
            break;
        }

        err = fh->WriteNBytes(base64Str.c_str(), base64Str.length());
        if (0 != err) {
            break;
        }

    } while(0);

    return err;
}

int FileLogRecordBody::Decode(const std::string& data)
{
    readString(data.c_str(), mTableName);
    readString(data.c_str() + mTableName.length() + 4, mKey);
    readString(data.c_str() + mTableName.length() + 4
            + mKey.length() + 4, mValue);

    return 0;
}

int FileLogRecordBody::Encode(std::string& str)
{
    uint32_t tempSize = 0;

    tempSize = htonl(mTableName.length());
    str.append((char *)&tempSize, 4);
    str.append(mTableName.c_str(), mTableName.length());

    tempSize = htonl(mKey.length());
    str.append((char *)&tempSize, 4);
    str.append(mKey.c_str(), mKey.length());

    tempSize = htonl(mValue.length());
    str.append((char *)&tempSize, 4);
    str.append(mValue.c_str(), mValue.length());

    return 0;
}


