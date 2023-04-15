#ifndef COMMONCPP_H
#define COMMONCPP_H

#include<string>

using namespace std;

namespace CppConstructs
{

    const std::string eql = " == ";
    const std::string neql = " != ";
    const std::string orS = " || ";
    const std::string spc = " ";
    const std::string lb = "{";
    const std::string rb = "}";
    const std::string lsb = "(";
    const std::string rsb = ")";
    const std::string lsqb = "[";
    const std::string rsqb = "]";
    const std::string cln = ":";
    const std::string smcln = ";";
    const std::string com = ", ";
    const std::string b_und = "_";
    const std::string es = "";
    const std::string tab = "\t";
    const std::string tab2 = "\t\t";
    const std::string tab3 = "\t\t\t";
    const std::string tab4 = "\t\t\t\t";


    std::string PrintVarDeclaration(std::string varType, std::string varName);
    std::string PrintVarDeclaration(std::string varType, std::string varName,
                                    std::string value);
    std::string PrintVarDefinition(std::string varName, std::string value);

    std::string PrintInclude(std::string fileName, bool isLocalfile = false);
    std::string PrintMemcpy(std::string dst, std::string src, std::string len);
    std::string PrintSizeOf(std::string var);
    std::string PutInSqBraces(std::string var);
    std::string PutInQuotes(std::string var);

} // namespace CppConstructs

#endif // COMMONCPP_H

