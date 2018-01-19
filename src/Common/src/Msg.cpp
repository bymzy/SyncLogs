

#include <arpa/inet.h>
#include <string.h>
#include "Msg.hpp"
#include "Common.hpp"
#include "Log.hpp"

pthread_mutex_t Msg::lock = PTHREAD_MUTEX_INITIALIZER;

Msg& Msg::operator << (const int& r)
{
    int temp = htonl(r);
    if (LeftSize() < sizeof(r)) {
        Reverse(sizeof(r));
    }
    memcpy(mData + HEAD_LENGTH +  mWriteOffset, &temp, sizeof(int));
    mWriteOffset += sizeof(int);
    return *this;
}

Msg& Msg::operator << (const std::string& r)
{
    this->operator<<(r.length());
    return WriteByte(r.c_str(), r.length());
}

Msg&
Msg::WriteByte(const char *r, uint64_t len)
{
    if (LeftSize() < len) {
        Reverse(len);
    }
    memcpy(mData + HEAD_LENGTH + mWriteOffset, r, len);
    mWriteOffset += len;
    return *this;
}

Msg&
Msg::operator << (const uint16_t& r)
{
    uint16_t temp = htons(r);
    if (LeftSize() < sizeof(r)) {
        Reverse(sizeof(r));
    }

    memcpy(mData + HEAD_LENGTH + mWriteOffset, &temp, sizeof(r));
    mWriteOffset += sizeof(r);
    return *this;
}

Msg&
Msg::operator << (const uint32_t& r)
{
    uint32_t temp = htonl(r);
    if (LeftSize() < sizeof(r)) {
        Reverse(sizeof(r));
    }

    memcpy(mData + HEAD_LENGTH + mWriteOffset, &temp, sizeof(r));
    mWriteOffset += sizeof(r);
    return *this;
}

Msg&
Msg::operator << (const uint64_t& r)
{
    uint64_t temp = htonll(r);
    if (LeftSize() < sizeof(r)) {
        Reverse(sizeof(r));
    }

    memcpy(mData + HEAD_LENGTH + mWriteOffset, &temp, sizeof(r));
    mWriteOffset += sizeof(r);
    return *this;
}

Msg& Msg::operator >> (int& r)
{
    int temp = 0;
    memcpy(&temp, mData + HEAD_LENGTH + mReadOffset, sizeof(int));
    r = ntohl(temp);
    mReadOffset += sizeof(int);
    return *this;
}

Msg& Msg::operator >> (std::string& r)
{
    uint64_t len = 0;
    this->operator>>(len);
    r.assign(mData + HEAD_LENGTH + mReadOffset, len);
    mReadOffset += len;
    return *this;
}

Msg& Msg::ReadByte(char *r, uint64_t len)
{
    memcpy(r, mData + HEAD_LENGTH + mReadOffset, len);
    mReadOffset += len;
    return *this;
}

Msg& 
Msg::operator >> (uint16_t& r)
{
    uint16_t temp = 0;
    memcpy(&temp, mData + HEAD_LENGTH + mReadOffset, sizeof(r));
    r = ntohs(temp);
    mReadOffset += sizeof(r);
    return *this;
}

Msg& 
Msg::operator >> (uint32_t& r)
{
    uint32_t temp = 0;
    memcpy(&temp, mData + HEAD_LENGTH + mReadOffset, sizeof(r));
    r = ntohl(temp);
    mReadOffset += sizeof(r);
    return *this;
}

Msg& 
Msg::operator >> (uint64_t& r)
{
    uint64_t temp = 0;
    memcpy(&temp, mData + HEAD_LENGTH + mReadOffset, sizeof(r));
    r = ntohll(temp);
    mReadOffset += sizeof(r);
    return *this;
}




