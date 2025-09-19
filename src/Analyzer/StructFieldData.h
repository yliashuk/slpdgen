#ifndef STRUCTFIELDDATABUILDER_H
#define STRUCTFIELDDATABUILDER_H

#include <memory>
#include <string>
#include <optional>
using namespace std;

struct FieldInfo
{
    string name;
    string type;
    std::optional<string> sizeVar;
    std::optional<uint64_t> constantSize;
    std::optional<string> specialType; //"local" or "remote"
    std::optional<uint64_t> initValue;
    std::optional<uint64_t> fromVal;
    std::optional<uint64_t> toVal;

    bool isArray() const;
    bool hasDynamicSize() const;
};

class FieldInfoBuilder
{
public:
    FieldInfoBuilder& setCommon(string type, string VarName);
    FieldInfoBuilder& setArraySize(string str);
    FieldInfoBuilder& setArraySize(uint64_t num);
    FieldInfoBuilder& setInitValue(uint64_t val);
    FieldInfoBuilder& setValRange(uint64_t fromVal, uint64_t toVal);
    FieldInfoBuilder& setSpecialType(string type);
    unique_ptr<FieldInfo> build();
private:
    FieldInfo _FieldInfo;
};
#endif // STRUCTFIELDDATABUILDER_H
