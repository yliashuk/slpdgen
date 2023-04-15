#include "CommonCpp.h"

using namespace CppConstructs;

std::string CppConstructs::PrintVarDeclaration(std::string varType, std::string varName,
                                               std::string value)
{
    return varType + spc +varName  + " = " + value + smcln;
}


std::string CppConstructs::PrintVarDeclaration(std::string varType, std::string varName)
{
    return varType + spc +varName + smcln;
}

std::string CppConstructs::PrintVarDefinition(std::string varName, std::string value)
{
    return varName  + " = " + value + smcln;
}

std::string CppConstructs::PrintInclude(std::string fileName, bool isLocalfile)
{
    std::string lb, rb;
    if(isLocalfile)
    {
        lb = '"';
        rb = '"';
    }
    else {
        lb = '<';
        rb = '>';
    }
    return std::string("#include") + lb + fileName + rb;
}

std::string CppConstructs::PrintMemcpy(std::string dst, std::string src, std::string len)
{
     return std::string("memcpy") + lsb +  dst + com + src + com + len + rsb + smcln;
}

std::string CppConstructs::PutInSqBraces(std::string var)
{
    return "[" + var + "]";
}

std::string CppConstructs::PrintSizeOf(std::string var)
{
    return "sizeof" + lsb + var + rsb;
}

std::string CppConstructs::PutInQuotes(string var)
{
    return '"' + var + '"';
}
