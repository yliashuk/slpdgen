#ifndef STRUCTFIELDDATABUILDER_H
#define STRUCTFIELDDATABUILDER_H

#include <memory>
#include <string>
#include <optional>
using namespace std;

struct FieldInfo
{
    string type;
    string name;
    std::optional<string> sizeVar;
    std::optional<uint64_t> constantSize;
    std::optional<string> specialType; //"local" or "remote"
    std::optional<uint64_t> initValue;
    std::optional<uint64_t> fromVal;
    std::optional<uint64_t> toVal;

    bool IsArray() const;
    bool HasDynamicSize() const;
};

class FieldInfoBuilder
{
public:
    FieldInfoBuilder& SetCommon(string type, string VarName);
    FieldInfoBuilder& SetArraySize(string str);
    FieldInfoBuilder& SetArraySize(uint64_t num);
    FieldInfoBuilder& SetInitValue(uint64_t val);
    FieldInfoBuilder& SetValRange(uint64_t fromVal, uint64_t toVal);
    FieldInfoBuilder& SetSpecialType(string type);
    unique_ptr<FieldInfo> Build();
private:
    FieldInfo _FieldInfo;
};
#endif // STRUCTFIELDDATABUILDER_H
