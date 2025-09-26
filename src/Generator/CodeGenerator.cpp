#include "CodeGenerator.h"
#include <fstream>
#include "VersionInfo.h"
#include "MsgHandlerGen.h"
#include "CppConstructs/FunctionsSrc.h"

CodeGenerator::CodeGenerator(AppOptions options, Formater exchangeDescription)
    : _fName(options.fileName), _options(options),
      _exchangeDescription(exchangeDescription), _stdTypeHandler(options){}

void CodeGenerator::Generate()
{
    for(auto info: _exchangeDescription.codeList)
    {
        auto decl = EnumDecl(info, EnumerableType::Code);
        _types.push_back({decl.GetName(),fieldType::Code});
        _enumDeclarations.push_back(decl);
    }

    for(auto info: _exchangeDescription.typeList)
    {
        auto decl = EnumDecl(info, EnumerableType::Type);
        _types.push_back({decl.GetName(),fieldType::Type});
        _enumDeclarations.push_back(decl);
    }

    for(auto info: _exchangeDescription.headerList) {
        _header = GenStructDecl(info, ComplexType::Header);
    }

    for(auto info: _exchangeDescription.enumList)
    {
        auto decl = EnumDecl(info, EnumerableType::Enum);
        _types.push_back({decl.GetName(),fieldType::Enum});
        _enumDeclarations.push_back(decl);
    }

    for(auto info: _exchangeDescription.structList)
    {
        auto decl = GenStructDecl(info, ComplexType::Struct);
        _types.push_back({decl.GetName(),fieldType::Struct});
        _structDeclarations.push_back(decl);
    }

    for(auto info: _exchangeDescription.packetList)
    {
        auto decl = GenStructDecl(info, ComplexType::Message);
        _types.push_back({decl.GetName(),fieldType::Struct});
        _structDeclarations.push_back(decl);
    }

    _handlerGen.reset(new MsgHandlerGen(_options, EnumErrorCode(), _header));

    AnalizeRules();

    if(_options.isCpp) {
        GenerateSourceQt();
        GenerateHeaderQt();
    }
    else {
        GenerateSource();
        GenerateHeader();
    }
}

void CodeGenerator::AnalizeRules()
{
    auto rules = _exchangeDescription.rules;

    auto Rdm = [=](auto command, auto type, auto msg) {
        auto name = msg.value_or("");
        auto typeDescription = FindByName(_structDeclarations, name);
        return RulesDefinedMessage{command, type, typeDescription};
    };

    for(auto rule : rules)
    {
        if(rule.hasResponse())
        {
            _rdms += Rdm(*rule.command, *rule.responseType, rule.responsePacket);
        }
        _rdms += Rdm(*rule.command, *rule.sendType, rule.sendPacket);
    }
}

EnumDescription CodeGenerator::EnumDecl(Enum IntermediateEnum, EnumerableType type)
{
    EnumDescription decl;
    decl.type = type;
    decl.SetName(IntermediateEnum.name);
    decl.fields = IntermediateEnum.fields;

    if(IntermediateEnum.bitWidth.has_value()) {
        decl.SetBitWidth(*IntermediateEnum.bitWidth);
    }

    if(!_options.isCpp)
        decl.SetPrefix(toLower(_fName));

    return decl;
}

EnumDescription CodeGenerator::EnumErrorCode()
{
    EnumDescription decl;
    decl.type = EnumerableType::Enum;
    decl.SetName("ErrorCode");

    vector<FieldDataEnum> fields;
    fields.push_back({"OK", 0});
    fields.push_back({"ERROR", 1});
    decl.fields = fields;

    if(!_options.isCpp)
        decl.SetPrefix(toLower(_fName));

    return decl;
}

Parameter CodeGenerator::GetAddrParameter()
{
    Parameter parameter;

    if(!_options.isCpp)
         parameter.type = toLower(_fName) + "_Addr*";
    else
        parameter.type = "Addr*";

    parameter.name = "addr";
    return parameter;
}

StructCpp CodeGenerator::GenAddrDecl()
{
    StructCpp decl;
    decl.AddFields({
                       {_options.isQt ? "QHostAddress" : "uint32_t", "ip"},
                       {"uint16_t", "port"}
                   });

    decl.SetName(_options.isCpp ? "Addr" : toLower(_fName) + "_Addr");
    decl.SetTypeDef(!_options.isCpp);

    return decl;
}

