#ifndef APPOPTIONS_H
#define APPOPTIONS_H

#include "string"

struct AppOptions
{
    bool isJson;

    bool isC;
    bool isCpp;
    bool isQt;
    bool hasAddr;

    std::string fileName;
    std::string filePath;
};

#endif // APPOPTIONS_H
