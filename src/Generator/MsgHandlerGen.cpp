#include "MsgHandlerGen.h"
#include "CppConstructs/SwitchCpp.h"

MsgHandlerGen::MsgHandlerGen(AppOptions options,
                             EnumDescription errorEnum,
                             ComplexTypeDescription header)
    : _fName(options.fileName),
      _options(options),
      _errorEnum(errorEnum),
      _header(header)
{}

Function MsgHandlerGen::ParseFun(const vector<RulesDefinedMessage>& rdms)
{
    Function parseFun;
    vector<Parameter> parameters;

    string returnType = _errorEnum.GetPrefix() + _errorEnum.GetName();

    if(!(_options.isCpp))
        parameters.push_back({toLower(_fName) + "_Obj", "*obj"});

    if(_options.hasAddr)
        parameters.push_back(AddrParameter());

    parameters.push_back({"void*", "p"});
    parameters.push_back({"uint16_t", "dataLen"});

    if(_options.isCpp) {
        parseFun.SetContainedClass(_fName);
        parseFun.SetDeclaration("parse", _errorEnum.GetName(), parameters);
    } else {
        parseFun.SetDeclaration(toLower(_fName) + "_parse", toLower(_fName) +
                                 "_" + _errorEnum.GetName(), parameters);
    }

    vector<string> body;
    auto stat = IfElseStatementCpp().AddCase("dataLen == 0", fmt("return(%s)1;", {returnType}));

    body << stat.GetDefinition()
         << "char* l_p = (char*)p;"
         << "uint8_t *op_status;"
         << "uint8_t status_Buf = 1;"
         << "op_status = &status_Buf;"
         << _header.AsVarDecl() + ";";

    body << fmt("%{%s->}%s;", {_options.isCpp ? "base" : "", _header.
                                  SerDesCall(FunType::Des,"&header")});

    IfElseStatementCpp handler;
    handler.AddCase("*op_status == 0", fmt("return(%s)1;", {returnType}));

    body << handler.GetDefinition();
    body << "l_p += " + _header.Size().Get() + ";\n";

    SwitchCpp switchOperator;
    switchOperator.SetSwitchingParameter("header." + CodeVarName());

    vector<string> buffer;
    IfElseStatementCpp ifLocalStatement, if_else_dataLenStatement;

    string lastRdmCommand = rdms.begin()->command;

    string callCalcFun;
    // call function defined in this c/cpp file
    string pCallPrefix = _options.isCpp ? "base->" : "";

    for(auto rdm = rdms.begin(); rdm < rdms.end(); rdm++)
    {
        // add switch case
        if(lastRdmCommand != rdm->command)
        {
            string prefix = _options.isCpp ? "" : toLower(_fName);
            string switchVal = fmt("%{%s_}%s", {prefix, lastRdmCommand});
            switchOperator.AddCase(switchVal, ifLocalStatement.GetDefinition());

            ifLocalStatement.Clear();
            lastRdmCommand = rdm->command;
        }

        string cbPrefix = !_options.isCpp ? "obj->_CBsStruct." : "";
        string receiveMsgCbPtr = cbPrefix + ReceiveMsgCb(*rdm).FunctionName();

        vector<string> cbArgNames = ReceiveMsgCbArgNames(*rdm);
        string receiveMsgCbCall = cbPrefix + ReceiveMsgCb(*rdm).GetCall(cbArgNames,true);

        auto checkReceiveMsgCbPtr = IfElseStatementCpp()
                .AddCase(fmt("%s == 0", {receiveMsgCbPtr}),
                         fmt("return(%s)1;", {returnType}))
                .GetDefinition();

        if(rdm->packet)
        {
            buffer.clear();
            if(!_options.isCpp) { buffer += checkReceiveMsgCbPtr; }
            buffer << rdm->packet->GetCodeName() + " str = {};";

            // for print call calc size fun after check header.type
            callCalcFun = pCallPrefix + rdm->packet->SizeCalcFunCall(Des);

            if(!_options.isCpp)
            {
                buffer << "char buffer[size.d + 1];"
                       << "memset(&buffer[0], 0, size.d + 1);"
                       << "buf_p = &buffer[0];";
            }

            string prefix = _options.isCpp ? "base->" : "";
            buffer << prefix + rdm->packet->SerDesCall(FunType::Des,"&str") + ";";

            IfElseStatementCpp handler;
            handler.AddCase("*op_status == 0", "return(" + returnType + ")1;");
            buffer << handler.GetDefinition();

            buffer << receiveMsgCbCall;
            if_else_dataLenStatement.AddCase("header.dataLen >= size.r", buffer);
        }
        else
        {
            buffer.clear();
            if(!_options.isCpp) { buffer += checkReceiveMsgCbPtr; }

            buffer << receiveMsgCbCall;
            if_else_dataLenStatement.AddCase("header.dataLen == 0", buffer);
        }
        {
            vector<string> caseBody;
            vector<string> statementDef = if_else_dataLenStatement.GetDefinition();

            // Print call calc size fun
            if(rdm->packet)
            {
                caseBody << fmt("c_size_t size = %s;",{callCalcFun});
            }

            caseBody << statementDef;

            string prefix = !(_options.isCpp) ? toLower(_fName) : "";
            auto statement = fmt("header.%s == %{%s_}%s", {TypeVarName(), prefix, rdm->type});
            ifLocalStatement.AddCase(statement, caseBody);

            if_else_dataLenStatement.Clear();
        }
        // added last rule handler
        if(rdm == rdms.end() - 1) {
            string prefix = _options.isCpp ? "" : toLower(_fName);
            string switchVal = fmt("%{%s_}%s", {prefix, lastRdmCommand});
            switchOperator.AddCase(switchVal, ifLocalStatement.GetDefinition());
        }
    }
    body << switchOperator.Declaration();
    body << "return(" + returnType + ")0;";
    parseFun.SetBody(body);

    return parseFun;
}

