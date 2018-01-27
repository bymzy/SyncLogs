

#ifndef __FILE_HANDLER_HPP__
#define __FILE_HANDLER_HPP__

#include <string>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

#include "Pch.hpp"

class FileHandler
{
public:
    FileHandler(std::string name):
        mFileName(name), mOffset(0)
    {
        mFD = -1;
        mBuffer = (char *)malloc(sizeof(char) * LOG_FILE_BUFFER_SIZE);
        assert(NULL != mBuffer);
        mWriteOffset = 0;
    }
    ~FileHandler()
    {
        if (-1 != mFD) {
            Close();
        }
    }

public:
    int OpenFile(bool create);
    int ReadNBytes(char *buf, size_t count);
    int WriteNBytes(const char *buf, size_t count);
    int ReadCString(char **buf, uint32_t& size);
    int Close();
    int Flush();
    off_t GetOffset()
    {
        return mOffset;
    }
    void SetOffset(off_t offset)
    {
        mOffset = offset;
    }

    int Truncate(off_t offset)
    {
        return ftruncate(mFD, offset);
    }

private:
    int WriteToFile(const char *buf, size_t count);
    int WriteAllBufferToFile();
    int Sync();

private:
    int mFD;
    std::string mFileName;
    off_t mOffset;
    char *mBuffer;
    uint32_t mWriteOffset;
};

#endif


