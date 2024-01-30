#ifndef RULE_H
#define RULE_H

#include <string>
#include <optional>

struct Rule
{
    std::optional<std::string> command;
    std::optional<std::string> sendType;
    std::optional<std::string> sendPacket;
    std::optional<std::string> responseType;
    std::optional<std::string> responsePacket;

    bool isEmptySend() const;
    bool hasResponse() const;
    bool isEmptyResponse() const;
    bool hasResponseData() const;
    bool isValid() const;

    static Rule Reverse(const Rule& rule);
};

class RuleBuilder
{
public:
    RuleBuilder& command(std::string command);
    RuleBuilder& sendType(std::string sendType);
    RuleBuilder& sendPacket(std::string sendPacket);
    RuleBuilder& responseType(std::string responseType);
    RuleBuilder& responsePacket(std::string responsePacket);

    Rule build();

private:
    Rule _rule;
};


#endif // RULE_H
