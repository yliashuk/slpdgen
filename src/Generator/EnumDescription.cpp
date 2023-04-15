#include "EnumDescription.h"

EnumDescription::EnumDescription()
{

}

size_t EnumDescription::Size()
{
    size_t maxVal = 0;
    for(auto var: fields)
        if(maxVal < var.second)
            maxVal = var.second;
    if(maxVal < 255)
        return 1;
    else
        return 2;
}

vector<string> EnumDescription::PrintDecl(bool withEnumText)
{
    vector<string> strings;
    strings.push_back("typedef enum");
    strings.push_back(string("{"));

    for(auto var : fields)
    {
        strings.push_back(string("\t") + _prefix + var.first + " = " +
                          to_string(var.second));
        if(fields.back() != var)
            strings.back() += ",";
    }
    if(withEnumText)
        strings.push_back(string("}" + PrintType() + name + ";"));
    else
        strings.push_back(string("}" + _prefix + name + ";"));
    strings.push_back("");
    return strings;
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
    _prefix = prefix + b_und;
}

string EnumDescription::GetPrefix()
{
   // if(_prefix.empty())
   //     return "";
    return _prefix /*+ b_und*/;
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
