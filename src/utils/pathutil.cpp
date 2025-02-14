// pathutil 
// BouKiCHi 2021


#ifndef _WIN32
#include <unistd.h>
#define _CHDIR chdir
#else
#include <direct.h>
#define _CHDIR _chdir
#endif

#include "pathutil.h"

#include <string>

PathUtil::PathUtil() {}

PathUtil::PathUtil(char* filename)
{
    Split(filename);
}

void PathUtil::Split(const char* filename) {
    std::string path(filename);
    if (path.find("file://") == 0) {
        path = path.substr(7);
    }

    _splitpath(path.c_str(), drive, dir, name, ext);
    _makepath(directoryName, drive, dir, NULL, NULL);
    _makepath(fileName, NULL, NULL, name, ext);
}

const char * PathUtil::GetFilename() {
    return fileName;
}

const char* PathUtil::GetDirectory()
{
    return directoryName;
}

const char* PathUtil::GetWorkingDirectory() {
    _getcwd(workDirectory, _MAX_PATH);
    return workDirectory;
}


