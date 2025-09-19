#ifndef IFELSE_H
#define IFELSE_H
#include <string>
#include <vector>

using namespace std;

namespace CppConstructs
{

    enum class ConditionType
    {
        If,
        IfElse
    };

    class ConditionCpp
    {
    public:
        ConditionCpp();
        ConditionCpp &addCase(string statement, vector<string> content);
        ConditionCpp &addCase(string statement, string content);

        vector<string> definition(ConditionType structure = ConditionType::If) const;

        void clear();

    private:
        vector<pair<string,vector<string>>> _caseContent;
    };
}
#endif // IFELSE_H
