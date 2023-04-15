#include "SwitchCpp.h"

using CppConstructs::SwitchCpp;

SwitchCpp::SwitchCpp(){}

void SwitchCpp::SetSwitchingParameter(string parameter)
{
    this->_switchingParameter = parameter;
}

void SwitchCpp::AddCase(string switchValue, vector<string> content)
{
    this->_switchContent.push_back({switchValue,content});
}

vector<string> SwitchCpp::GetDeclaration() const
{
   vector<string> content;
   if(this->_switchContent.empty()) return content;
   content.push_back("switch" + lsb + _switchingParameter + rsb);
   content.push_back(lb);
   for(const auto &var: _switchContent)
   {
        content.push_back(tab + "case" + spc + var.first + cln);
        content.push_back(tab +lb);
        for(const auto &string: var.second)
        {
            content.push_back(tab2 + string);
        }
        content.push_back(tab2 + "break" + smcln);
        content.push_back(tab +rb);
   }
   content.push_back(rb);
   return content;
}
