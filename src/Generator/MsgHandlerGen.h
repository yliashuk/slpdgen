#ifndef MSGHANDLERGEN_H
#define MSGHANDLERGEN_H

#include <set>
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

    Function sendMsgFun(const RulesDefinedMessage& msg);
    Function parseFun(const std::set<RulesDefinedMessage> &rdms);

    Function sendCb();
    Function checkHeaderCb();
    Function receiveMsgCb(RulesDefinedMessage msg);
    string receiveFunName(RulesDefinedMessage msg);
    std::vector<Parameter> getHeaderParams(std::vector<string> initTypes);

private:
    const string _fName;
    const AppOptions _options;

    // base class prefix for cpp internal implementation
    const string _bpfx;
    // file prefix for cpp internal implementation
    const string _fpfx;

    EnumDescription _errorEnum;
    ComplexTypeDescription _header;

    Strings headerValidationCode();
    Strings returnErrorOnNull(string variable);

    Parameter addrParameter();

    string codeVarName();
    string typeVarName();
    Strings receiveMsgCbArgNames(const RulesDefinedMessage& rdm);
    string sendFunName(RulesDefinedMessage msg);


};

#endif // PARSEFUN_H
