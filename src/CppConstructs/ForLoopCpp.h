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
        void setDeclaration(string init, string condition, string increment);
        void setBody(vector<string> body);
        vector<string> definition() const;

    private:
        string _init;
        string _condition;
        string _increment;
        vector<string> _body;
    };
}
#endif // CYCLEFORCONSTRUCTS_H
