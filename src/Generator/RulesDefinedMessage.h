#ifndef RULESDEFINEDMESSAGE_H
#define RULESDEFINEDMESSAGE_H

#include "ComplexTypeDescription.h"

struct RulesDefinedMessage
{
    std::string command;
    std::string type;

    std::optional<ComplexTypeDescription> packet;
};

#endif // RULESDEFINEDMESSAGE_H
