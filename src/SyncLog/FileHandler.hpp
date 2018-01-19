

#ifndef __FILE_HANDLER_HPP__
#define __FILE_HANDLER_HPP__

#include <string>
#include <stdint.h>
#include <unistd.h>

class FileHandler
{
public:
    FileHandler(std::string name):
        mFileName(name), mOffset(0)
    {
        mFD = -1;
    }
    ~FileHandler()
    {
        if (-1 != mFD) {
            close(mFD);
            mFD = -1;
        }
    }

public:
    int OpenFile(bool create);
    int ReadNBytes(char *buf, size_t count);
    int WriteNBytes(const char *buf, size_t count);
    int ReadCString(char **buf, uint32_t& size);

private:
    int mFD;
    std::string mFileName;
    off_t mOffset;
};

#endif


