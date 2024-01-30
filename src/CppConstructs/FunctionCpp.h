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
        void SetDeclaration(string functionName, string returnType,vector<Parameter> parameters);
        void SetDeclaration(string functionName, string returnType, Parameter parameter);
        void SetDeclaration(string functionName, string returnType);

        void SetExternDeclaration(bool enable);
        void SetStaticDeclaration(bool enable);

        string FunctionName() const;

        Parameter FunctionPointer() const;
        string FunctionPointerDeclaration() const;

        vector<string> Definition() const;

        string GetCall(vector<string> argumentNames, bool isPointer) const;
        string GetCall(vector<Parameter> argumentNames) const;
        string GetCall() const;

        vector<Parameter> GetParameters() const;

        void SetBody(vector<string> body);

        string Declaration() const;

        void SetContainedClass(string name);
        void ResetContainedClass();

    private:
         enum class ParameterStringType {Call, DeclAndDef};
         enum class SignatureType {Decl, Def};

         bool _hasExtern {};
         bool _hasStatic {};

        string _containedClass;
        string _functionName;
        string _returnType;
        vector<Parameter> _parameters;
        vector<string> _body;

        string GetSignature(SignatureType type) const;
        string GetParametersString(ParameterStringType type) const;
    };
}

#endif // FUNCTIONCPP_H
