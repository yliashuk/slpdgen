#include "FunctionCpp.h"
#include "Utils/StringUtils.h"

using CppConstructs::Function;
using CppConstructs::Parameter;
using namespace Utils;

Function::Function(){}

void Function::SetDeclaration(string functionName, string returnType, vector<Parameter> parameters)
{
    this->_functionName = functionName;
    this->_returnType = returnType;
    this->_parameters = parameters;
}

void Function::SetDeclaration(string functionName, string returnType, Parameter parameter)
{
    this->_functionName = functionName;
    this->_returnType = returnType;
    this->_parameters.push_back(parameter);
}

void Function::SetDeclaration(string functionName, string returnType)
{
    this->_functionName = functionName;
    this->_returnType = returnType;
}

void Function::SetExternDeclaration(bool enable)
{
    _hasExtern = enable;
}

void Function::SetStaticDeclaration(bool enable)
{
    _hasStatic = enable;
}

string Function::FunctionName() const
{
    return  _functionName;
}

Parameter Function::FunctionPointer() const
{
    string parameterString = GetParametersString(ParameterStringType::DeclAndDef);
    return {_returnType, fmt("(*%s)(%s)", {_functionName, parameterString})};
}

string Function::FunctionPointerDeclaration() const
{
    string parametersString = GetParametersString(ParameterStringType::DeclAndDef);
    return fmt("%s (*%s)(%s);",{_returnType, _functionName, parametersString});
}

void Function::SetBody(vector<string> body)
{
    _body = body;
}

string Function::Declaration() const
{
    return (_hasExtern ? "extern " : "") + GetSignature(SignatureType::Decl) + ";";
}

void Function::SetContainedClass(string name)
{
    _containedClass = name;
}

void Function::ResetContainedClass()
{
    _containedClass.clear();
}

vector<string> Function::Definition() const
{
    return GetSignature(SignatureType::Def) << "{" << fmt("\t%s", _d(_body)) << "}";
}

string Function::GetCall(vector<string> argumentNames, bool isPointer) const
{
    string parameterString;
    for (const auto &parameter : argumentNames){
        parameterString += (parameter);
        if(parameter != argumentNames.back())
            parameterString += ", ";
    }
    string postfixP;
    if(isPointer)
        postfixP = "_p";

    return fmt("%s(%s);", {_functionName, parameterString});
}

string Function::GetCall(vector<Parameter> argumentNames) const
{
    vector<string> names;
    for(const auto &param : argumentNames)
        names.push_back(param.name);
   return GetCall(names, false);
}

string Function::GetCall() const
{
    string parameterString = GetParametersString(ParameterStringType::Call);
    return fmt("%s(%s)", {_functionName, parameterString});
}

vector<Parameter> Function::GetParameters() const
{
    return _parameters;
}

string Function::GetSignature(SignatureType type) const
{
    string parametersString = GetParametersString(ParameterStringType::DeclAndDef);

    return fmt("%{%s }%s %{%s::}%s(%s)", {_hasStatic ? "static" : "",
                                          _returnType,
                                          type == SignatureType::Def ? _containedClass : "",
                                          _functionName,
                                          parametersString});
}

string Function::GetParametersString(ParameterStringType type) const
{
    string parameterString;

    for (const auto &parameter : _parameters)
    {
        auto hasType = ( type == ParameterStringType::DeclAndDef );
        parameterString += ( (hasType ? (parameter.type  + " ") : "") + parameter.name);
        if(parameter.name != _parameters.back().name)
            parameterString += ", ";
    }

    return parameterString;
}
