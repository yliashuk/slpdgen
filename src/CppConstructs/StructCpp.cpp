#include "StructCpp.h"

using namespace CppConstructs;


StructCpp::StructCpp(const string& structName)
{
    _structName = structName;
}

void StructCpp::SetTypeDef(bool state)
{
    _hasTypeDef = state;
}

void StructCpp::SetName(const string& structName)
{
    _structName = structName;
}

string StructCpp::GetName() const
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

Strings StructCpp::Declaration() const
{
    string header = (_hasTypeDef ? "typedef " : "") + "struct"s
            + (_hasTypeDef ? "" : " " + _structName);
    string end = "}" + (_hasTypeDef ? _structName : "") + ";";

    return header << "{" << fmt("\t%s %s;%{ //%s}", _fields) << end;
}
