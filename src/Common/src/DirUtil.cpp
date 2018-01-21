

#include <errno.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>

#include "DirUtil.hpp"

int DirUtil::OpenDir()
{
    int err = 0;

    do {
        if (mDirName[mDirName.length() - 1] != '/') {
            mDirName.append("/");
        }

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

int DirUtil::GetAllFileWithSuffix(std::string suffix, std::set<std::string>& vec)
{
    struct dirent *entry;
    struct stat statbuf;
    uint32_t suffixLength = 0;
    uint32_t entryLength = 0;

    chdir(mDirName.c_str());
    while (NULL != (entry = readdir(mDp))) {
        lstat(entry->d_name, &statbuf);
        if (S_ISREG(statbuf.st_mode)) {

            suffixLength = suffix.length();
            entryLength = strlen(entry->d_name);

            if (strcmp(entry->d_name + entryLength - suffixLength, suffix.c_str()) == 0) {
                vec.insert(mDirName + std::string(entry->d_name));
            }
        }
    }
    chdir("/");

    return 0;
}


