

#include <errno.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>

#include "Log.hpp"

#include "DirUtil.hpp"

int DirUtil::OpenDir()
{
    int err = 0;

    do {
        mDp= opendir(mDirName.c_str());
        if (mDp == NULL) {
            err = errno;
            break;
        }
    } while(0);

    return err;
}

struct dirent *DirUtil::GetNextEntry()
{
    return readdir(mDp);
}

int DirUtil::GetAllFileWithSuffix(std::string suffix, std::vector<std::string>& vec)
{
    struct dirent *entry;
    struct stat statbuf;
    uint32_t suffixLength = 0;

    while (NULL != (entry = readdir(mDp))) {
        lstat(entry->d_name, &statbuf);
        if (S_ISREG(statbuf.st_mode)) {
            suffixLength = suffix.length();
            
            debug_log(entry->d_name);
            if (strcmp(entry->d_name - suffixLength + 1, suffix.c_str()) == 0) {
                vec.push_back(std::string(entry->d_name));
            }
        }
    }

    return 0;
}


