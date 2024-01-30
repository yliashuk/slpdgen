#include "StructCpp.h"

using namespace std;
using namespace CppConstructs;

StructCpp::StructCpp(const String& structName)
{
    _structName = structName;
}

void StructCpp::SetTypeDef(bool state)
{
    _hasTypeDef = state;
}

void StructCpp::SetName(const String &structName)
{
    _structName = structName;
}

String StructCpp::GetName() const
{
    return _structName;
}

void StructCpp::AddField(const Field &field)
{
    _fields += field;
}

void StructCpp::AddFields(const Fields &fields)
{
    _fields += fields;
}

StructCpp::Fields StructCpp::GetFields()
{
    return _fields;
}

Strings StructCpp::Declaration() const
{
    String header = {(_hasTypeDef ? "typedef " : "") + "struct"s
            + (_hasTypeDef ? "" : " " + _structName)};
    String end = "}" + (_hasTypeDef ? _structName : "") + ";";

    return header << "{" << fmt("\t%s %s;%{ //%s}", _fields) << end;
}
