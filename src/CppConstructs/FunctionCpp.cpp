#include "FunctionCpp.h"
#include "Utils/StringUtils.h"

using CppConstructs::Function;
using CppConstructs::Parameter;
using namespace Utils;

Function::Function(){}

void Function::setDeclaration(string functionName, string returnType, vector<Parameter> parameters)
{
    this->_name = functionName;
    this->_returnType = returnType;
    this->_parameters = parameters;
}

void Function::setDeclaration(string functionName, string returnType, Parameter parameter)
{
    this->_name = functionName;
    this->_returnType = returnType;
    this->_parameters.push_back(parameter);
}

void Function::setDeclaration(string functionName, string returnType)
{
    this->_name = functionName;
    this->_returnType = returnType;
}

void Function::setExtern(bool enable)
{
    _hasExtern = enable;
}

void Function::setStatic(bool enable)
{
    _hasStatic = enable;
}

string Function::name() const
{
    return  _name;
}

Parameter Function::pointer() const
{
    string parameters = getParametersString(ParameterStringType::DeclAndDef);
    return {_returnType, fmt("(*%s)(%s)", {_name, parameters})};
}

Parameter Function::pointerWrapper(string wrapperType) const
{
    string parameters = getParametersString(ParameterStringType::DeclAndDef);
    auto returnType = fmt("%s<%s(%s)>", {wrapperType, _returnType, parameters});
    return {returnType, _name};
}

void Function::setBody(vector<string> body)
{
    _body = body;
}

string Function::declaration() const
{
    return (_hasExtern ? "extern " : "") + getSignature(SignatureType::Decl) + ";";
}

void Function::setContainedClass(string name)
{
    _containedClass = name;
}

void Function::resetContainedClass()
{
    _containedClass.clear();
}

vector<string> Function::definition() const
{
    return getSignature(SignatureType::Def) << "{" << fmt("\t%s", _d(_body)) << "}";
}

string Function::getCall(vector<string> argumentNames) const
{
    string parameterString;
    for (const auto &parameter : argumentNames){
        parameterString += (parameter);
        if(parameter != argumentNames.back())
            parameterString += ", ";
    }

    return fmt("%s(%s)", {_name, parameterString});
}

string Function::getCall(vector<Parameter> argumentNames) const
{
    vector<string> names;
    for(const auto &param : argumentNames)
        names.push_back(param.name);
   return getCall(names);
}

string Function::getCall() const
{
    string parameterString = getParametersString(ParameterStringType::Call);
    return fmt("%s(%s)", {_name, parameterString});
}

vector<Parameter> Function::getParameters() const
{
    return _parameters;
}

string Function::getSignature(SignatureType type) const
{
    string parametersString = getParametersString(ParameterStringType::DeclAndDef);

    return fmt("%{%s }%s %{%s::}%s(%s)", {_hasStatic ? "static" : "",
                                          _returnType,
                                          type == SignatureType::Def ? _containedClass : "",
                                          _name,
                                          parametersString});
}

string Function::getParametersString(ParameterStringType type) const
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
