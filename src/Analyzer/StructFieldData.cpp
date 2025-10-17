#include "StructFieldData.h"

bool FieldInfo::IsArray() const {return sizeVar || constantSize ;}

bool FieldInfo::HasDynamicSize() const { return sizeVar.has_value(); }

FieldInfoBuilder &FieldInfoBuilder::SetCommon(string type, string VarName)
{
    this->_FieldInfo.type = type;
    this->_FieldInfo.name= VarName;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::SetArraySize(string str)
{
    this->_FieldInfo.sizeVar = str;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::SetArraySize(uint64_t num)
{
    this->_FieldInfo.constantSize = num;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::SetInitValue(uint64_t val)
{
    this->_FieldInfo.initValue = val;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::SetValRange(uint64_t fromVal, uint64_t toVal)
{
    this->_FieldInfo.fromVal = fromVal;
    this->_FieldInfo.toVal = toVal;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::SetSpecialType(string type)
{
    this->_FieldInfo.specialType = type;
    return *this;
}

unique_ptr<FieldInfo> FieldInfoBuilder::Build()
{
    return unique_ptr<FieldInfo>(new FieldInfo(_FieldInfo));
}
