#ifndef CYCLEFORCONSTRUCTS_H
#define CYCLEFORCONSTRUCTS_H
#include <string>
#include <vector>

using namespace std;

namespace CppConstructs
{
    class ForLoopCpp
    {
    public:
        ForLoopCpp();
        void SetDeclaration(string init, string condition, string increment);
        void SetBody(vector<string> body);
        vector<string> Definition() const;

    private:
        string _init;
        string _condition;
        string _increment;
        vector<string> _body;
    };
}
#endif // CYCLEFORCONSTRUCTS_H
