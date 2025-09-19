#ifndef SWITCHOPERATOR_H
#define SWITCHOPERATOR_H
#include <string>
#include <vector>

using namespace std;

namespace CppConstructs
{

    class SwitchCpp
    {
    public:
        SwitchCpp();
        void setSwitchingParameter(string parameter);
        void addCase(string switchValue, vector<string> content);
        vector<string> declaration() const;

    private:
        string _switchingParameter;
        vector<pair<string,vector<string>>> _switchContent;
    };
}
#endif // SWITCHOPERATOR_H
