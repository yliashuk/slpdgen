#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <set>
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
#include "StdTypeHandler.h"

using namespace CppConstructs;

class CodeGenerator
{
public:
    CodeGenerator(AppOptions options, Formater exchangeDescription);
    void generate();

private:
    string _fName;
    AppOptions _options;

    vector<pair<string,FieldType>> _types;
    vector<ComplexTypeDescription> _structDeclarations;
    ComplexTypeDescription _header;
    vector<EnumDescription> _enumDeclarations;
    Formater _exchangeDescription;

    StdTypeHandler _stdTypeHandler;
    std::set<RulesDefinedMessage> _rdms;
    std::unique_ptr<MsgHandlerGen> _handlerGen;

    bool bitcpyUsed = false;
    bool bitcpyaUsed = false;

    void analizeRules();

    EnumDescription enumDecl(Enum IntermediateEnum, EnumerableType type);
    EnumDescription enumErrorCode();

    StructCpp addrDecl();

    ComplexTypeDescription structDecl(Struct &IntermediateStruct, ComplexType type);

    void checkAndSetUsedCopyMethods();
    void setupCopyOptions(const StructFieldInfo &info, ComplexTypeDescription &type,
                          SizeExprPtr offset);

    string getVersion();

    void generateHeader();
    void generateCppHeader();
    void generateSource();
    void generateCppSource();

    Function initProtoFun();

    // only for C generation

    StructCpp protoObj();
    StructCpp localParams();
    Function localParamsFun();
    Function setCbsFun();
    Function resetCbsFun();
    StructCpp cbsStruct();
};

#endif // CODEGENERATOR_H
