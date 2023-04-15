#include "IfElseStatementCpp.h"

using CppConstructs::IfElseStatementCpp;

IfElseStatementCpp::IfElseStatementCpp(){}

IfElseStatementCpp& IfElseStatementCpp::AddCase(string statement, vector<string> content)
{
    this->_caseContent.push_back({statement,content});
    return  *this;
}
IfElseStatementCpp& IfElseStatementCpp::AddCase(string statement, string content)
{
    vector<string> buffer;
    buffer.push_back(content);
    this->_caseContent.push_back({statement,buffer});
    return *this;
}

vector<string> IfElseStatementCpp::GetDefinition(IfElseStructure structure) const
{
    vector<string> content;
    if(this->_caseContent.empty()) return content;

    for(const auto &var: _caseContent)
    {
        if(_caseContent.front() == var || structure == IfElseStructure::If)
            content.push_back("if" + lsb + var.first + rsb);
        else
            content.push_back("else if" + lsb + var.first + rsb);

        content.push_back(lb);

        for(const auto &string: var.second)
            content.push_back(tab + string);

        content.push_back(rb);
    }
    return content;
}

void IfElseStatementCpp::Clear()
{
    this->_caseContent.clear();
}
