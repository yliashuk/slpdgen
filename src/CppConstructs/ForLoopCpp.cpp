#include "ForLoopCpp.h"
#include "Utils/StringUtils.h"

using CppConstructs::ForLoopCpp;

ForLoopCpp::ForLoopCpp(){}

void ForLoopCpp::setDeclaration(string init, string condition, string increment)
{
    this->_init = init;
    this->_condition = condition;
    this->_increment = increment;
}

void ForLoopCpp::setBody(vector<string> body)
{
    _body = body;
}

vector<string> ForLoopCpp::definition() const
{
    if(this->_body.empty()) { return {};}
    vector<string> content;

    content << fmt("for(%s;%s;%s)", {_init, _condition, _increment});
    content << "{" << fmt("\t%s", _d(_body)) << "}";

    return content;
}
