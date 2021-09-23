#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


class PathUtil {
public:

    PathUtil(const char *filename);

    void Split(const char* filename);

    const char* GetFilename();
    const char* GetDirectory();

    const char* GetWorkingDirectory();

private:
    char directoryName[_MAX_PATH];
    char fileName[_MAX_PATH];

    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char name[_MAX_FNAME];
    char ext[_MAX_EXT];
    char filedir[_MAX_PATH];

    char workDirectory[_MAX_PATH];
};