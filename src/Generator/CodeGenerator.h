#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <map>
#include <fstream>
#include "Analyzer/Formater.h"
#include "ComplexTypeDescription.h"
#include "EnumDescription.h"
#include "CppConstructs/SwitchCpp.h"
#include "CppConstructs/IfElseStatementCpp.h"
#include "CppConstructs/FunctionCpp.h"
#include "CppConstructs/StructCpp.h"

using namespace CppConstructs;

struct SimpleRule
{
    string command;
    string sendType;
    string sendPacket;
    bool isEmptySend;
    bool hasResponse;
    bool hasEmptyResponse;
    string responseType;
    string responsePacket;
};

struct InterfaceFunction
{
    InterfaceFunction(string command, string type, Function func,
                      ComplexTypeDescription packet):
    command(command), type(type), withPacket(true), func(func), packet(packet){}
    InterfaceFunction(string command, string type, Function func):
    command(command), type(type), func(func) {}
    string command;
    string type;
    bool withPacket = false;
    Function func;
    ComplexTypeDescription packet;
};

class CodeGenerator
{
public:
    CodeGenerator(string fileName, Formater exchangeDescription, bool addrOption = true,
                  bool qtOption = true, bool qtCppOption = true);
    void Generate();

private:
    string _fileName, _fileNameLowerCase;
    vector<pair<string,fieldType>> _types;
    vector<ComplexTypeDescription> _structDeclarations;
    ComplexTypeDescription _headerDeclarations;
    vector<EnumDescription> _enumDeclarations;
    Formater _exchangeDescription;

    string _typeObjName;
    EnumDescription _errorEnum;
    StructCpp _cbsStruct, _protocolObj, _localParams;
    bool _qtOption, _qtCppOption, _addrOption;

    vector<pair<string,string>> _localVar;

    Function _cbSendFun, _initFun, _setCbsFun, _resetCbsFun;
    Function _getLocalParamsFun, _parseFun;

    ofstream _oStream;

    vector<string> MainDesFormat();
    vector<string> MainSerFormaters();
    EnumDescription GenEnumDecl(Enum IntermediateEnum, EnumerableType type);
    EnumDescription GenEnumErrorCodeDecl();
    Parameter GetAddrParameter();
    StructCpp GenAddrDecl();
    ComplexTypeDescription GenStructDecl(Struct &IntermediateStruct, ComplexType type);
    void TestPrint(vector<string> strings);
    void PrintToFile(ofstream &stream, string str);
    void PrintToFile(ofstream &stream, vector<string> strings);
    pair<string,size_t> ConvertToCStdType(string slpdType);
    string GetVersion();
    void GenerateHeader();
    void GenerateHeaderQt();
    void GenerateSource();
    void GenerateSourceQt();
    vector<string> ConstructDefinition();

    vector<ComplexTypeDescription>::iterator
    FindDeclaration(vector<ComplexTypeDescription>& DataStruct, string typeName);

    vector<EnumDescription>::iterator
    FindDeclaration(vector<EnumDescription>& DataStruct, string content);


    vector<InterfaceFunction> interfaceSenderFunctions;
    vector<InterfaceFunction> interfaceReceiveFunctions;

    void AnalizeRules(vector<Rule> slpdRules,
                      vector<ComplexTypeDescription> structDeclarations,
                      ComplexTypeDescription header);

    vector<string> GetInterfaceSendFuncBody(string headerVar, string command,
                                            string sendType,
                                            vector<Parameter> headerParameteres);
    vector<string> GetInterfaceSendFuncBody(string headerCommandVar,
                                            string command, string sendType,
                                            ComplexTypeDescription packet,
                                            vector<Parameter> headerRemoteParam);
    vector<string> SetBodySetCbs();
    vector<string> SetBodyResetCbs();
    vector<string> SetBodyLocaParamsStruct(vector<Parameter> params);
    vector<string> SetBodyGetLocaParamsFun(vector<Parameter> params);
    string PrintClassText(string var);
    string PrintStructText(string var);
    vector<string> PrintMemoryManager();
};

#endif // CODEGENERATOR_H
