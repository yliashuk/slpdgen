#include "EnumDescription.h"
#include "Utils/StringUtils.h"

using namespace Utils;

EnumDescription::EnumDescription(){}

SizeExprPtr EnumDescription::Size() const
{
    if(_bitWidth) return {Literal::Create(_bitWidth)};

    auto max = std::max_element(fields.begin(), fields.end(), [](auto a, auto b) {
        return a.second < b.second;
    })->second;

    if(max > 255) { return 16_lit; }
    else          { return 8_lit; }
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
        content << "}" + PrintType() + _name + ";"; }
    else {
        content << "}" + _prefix + _name + ";";
    }
    return content;
}

void EnumDescription::SetName(string name)
{
    this->_name = name;
}

string EnumDescription::GetName() const
{
    return this->_name;
}

void EnumDescription::SetPrefix(string prefix)
{
    _prefix = prefix + "_";
}

string EnumDescription::GetPrefix()
{
    return _prefix;
}

void EnumDescription::SetBitWidth(size_t size)
{
    _bitWidth = size;
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
