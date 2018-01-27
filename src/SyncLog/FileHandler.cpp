

#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <stdlib.h>

#include "FileHandler.hpp"
#include "include/Util.hpp"
#include "include/Common.hpp"
#include "include/Log.hpp"

int FileHandler::OpenFile(bool create)
{
    int err = 0;
    assert(-1 == mFD);

    do {
        if (create) {
            err = access(mFileName.c_str(), F_OK);
            if (0 == err) {
                err = EEXIST;
                break;
            }
            err = 0;

            /* create file and open file with 0644*/
            mFD = open(mFileName.c_str(), O_RDWR | O_APPEND | O_CREAT, 0644);
            if (mFD < -1) {
                err = errno;
                break;
            }
        } else {
            /* open file for read write */
            mFD = open(mFileName.c_str(), O_RDWR | O_APPEND);
            if (mFD < -1) {
                err = errno;
                break;
            }
        }
    } while(0);

    return err;
}

int FileHandler::Flush()
{
    int err = 0;
    err = WriteAllBufferToFile();
    assert(0 == err);

    err = Sync();
    assert(0  == err);

    return err;
}

int FileHandler::Sync()
{
    int err = 0;
    err = fsync(mFD);
    if (err < 0) {
        err = errno;
    }

    return err;
}

int FileHandler::Close()
{
    int err = 0;
    do {
        if (-1 == mFD) {
            break;
        }

        err = Flush();
        assert(0 == err);

        close(mFD);
        mFD = -1;

    } while(0);

    return err;
}

int FileHandler::ReadNBytes(char *buf, size_t toRead)
{
    int err = 0;
    ssize_t ret = 0;
    size_t readed = 0;

    while (toRead > 0) {
        ret = pread(mFD, buf + readed, toRead, mOffset);
        if (ret < 0) {
            err = errno;
            break;
        }
        if (ret == 0) {
            error_log("file length not match");
            err = EIO;
            break;
        }

        toRead -= ret;
        readed += ret;
        mOffset += ret;
    }

    return err;
}

int FileHandler::WriteAllBufferToFile()
{
    int err = 0;
    err = WriteToFile(mBuffer, mWriteOffset);
    assert(0 == err);
    mWriteOffset = 0;

    return err;
}

int FileHandler::WriteNBytes(const char *buf, size_t toWrite)
{
    do {
        if (toWrite > LOG_FILE_BUFFER_SIZE) {
            assert(0 == WriteAllBufferToFile());
            assert(0 == WriteToFile(buf, toWrite));
            break;
        } 

        if (LOG_FILE_BUFFER_SIZE < (mWriteOffset + toWrite)) {
            assert(0 == WriteAllBufferToFile());
        }

        /*LOG_FILE_BUFFER_SIZE > (mWriteOffset + toWrite)*/

        memcpy(mBuffer + mWriteOffset, buf, toWrite);
        mWriteOffset += toWrite;

    } while(0);

    return 0;
}

int FileHandler::WriteToFile(const char *buf, size_t toWrite)
{
    int err = 0;
    size_t writed = 0;
    ssize_t ret = 0;

    while (toWrite > 0) {
        ret = pwrite(mFD, buf + writed, toWrite, mOffset);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            err = errno;
            break;
        }

        toWrite -= ret;
        writed += ret;
        mOffset += ret;
    }

    if (0 != err) {
        error_log("WriteToFie failed, err: " << err
                << ", errstr: " << strerror(err));
    }
    return err;
}

int FileHandler::ReadCString(char **buf, uint32_t& size)
{
    int err = 0;
    uint32_t tempSize = 0;
    char *tempBuf = NULL;
    
    do {
        err = ReadNBytes((char *)&tempSize, 4);
        if (0 != err) {
            break;
        }
        tempSize = ntohl(tempSize);

        tempBuf = (char *)malloc(sizeof(char) * tempSize);
        assert(NULL != tempBuf);
        err = ReadNBytes(tempBuf, tempSize);
        if (0 != err) {
            break;
        }

        *buf = tempBuf;
        size = tempSize;
    } while(0);

    return err;
}


