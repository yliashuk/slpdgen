#ifndef CYCLEFORCONSTRUCTS_H
#define CYCLEFORCONSTRUCTS_H
#include <string>
#include <vector>
#include "CommonCpp.h"

using namespace std;

namespace CppConstructs
{
    class ForLoopCpp
    {
    public:
        ForLoopCpp();
        void SetDeclaration(string init, string condition, string increment);
        void SetBody(vector<string> body);
        void SetBody(string body);
        vector<string> GetDefinition() const;

    private:
        string _init;
        string _condition;
        string _increment;
        vector<string> _body;
    };
}
#endif // CYCLEFORCONSTRUCTS_H
