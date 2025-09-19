#include "StructCpp.h"

using namespace std;
using namespace CppConstructs;

StructCpp::StructCpp(const String& structName)
{
    _structName = structName;
}

void StructCpp::setTypeDef(bool state)
{
    _hasTypeDef = state;
}

void StructCpp::setName(const String &structName)
{
    _structName = structName;
}

String StructCpp::getName() const
{
    return _structName;
}

void StructCpp::addField(const Field &field)
{
    _fields += field;
}

void StructCpp::addFields(const Fields &fields)
{
    _fields += fields;
}

StructCpp::Fields StructCpp::getFields()
{
    return _fields;
}

Strings StructCpp::declaration() const
{
    String header = {(_hasTypeDef ? "typedef " : "") + "struct"s
            + (_hasTypeDef ? "" : " " + _structName)};
    String end = "}" + (_hasTypeDef ? _structName : "") + ";";

    return header << "{" << fmt("\t%s %s%{ : %s};%{ //%s}", _fields) << end;
}
