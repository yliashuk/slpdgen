#include "MsgHandlerGen.h"
#include "CppConstructs/SwitchCpp.h"

MsgHandlerGen::MsgHandlerGen(AppOptions options,
                             EnumDescription errorEnum,
                             ComplexTypeDescription header)
    : _fName(options.fileName),
      _options(options),
      _bpfx(_options.isCpp ? "base->" : ""),
      _fpfx(_options.isCpp ? "" : toLower(_fName) + '_'),
      _errorEnum(errorEnum),
      _header(header)
{}

Function MsgHandlerGen::parseFun(const vector<RulesDefinedMessage>& rdms)
{
    Function parseFun;
    vector<Parameter> params;

    if(_options.isC)
        params.push_back({_fpfx + "Obj", "*obj"});

    if(_options.hasAddr)
        params.push_back(addrParameter());

    params.push_back({"void*", "p"});
    params.push_back({"uint16_t", "dataLen"});

    if(_options.isCpp) { parseFun.setContainedClass(_fName); }
    parseFun.setDeclaration(_fpfx + "parse", _fpfx + _errorEnum.getName(), params);

    vector<string> body;

    body << returnErrorOnNull("dataLen")
         << "char* l_p = (char*)p;"
         << "uint8_t op_status;"
         << fmt("%s = {%s};", {_header.asVarDecl(), _options.isCpp ? "" : "0"});

    body << fmt("size_t offset = %{%s->}%s;",
    {_options.isCpp ? "base" : "", _header.desCall("l_p", "0", "&header", "&op_status")});
    body << "(void)offset;",

    body << returnErrorOnNull("op_status");
    body << headerValidationCode();

    SwitchCpp switchOperator;
    switchOperator.setSwitchingParameter("header." + codeVarName());

    vector<string> buffer;
    ConditionCpp ifLocalStatement, if_else_dataLenStatement;

    string lastRdmCommand = rdms.begin()->command;

    string callCalcFun;

    for(auto rdm = rdms.begin(); rdm < rdms.end(); rdm++)
    {
        // add switch case
        if(lastRdmCommand != rdm->command)
        {
            string switchVal = fmt("%s%s", {_fpfx, lastRdmCommand});
            switchOperator.addCase(switchVal, ifLocalStatement.definition());

            ifLocalStatement.clear();
            lastRdmCommand = rdm->command;
        }

        string cbPrefix = _options.isC ? "obj->_CBsStruct." : "";
        string receiveMsgCbPtr = cbPrefix + receiveMsgCb(*rdm).name();

        vector<string> cbArgNames = receiveMsgCbArgNames(*rdm);
        string receiveMsgCbCall = cbPrefix + receiveMsgCb(*rdm).getCall(cbArgNames);

        if(rdm->packet)
        {
            buffer.clear();

            buffer << fmt("%s str = {%s};", {rdm->packet->getCodeName(),
                                             _options.isCpp ? "" : "0"});
            if(_options.isC)
            {
                buffer << "char buffer[size.d + 1];"
                       << "memset(&buffer[0], 0, size.d + 1);"
                       << "buf_p = &buffer[0];";
            }

            buffer << _bpfx + rdm->packet->desCall("l_p", "offset", "&str", "&op_status") + ";";
            buffer << returnErrorOnNull("op_status");

            buffer << receiveMsgCbCall + ';';
            if_else_dataLenStatement.addCase("header.dataLen * 8 >= size.r", buffer);
        }
        else
        {
            buffer.clear();

            buffer << receiveMsgCbCall + ';';
            if_else_dataLenStatement.addCase("header.dataLen == 0", buffer);
        }
        {
            vector<string> caseBody;
            vector<string> statementDef = if_else_dataLenStatement.definition();

            if(!_options.isQt)
            { caseBody << returnErrorOnNull(receiveMsgCbPtr); }

            // Print call calc size fun after check header.type
            if(rdm->packet)
            {
                callCalcFun = _bpfx + rdm->packet->sizeCalcFunCall(Des);
                caseBody << fmt("c_size_t size = %s;",{callCalcFun});
            }

            caseBody << statementDef;

            auto statement = fmt("header.%s == %s%s", {typeVarName(), _fpfx, rdm->type});
            ifLocalStatement.addCase(statement, caseBody);

            if_else_dataLenStatement.clear();
        }
        // added last rule handler
        if(rdm == rdms.end() - 1) {
            string switchVal = fmt("%s%s", {_fpfx, lastRdmCommand});
            switchOperator.addCase(switchVal, ifLocalStatement.definition());
        }
    }
    body << switchOperator.declaration();
    body << "return(" + _errorEnum.getPrefix() + _errorEnum.getName() + ")0;";
    parseFun.setBody(body);

    return parseFun;
}

