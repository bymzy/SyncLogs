

#ifndef __DIR_UTIL_HPP__
#define __DIR_UTIL_HPP__

#include <sys/types.h>
#include <dirent.h>

#include <string>
#include <set>

class DirUtil
{
public:
    DirUtil(std::string dirName):mDp(NULL),
        mDirName(dirName)
    {
    }
    ~DirUtil()
    {
        if (NULL != mDp) {
            closedir(mDp);
        }
    }

public:
    int OpenDir();
    struct dirent *GetNextEntry();
    int GetAllFileWithSuffix(std::string suffix, std::set<std::string>& fileSet);

public:
    DIR *mDp;
    std::string mDirName;
};

#endif


