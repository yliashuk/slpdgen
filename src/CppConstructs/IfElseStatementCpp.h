#ifndef IFELSE_H
#define IFELSE_H
#include <string>
#include <vector>
#include "CommonCpp.h"
using namespace std;

namespace CppConstructs
{

    enum class IfElseStructure
    {
        If,
        IfElse
    };

    class IfElseStatementCpp
    {
    public:
        IfElseStatementCpp();
        IfElseStatementCpp &AddCase(string statement, vector<string> content);
        IfElseStatementCpp &AddCase(string statement, string content);

        vector<string> GetDefinition(IfElseStructure structure = IfElseStructure::If) const;

        void Clear();

    private:
        vector<pair<string,vector<string>>> _caseContent;
    };
}
#endif // IFELSE_H