Parameter MsgHandlerGen::addrParameter()
{
    Parameter parameter;
    parameter.name = "addr";
    parameter.type = _fpfx + "Addr*";
    return parameter;
}

string MsgHandlerGen::codeVarName()
{
    for(auto field : _header.fields())
    {
        if(field.type.second == FieldType::Code) {
            return field.name;
        }
    }
    return {};
}

string MsgHandlerGen::typeVarName()
{
    for(auto field : _header.fields())
    {
        if(field.type.second == FieldType::Type) {
            return field.name;
        }
    }
    return {};
}

Function MsgHandlerGen::receiveMsgCb(RulesDefinedMessage msg)
{
    Function receiveFun;
    vector<Parameter> funParams;

    if(_options.isC) { funParams += {_fpfx + "Obj", "*obj"};}
    if(_options.hasAddr) { funParams += addrParameter(); }
    if(msg.packet){funParams += {msg.packet->getCodeName(), "*str"};}
    funParams += getHeaderParams({"local", "remote"});


    receiveFun.setDeclaration(receiveFunName(msg), "void", funParams);

    return receiveFun;
}

string MsgHandlerGen::receiveFunName(RulesDefinedMessage msg)
{
    auto name = fmt("%s_%s" , { msg.type, msg.command});
    name = toLower(name);
    name += msg.packet ? "_r" : "_r_empty";

    return name;
}

Strings MsgHandlerGen::receiveMsgCbArgNames(const RulesDefinedMessage &rdm)
{
    vector<string> args;

    if(_options.isC){ args.push_back("obj"); }
    if(_options.hasAddr){ args.push_back(addrParameter().name); }
    if(rdm.packet){ args.push_back("&str"); }

    auto headerParams = getHeaderParams({"local", "remote"});
    for(auto param : headerParams) {
        args += fmt("header.%s", {param.name});
    }

    return args;
}

std::vector<Parameter> MsgHandlerGen::getHeaderParams(std::vector<string> initTypes)
{
    std::vector<Parameter> params;

    for(auto field : _header.fields())
    {
        if(contains(initTypes, field.info.specialType.value_or("")))
        {
            params += {field.type.first, field.name};
        }
    }
    return params;
}

Strings MsgHandlerGen::headerValidationCode()
{
    vector<string> body;
    auto fun = checkHeaderCb();
    auto funName = fun.name();

    auto callParams = _options.isC ? Strings{"obj", "&header"} : Strings{"&header"};
    auto call = fun.getCall(callParams);

    body << "";
    if(_options.isQt)
    {
        auto signal = fmt("&%s::%s", {_fName, funName});
        body << fmt("static auto method = QMetaMethod::fromSignal(%s);", {signal});
        body << fmt("bool isValid = isSignalConnected(method) ? %s : true;", {call});
    } else if(_options.isCpp)
    {
        body << fmt("bool isValid = %s ? %s : true;",{funName, call});
    } else
    {
        auto p = "obj->_CBsStruct.";
        body << fmt("bool isValid = %s ? %s : true;",{p + funName, p + call});
    }
    body << returnErrorOnNull("isValid");
    body << "";

    return body;
}

Strings MsgHandlerGen::returnErrorOnNull(string variable)
{
    string returnType = _errorEnum.getPrefix() + _errorEnum.getName();
    return ConditionCpp().addCase(fmt("%s == 0", {variable}),
                fmt("return(%s)1;", {returnType})).definition();
}

string MsgHandlerGen::sendFunName(RulesDefinedMessage msg)
{
    string name = fmt("%s%s_%s" , {_fpfx, msg.type, msg.command});
    return toLower(name) + (msg.packet ? "_s" : "_s_empty");
}

