#include "IfElseStatementCpp.h"
#include "Utils/StringUtils.h"

using CppConstructs::ConditionCpp;

ConditionCpp::ConditionCpp(){}

ConditionCpp& ConditionCpp::addCase(string statement, vector<string> content)
{
    this->_caseContent.push_back({statement,content});
    return  *this;
}
ConditionCpp& ConditionCpp::addCase(string statement, string content)
{
    vector<string> buffer;
    buffer.push_back(content);
    this->_caseContent.push_back({statement,buffer});
    return *this;
}

vector<string> ConditionCpp::definition(ConditionType structure) const
{
    vector<string> content;
    if(this->_caseContent.empty()) return content;

    for(const auto &var: _caseContent)
    {
        if(_caseContent.front() == var || structure == ConditionType::If)
        {
            content << fmt("if(%s)", {var.first});
        } else {
            content << fmt("else if(%s)", {var.first});
        }
        content << "{" << fmt("\t%s", _d(var.second)) << "}";
    }
    return content;
}

void ConditionCpp::clear()
{
    this->_caseContent.clear();
}
