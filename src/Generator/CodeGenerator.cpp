#include "CodeGenerator.h"
#include <fstream>
#include "VersionInfo.h"
#include "MsgHandlerGen.h"
#include "CppConstructs/FunctionsSrc.h"
#include "Utils/AutoEndlStream.h"

CodeGenerator::CodeGenerator(AppOptions options, Formater exchangeDescription)
    : _fName(options.fileName), _options(options),
      _exchangeDescription(exchangeDescription), _stdTypeHandler(options){}

void CodeGenerator::generate()
{
    for(auto info: _exchangeDescription.codeList)
    {
        auto decl = enumDecl(info, EnumerableType::Code);
        _types.push_back({decl.getName(),FieldType::Code});
        _enumDeclarations.push_back(decl);
    }

    for(auto info: _exchangeDescription.typeList)
    {
        auto decl = enumDecl(info, EnumerableType::Type);
        _types.push_back({decl.getName(),FieldType::Type});
        _enumDeclarations.push_back(decl);
    }

    for(auto info: _exchangeDescription.headerList) {
        _header = structDecl(info, ComplexType::Header);
    }

    for(auto info: _exchangeDescription.enumList)
    {
        auto decl = enumDecl(info, EnumerableType::Enum);
        _types.push_back({decl.getName(),FieldType::Enum});
        _enumDeclarations.push_back(decl);
    }

    for(auto info: _exchangeDescription.structList)
    {
        auto decl = structDecl(info, ComplexType::Struct);
        _types.push_back({decl.getName(),FieldType::Struct});
        _structDeclarations.push_back(decl);
    }

    for(auto info: _exchangeDescription.packetList)
    {
        auto decl = structDecl(info, ComplexType::Message);
        _types.push_back({decl.getName(),FieldType::Struct});
        _structDeclarations.push_back(decl);
    }

    _handlerGen.reset(new MsgHandlerGen(_options, enumErrorCode(), _header));

    analizeRules();
    checkAndSetUsedCopyMethods();

    if(_options.isCpp) {
        generateCppSource();
        generateCppHeader();
    }
    else {
        generateSource();
        generateHeader();
    }
}

