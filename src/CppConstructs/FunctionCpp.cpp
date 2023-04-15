#include "FunctionCpp.h"

using CppConstructs::Function;
using CppConstructs::Parameter;

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
    if(enable)
        _functionPrefix = "extern" + spc;
    else
        _functionPrefix.clear();
}

void Function::SetStaticDeclaration(bool enable)
{
    if(enable)
        _functionPrefix = "static" + spc;
    else
        _functionPrefix.clear();
}

string Function::GetFunctionName() const
{
    return  _functionName;
}

string Function::GetFunctionPointerDeclaration() const
{
    string parameterString = GetParametersString(ParameterStringType::DeclAndDef);
    return _returnType + spc + lsb + "*" + _functionName + rsb + lsb  + parameterString + rsb + smcln;
}

void Function::SetBody(vector<string> body)
{
    _body = body;
}

string Function::GetDeclaration() const
{
    return  _functionPrefix + GetSignature(SignatureType::Decl) + smcln;
}

void Function::SetContainedClass(string name)
{
    _hasContainedClass = true;
    _containedClass = name;
}

void Function::ResetContainedClass()
{
    _hasContainedClass = false;
}

vector<string> Function::GetDefinition() const
{
    vector<string> content;

    content.push_back(GetSignature(SignatureType::Def));

    content.push_back(lb);

    for(const auto &string: _body)
        content.push_back(tab + string);

    content.push_back(rb);

    return content;
}

string Function::GetCall(vector<string> argumentNames, bool isPointer) const
{
    string parameterString;
    for (const auto &parameter : argumentNames){
        parameterString += (parameter);
        if(parameter != argumentNames.back())
            parameterString += com;
    }
    string postfixP;
    if(isPointer)
        postfixP = "_p";

    return _functionName  + lsb  + parameterString + rsb + smcln;
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
    return _functionName + lsb  + parameterString + rsb;
}

vector<Parameter> Function::GetParameters() const
{
    return _parameters;
}

string Function::GetSignature(SignatureType type) const
{
    string parameterString = GetParametersString(ParameterStringType::DeclAndDef);

    if(type == SignatureType::Def){
        string scopeRes;
        if(_hasContainedClass) scopeRes = "::";
        return _returnType + spc + _containedClass + scopeRes + spc + _functionName + lsb  + parameterString + rsb;
    }
    else
        return _returnType + spc + _functionName + lsb  + parameterString + rsb;
}

string Function::GetParametersString(ParameterStringType type) const
{
    string parameterString;

    for (const auto &parameter : _parameters)
    {
        auto hasType = ( type == ParameterStringType::DeclAndDef );
        parameterString += ( (hasType ? (parameter.type  + spc) : "") + parameter.name);
        if(parameter.name != _parameters.back().name)
            parameterString += com;
    }

    return parameterString;
}
