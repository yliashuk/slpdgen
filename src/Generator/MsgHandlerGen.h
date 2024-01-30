#ifndef MSGHANDLERGEN_H
#define MSGHANDLERGEN_H

#include "AppOptions.h"
#include "EnumDescription.h"
#include "ComplexTypeDescription.h"
#include "RulesDefinedMessage.h"


class MsgHandlerGen
{
public:
    MsgHandlerGen(AppOptions options,
                  EnumDescription errorEnum,
                  ComplexTypeDescription header);

    Function SendMsgFun(const RulesDefinedMessage& msg);
    Function ParseFun(const vector<RulesDefinedMessage> &rdms);

    Function SendCb();
    Function ReceiveMsgCb(RulesDefinedMessage msg);
    string ReceiveFunName(RulesDefinedMessage msg);
    std::vector<Parameter> GetHeaderParams(std::vector<string> initTypes);

private:
    string _fName;
    AppOptions _options;

    EnumDescription _errorEnum;
    ComplexTypeDescription _header;

    Parameter AddrParameter();

    string CodeVarName();
    string TypeVarName();
    Strings ReceiveMsgCbArgNames(const RulesDefinedMessage& rdm);
    string SendFunName(RulesDefinedMessage msg);
};

#endif // PARSEFUN_H
