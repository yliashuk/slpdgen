#ifndef RULESDEFINEDMESSAGE_H
#define RULESDEFINEDMESSAGE_H

#include "ComplexTypeDescription.h"

struct RulesDefinedMessage
{
    std::string command;
    std::string type;

    std::optional<ComplexTypeDescription> packet;

    bool operator<(const RulesDefinedMessage& other) const
    {
        if(this->command != other.command) {
            return this->command < other.command;
        }
        if(this->type != other.type) {
            return this->type < other.type;
        }
        if(this->packet.has_value() != other.packet.has_value()) {
            return this->packet.has_value() < other.packet.has_value();
        }
        return false;
    }
};

#endif // RULESDEFINEDMESSAGE_H
