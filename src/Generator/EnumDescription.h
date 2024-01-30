#ifndef ENUMDESCRIPTION_H
#define ENUMDESCRIPTION_H

#include <string>
#include <algorithm>
#include <vector>
#include "Analyzer/Formater.h"

using namespace std;

enum class EnumerableType
{
    Code,
    Type,
    Enum
};

class EnumDescription
{
public:
    EnumerableType type;

    EnumDescription();

    void SetName(string name);
    string GetName() const;

    void SetPrefix(string prefix);
    string GetPrefix();

    size_t Size();

    vector<string> Declaration(bool withEnumText = true);

    vector<FieldDataEnum> fields;

private:
    string name;
    string _prefix;
    string PrintType();
};

#endif // ENUMDESCRIPTION_H
