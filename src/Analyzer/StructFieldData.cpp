#include "StructFieldData.h"

FieldInfoBuilder &FieldInfoBuilder::SetCommon(string type, string VarName)
{
    this->_FieldInfo.type = type;
    this->_FieldInfo.varName= VarName;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::SetLenDefiningVar(string str)
{
    this->_FieldInfo.lenDefiningVar = str;
    this->_FieldInfo.isNumOfCeils = true;
    this->_FieldInfo.withLenDefiningVar = true;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::SetLenDefiningVar(uint64_t num)
{
    this->_FieldInfo.constLenDefiningVar = num;
    this->_FieldInfo.isNumOfCeils = true;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::SetDefaultVal(uint64_t val)
{
    this->_FieldInfo.defaultVal = val;
    this->_FieldInfo.defaultValue = true;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::SetValRange(uint64_t fromVal, uint64_t toVal)
{
    this->_FieldInfo.fromVal = fromVal;
    this->_FieldInfo.toVal = toVal;
    this->_FieldInfo.valueRange = true;
    return *this;
}

FieldInfoBuilder &FieldInfoBuilder::SetSpecialType(string type)
{
    this->_FieldInfo.specialType = type;
    this->_FieldInfo.isWithSpecialType = true;
    return *this;
}

unique_ptr<FieldInfo> FieldInfoBuilder::Build()
{
    return unique_ptr<FieldInfo>(new FieldInfo(_FieldInfo));
}
