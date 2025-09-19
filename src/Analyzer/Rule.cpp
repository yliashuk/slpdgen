#include "Rule.h"

bool Rule::isEmptySend() const
{
    return !sendPacket.has_value();
}

bool Rule::hasResponse() const
{
    return responseType.has_value();
}

bool Rule::isEmptyResponse() const
{
    return hasResponse() && !responsePacket.has_value();
}

bool Rule::hasResponseData() const
{
    return hasResponse() && responsePacket.has_value();
}

bool Rule::isValid() const
{
    return command.has_value() && sendType.has_value();
}

Rule Rule::reverse(const Rule &rule)
{
    return  {
        rule.command,
        rule.sendType, rule.responsePacket,
        rule.responseType, rule.sendPacket
    };
}

RuleBuilder &RuleBuilder::command(std::string command)
{
    _rule.command = command;
    return *this;
}

RuleBuilder &RuleBuilder::sendType(std::string sendType)
{
    _rule.sendType = sendType;
    return *this;
}

RuleBuilder &RuleBuilder::sendPacket(std::string sendPacket)
{
    _rule.sendPacket = sendPacket;
    return *this;
}

RuleBuilder &RuleBuilder::responseType(std::string responseType)
{
    _rule.responseType = responseType;
    return *this;
}

RuleBuilder &RuleBuilder::responsePacket(std::string responsePacket)
{
    _rule.responsePacket = responsePacket;
    return *this;
}

Rule RuleBuilder::build()
{
    return _rule;
}
