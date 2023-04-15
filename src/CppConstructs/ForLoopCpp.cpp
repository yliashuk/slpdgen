#include "ForLoopCpp.h"

using CppConstructs::ForLoopCpp;

ForLoopCpp::ForLoopCpp(){}

void ForLoopCpp::SetDeclaration(string init, string condition, string increment)
{
    this->_init = init;
    this->_condition = condition;
    this->_increment = increment;
}

void ForLoopCpp::SetBody(vector<string> body)
{
    _body = body;
}

void ForLoopCpp::SetBody(string body)
{
    _body.clear();
    _body.push_back(body);
}

vector<string> ForLoopCpp::GetDefinition() const
{
    vector<string> content;
    if(this->_body.empty()) return content;

    content.push_back("for" + lsb + _init + smcln + _condition + smcln + _increment + rsb);
    content.push_back(lb);
    for(const auto &string: _body)
        content.push_back(tab + string);
    content.push_back(rb);
    return content;
}
