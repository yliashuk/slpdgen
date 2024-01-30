#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <memory>
#include "AppOptions.h"
#include "Analyzer/Formater.h"
#include "ComplexTypeDescription.h"
#include "EnumDescription.h"
#include "CppConstructs/SwitchCpp.h"
#include "CppConstructs/IfElseStatementCpp.h"
#include "CppConstructs/FunctionCpp.h"
#include "CppConstructs/StructCpp.h"
#include "RulesDefinedMessage.h"
#include "MsgHandlerGen.h"

using namespace CppConstructs;

class CodeGenerator
{
public:
    CodeGenerator(AppOptions options, Formater exchangeDescription);
    void Generate();

private:
    string _fName;
    AppOptions _options;

    vector<pair<string,fieldType>> _types;
    vector<ComplexTypeDescription> _structDeclarations;
    ComplexTypeDescription _header;
    vector<EnumDescription> _enumDeclarations;
    Formater _exchangeDescription;

    vector<RulesDefinedMessage> _rdms;
    std::unique_ptr<MsgHandlerGen> _handlerGen;

    void AnalizeRules();

    EnumDescription EnumDecl(Enum IntermediateEnum, EnumerableType type);
    EnumDescription EnumErrorCode();

    StructCpp GenAddrDecl();
    Parameter GetAddrParameter();

    ComplexTypeDescription GenStructDecl(Struct &IntermediateStruct, ComplexType type);

    string GetVersion();

    void GenerateHeader();
    void GenerateHeaderQt();
    void GenerateSource();
    void GenerateSourceQt();

    Function InitProtoFun();

    // only for C generation

    StructCpp ProtoObj();
    StructCpp LocalParams();
    Function LocalParamsFun();
    Function SetCbsFun();
    Function ResetCbsFun();
    StructCpp CbsStruct();
    vector<string> MemoryManager();

    static pair<string,size_t> ConvertToCStdType(string slpdType);
};

#endif // CODEGENERATOR_H
