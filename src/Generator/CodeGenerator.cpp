#include "CodeGenerator.h"
#include <fstream>
CodeGenerator::CodeGenerator(string fileName, Formater exchangeDescription,
                             bool addrOption, bool qtOption, bool qtCppOption)
{
    _qtOption = qtOption;
    _qtCppOption = qtCppOption;
    _addrOption = addrOption;
    _fileName = fileName;
    _fileNameLowerCase = fileName;

    std::transform(_fileNameLowerCase.begin(), _fileNameLowerCase.end(),
                   _fileNameLowerCase.begin(),
                   [] (unsigned char c){ return std::tolower(c);
    });

    _exchangeDescription = exchangeDescription;

    vector<Parameter> params;

    if(addrOption)
        params.push_back(GetAddrParameter());

    params.push_back({"void*", "buffer"});
    params.push_back({"uint16_t", "size"});

    if(qtOption || qtCppOption)
    {
        _cbSendFun.SetDeclaration("send_cb", "void", params);

        vector<Parameter> parameters;
        //parameters.push_back({ProtocolObj.GetName(),"*obj"});
        parameters.push_back({"CBsStruct", "*str"});
        _setCbsFun.SetDeclaration("setCBs","void", parameters);
        _resetCbsFun.SetDeclaration("resetCBs", "void");
        _localParams.SetName("LocalParams");
        _cbsStruct.SetName("CBsStruct");
    }
    else
    {
        _typeObjName = _fileNameLowerCase + "_Obj";
        _protocolObj.SetName(b_und + _fileNameLowerCase + "_Obj");
        _cbSendFun.SetDeclaration(_fileNameLowerCase + "_send_cb", "void", params);
        vector<Parameter> parameters;
        parameters.push_back({_typeObjName, "*obj"});
        parameters.push_back({_fileNameLowerCase + b_und + "CBsStruct", "*str"});
        _setCbsFun.SetDeclaration(_fileNameLowerCase + "_setCBs", "void", parameters);
        _resetCbsFun.SetDeclaration(_fileNameLowerCase +"_resetCBs", "void",
                                {_typeObjName, "*obj"});

        _cbsStruct.SetName(_fileNameLowerCase + "_CBsStruct");
    }
}

