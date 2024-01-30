#include "SwitchCpp.h"
#include "Utils/StringUtils.h"

using CppConstructs::SwitchCpp;

SwitchCpp::SwitchCpp(){}

void SwitchCpp::SetSwitchingParameter(string parameter)
{
    this->_switchingParameter = parameter;
}

void SwitchCpp::AddCase(string switchValue, vector<string> content)
{
    this->_switchContent.push_back({switchValue, content});
}

vector<string> SwitchCpp::Declaration() const
{
    if(this->_switchContent.empty()){ return {}; }

   vector<string> content;

   content << fmt("switch(%s)\n\t{", {_switchingParameter});
   for(const auto &var: _switchContent)
   {
       content << fmt("\tcase %s:\n\t\t{", {var.first}) <<
                  fmt("\t\t%s", _d(var.second)) <<
                  "\t\tbreak;\n\t\t}";
   }
   content << "}";
   return content;
}