ComplexTypeDescription CodeGenerator::GenStructDecl(Struct& IntermediateStruct, ComplexType type)
{
    ComplexTypeDescription decl;
    decl.SetOption(_options);

    if(!_options.isCpp) {
        decl.SetPrefix(toLower(_fName));
    }

    decl.SetBlockType(type);
    decl.SetName(IntermediateStruct.name);
    for(auto var: IntermediateStruct.fields)
    {
        auto type = var.second.type;
        if(auto stdType = _stdTypeHandler.CheckType(type, var.second.isArrayField))
        {
            decl.addField(var, {stdType->first, fieldType::std}, stdType->second);
        } else
        {
            auto it = FindInVector(_types, type);
            if(it != _types.end())
            {
                if(it->second == fieldType::Struct)
                {
                    auto p = FindByName(_structDeclarations, type);
                    decl.addField(var, {p->GetName(), fieldType::Struct}, p->Size());
                }
                else if(it->second == fieldType::Enum)
                {
                    auto p = FindByName(_enumDeclarations, type);
                    decl.addField(var, {p->GetName(), fieldType::Enum}, p->Size());
                }
                else if(it->second == fieldType::Code)
                {
                    auto p = FindByName(_enumDeclarations, type);
                    decl.addField(var, {p->GetName(), fieldType::Code}, p->Size());
                }
                else if(it->second == fieldType::Type)
                {
                    auto p = FindByName(_enumDeclarations, type);
                    decl.addField(var, {p->GetName(), fieldType::Type}, p->Size());
                }
            }
            else {
                throw invalid_argument(fmt("type not found: \"%s\" in \"%s\"", {
                                               type, IntermediateStruct.name}));
            }
        }
    }
    return decl;
}

string CodeGenerator::GetVersion()
{
    return fmt("// SLPD Version %s.%s.%s Build %s %s\n\n",
    {to_string(MAJOR_VERSION), to_string(MINOR_VERSION),
     to_string(PATCH_VERSION), APP_TIME, APP_DATE});
}

void CodeGenerator::GenerateHeader()
{
    ofstream oStream;
    oStream.open(toUpper(_fName) + ".h");
    oStream<< GetVersion();
    oStream << fmt("#ifndef %s_H", {toUpper(_fName)}) << endl;
    oStream << fmt("#define %s_H", {toUpper(_fName)}) << endl;
    oStream << endl;
    oStream << "#include <stdint.h>" << endl;

    oStream << _stdTypeHandler.BitFieldTypes() << endl;

    oStream << EnumErrorCode().Declaration(false) << endl;

    if(_options.hasAddr) {
        oStream << GenAddrDecl().Declaration() << endl;
    }

    for(auto var : _enumDeclarations) {
        oStream << var.Declaration() << endl;
    }

    for(auto var : _structDeclarations) {
        oStream << var.Declaration() << endl;
    }

    oStream << fmt("typedef struct %s %s;\n",
    {ProtoObj().GetName(), toLower(_fName) + "_Obj"}) << endl;

    oStream << CbsStruct().Declaration() << endl;
    oStream << ProtoObj().Declaration() << endl;

    oStream<<"/* senders */"<< endl;
    for(auto rdm :_rdms) {
        oStream << _handlerGen->SendMsgFun(rdm).Declaration() << endl;
    }

    oStream << endl;
    oStream << ResetCbsFun().Declaration() << endl;
    oStream << SetCbsFun().Declaration() << endl;
    oStream << InitProtoFun().Declaration() << endl;
    oStream << _handlerGen->ParseFun(_rdms).Declaration() << endl;

    oStream << "#endif" << endl;
    oStream.close();
}

