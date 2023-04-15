#include "StructCpp.h"

using CppConstructs::StructCpp;

StructCpp::StructCpp()
{

}

StructCpp::StructCpp(string structName)
{
    _structName = structName;
}

void StructCpp::SetName(string structName)
{
    _structName = structName;
}

string StructCpp::GetName() const
{
    return _structName;
}

void StructCpp::SetBody(vector<string> body)
{
    _body = body;
}

vector<string> StructCpp::GetDeclaration(bool hasTypeDef) const
{
    vector<string> content;
    string header;

    if(hasTypeDef)
         header = "typedef struct";
    else
        header = "struct " + _structName;


    if(this->_body.empty()) return content;
    content.push_back(header);
    content.push_back(lb);
    for(const auto &var: _body)
    {
             content.push_back(tab + var);
    }
    if(hasTypeDef)
        content.push_back(rb + _structName + smcln);
    else
        content.push_back(rb + smcln);

    return content;
}
