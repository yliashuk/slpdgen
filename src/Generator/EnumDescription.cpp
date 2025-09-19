#include "EnumDescription.h"
#include "Utils/StringUtils.h"

using namespace Utils;

EnumDescription::EnumDescription(){}

SizeExprPtr EnumDescription::size() const
{
    if(_bitWidth) return {Literal::create(_bitWidth)};

    auto max = std::max_element(fields.begin(), fields.end(), [](auto a, auto b) {
        return a.second < b.second;
    })->second;

    if(max > 255) { return 16_lit; }
    else          { return 8_lit; }
}

vector<string> EnumDescription::declaration(bool withEnumText)
{
    vector<string> content;
    content << "typedef enum" << "{";

    for(auto var : fields)
    {
        content << fmt("\t%s%s = %s", {_prefix, var.first, to_string(var.second)});
        if(fields.back() != var) { content.back() += ","; }
    }
    if(withEnumText) {
        content << "}" + printType() + _name + ";"; }
    else {
        content << "}" + _prefix + _name + ";";
    }
    return content;
}

void EnumDescription::setName(string name)
{
    this->_name = name;
}

string EnumDescription::getName() const
{
    return this->_name;
}

void EnumDescription::setPrefix(string prefix)
{
    _prefix = prefix + "_";
}

string EnumDescription::getPrefix()
{
    return _prefix;
}

void EnumDescription::setBitWidth(size_t size)
{
    _bitWidth = size;
}

string EnumDescription::printType()
{
    switch (type)
    {
    case EnumerableType::Code: return _prefix + "CODE_";
    case EnumerableType::Enum: return _prefix + "enum_";
    case EnumerableType::Type: return _prefix + "TYPE_";
    default: return "";
    }
}