void CodeGenerator::GenerateHeaderQt()
{
    ofstream oStream;
    oStream.open(toUpper(_fName) + ".h");
    oStream << GetVersion();
    oStream << fmt("#ifndef %s_H", {toUpper(_fName)}) << endl;
    oStream << fmt("#define %s_H", {toUpper(_fName)}) << endl;
    oStream << endl;
    oStream << "#include <stdint.h>" << endl;
    oStream << "#include <QObject>" << endl;
    oStream << "#include <vector>" << endl;

    if(_options.hasAddr)
        oStream << "#include <QHostAddress>" << endl;

    oStream << endl;
    oStream << fmt("namespace %sSpace{\n", {_fName});

    oStream << _stdTypeHandler.BitFieldTypes();

    oStream << EnumErrorCode().Declaration(false) << endl;

    if(_options.hasAddr)
        oStream << GenAddrDecl().Declaration();

    oStream << endl;

    if(!LocalParams().GetFields().empty()) {
        oStream << LocalParams().Declaration();
    }

    oStream << endl;
    for(auto var : _enumDeclarations) {
        oStream << var.Declaration() << endl;
    }

    for(auto var : _structDeclarations) {
        oStream << var.Declaration() << endl;
    }

    for(auto var : _structDeclarations) {
        oStream << var.CompareFun().Declaration() << endl;
    }
    oStream << endl;

    oStream << "class Base;" << endl;
    oStream << "class " + _fName + ": public QObject" << endl;
    oStream << "{" << endl;
    oStream << "\tQ_OBJECT" << endl;
    oStream << "signals:" << endl;

    for(auto rdm :_rdms) {
        _handlerGen->ReceiveMsgCb(rdm).SetExternDeclaration(false);
        oStream << "\t" + _handlerGen->ReceiveMsgCb(rdm).Declaration() << endl;
    }

    oStream << "\t" + _handlerGen->SendCb().Declaration() << endl;
    oStream << endl;
    oStream << "public:" << endl;
    oStream << "\t/* senders */" << endl;
    for(auto rdm : _rdms) {
        oStream << "\t" + _handlerGen->SendMsgFun(rdm).Declaration() << endl;
    }

    oStream << endl;
    oStream << "\t" + InitProtoFun().Declaration() << endl;

    if(!LocalParams().GetFields().empty()) {
        oStream << "\t" + LocalParamsFun().Declaration() << endl;
    }

    oStream << "\t" + _handlerGen->ParseFun(_rdms).Declaration() << endl;
    oStream << "\t" + _fName + "(QObject *parent = nullptr);";
    oStream << endl;

    if(_options.hasAddr) {
        oStream << "\tusing addr_t = Addr;" << endl;
    }

    oStream << "\nprivate:" << endl;
    oStream << "\tBase* base;" << endl;
    oStream << "};" << endl;
    oStream << "}//end namespace" << endl << endl;
    oStream << "#endif" << endl;
    oStream.close();
}

void CodeGenerator::GenerateSource()
{
    ofstream oStream;
    oStream.open(toUpper(_fName) + ".c");
    oStream << GetVersion();
    oStream << "#include <string.h>" << endl;
    oStream << fmt("#include \"%s.h\"\n", {_fName});

    oStream << MemoryManager;

    oStream << BitMaskFun;
    oStream << MinFun;
    oStream << BitCpyFun;

    oStream << endl;
    oStream << CalcSizeHelper::CSizeDef();
    oStream << endl;
    oStream << InitProtoFun().Definition();
    oStream << endl;
    oStream << _header.Declaration();
    oStream << endl;
    oStream << _header.SerDesDefinition(FunType::Ser);

    oStream << _header.SerDesDefinition(FunType::Des);

    for(auto var : _structDeclarations)
    {
        oStream << var.SerDesDefinition(FunType::Ser);
        oStream << var.SerDesDefinition(FunType::Des);
        oStream << var.SizeCalcFun(FunType::Ser);
        oStream << var.SizeCalcFun(FunType::Des);
    }

    for( auto& rdm: _rdms) {
        oStream << _handlerGen->SendMsgFun(rdm).Definition();
    }

    oStream << endl;
    oStream << SetCbsFun().Definition();
    oStream << ResetCbsFun().Definition();
    oStream << _handlerGen->ParseFun(_rdms).Definition();

    oStream.close();
}

void CodeGenerator::GenerateSourceQt()
{
    ofstream oStream;
    oStream.open(toUpper(_fName) + ".cpp");
    oStream << GetVersion();
    oStream << "#include <memory>" << endl;
    oStream << "#include <string.h>" << endl;
    oStream << fmt("#include \"%s.h\"", {_fName}) << endl;
    oStream << fmt("namespace %sSpace \n{\n", {_fName});
    oStream << BitMaskFun;
    oStream << MinFun;
    oStream << BitCpyFun;
    oStream << endl;
    oStream << CalcSizeHelper::CSizeDef();
    oStream << endl;


    for(auto var : _structDeclarations) {
        oStream << var.CompareFun().Definition();
        oStream << endl;
    }
    oStream << endl;

    oStream << _header.Declaration();
    oStream << endl;
    oStream << "class Base: QObject\n{" << endl;
    oStream << "public:" << endl;
    oStream << "Base(QObject *parent = nullptr):QObject(parent){}" << endl;

    for(auto field: LocalParams().GetFields()) {
        oStream << fmt("%s %s;", {field.type, field.name}) << endl;
    }

    oStream << endl;
    oStream << _header.SerDesDefinition(FunType::Ser, false);
    oStream << _header.SerDesDefinition(FunType::Des, false);
    for(auto var : _structDeclarations)
    {
        oStream << var.SerDesDefinition(FunType::Ser, false);
        oStream << var.SerDesDefinition(FunType::Des, false);
        oStream << var.SizeCalcFun(FunType::Ser, false);
        oStream << var.SizeCalcFun(FunType::Des, false);
    }
    oStream << "};" << endl; //end Base class

    oStream << InitProtoFun().Definition();

    if(!LocalParams().GetFields().empty())
        oStream << LocalParamsFun().Definition();

    oStream << endl;

    for( auto& rdm: _rdms) {
        oStream << _handlerGen->SendMsgFun(rdm).Definition();
    }

    oStream << _handlerGen->ParseFun(_rdms).Definition();

    oStream << fmt("%s::%s(QObject *parent):QObject(parent)", {_fName, _fName});
    oStream << "\n{" << "\n\tbase = new Base(this);" << "\n}";

    oStream << "\n}\n"; //end namespace
    oStream.close();
}