void CodeGenerator::Generate()
{
    for(auto info: _exchangeDescription.codeList)
    {
        auto decl = GenEnumDecl(info, EnumerableType::Code);
        _types.push_back({decl.GetName(),fieldType::Code});
        _enumDeclarations.push_back(decl);
    }

    for(auto info: _exchangeDescription.typeList)
    {
        auto decl = GenEnumDecl(info, EnumerableType::Type);
        _types.push_back({decl.GetName(),fieldType::Type});
        _enumDeclarations.push_back(decl);
    }

    for(auto info: _exchangeDescription.headerList) {
        _headerDeclarations = GenStructDecl(info, ComplexType::Header);
    }

    for(auto info: _exchangeDescription.enumList)
    {
        auto decl = GenEnumDecl(info, EnumerableType::Enum);
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

    vector<Parameter> paramBuf;
    for(auto var:_headerDeclarations.fields)
    {
        if(var.data.isWithInitType && var.data.initType == "local") {
            _localVar.push_back({var.fieldName, var.fieldName});
            paramBuf.push_back({var.type.first, var.fieldName});
        }
    }

    _errorEnum = GenEnumErrorCodeDecl();

    //initFunc.ExternDeclaration(true);

    if(_qtOption || _qtCppOption)
    {
        _localParams.SetBody(SetBodyLocaParamsStruct(paramBuf));
        _getLocalParamsFun.SetDeclaration("GetLocalParams", _localParams.GetName());
        _getLocalParamsFun.SetBody(SetBodyGetLocaParamsFun(paramBuf));
        _getLocalParamsFun.SetContainedClass(_fileName);
    }

    if(_qtOption || _qtCppOption) {
        _initFun.SetDeclaration("init", "void", paramBuf);
        _initFun.SetContainedClass(_fileName);
    }
    else
        _initFun.SetDeclaration(_fileNameLowerCase + "_init",_typeObjName,paramBuf);

    vector<string> strings;
    auto it = paramBuf.begin();
    auto it1 = _localVar.begin();
    if(!(_qtOption || _qtCppOption))
        strings.push_back(_typeObjName + spc + "obj" + smcln);

    for(;it1 != _localVar.end() && it != paramBuf.end(); it++, it1++)
    {
        if(!(_qtOption || _qtCppOption))
            strings.push_back(PrintVarDefinition("obj." + it1->second, it->name));
        else
            strings.push_back(PrintVarDefinition(PrintClassText(it1->second),
                                                 it->name));
    }

    if(!(_qtOption || _qtCppOption))
        strings.push_back("return obj" + smcln);

    _initFun.SetBody(strings);

    AnalizeRules(_exchangeDescription.rules, _structDeclarations, _headerDeclarations);

    _setCbsFun.SetBody(SetBodySetCbs());
    _resetCbsFun.SetBody(SetBodyResetCbs());

    strings.clear();
    for(auto var : paramBuf)
        strings.push_back(var.type + spc + var.name + smcln);

    strings.push_back(_cbsStruct.GetName() + " _CBsStruct" + smcln);
    _protocolObj.SetBody(strings);

    if(_qtOption || _qtCppOption) {
        GenerateSourceQt();
        GenerateHeaderQt();
    }
    else {
        GenerateSource();
        GenerateHeader();
    }
}

vector<string> CodeGenerator::MainDesFormat()
{
    vector<Parameter> parameters;
    Parameter parameter;

    if(!(_qtOption || _qtCppOption))
        parameters.push_back({_typeObjName, "*obj"});

    if(_addrOption)
        parameters.push_back(GetAddrParameter());

    parameters.push_back({"void*", "p"});
    parameters.push_back({"uint16_t", "dataLen"});
    //ParseFun.ExternDeclaration(true);
    if(_qtOption || _qtCppOption) {
        _parseFun.SetContainedClass(_fileName);
        _parseFun.SetDeclaration("parse", _errorEnum.GetName(), parameters);
    } else {
        _parseFun.SetDeclaration(_fileNameLowerCase + "_parse", _fileNameLowerCase +
                                b_und + _errorEnum.GetName(),parameters);
    }

    vector<string> strings;
    auto stat = IfElseStatementCpp().AddCase("dataLen == 0","return" + lsb +
                                             _errorEnum.GetPrefix() +
                                             _errorEnum.GetName() +rsb+ "1" + smcln);
    auto buff = stat.GetDefinition();
    strings.push_back(PrintVarDeclaration("char*", "l_p", "(char*)p"));
    strings.insert(strings.begin(), buff.begin(), buff.end());
    strings.push_back(PrintVarDeclaration("uint8_t", "*op_status"));
    strings.push_back(PrintVarDeclaration("uint8_t", "status_Buf","1"));
    strings.push_back(PrintVarDefinition("op_status", "&status_Buf"));
    strings.push_back(_headerDeclarations.PrintVarDecl() + smcln);
    strings.push_back(PrintClassText(_headerDeclarations.
                                     PrintSerDesCall(FunType::Des,"&header") + smcln));
    IfElseStatementCpp handler;
    handler.AddCase("*op_status" + eql + "0","return" + lsb + _errorEnum.GetPrefix() +
                    _errorEnum.GetName() +rsb+  "1;");

    auto buf = handler.GetDefinition();
    strings.insert(strings.end(), buf.begin(), buf.end());
    strings.push_back("l_p += " + _headerDeclarations.Size().Get() + smcln);
    SwitchCpp switchOperator;
    string f, typeName;
    for(auto var:_headerDeclarations.fields)
    {
        if( var.type.first == _exchangeDescription.codeList.at(0).first)
           switchOperator.SetSwitchingParameter("header." + var.fieldName);
        if( var.type.first == _exchangeDescription.typeList.at(0).first)
            typeName = var.fieldName;
    }
    strings.push_back(f);

    vector<string> buffer;
    string varType, varCommand, localStatement, dataLenStatement;
    IfElseStatementCpp if_else_localStatement, if_else_dataLenStatement;

    vector<string> specialParameterFromHeader, parameterBuffer;
    for(auto var:_headerDeclarations.fields) {
        if(var.data.initType == "local" || var.data.initType == "remote")
            specialParameterFromHeader.push_back("header." + var.fieldName);
    }
    if(!(_qtOption || _qtCppOption)) parameterBuffer.push_back("obj");
    if(_addrOption) parameterBuffer.push_back(GetAddrParameter().name);

    parameterBuffer.push_back("&str");
    parameterBuffer.insert(parameterBuffer.end(), specialParameterFromHeader.begin(),
                           specialParameterFromHeader.end());

    varType = interfaceReceiveFunctions.begin()->type;
    varCommand = interfaceReceiveFunctions.begin()->command;
    for(auto var = interfaceReceiveFunctions.begin();
        var<interfaceReceiveFunctions.end(); var++)
    {
        if(varCommand != var->command || varType != var->type)
        {
            if(_qtOption || _qtCppOption)
            {
                if_else_localStatement.AddCase("header." +
                                               typeName + eql + varType,
                                               if_else_dataLenStatement.GetDefinition());
            }
            else
            {
                if_else_localStatement.AddCase("header." +
                                               typeName + eql + _fileNameLowerCase +
                                               b_und + varType,
                                               if_else_dataLenStatement.GetDefinition());
            }

            if_else_dataLenStatement.Clear();
            varType = var->type;
        }

        if(varCommand != var->command)
        {
            if(_qtOption || _qtCppOption)
                switchOperator.AddCase(varCommand,if_else_localStatement.GetDefinition());
            else {
                switchOperator.AddCase(_fileNameLowerCase + b_und + varCommand,
                                       if_else_localStatement.GetDefinition());
            }

            if_else_localStatement.Clear();
            varCommand = var->command;
            //if_else_dataLenStatement.clear();
        }
        if(var->withPacket)
        {
            buffer.clear();
            buffer.push_back(var->packet.GetCodeName() + spc +"str" + smcln);
            if(var->packet.WithVarArray)
            {
                int a = 0;
                for(auto arr : var->packet.varArrays)
                {
                    string allocateSize = lsb + "header.dataLen / " +
                            arr.varArraySize.Get() + rsb + "*" +
                            PrintSizeOf(arr.varArrayTypeName)+ "+" +
                            arr.varArraySize.Get();

                    if(_qtOption)
                    {
                        buffer.push_back("std::unique_ptr<char[]> buffer" +
                                         to_string(a) + lsb + "new char" +
                                         PutInSqBraces(allocateSize) + rsb + smcln);
                        buffer.push_back("str." + arr.varArrayType + "=" + lsb +
                                         arr.varArrayTypeName+ "*" + rsb + "buffer" +
                                         to_string(a) + ".get()" + smcln);
                    }
                    else if(!_qtCppOption)
                    {
                        buffer.push_back("uint16_t" + spc + "buffer" + to_string(a) +
                                         PutInSqBraces(allocateSize) + smcln);
                        buffer.push_back("str." + arr.varArrayType + "=" + lsb +
                                         arr.varArrayTypeName+ "*" + rsb + "buffer" +
                                         to_string(a) + smcln);
                    }
                    a++;
                }
            }
            buffer.push_back(PrintClassText(
                                 var->packet.PrintSerDesCall(FunType::Des,"&str") +
                                 smcln));

            IfElseStatementCpp handler;
            handler.AddCase("*op_status" + eql + "0","return" + lsb +
                            _errorEnum.GetPrefix() + _errorEnum.GetName() + rsb + "1;");
            auto strings = handler.GetDefinition();
            buffer.insert(buffer.end(), strings.begin(), strings.end());
            if((_qtOption || _qtCppOption) == false)
            {
                auto str = IfElseStatementCpp().
                        AddCase(PrintStructText(var->func.GetFunctionName()) + eql +
                                "0","return" + lsb + _errorEnum.GetPrefix() +
                                _errorEnum.GetName() +rsb + "1;").GetDefinition();

                buffer.insert(buffer.begin(), str.begin(), str.end());
            }
            buffer.push_back(PrintStructText(var->func.GetCall(parameterBuffer,true)));

            if(var->packet.WithVarArray)
                if_else_dataLenStatement.AddCase("header.dataLen" + neql +"0",buffer);
            else {
                if_else_dataLenStatement.AddCase("header.dataLen" + eql +
                                                 var->packet.Size().Get(),buffer);
            }
        }
        else
        {
            buffer.clear();
            if((_qtOption || _qtCppOption) == false)
            {
                auto str = IfElseStatementCpp().
                        AddCase(PrintStructText(var->func.GetFunctionName()) + eql +
                                "0", "return" + lsb + _errorEnum.GetPrefix() +
                                _errorEnum.GetName() + rsb +  "1;").GetDefinition();
                buffer.insert(buffer.begin(),str.begin(),str.end());
            }
            vector<string> buf;
            if(!(_qtOption || _qtCppOption)) buf.push_back("obj");
            if(_addrOption) buf.push_back(GetAddrParameter().name);

            buf.insert(buf.end(),specialParameterFromHeader.begin(),
                       specialParameterFromHeader.end());
            buffer.push_back(PrintStructText(var->func.GetCall(buf, true)));
            if_else_dataLenStatement.AddCase("header.dataLen" + eql + "0",buffer);
        }
        if(var == --interfaceReceiveFunctions.end())
        {
            if(_qtOption || _qtCppOption) {
                if_else_localStatement.AddCase("header." + typeName + eql + varType,
                                               if_else_dataLenStatement.GetDefinition());
            } else
            {
                if_else_localStatement.AddCase("header." + typeName + eql +
                                               _fileNameLowerCase + b_und + varType,
                                               if_else_dataLenStatement.GetDefinition());
            }
            if_else_dataLenStatement.Clear();
            varType = var->type;

            if(_qtOption || _qtCppOption) {
                switchOperator.AddCase(varCommand,
                                       if_else_localStatement.GetDefinition());
            }
            else {
                switchOperator.AddCase(_fileNameLowerCase + b_und + varCommand,
                                       if_else_localStatement.GetDefinition());
            }
            if_else_localStatement.Clear();
            varCommand = var->command;
        }
    }
    auto text = switchOperator.GetDeclaration();
    strings.insert(strings.end(),text.begin(),text.end());
    strings.push_back("return" + lsb + _errorEnum.GetPrefix() + _errorEnum.GetName() +
                      rsb +"0;");
    _parseFun.SetBody(strings);

    return _parseFun.GetDefinition();
}

vector<string> CodeGenerator::MainSerFormaters()
{
    vector<string> strings;
    for( auto iface: interfaceSenderFunctions)
    {
        auto buf = iface.func.GetDefinition();
        strings.insert(strings.end(),buf.begin(),buf.end());
    }
    return strings;
}

EnumDescription CodeGenerator::GenEnumDecl(Enum IntermediateEnum, EnumerableType type)
{
    EnumDescription decl;
    decl.type = type;
    decl.SetName(IntermediateEnum.first);
    decl.fields = IntermediateEnum.second;

    if((_qtOption || _qtCppOption) == false)
        decl.SetPrefix(_fileNameLowerCase);

    return decl;
}

EnumDescription CodeGenerator::GenEnumErrorCodeDecl()
{
    EnumDescription decl;
    decl.type = EnumerableType::Enum;
    decl.SetName("ErrorCode");

    vector<FieldDataEnum> fields;
    fields.push_back({"OK", 0});
    fields.push_back({"ERROR", 1});
    decl.fields = fields;

    if((_qtOption || _qtCppOption) == false)
        decl.SetPrefix(_fileNameLowerCase);

    return decl;
}

Parameter CodeGenerator::GetAddrParameter()
{
    Parameter parameter;

    if((_qtOption || _qtCppOption) == false)
         parameter.type = _fileNameLowerCase + "_Addr*";
    else
        parameter.type = "Addr*";

    parameter.name = "addr";
    return parameter;
}

StructCpp CodeGenerator::GenAddrDecl()
{
    StructCpp decl;
    decl.SetName("Addr");
    vector<string> body;

    if((_qtOption || _qtCppOption) == false)
        body.push_back("uint32_t ip;");
    else
        body.push_back("QHostAddress ip;");

    body.push_back("uint16_t port;");

    decl.SetBody(body);

    if((_qtOption || _qtCppOption) == false)
        decl.SetName(_fileNameLowerCase + "_Addr");
    else
        decl.SetName("Addr");

    return decl;
}

ComplexTypeDescription CodeGenerator::GenStructDecl(Struct& IntermediateStruct, ComplexType type)
{
    ComplexTypeDescription decl;
    if((_qtOption || _qtCppOption) == false){
        decl.SetPrefix(_fileNameLowerCase);
        decl.cObjType = _fileNameLowerCase + "_Obj";
    } else
        decl.qtOption = true;

    decl.qtCppOption = _qtCppOption;
    decl.SetBlockType(type);
    decl.SetName(IntermediateStruct.first);
    for(auto var: IntermediateStruct.second)
    {
        if(var.second.isStdType)
        {
            auto fieldType = ConvertToCStdType(var.second.type);
            decl.addField(var, {fieldType.first, fieldType::std}, fieldType.second);
        } else
        {
            auto it = FindInVector(_types,var.second.type);
            if(it != _types.end())
            {
                if(it->second == fieldType::Struct)
                {
                    auto p = FindDeclaration(_structDeclarations, var.second.type);
                    decl.addField(var, {p->name,fieldType::Struct}, p->Size());
                }
                else if(it->second == fieldType::Enum)
                {
                    auto p = FindDeclaration(_enumDeclarations, var.second.type);
                    decl.addField(var, {p->GetName(),fieldType::Enum}, p->Size());
                }
                else if(it->second == fieldType::Code)
                {
                    auto p = FindDeclaration(_enumDeclarations, var.second.type);
                    decl.addField(var, {p->GetName(),fieldType::Code}, p->Size());
                }
                else if(it->second == fieldType::Type)
                {
                    auto p = FindDeclaration(_enumDeclarations, var.second.type);
                    decl.addField(var, {p->GetName(),fieldType::Type}, p->Size());
                }
            }
            else {
                throw invalid_argument("type not found: " +
                                       PutInQuotes(var.second.type) + " in " +
                                       PutInQuotes(IntermediateStruct.first));
            }
        }
    }
    return decl;
}


void CodeGenerator::TestPrint(vector<string> strings)
{
    for(auto str: strings)
        cout<<str<<endl;
}

void CodeGenerator::PrintToFile(ofstream& stream, vector<string> strings)
{
    for(auto str: strings)
        stream<<str<<endl;
}

void CodeGenerator::PrintToFile(ofstream& stream, string str)
{
        stream<<str<<endl;
}


pair<string,size_t> CodeGenerator::ConvertToCStdType(string slpdType)
{
    if(slpdType ==  "char")  return {"char",1};
    if(slpdType ==  "s8")  return {"int8_t",1};
    if(slpdType ==  "s16") return {"int16_t",2};
    if(slpdType ==  "s32") return {"int32_t",4};
    if(slpdType ==  "s64") return {"int64_t",8};
    if(slpdType ==  "i8")  return {"int8_t",1};
    if(slpdType ==  "i16") return {"int16_t",2};
    if(slpdType ==  "i32") return {"int32_t",4};
    if(slpdType ==  "i64") return {"int64_t",8};
    if(slpdType ==  "u8")  return {"uint8_t",1};
    if(slpdType ==  "u16") return {"uint16_t",2};
    if(slpdType ==  "u32") return {"uint32_t",4};
    if(slpdType ==  "u64") return {"uint64_t",8};
    if(slpdType ==  "f32") return {"float",4};
    if(slpdType ==  "f64") return {"double",8};
    throw invalid_argument("ConvertTo_C_stdType: received not std type");
}

string CodeGenerator::GetVersion()
{
    return string("// SLPD Version 1.1 Build " + string(__TIME__) + ' ' +
                  string(__DATE__) +"\n\n");
}

void CodeGenerator::GenerateHeader()
{
    string str = _fileName;
    std::transform(str.begin(), str.end(),str.begin(), ::toupper);
    _oStream.open(_fileName + ".h");
    _oStream<<GetVersion();
    _oStream << "#ifndef" + spc + str + "_H" << endl;
    _oStream << "#define" + spc + str + "_H" << endl;
    _oStream << endl;
    _oStream << PrintInclude("stdint.h") << endl;
    _oStream << endl;

    PrintToFile(_oStream,_errorEnum.PrintDecl(false));

    if(_addrOption)
        PrintToFile(_oStream,GenAddrDecl().GetDeclaration());

    _oStream<<endl;

    for(auto var : _enumDeclarations) {
        PrintToFile(_oStream,var.PrintDecl());
    }

    for(auto var : _structDeclarations) {
        PrintToFile(_oStream,var.PrintDecl());
    }

    _oStream << "typedef struct " + _protocolObj.GetName() + spc + _typeObjName + smcln
           << endl;

    _oStream<<endl;

    vector<string> CBsStructBody;

    for(auto var :interfaceReceiveFunctions) {
        CBsStructBody.push_back(var.func.GetFunctionPointerDeclaration());
    }
    CBsStructBody.push_back(_cbSendFun.GetFunctionPointerDeclaration());

    _cbsStruct.SetBody(CBsStructBody);
    PrintToFile(_oStream, _cbsStruct.GetDeclaration());

    _oStream << endl;

    PrintToFile(_oStream, _protocolObj.GetDeclaration(false));

    _oStream << endl;

    _oStream<<"/* senders */"<<endl;
    for(auto var :interfaceSenderFunctions) {
        _oStream << var.func.GetDeclaration() << endl;
    }

    _oStream << endl;
    PrintToFile(_oStream, _resetCbsFun.GetDeclaration());
    PrintToFile(_oStream, _setCbsFun.GetDeclaration());
    PrintToFile(_oStream, _initFun.GetDeclaration());
    PrintToFile(_oStream, _parseFun.GetDeclaration());

    _oStream << "#endif" << endl;
    _oStream.close();
}

void CodeGenerator::GenerateHeaderQt()
{
    string str = _fileName;
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    _oStream.open(_fileName + ".h");
    _oStream<<GetVersion();
    _oStream << "#ifndef" +spc + str + "_H" << endl;
    _oStream << "#define" +spc + str + "_H" << endl;
    _oStream << endl;
    _oStream << PrintInclude("stdint.h") << endl;
    _oStream << PrintInclude("QObject") << endl;
    _oStream << PrintInclude("vector") << endl;

    if(_addrOption)
        _oStream << PrintInclude("QHostAddress") << endl;

    _oStream << endl;
    _oStream << "namespace " + _fileName + "Space" + lb << endl;
    _oStream << endl;

    PrintToFile(_oStream,_errorEnum.PrintDecl(false));

    if(_addrOption)
        PrintToFile(_oStream,GenAddrDecl().GetDeclaration());

    _oStream << endl;

    if(!_localVar.empty())
        PrintToFile(_oStream, _localParams.GetDeclaration());

    _oStream << endl;
    for(auto var : _enumDeclarations) {
        PrintToFile(_oStream,var.PrintDecl());
    }

    for(auto var : _structDeclarations) {
        PrintToFile(_oStream,var.PrintDecl());
    }

    if(_qtCppOption)
    {
        for(auto var : _structDeclarations) {
            PrintToFile(_oStream, var.GetCompareFun().GetDeclaration());
        }
        _oStream << endl;
    }

    PrintToFile(_oStream, "class Base;");
    PrintToFile(_oStream, "class "+ _fileName + ": public QObject");
    PrintToFile(_oStream, lb);
    PrintToFile(_oStream, tab + "Q_OBJECT");
    PrintToFile(_oStream, "signals:");
    vector<string> CBsStructBody;

    for(auto var :interfaceReceiveFunctions) {
        var.func.SetExternDeclaration(false);
        PrintToFile(_oStream, tab + var.func.GetDeclaration());
    }

    PrintToFile(_oStream, tab + _cbSendFun.GetDeclaration());
    _oStream << endl;
    PrintToFile(_oStream, "public:");
    _oStream << tab + "/* senders */" << endl;
    for(auto var : interfaceSenderFunctions) {
        _oStream << tab + var.func.GetDeclaration() << endl;
    }

    _oStream << endl;
    PrintToFile(_oStream,tab + _initFun.GetDeclaration());

    if(!_localVar.empty())
        PrintToFile(_oStream, tab + _getLocalParamsFun.GetDeclaration());

    PrintToFile(_oStream, tab + _parseFun.GetDeclaration());
    PrintToFile(_oStream, tab + _fileName + "(QObject *parent = nullptr)" + smcln);
    _oStream << endl;
    PrintToFile(_oStream, "private:");
    PrintToFile(_oStream, tab + "Base* base;");
    PrintToFile(_oStream, rb + smcln);
    _oStream << rb << "//end namespace" << endl << endl;
    _oStream << "#endif" << endl;
    _oStream.close();
}

void CodeGenerator::GenerateSource()
{
    _oStream.open(_fileName + ".c");
    _oStream << GetVersion();
    _oStream << PrintInclude("string.h") << endl;
    _oStream << PrintInclude(_fileName + ".h", true) << endl;

    //for(auto var:localVar)
    //    PrintToFile(stream,PrintVarDeclaration("uint16_t",var.second));

    _oStream << endl;
    PrintToFile(_oStream, _initFun.GetDefinition());
    _oStream << endl;
    PrintToFile(_oStream, _headerDeclarations.PrintDecl());
    PrintToFile(_oStream, _headerDeclarations.PrintSerDesDeclaration(FunType::Ser));
    PrintToFile(_oStream, _headerDeclarations.PrintSerDesDeclaration(FunType::Des));
    for(auto var : _structDeclarations) {
        PrintToFile(_oStream, var.PrintSerDesDeclaration(FunType::Ser));
        PrintToFile(_oStream, var.PrintSerDesDeclaration(FunType::Des));
    }

    PrintToFile(_oStream, MainSerFormaters());

    _oStream << endl;
    PrintToFile(_oStream, _setCbsFun.GetDefinition());
    PrintToFile(_oStream, _resetCbsFun.GetDefinition());
    PrintToFile(_oStream, MainDesFormat());

    _oStream.close();
}

void CodeGenerator::GenerateSourceQt()
{
    _oStream.open(_fileName + ".cpp");
    _oStream << GetVersion();
    _oStream << PrintInclude("memory") << endl;
    _oStream << PrintInclude("string.h") << endl;
    _oStream << PrintInclude(_fileName + ".h", true) << endl;
    _oStream << "namespace " + _fileName + "Space" << endl << lb << endl;

    if(_qtCppOption)
    {
        for(auto var : _structDeclarations) {
            PrintToFile(_oStream, var.GetCompareFun().GetDefinition());
            _oStream << endl;
        }
        _oStream << endl;
    }

    PrintToFile(_oStream, _headerDeclarations.PrintDecl());

    _oStream << "class Base: QObject" << endl << lb << endl;
    _oStream << "public:" << endl;
    _oStream << "Base(QObject *parent = nullptr):QObject(parent){}" << endl;

    for(auto var:_localVar)
        PrintToFile(_oStream, PrintVarDeclaration("uint16_t", var.second));

    _oStream << endl;
    PrintToFile(_oStream,_headerDeclarations.PrintSerDesDeclaration(FunType::Ser, false));
    PrintToFile(_oStream,_headerDeclarations.PrintSerDesDeclaration(FunType::Des, false));
    for(auto var : _structDeclarations) {
        PrintToFile(_oStream,var.PrintSerDesDeclaration(FunType::Ser, false));
        PrintToFile(_oStream,var.PrintSerDesDeclaration(FunType::Des, false));
    }
    _oStream << rb + smcln << endl; //end Base class

    PrintToFile(_oStream, _initFun.GetDefinition());

    if(!_localVar.empty())
        PrintToFile(_oStream,_getLocalParamsFun.GetDefinition());

    _oStream << endl;
    PrintToFile(_oStream, MainSerFormaters());
    PrintToFile(_oStream, MainDesFormat());
    PrintToFile(_oStream, ConstructDefinition());
    _oStream << rb << endl; //end namespace
    _oStream.close();
}

vector<string> CodeGenerator::ConstructDefinition()
{
    vector<string> strings;
    strings.push_back(_fileName + "::" + _fileName + lsb + "QObject *parent" +
                      rsb + cln + "QObject(parent)");
    strings.push_back(lb);
    strings.push_back(tab + "base = new Base(this)" + smcln);
    strings.push_back(rb);
    return strings;
}

vector<ComplexTypeDescription>::iterator CodeGenerator::
FindDeclaration(vector<ComplexTypeDescription>& DataStruct, string typeName)
{
    auto it = std::find_if(DataStruct.begin(), DataStruct.end(),
                           [typeName](const ComplexTypeDescription& p)
                            {
                                return p.name == typeName ;
                            });
    return it;
}

vector<EnumDescription>::iterator CodeGenerator::
FindDeclaration(vector<EnumDescription> &DataStruct, string content)
{
    auto it = std::find_if(DataStruct.begin(), DataStruct.end(),
                           [content](const EnumDescription& p)
                            {
                                return p.GetName() == content ;
                            });
    return it;
}

void CodeGenerator::AnalizeRules(vector<Rule> slpdRules,
                                 vector<ComplexTypeDescription> structDeclarations,
                                 ComplexTypeDescription header)
{
        string codeFieldName;
        vector<Parameter> specialParameter, specialParameterRemote;
        for(auto var : header.fields)
        {
            if(var.data.initType == "local" || var.data.initType == "remote")
                specialParameter.push_back({var.type.first, var.fieldName});

            if(var.data.initType == "remote")
                specialParameterRemote.push_back({var.type.first, var.fieldName});

            if(var.data.type == _exchangeDescription.codeList.at(0).first)
                codeFieldName = var.fieldName;
        }

        vector<SimpleRule> rules;
        for(const auto &rule : slpdRules)
        {
            SimpleRule sRule;
            sRule.command = rule.command;
            sRule.sendType = rule.sendType;
            sRule.responseType = rule.responseType;
            sRule.hasResponse = rule.isWithResponse;

            sRule.sendPacket = rule.sendPacket;
            sRule.responsePacket = rule.responsePacket;
            sRule.isEmptySend = rule.isEmptySend;
            sRule.hasEmptyResponse = rule.isEmptyResponse;
            rules.push_back(sRule);
            if(rule.isReverse)
            {
                sRule.sendPacket = rule.responsePacket;
                sRule.responsePacket = rule.sendPacket;
                sRule.isEmptySend = rule.isEmptyResponse;
                sRule.hasEmptyResponse = rule.isEmptySend;
                rules.push_back(sRule);
            }
        }

        for(const auto &rule : rules)
        {
            Parameter instancePointer = {_typeObjName, "*obj"};
            Parameter sendParam, receiveParam;
            Function func, funcReceiveSend, funcReceiveResponse;

            if(_qtOption || _qtCppOption){
                func.SetContainedClass(_fileName);
                funcReceiveSend.SetContainedClass(_fileName);
                funcReceiveResponse.SetContainedClass(_fileName);
            }

            vector<ComplexTypeDescription>::iterator it = FindDeclaration(structDeclarations,rule.sendPacket);
            vector<ComplexTypeDescription>::iterator it_1;

            if(!rule.hasEmptyResponse && rule.hasResponse)
                it_1 = FindDeclaration(structDeclarations, rule.responsePacket);

            if(!rule.isEmptySend)
                sendParam = {it->GetCodeName(), "*str"};

            if(!rule.hasEmptyResponse && rule.hasResponse)
                receiveParam = {it_1->GetCodeName(), "*str"};

            std::string funNameS, funNameR;
            if(_qtOption || _qtCppOption) {
                funNameS = rule.sendType + b_und + rule.command;
                funNameR = rule.responseType + b_und + rule.command;
            }
            else
            {
                funNameS = _fileNameLowerCase + b_und + rule.sendType + b_und +
                        rule.command;
                funNameR = _fileNameLowerCase + b_und + rule.responseType + b_und +
                        rule.command;
            }

            std::string funNameSwl = rule.sendType + b_und + rule.command;
            std::string funNameRwl = rule.responseType + b_und + rule.command;

            std::transform(funNameS.begin(), funNameS.end(), funNameS.begin(),
                [](unsigned char c){ return std::tolower(c); });
            std::transform(funNameR.begin(), funNameR.end(), funNameR.begin(),
                [](unsigned char c){ return std::tolower(c); });
            std::transform(funNameSwl.begin(), funNameSwl.end(), funNameSwl.begin(),
                [](unsigned char c){ return std::tolower(c); });
            std::transform(funNameRwl.begin(), funNameRwl.end(), funNameRwl.begin(),
                [](unsigned char c){ return std::tolower(c); });

            vector<string> body;

            enum class ParamsType {Send, Receive};
            enum class RulePart {Left, Right};
            auto getFunParams = [=](ParamsType type, RulePart part)
            {
                vector<Parameter> funParams;

                if(!(_qtOption || _qtCppOption)) funParams.push_back(instancePointer);
                if(_addrOption) funParams.push_back(GetAddrParameter());

                    if(part == RulePart::Left && !rule.isEmptySend)
                        funParams.push_back(sendParam);

                    if(part == RulePart::Right && !rule.hasEmptyResponse)
                        funParams.push_back(receiveParam);

                    vector<Parameter> params{};

                    if(type == ParamsType::Send)
                        params = specialParameterRemote;

                    if(type == ParamsType::Receive)
                        params = specialParameter;

                    funParams.insert(funParams.end(), params.begin(), params.end());

                    return funParams;
            };

            funcReceiveSend.SetExternDeclaration(true);
            funcReceiveResponse.SetExternDeclaration(true);

            if(!rule.isEmptySend)
            {
                auto funParams = getFunParams(ParamsType::Send, RulePart::Left);

                func.SetDeclaration(funNameS + "_s", "void", funParams);
                func.SetBody(GetInterfaceSendFuncBody(codeFieldName, rule.command,
                                                      rule.sendType, *it,
                                                      specialParameterRemote));

                InterfaceFunction interfaceFunction(rule.command, rule.sendType,
                                                    func, *it);
                interfaceSenderFunctions.push_back(interfaceFunction);

                funParams =  getFunParams(ParamsType::Receive, RulePart::Left);

                funcReceiveSend.SetDeclaration(funNameSwl + "_r", "void", funParams);
            }
            else
            {
                auto funParams = getFunParams(ParamsType::Send, RulePart::Left);

                func.SetDeclaration(funNameS + "_s" + "_empty", "void", funParams);
                func.SetBody(GetInterfaceSendFuncBody(codeFieldName, rule.command,
                                                      rule.sendType,
                                                      specialParameterRemote));

                InterfaceFunction interfaceFunction(rule.command, rule.sendType, func);
                interfaceSenderFunctions.push_back(interfaceFunction);

                funParams = getFunParams(ParamsType::Receive, RulePart::Left);

                funcReceiveSend.SetDeclaration(funNameSwl + "_r" + "_empty", "void",
                                               funParams);
            }

            if(rule.hasResponse)
            {
                if(rule.hasEmptyResponse)
                {
                    auto funParams = getFunParams(ParamsType::Send, RulePart::Right);

                    func.SetDeclaration(funNameR + "_s"+ "_empty", "void", funParams);
                    func.SetBody(GetInterfaceSendFuncBody(codeFieldName, rule.command,
                                                          rule.responseType,
                                                          specialParameterRemote));

                    InterfaceFunction interfaceFunction(rule.command,
                                                        rule.responseType, func);
                    interfaceSenderFunctions.push_back(interfaceFunction);

                    funParams = getFunParams(ParamsType::Receive, RulePart::Right);

                    funcReceiveResponse.SetDeclaration(funNameRwl + "_r" + "_empty",
                                                       "void", funParams);
                    InterfaceFunction interfaceFunctionR(rule.command,
                                                         rule.responseType,
                                                         funcReceiveResponse);
                    interfaceReceiveFunctions.push_back(interfaceFunctionR);
                }
                else
                {
                    auto funParams = getFunParams(ParamsType::Send, RulePart::Right);

                    func.SetDeclaration(funNameR + "_s","void",funParams);
                    func.SetBody(GetInterfaceSendFuncBody(codeFieldName, rule.command,
                                                          rule.responseType, *it_1,
                                                          specialParameterRemote));

                    InterfaceFunction interfaceFunction(rule.command,
                                                        rule.responseType,
                                                        func, *it_1);
                    interfaceSenderFunctions.push_back(interfaceFunction);

                    funParams = getFunParams(ParamsType::Receive, RulePart::Right);

                    funcReceiveResponse.SetDeclaration(funNameRwl + "_r", "void",
                                                       funParams);
                    InterfaceFunction interfaceFunctionR(rule.command,
                                                         rule.responseType,
                                                         funcReceiveResponse, *it_1);
                    interfaceReceiveFunctions.push_back(interfaceFunctionR);
                }
            }

            if(!rule.isEmptySend) {
                InterfaceFunction interfaceFunctionR(rule.command, rule.sendType,
                                                     funcReceiveSend, *it);
                interfaceReceiveFunctions.push_back(interfaceFunctionR);
            }
            else {
                InterfaceFunction interfaceFunctionR(rule.command, rule.sendType,
                                                     funcReceiveSend);
                interfaceReceiveFunctions.push_back(interfaceFunctionR);
            }
        }
}

vector<string> CodeGenerator::GetInterfaceSendFuncBody(string headerVar, string command,
                                                       string sendType,
                                                       vector<Parameter> headerParameteres)
{
    vector<string> strings;
    strings.push_back(_headerDeclarations.BlType() + b_und + _headerDeclarations.name +
                      spc + "header;");

    for(auto var: _headerDeclarations.fields)
    {
        if(var.type.first == _exchangeDescription.typeList.at(0).first)
        {
            if(_qtOption || _qtCppOption) {
                 strings.push_back("header." +  var.fieldName + " = " +
                                   sendType + smcln);
            }
            else {
                strings.push_back("header." +  var.fieldName + " = " +
                                  _fileNameLowerCase + b_und + sendType + smcln);
            }
        }
    }

    for(auto param: headerParameteres )
    {
        for(auto var: _headerDeclarations.fields)
        {
            if(var.fieldName == param.name) {
                strings.push_back("header." +  var.fieldName + " = " + param.name +
                                  smcln);
            }
        }
    }

    strings.push_back("header.dataLen = 0" + smcln);

    for(auto param: _localVar)
    {
        for(auto var: _headerDeclarations.fields)
        {
            if(var.fieldName == param.first)
            {
                string strParam;
                if(_qtOption || _qtCppOption)
                    strParam = PrintClassText(param.second);
                else
                    strParam = "obj->" + param.second;

                strings.push_back("header." +  var.fieldName + " = " + strParam +
                                  smcln);
            }
        }
    }

    if(_qtOption || _qtCppOption)
    {
        strings.push_back("header." +  headerVar + " = "+ command + smcln);
        strings.push_back("std::unique_ptr<char[]> alloc" + lsb + "new char" +
                          PutInSqBraces(_headerDeclarations.Size().Get() +
                                           " + header.dataLen") + rsb + smcln);
        strings.push_back("char *buffer = alloc.get()" + smcln);
    }
    else
    {
        strings.push_back("header." +  headerVar + " = "+ _fileNameLowerCase + b_und +
                          command + smcln);
        strings.push_back("char buffer" + lsqb + _headerDeclarations.Size().Get() +
                          " + header.dataLen" + rsqb + smcln);
    }

    strings.push_back(PrintClassText(_headerDeclarations.PrintSerCall("buffer",
                                                                     "&header")));

    vector<string> params;

    if(_addrOption)
        params.push_back(GetAddrParameter().name);

    params.push_back("buffer");
    params.push_back(_headerDeclarations.Size().Get());

    if((_qtOption || _qtCppOption) == false)
    {
        auto str = IfElseStatementCpp().
                AddCase(PrintStructText(_cbSendFun.GetFunctionName()) + neql + "0",
                        PrintStructText(_cbSendFun.GetCall(params, false))).GetDefinition();
        strings.insert(strings.end(),str.begin(),str.end());
    } else
        strings.push_back(PrintStructText(_cbSendFun.GetCall(params, false)));

    return strings;
}

vector<string> CodeGenerator::GetInterfaceSendFuncBody(string headerCommandVar,
                                                       string command, string sendType,
                                                       ComplexTypeDescription packet,
                                                       vector<Parameter> headerRemoteParam)
{
    vector<string> strings;

    if(_qtCppOption)
    {
        for(auto field : packet.fields)
        {
            if(field.data.withLenDefiningVar) {
                strings.push_back("str->" + field.data.lenDefiningVar + " = str->" +
                                  field.fieldName + ".size()" + smcln);
            }
        }
    }

    strings.push_back(_headerDeclarations.BlType() + b_und + _headerDeclarations.name +
                      spc + "header;");

    for(auto var: _headerDeclarations.fields)
    {
        if(var.type.first == _exchangeDescription.typeList.at(0).first)
        {
            if(_qtOption || _qtCppOption) {
                 strings.push_back("header." +  var.fieldName + " = "+ sendType +
                                   smcln);
            } else {
                strings.push_back("header." +  var.fieldName + " = " +
                                  _fileNameLowerCase + b_und + sendType + smcln);
            }
        }
    }

    for(auto param: headerRemoteParam )
    {
        for(auto var: _headerDeclarations.fields)
            if(var.fieldName == param.name) {
                strings.push_back("header." +  var.fieldName + " = "+ param.name +
                                  smcln);
            }
    }
    strings.push_back("header.dataLen = " + packet.Size().Get() + smcln);

    for(auto param: _localVar)
    {
        for(auto var: _headerDeclarations.fields)
        {
            if(var.fieldName == param.first)
            {
                string strParam;
                if(_qtOption || _qtCppOption)
                    strParam = PrintClassText(param.second);
                else
                    strParam = "obj->" + param.second;
                strings.push_back("header." +  var.fieldName + " = "+ strParam + smcln);
            }
        }
    }

    if(_qtOption || _qtCppOption)
    {
         strings.push_back("header." +  headerCommandVar + " = "+ command + smcln);
         strings.push_back("std::unique_ptr<char[]> alloc" + lsb + "new char" +
                           PutInSqBraces(_headerDeclarations.Size().Get() +
                                            " + header.dataLen") + rsb + smcln);
         strings.push_back("char *buffer = alloc.get()" + smcln);
    }
    else
    {
        strings.push_back("header." +  headerCommandVar + " = " + _fileNameLowerCase +
                          b_und + command + smcln);
        strings.push_back("char buffer" + lsqb + _headerDeclarations.Size().Get() +
                          " + header.dataLen" + rsqb + smcln);
    }

    strings.push_back(PrintClassText(_headerDeclarations.PrintSerCall("buffer",
                                                                     "&header")));
    strings.push_back(PrintClassText(packet.PrintSerCall("buffer + " +
                                                         _headerDeclarations.Size().Get(),
                                                         "str")));

    vector<string> params;

    if(_addrOption)
        params.push_back(GetAddrParameter().name);

    params.push_back("buffer");
    string buf;
    buf += (_headerDeclarations.Size() + packet.Size()).Get();

    params.push_back(buf);

    if((_qtOption || _qtCppOption) == false)
    {
        auto str = IfElseStatementCpp().
                AddCase(PrintStructText(_cbSendFun.GetFunctionName()) + neql + "0",
                        PrintStructText(_cbSendFun.GetCall(params, false))).GetDefinition();
        strings.insert(strings.end(),str.begin(),str.end());
    } else
        strings.push_back(PrintStructText(_cbSendFun.GetCall(params, false)));

    return strings;
}

vector<string> CodeGenerator::SetBodySetCbs()
{
    vector<string> content;
    for(auto var :interfaceReceiveFunctions) {
        content.push_back("obj->_CBsStruct." + var.func.GetFunctionName() + " = " +
                          "str->" + var.func.GetFunctionName() + smcln);
    }
    content.push_back("obj->_CBsStruct." + _cbSendFun.GetFunctionName() + " = " +
                      "str->" + _cbSendFun.GetFunctionName() + smcln);
    return  content;
}

vector<string> CodeGenerator::SetBodyResetCbs()
{
    vector<string> content;
    for(auto var :interfaceReceiveFunctions) {
        content.push_back("obj->_CBsStruct." + var.func.GetFunctionName() + " = " +
                          "0" + smcln);
    }
    content.push_back("obj->_CBsStruct." + _cbSendFun.GetFunctionName() + " = " +  "0" +
                      smcln);
    return  content;
}

vector<string> CodeGenerator::SetBodyLocaParamsStruct(vector<Parameter> params)
{
    vector<string> strings;
    for (auto param : params)
        strings.push_back(param.type + spc + param.name + smcln);

    return strings;
}

vector<string> CodeGenerator::SetBodyGetLocaParamsFun(vector<Parameter> params)
{
    vector<string> strings;
    string var = "buf";
    strings.push_back(PrintVarDeclaration(_localParams.GetName(),var));

    for (auto param : params) {
        strings.push_back(PrintVarDefinition(var + "." +
                                             param.name,PrintClassText(param.name)));
    }
    strings.push_back("return " + var + smcln);

    return strings;
}

string CodeGenerator::PrintClassText(string var)
{
    if(_qtOption || _qtCppOption)
        return "base->" + var;
    else
        return var;
}

string CodeGenerator::PrintStructText(string var)
{
    if(!(_qtOption || _qtCppOption))
        return "obj->_CBsStruct." + var;
    else
        return var;
}