Parameter MsgHandlerGen::AddrParameter()
{
    Parameter parameter;
    parameter.name = "addr";
    parameter.type = _options.isCpp ? "Addr*" : toLower(_fName) + "_Addr*";
    return parameter;
}

string MsgHandlerGen::CodeVarName()
{
    for(auto field : _header.GetFields())
    {
        if(field.type.second == fieldType::Code) {
            return field.fieldName;
        }
    }
    return {};
}

string MsgHandlerGen::TypeVarName()
{
    for(auto field : _header.GetFields())
    {
        if(field.type.second == fieldType::Type) {
            return field.fieldName;
        }
    }
    return {};
}

Function MsgHandlerGen::ReceiveMsgCb(RulesDefinedMessage msg)
{
    Function receiveFun;
    vector<Parameter> funParams;

    if(!_options.isCpp) { funParams += {toLower(_fName) + "_Obj", "*obj"};}
    if(_options.hasAddr) { funParams += AddrParameter(); }
    if(msg.packet){funParams += {msg.packet->GetCodeName(), "*str"};}
    funParams += GetHeaderParams({"local", "remote"});


    receiveFun.SetDeclaration(ReceiveFunName(msg), "void", funParams);

    return receiveFun;
}

string MsgHandlerGen::ReceiveFunName(RulesDefinedMessage msg)
{
    auto name = fmt("%s_%s" , { msg.type, msg.command});
    toLower(name);
    name += msg.packet ? "_r" : "_r_empty";

    return name;
}

Strings MsgHandlerGen::ReceiveMsgCbArgNames(const RulesDefinedMessage &rdm)
{
    vector<string> args;

    if(!_options.isCpp){ args.push_back("obj"); }
    if(_options.hasAddr){ args.push_back(AddrParameter().name); }
    if(rdm.packet){ args.push_back("&str"); }

    auto headerParams = GetHeaderParams({"local", "remote"});
    for(auto param : headerParams) {
        args += fmt("header.%s", {param.name});
    }

    return args;
}

std::vector<Parameter> MsgHandlerGen::GetHeaderParams(std::vector<string> initTypes)
{
    std::vector<Parameter> params;

    for(auto field : _header.GetFields())
    {
        if(contains(initTypes, field.data.initType))
        {
            params += {field.type.first, field.fieldName};
        }
    }
    return params;
}