Function CodeGenerator::InitProtoFun()
{
    Function initFun;
    vector<string> body;

    auto params = _handlerGen->GetHeaderParams({"local"});

    initFun.SetDeclaration(fmt("%{%s_}init", {_options.isCpp ? "" : toLower(_fName)}),
                           _options.isCpp ? "void" : toLower(_fName) + "_Obj",
                           params);

    if(!_options.isCpp) {
        body << fmt("%s obj;", {toLower(_fName) + "_Obj"});
    }

    for(auto &param : params)
    {
        body << fmt("%s%s = %s;",
        {_options.isCpp ? "base->" : "obj.", param.name, param.name});
    }

    if(!_options.isCpp) { body << "return obj;"; }

    initFun.SetBody(body);
    if(_options.isCpp) { initFun.SetContainedClass(_fName); }

    return initFun;
}

StructCpp CodeGenerator::ProtoObj()
{
    StructCpp protoObj;
     protoObj.SetName(fmt("_%s_Obj", {toLower(_fName)}));

    auto params = _handlerGen->GetHeaderParams({"local"});

    for(auto& param : params) {
        protoObj.AddField({param.type, param.name});
    }

    protoObj.AddField({CbsStruct().GetName(), "_CBsStruct"});

    return protoObj;
}

Function CodeGenerator::SetCbsFun()
{
    Function setCbsFun;
    std::vector<Parameter> params;

    params += {toLower(_fName) + "_Obj", "*obj"};
    params += {fmt("%s_CBsStruct", {toLower(_fName)}), "*str"};
    setCbsFun.SetDeclaration(toLower(_fName) + "_setCBs", "void", params);

    vector<string> body;
    for(auto rdm :_rdms) {
        auto cbName = _handlerGen->ReceiveMsgCb(rdm).FunctionName();
        body << fmt("obj->_CBsStruct.%s = str->%s;", {cbName, cbName});
    }

    auto sendCbName = _handlerGen->SendCb().FunctionName();
    body << fmt("obj->_CBsStruct.%s = str->%s;", {sendCbName, sendCbName});

    setCbsFun.SetBody(body);

    return setCbsFun;
}

Function CodeGenerator::ResetCbsFun()
{
    Function resetCbsFun;
    resetCbsFun.SetDeclaration(toLower(_fName) +"_resetCBs", "void", {
                                   toLower(_fName) + "_Obj", "*obj"});

    vector<string> body;
    for(auto rdm :_rdms) {
        body << "obj->_CBsStruct." + _handlerGen->ReceiveMsgCb(rdm).FunctionName() + " = 0;";
    }
    body << "obj->_CBsStruct." + _handlerGen->SendCb().FunctionName() + " = 0;";

    resetCbsFun.SetBody(body);

    return resetCbsFun;
}

StructCpp CodeGenerator::CbsStruct()
{
    StructCpp cbsStruct;

    cbsStruct.SetName(toLower(_fName) + "_CBsStruct");
    cbsStruct.SetTypeDef(true);

    for(auto rdm :_rdms) {
        auto param = _handlerGen->ReceiveMsgCb(rdm).FunctionPointer();
        cbsStruct.AddField({param.type, param.name});
    }
    auto param = _handlerGen->SendCb().FunctionPointer();
    cbsStruct.AddField({param.type, param.name});

    return cbsStruct;
}

StructCpp CodeGenerator::LocalParams()
{
    StructCpp localParams;
    localParams.SetName("LocalParams");

    for(auto& param : _handlerGen->GetHeaderParams({"local"})) {
        localParams.AddField({param.type, param.name});
    }

    return localParams;
}

Function CodeGenerator::LocalParamsFun()
{
    Function localParamsFun;
    auto localParams = LocalParams();

    localParamsFun.SetDeclaration("GetLocalParams", localParams.GetName());
    localParamsFun.SetContainedClass(_fName);


    vector<string> body;
    string var = "buf";
    body << fmt("%s %s;", {localParams.GetName(), var});

    for (auto field : localParams.GetFields()) {
        body << fmt("%s.%s = %s%s;", {var, field.name,
                                      _options.isCpp ? "base->" : "",
                                      field.name});
    }
    body << fmt("return %s;", {var});

    localParamsFun.SetBody(body);

    return localParamsFun;
}