void CodeGenerator::analizeRules()
{
    auto rules = _exchangeDescription.rules;

    auto Rdm = [=](auto command, auto type, auto msg) {
        auto name = msg.value_or("");
        auto declaration = findByNameOpt(_structDeclarations, name);
        return RulesDefinedMessage{command, type, declaration};
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

EnumDescription CodeGenerator::enumDecl(Enum IntermediateEnum, EnumerableType type)
{
    EnumDescription decl;
    decl.type = type;
    decl.setName(IntermediateEnum.name);
    decl.fields = IntermediateEnum.fields;

    if(IntermediateEnum.bitWidth.has_value()) {
        decl.setBitWidth(*IntermediateEnum.bitWidth);
    }

    if(!_options.isCpp)
        decl.setPrefix(toLower(_fName));

    return decl;
}

EnumDescription CodeGenerator::enumErrorCode()
{
    EnumDescription decl;
    decl.type = EnumerableType::Enum;
    decl.setName("ErrorCode");

    vector<EnumFieldInfo> fields;
    fields.push_back({"OK", 0});
    fields.push_back({"ERROR", 1});
    decl.fields = fields;

    if(!_options.isCpp)
        decl.setPrefix(toLower(_fName));

    return decl;
}

StructCpp CodeGenerator::addrDecl()
{
    StructCpp decl;
    decl.addFields({
                       {_options.isQt ? "QHostAddress" : "uint32_t", "ip"},
                       {"uint16_t", "port"}
                   });

    decl.setName(_options.isCpp ? "Addr" : toLower(_fName) + "_Addr");
    decl.setTypeDef(!_options.isCpp);

    return decl;
}

ComplexTypeDescription CodeGenerator::structDecl(Struct& IntermediateStruct, ComplexType type)
{
    ComplexTypeDescription decl;
    decl.setOption(_options);

    if(!_options.isCpp) {
        decl.setPrefix(toLower(_fName));
    }

    if(type == ComplexType::Message) {
        decl.addContextOffset(_header.size());
    }

    decl.setBlockType(type);
    decl.setName(IntermediateStruct.name);
    for(auto var: IntermediateStruct.fields)
    {
        auto fType = var.second.type;
        if(auto stdType = _stdTypeHandler.checkType(fType, var.second.isArray()))
        {
            decl.addField(var, {stdType->first, FieldType::std}, stdType->second);
        } else
        {
            auto it = findInVector(_types, fType);
            if(it != _types.end())
            {
                if(it->second == FieldType::Struct)
                {
                    auto p = findByName(_structDeclarations, fType);
                    setupCopyOptions(var, *p, _header.size() + decl.size());
                    decl.addField(var, {p->getName(), FieldType::Struct}, p->size());
                }
                else if(it->second == FieldType::Enum)
                {
                    auto p = findByName(_enumDeclarations, fType);
                    decl.addField(var, {p->getName(), FieldType::Enum}, p->size());
                }
                else if(it->second == FieldType::Code)
                {
                    auto p = findByName(_enumDeclarations, fType);
                    decl.addField(var, {p->getName(), FieldType::Code}, p->size());
                }
                else if(it->second == FieldType::Type)
                {
                    auto p = findByName(_enumDeclarations, fType);
                    decl.addField(var, {p->getName(), FieldType::Type}, p->size());
                }
            }
            else {
                throw invalid_argument(fmt("type not found: \"%s\" in \"%s\"", {
                                               fType, IntermediateStruct.name}));
            }
        }
    }
    return decl;
}

void CodeGenerator::checkAndSetUsedCopyMethods()
{
    auto checkFieldMethod = [=](bool usedAlignedCopy)
    {
        if(usedAlignedCopy) { bitcpyaUsed = true; }
        else { bitcpyUsed = true; }
    };

    for(const auto& field : _header.fields())
    {
        checkFieldMethod(_header.shouldUseAlignedCopy(field));
    }

    for(const auto& decl : _structDeclarations)
    {
        for(const auto& field : decl.fields())
        {
            checkFieldMethod(decl.shouldUseAlignedCopy(field));
        }
    }
}

void CodeGenerator::setupCopyOptions(const StructFieldInfo &info,
                                     ComplexTypeDescription &type, SizeExprPtr offset)
{
    if(info.second.isArray() && !type.size()->isMultipleOf(8)) {
        type.setAlignedCopyPreferred(false);
    } else {
        type.addContextOffset(offset);
    }
}

string CodeGenerator::getVersion()
{
    return fmt("// SLPD Version %s.%s.%s Build %s %s\n",
    {to_string(MAJOR_VERSION), to_string(MINOR_VERSION),
     to_string(PATCH_VERSION), APP_TIME, APP_DATE});
}

void CodeGenerator::generateHeader()
{
    ofstream oStream;
    oStream.open(_fName + ".h");
    AutoEndlStream out(oStream);

    out << getVersion();
    out << fmt("#ifndef %s_H", {toUpper(_fName)});
    out << fmt("#define %s_H", {toUpper(_fName)});
    out << "";
    out << "#include <stdint.h>";
    out << "#include <stdbool.h>";

    out << _stdTypeHandler.bitfieldArrayTypes();

    out << enumErrorCode().declaration(false);

    if(_options.hasAddr) {
        out << addrDecl().declaration();
    }

    for(auto var : _enumDeclarations) {
        out << var.declaration();
    }

    out << _header.declaration();
    for(auto var : _structDeclarations) {
        out << var.declaration();
    }

    out << fmt("typedef struct %s %s;\n",
    {protoObj().getName(), toLower(_fName) + "_Obj"});

    out << cbsStruct().declaration();
    out << protoObj().declaration();

    out <<"/* senders */";
    for(auto rdm :_rdms) {
        out << _handlerGen->sendMsgFun(rdm).declaration();
    }

    out << "";
    out << resetCbsFun().declaration();
    out << setCbsFun().declaration();
    out << initProtoFun().declaration();
    out << _handlerGen->parseFun(_rdms).declaration();

    out << "#endif";
}

void CodeGenerator::generateCppHeader()
{
    const auto& qt = _options.isQt;
    const auto& ip = _options.hasAddr;

    ofstream oStream;
    oStream.open(_fName + ".h");
    AutoEndlStream out(oStream);

    out << getVersion();
    out << fmt("#ifndef %s_H", {toUpper(_fName)});
    out << fmt("#define %s_H", {toUpper(_fName)});
    out << "";
    out << "#include <memory>";
    out << "#include <vector>";
    out << "#include <cstdint>";

    if(qt)
    {
        out << "#include <QObject>";
        if(ip){ out << "#include <QHostAddress>"; }
    } else {
        out << "#include <functional>";
    }

    out << fmt("namespace %sSpace{", {_fName});

    out << _stdTypeHandler.bitfieldArrayTypes();
    out << enumErrorCode().declaration(false);

    if(_options.hasAddr)
        out << addrDecl().declaration();

    if(!localParams().getFields().empty()) {
        out << localParams().declaration();
    }
    for(auto var : _enumDeclarations) {
        out << var.declaration();
    }

    out << _header.declaration();
    for(auto var : _structDeclarations) {
        out << var.declaration();
    }

    for(auto var : _structDeclarations) {
        out << var.compareFun().declaration();
    }
    out << "";
    out << "class Base;";
    out << "class " + _fName + (qt ? ": public QObject" : "");
    out << "{";
    if(qt){ out << "\tQ_OBJECT"; }
    out << (!qt ? "public:\n\t// " : ""s) + "signals:";

    const auto sig = "std::function";
    for(auto rdm :_rdms)
    {
        if(qt)
        {
            out << "\t" + _handlerGen->receiveMsgCb(rdm).declaration();
        } else
        {
            auto signal = _handlerGen->receiveMsgCb(rdm).pointerWrapper(sig);
            out << fmt("\t%s %s;", {signal.type, signal.name});
        }
    }

    if(qt)
    {
        out << "\t" + _handlerGen->checkHeaderCb().declaration();
        out << "\t" + _handlerGen->sendCb().declaration();
    } else
    {
        auto sigCheck = _handlerGen->checkHeaderCb().pointerWrapper(sig);
        out << fmt("\t%s %s;", {sigCheck.type, sigCheck.name});
        auto sigSend = _handlerGen->sendCb().pointerWrapper(sig);
        out << fmt("\t%s %s;", {sigSend.type, sigSend.name});
    }

    out << "";
    if(qt){ out << "public:"; }
    out << "\t// senders:";
    for(auto rdm : _rdms) {
        out << "\t" + _handlerGen->sendMsgFun(rdm).declaration();
    }

    out << "";
    out << "\t" + initProtoFun().declaration();

    if(!localParams().getFields().empty()) {
        out << "\t" + localParamsFun().declaration();
    }

    out << "\t" + _handlerGen->parseFun(_rdms).declaration();
    out << fmt("\t%s(%s);", {_fName, qt ? "QObject *parent = nullptr" : ""});
    if(!qt){ out << fmt("\t~%s();", {_fName}); };

    if(ip) { out << "\tusing addr_t = Addr;"; }

    out << "private:";
    if(qt) {out << "\tBase* base;";}
    else { out << "\tstd::unique_ptr<Base> base;"; }

    out << "};";
    out << "}//end namespace";
    out << "#endif";
}

void CodeGenerator::generateSource()
{
    ofstream oStream;
    oStream.open(_fName + ".c");
    AutoEndlStream out(oStream);

    out << getVersion();
    out << "#include <string.h>";
    out << fmt("#include \"%s.h\"", {_fName});

    out << MemoryManager;

    if(bitcpyUsed)
    {
        out << BitMaskFun;
        out << MinFun;
        out << BitCpyFun;
    }
    if(bitcpyaUsed)
    {
        out << BitCpyAlignedFun;
    }

    out << "";
    out << CalcSizeHelper::cSizeDef();
    out << initProtoFun().definition();
    out << _header.serdesDefinition(FunType::Ser);

    out << _header.serdesDefinition(FunType::Des);

    for(auto var : _structDeclarations)
    {
        out << var.serdesDefinition(FunType::Ser);
        out << var.serdesDefinition(FunType::Des);
        out << var.sizeCalcFun(FunType::Ser);
        out << var.sizeCalcFun(FunType::Des);
    }

    for( auto& rdm: _rdms) {
        out << _handlerGen->sendMsgFun(rdm).definition();
    }

    out << setCbsFun().definition();
    out << resetCbsFun().definition();
    out << _handlerGen->parseFun(_rdms).definition();
}

void CodeGenerator::generateCppSource()
{
    auto qt = _options.isQt;

    ofstream oStream;
    oStream.open(_fName + ".cpp");
    AutoEndlStream out(oStream);

    out << getVersion();
    if(qt) {out << "#include <QMetaMethod>";}
    out << "#include <string.h>";
    out << fmt("#include \"%s.h\"", {_fName});
    out << fmt("namespace %sSpace \n{", {_fName});

    if(bitcpyUsed)
    {
        out << BitMaskFun;
        out << MinFun;
        out << BitCpyFun;
    }
    if(bitcpyaUsed)
    {
        out << BitCpyAlignedFun;
    }

    out << "";
    out << CalcSizeHelper::cSizeDef();

    for(auto var : _structDeclarations) {
        out << var.compareFun().definition();
    }

    out << "class Base" + (qt ? ": QObject" : ""s) + "\n{";
    out << "public:";
    if(qt){ out << "Base(QObject *parent = nullptr):QObject(parent){}"; }

    for(auto field: localParams().getFields()) {
        out << fmt("%s %s;", {field.type, field.name});
    }

    out << "";
    out << _header.serdesDefinition(FunType::Ser, false);
    out << _header.serdesDefinition(FunType::Des, false);
    for(auto var : _structDeclarations)
    {
        out << var.serdesDefinition(FunType::Ser, false);
        out << var.serdesDefinition(FunType::Des, false);
        out << var.sizeCalcFun(FunType::Ser, false);
        out << var.sizeCalcFun(FunType::Des, false);
    }
    out << "};"; //end Base class

    out << initProtoFun().definition();

    if(!localParams().getFields().empty())
        out << localParamsFun().definition();

    out << "";

    for( auto& rdm: _rdms) {
        out << _handlerGen->sendMsgFun(rdm).definition();
    }

    out << _handlerGen->parseFun(_rdms).definition();

    if(qt)
    {
        out << fmt("%s::%s(QObject *parent):QObject(parent)", {_fName, _fName});
        out << "{" << "\tbase = new Base(this);" << "}";
    } else
    {
        out << fmt("%s::%s()", {_fName, _fName});
        out << "{" << "\tbase = std::unique_ptr<Base>(new Base);" << "}";
        out << fmt("%s::~%s() = default;", {_fName, _fName});
    }
    out << "}"; //end namespace
}

Function CodeGenerator::initProtoFun()
{
    Function initFun;
    vector<string> body;

    auto params = _handlerGen->getHeaderParams({"local"});

    initFun.setDeclaration(fmt("%{%s_}init", {_options.isCpp ? "" : toLower(_fName)}),
                           _options.isCpp ? "void" : toLower(_fName) + "_Obj",
                           params);

    if(!_options.isCpp) {
        body << fmt("%s obj = {0};", {toLower(_fName) + "_Obj"});
    }

    for(auto &param : params)
    {
        body << fmt("%s%s = %s;",
        {_options.isCpp ? "base->" : "obj.", param.name, param.name});
    }

    if(!_options.isCpp) { body << "return obj;"; }

    initFun.setBody(body);
    if(_options.isCpp) { initFun.setContainedClass(_fName); }

    return initFun;
}

StructCpp CodeGenerator::protoObj()
{
    StructCpp protoObj;
     protoObj.setName(fmt("_%s_Obj", {toLower(_fName)}));

    auto params = _handlerGen->getHeaderParams({"local"});

    for(auto& param : params) {
        protoObj.addField({param.type, param.name});
    }

    protoObj.addField({cbsStruct().getName(), "_CBsStruct"});

    return protoObj;
}

Function CodeGenerator::setCbsFun()
{
    Function setCbsFun;
    std::vector<Parameter> params;

    params += {toLower(_fName) + "_Obj", "*obj"};
    params += {fmt("%s_CBsStruct", {toLower(_fName)}), "*str"};
    setCbsFun.setDeclaration(toLower(_fName) + "_setCBs", "void", params);

    vector<string> body;
    for(auto rdm :_rdms) {
        auto cbName = _handlerGen->receiveMsgCb(rdm).name();
        body << fmt("obj->_CBsStruct.%s = str->%s;", {cbName, cbName});
    }

    auto checkHeaderCb = _handlerGen->checkHeaderCb().name();
    body << fmt("obj->_CBsStruct.%s = str->%s;", {checkHeaderCb, checkHeaderCb});
    auto sendCbName = _handlerGen->sendCb().name();
    body << fmt("obj->_CBsStruct.%s = str->%s;", {sendCbName, sendCbName});

    setCbsFun.setBody(body);

    return setCbsFun;
}

Function CodeGenerator::resetCbsFun()
{
    Function resetCbsFun;
    resetCbsFun.setDeclaration(toLower(_fName) +"_resetCBs", "void", {
                                   toLower(_fName) + "_Obj", "*obj"});

    vector<string> body;
    for(auto rdm :_rdms) {
        body << "obj->_CBsStruct." + _handlerGen->receiveMsgCb(rdm).name() + " = 0;";
    }
    body << "obj->_CBsStruct." + _handlerGen->checkHeaderCb().name() + " = 0;";
    body << "obj->_CBsStruct." + _handlerGen->sendCb().name() + " = 0;";

    resetCbsFun.setBody(body);

    return resetCbsFun;
}

StructCpp CodeGenerator::cbsStruct()
{
    StructCpp cbsStruct;

    cbsStruct.setName(toLower(_fName) + "_CBsStruct");
    cbsStruct.setTypeDef(true);

    for(auto rdm :_rdms) {
        auto param = _handlerGen->receiveMsgCb(rdm).pointer();
        cbsStruct.addField({param.type, param.name});
    }
    auto checkHeaderParam = _handlerGen->checkHeaderCb().pointer();
    cbsStruct.addField({checkHeaderParam.type, checkHeaderParam.name});
    auto sendParam = _handlerGen->sendCb().pointer();
    cbsStruct.addField({sendParam.type, sendParam.name});

    return cbsStruct;
}

StructCpp CodeGenerator::localParams()
{
    StructCpp localParams;
    localParams.setName("LocalParams");

    for(auto& param : _handlerGen->getHeaderParams({"local"})) {
        localParams.addField({param.type, param.name});
    }

    return localParams;
}

Function CodeGenerator::localParamsFun()
{
    Function localParamsFun;
    StructCpp params = localParams();

    localParamsFun.setDeclaration("GetLocalParams", params.getName());
    localParamsFun.setContainedClass(_fName);


    vector<string> body;
    string var = "buf";
    body << fmt("%s %s;", {params.getName(), var});

    for (auto field : params.getFields()) {
        body << fmt("%s.%s = %s%s;", {var, field.name,
                                      _options.isCpp ? "base->" : "",
                                      field.name});
    }
    body << fmt("return %s;", {var});

    localParamsFun.setBody(body);

    return localParamsFun;
}