string MsgHandlerGen::SendFunName(RulesDefinedMessage msg)
{
    std::string name;
    if(_options.isCpp) {
        name = fmt("%s_%s", {msg.type, msg.command});
    }
    else {
        name = fmt("%s_%s_%s" , {toLower(_fName), msg.type, msg.command});
    }
    toLower(name);
    name += msg.packet ? "_s" : "_s_empty";

    return name;
}

Function MsgHandlerGen::SendMsgFun(const RulesDefinedMessage& msg)
{
    Strings body;

    if(_options.isCpp && msg.packet)
    {
        for(auto field : msg.packet->GetFields())
        {
            if(field.data.hasDynamicSize)
            {
                body << fmt("str->%s = str->%s.size();",
                {field.data.lenDefiningVar, field.fieldName});
            }
        }
    }

    body << fmt("%s_%s header;", {_header.BlType(), _header.GetName()});

    body << fmt("header.%s = %{%s_}%s;", {TypeVarName(),
                                             _options.isCpp ? "" : toLower(_fName),
                                             msg.type});


    for(auto param: GetHeaderParams({"remote"}))
    {
        for(auto var: _header.GetFields())
        {
            if(var.fieldName == param.name) {
                body += fmt("header.%s = %s;", {var.fieldName, param.name});
            }
        }
    }

    string prefix = _options.isCpp ? "base->" : "";
    string size = msg.packet ? prefix + msg.packet->SizeCalcFunCall(Ser) : "0";

    body += fmt("header.dataLen = %s;", {size});

    for(auto param: GetHeaderParams({"local"}))
    {
        for(auto field: _header.GetFields())
        {
            if(field.fieldName == param.name)
            {
                string strParam = fmt("%s->%s", {_options.isCpp ? "base" : "obj", param.name});
                body += fmt("header.%s = %s;", {field.fieldName, strParam});
            }
        }
    }

    if(_options.isCpp)
    {
        body << fmt("header.%s = %s;", {CodeVarName(), msg.command});

        body << fmt("std::unique_ptr<char[]> alloc(new char[%s + header.dataLen]);",
        {_header.Size().Get()});

        body << "char *buffer = alloc.get();";
    }
    else
    {
        body << fmt("header.%s = %s_%s;", {CodeVarName(), toLower(_fName), msg.command});
        body << fmt("char buffer[%s + header.dataLen];", {_header.Size().Get()});
    }

    body << (_options.isCpp ? "base->" : "") + _header.SerCall("buffer", "&header");
    if(msg.packet) {
        body << (_options.isCpp ? "base->" : "") +
                msg.packet->SerCall("buffer + " + _header.Size().Get(), "str");
    }

    vector<string> params;

    if(_options.hasAddr){ params += AddrParameter().name; }

    params += "buffer";
    params += _header.Size().Get() + (msg.packet ? " + header.dataLen" : "");

    if(!_options.isCpp)
    {
        body << IfElseStatementCpp().AddCase(
                    fmt("obj->_CBsStruct.%s != 0",{SendCb().FunctionName()}),
                    fmt("obj->_CBsStruct.%s",{SendCb().GetCall(params, false)})
                    ).GetDefinition();
    } else {
        body << SendCb().GetCall(params, false);
    }

    Function sendFun;
    vector<Parameter> sendParams;

    if(!_options.isCpp){ sendParams += {toLower(_fName) + "_Obj", "*obj"}; }
    if(_options.hasAddr){ sendParams += AddrParameter(); }
    if(msg.packet){ sendParams += {msg.packet->GetCodeName(), "*str"}; }
    sendParams += GetHeaderParams({"remote"});

    sendFun.SetDeclaration(SendFunName(msg), "void", sendParams);
    sendFun.SetBody(body);
    if(_options.isCpp) { sendFun.SetContainedClass(_fName); }

    return sendFun;
}

Function MsgHandlerGen::SendCb()
{
    Function cbSend;
    vector<Parameter> params;

    if(_options.hasAddr)
        params.push_back(AddrParameter());

    params.push_back({"void*", "buffer"});
    params.push_back({"uint16_t", "size"});

    if(_options.isCpp) {
        cbSend.SetDeclaration("send_cb", "void", params);
    }
    else {
        cbSend.SetDeclaration(toLower(_fName) + "_send_cb", "void", params);
    }
    return cbSend;
}
