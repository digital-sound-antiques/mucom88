#pragma once

#include "../cmucom.h"
#include <string>

class MucomTag
{
public:
    MucomTag();
    void LoadTag(CMucom*);
    const char* GetTagInternal(CMucom* mucom, const char* tag);
    ~MucomTag();

    std::string composer;
    std::string title;
    std::string author;
    std::string time;
};

