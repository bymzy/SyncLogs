

#ifndef __DIR_UTIL_HPP__
#define __DIR_UTIL_HPP__

#include <sys/types.h>
#include <dirent.h>

#include <string>
#include <vector>

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
    int GetAllFileWithSuffix(std::string suffix, std::vector<std::string>& vec);

public:
    DIR *mDp;
    std::string mDirName;
};

#endif


