#include "EnumDescription.h"
#include "Utils/StringUtils.h"

using namespace Utils;

EnumDescription::EnumDescription(){}

size_t EnumDescription::Size()
{
    size_t maxVal = 0;
    for(auto var: fields)
    {
        if(maxVal < var.second) {
            maxVal = var.second;
        }
    }
    if(maxVal < 255) { return 1; }
    else {return 2; }
}

vector<string> EnumDescription::Declaration(bool withEnumText)
{
    vector<string> content;
    content << "typedef enum" << "{";

    for(auto var : fields)
    {
        content << fmt("\t%s%s = %s", {_prefix, var.first, to_string(var.second)});
        if(fields.back() != var) { content.back() += ","; }
    }
    if(withEnumText) {
        content << "}" + PrintType() + name + ";"; }
    else {
        content << "}" + _prefix + name + ";";
    }
    return content;
}

void EnumDescription::SetName(string name)
{
    this->name = name;
}

string EnumDescription::GetName() const
{
    return this->name;
}

void EnumDescription::SetPrefix(string prefix)
{
    _prefix = prefix + "_";
}

string EnumDescription::GetPrefix()
{
    return _prefix;
}

string EnumDescription::PrintType()
{
    switch (type)
    {
    case EnumerableType::Code: return _prefix + "CODE_";
    case EnumerableType::Enum: return _prefix + "enum_";
    case EnumerableType::Type: return _prefix + "TYPE_";
    default: return "";
    }
}
