#ifndef SWITCHOPERATOR_H
#define SWITCHOPERATOR_H
#include <string>
#include <vector>
#include "CommonCpp.h"
using namespace std;

namespace CppConstructs
{

    class SwitchCpp
    {
    public:
        SwitchCpp();
        void SetSwitchingParameter(string parameter);

        void AddCase(string switchValue, vector<string> content);

        vector<string> GetDeclaration() const;

    private:
        string _switchingParameter;
        vector<pair<string,vector<string>>> _switchContent;
    };
}
#endif // SWITCHOPERATOR_H
