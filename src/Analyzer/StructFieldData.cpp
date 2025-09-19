#include "StructFieldData.h"

bool FieldInfo::isArray() const {return sizeVar || constantSize ;}

bool FieldInfo::hasDynamicSize() const { return sizeVar.has_value(); }

FieldInfoBuilder &FieldInfoBuilder::setCommon(string type, string VarName)
{
    this->_FieldInfo.type = type;
    this->_FieldInfo.name= VarName;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::setArraySize(string str)
{
    this->_FieldInfo.sizeVar = str;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::setArraySize(uint64_t num)
{
    this->_FieldInfo.constantSize = num;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::setInitValue(uint64_t val)
{
    this->_FieldInfo.initValue = val;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::setValRange(uint64_t fromVal, uint64_t toVal)
{
    this->_FieldInfo.fromVal = fromVal;
    this->_FieldInfo.toVal = toVal;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::setSpecialType(string type)
{
    this->_FieldInfo.specialType = type;
    return *this;
}

unique_ptr<FieldInfo> FieldInfoBuilder::build()
{
    return unique_ptr<FieldInfo>(new FieldInfo(_FieldInfo));
}
