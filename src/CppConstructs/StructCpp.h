#ifndef STRUCTCPP_H
#define STRUCTCPP_H
#include <string>
#include <vector>
#include "CommonCpp.h"
using namespace std;

namespace CppConstructs
{

    class StructCpp
    {
    public:
        StructCpp();
        StructCpp(string structName);

        void SetName(string structName);
        string GetName() const;

        void SetBody(vector<string> body);

        vector<string> GetDeclaration(bool hasTypeDef = true) const;

    private:
        string _structName;
        vector<string> _body;
    };
}
#endif // STRUCTCPP_H
