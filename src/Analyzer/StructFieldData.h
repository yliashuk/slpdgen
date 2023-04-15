#ifndef STRUCTFIELDDATABUILDER_H
#define STRUCTFIELDDATABUILDER_H

#include <memory>
using namespace std;

struct FieldInfo
{
    string type;
    string varName;
    string lenDefiningVar;
    string specialType;
    uint64_t constLenDefiningVar = 0;
    uint64_t defaultVal = 0;
    uint64_t fromVal = 0;
    uint64_t toVal = 0;

    bool isNumOfCeils       = false;
    bool withLenDefiningVar = false;
    bool defaultValue       = false;
    bool valueRange         = false;
    bool isWithSpecialType  = false; //"local" or "remote"

};

class FieldInfoBuilder
{
public:
    FieldInfoBuilder& SetCommon(string type, string VarName);
    FieldInfoBuilder& SetLenDefiningVar(string str);
    FieldInfoBuilder& SetLenDefiningVar(uint64_t num);
    FieldInfoBuilder& SetDefaultVal(uint64_t val);
    FieldInfoBuilder& SetValRange(uint64_t fromVal, uint64_t toVal);
    FieldInfoBuilder& SetSpecialType(string type);
    unique_ptr<FieldInfo> Build();
private:
    FieldInfo _FieldInfo;
};
#endif // STRUCTFIELDDATABUILDER_H
