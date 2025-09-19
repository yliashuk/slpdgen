#ifndef FUNCTIONCPP_H
#define FUNCTIONCPP_H
#include <string>
#include <vector>

using namespace std;

namespace CppConstructs
{

    struct Parameter
    {
        string type;
        string name;
    };

    class Function
    {
    public:
        Function();
        void setDeclaration(string functionName, string returnType,vector<Parameter> parameters);
        void setDeclaration(string functionName, string returnType, Parameter parameter);
        void setDeclaration(string functionName, string returnType);

        void setExtern(bool enable);
        void setStatic(bool enable);

        string name() const;

        Parameter pointer() const;
        Parameter pointerWrapper(string wrapperName) const;

        vector<string> definition() const;

        string getCall(vector<string> argumentNames) const;
        string getCall(vector<Parameter> argumentNames) const;
        string getCall() const;

        vector<Parameter> getParameters() const;

        void setBody(vector<string> body);

        string declaration() const;

        void setContainedClass(string name);
        void resetContainedClass();

    private:
         enum class ParameterStringType {Call, DeclAndDef};
         enum class SignatureType {Decl, Def};

         bool _hasExtern {};
         bool _hasStatic {};

        string _containedClass;
        string _name;
        string _returnType;
        vector<Parameter> _parameters;
        vector<string> _body;

        string getSignature(SignatureType type) const;
        string getParametersString(ParameterStringType type) const;
    };
}

#endif // FUNCTIONCPP_H