Function MsgHandlerGen::sendMsgFun(const RulesDefinedMessage& msg)
{
    Strings body;

    body << fmt("%s_%s header;", {_header.blType(), _header.getName()});
    body << fmt("header.%s = %s%s;", {typeVarName(), _fpfx, msg.type});

    for(auto param: getHeaderParams({"remote"}))
    {
        for(auto var: _header.fields())
        {
            if(var.name == param.name) {
                body += fmt("header.%s = %s;", {var.name, param.name});
            }
        }
    }

    auto sizeCalcCall = [&] { return fmt("(%s%s + 7) / 8",
        {_bpfx, msg.packet->sizeCalcFunCall(Ser)});};

    string dataLen = msg.packet ? sizeCalcCall() : "0";
    body += fmt("header.dataLen = %s;", {dataLen});

    for(auto param: getHeaderParams({"local"}))
    {
        for(auto field: _header.fields())
        {
            if(field.name == param.name)
            {
                string strParam = fmt("%s->%s", {_options.isCpp ? "base" : "obj", param.name});
                body += fmt("header.%s = %s;", {field.name, strParam});
            }
        }
    }

    body << fmt("header.%s = %s%s;", {codeVarName(), _fpfx, msg.command});

    if(_options.isCpp)
    {
        body << fmt("std::unique_ptr<char[]> alloc(new char[(%s + 7) / 8%{ + %s}]);",
        {_header.size()->expand()->toString(), msg.packet ? "header.dataLen" : ""});

        body << "char *buffer = alloc.get();";
    }
    else
    {
        body << fmt("char buffer[(%s + 7) / 8%{ + %s}];",
        {_header.size()->expand()->toString(), msg.packet ? "header.dataLen" : ""});
    }

    body << _bpfx + _header.serCall("buffer", "0", "&header") + ";";
    if(msg.packet)
    {
        auto size = _header.size()->expand()->toString();
        body << _bpfx + msg.packet->serCall("buffer", size, "str") + ";";
    }

    vector<string> params;

    if(_options.hasAddr){ params += addrParameter().name; }

    params += "buffer";
    params += fmt("(%s + 7) / 8%{ + %s}",
    {_header.size()->expand()->toString(), msg.packet ? "header.dataLen" : ""});

    if(!_options.isCpp)
    {
        body << ConditionCpp().addCase(
                    fmt("obj->_CBsStruct.%s != 0",{sendCb().name()}),
                    fmt("obj->_CBsStruct.%s;",{sendCb().getCall(params)})
                    ).definition();
    } else if(!_options.isQt){
        body << ConditionCpp().addCase(sendCb().name(), sendCb()
                                       .getCall(params) + ';').definition();
    } else {
        body << sendCb().getCall(params) + ';';
    }

    Function sendFun;
    vector<Parameter> sendParams;

    if(_options.isC){ sendParams += {_fpfx + "Obj", "*obj"}; }
    if(_options.hasAddr){ sendParams += addrParameter(); }
    if(msg.packet){ sendParams += {msg.packet->getCodeName(), "*str"}; }
    sendParams += getHeaderParams({"remote"});

    sendFun.setDeclaration(sendFunName(msg), "void", sendParams);
    sendFun.setBody(body);
    if(_options.isCpp) { sendFun.setContainedClass(_fName); }

    return sendFun;
}

Function MsgHandlerGen::sendCb()
{
    Function cbSend;
    vector<Parameter> params;

    if(_options.hasAddr) { params.push_back(addrParameter()); }

    params.push_back({"void*", "buffer"});
    params.push_back({"uint16_t", "size"});

    auto name = _fpfx + "send_cb";
    cbSend.setDeclaration(name, "void", params);

    return cbSend;
}

Function MsgHandlerGen::checkHeaderCb()
{
    Function headerCheck;
    vector<Parameter> params;

    if(_options.isC) { params.push_back({_fpfx + "Obj", "*obj"}); }
    auto type = fmt("const %s_%s*", {_header.blType(), _header.getName()});
    params.push_back({type, "header"});

    auto name = _fpfx + "check_header";
    headerCheck.setDeclaration(name, "bool", params);

    return headerCheck;
}
