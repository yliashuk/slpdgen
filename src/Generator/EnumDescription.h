#ifndef ENUMDESCRIPTION_H
#define ENUMDESCRIPTION_H

#include <string>
#include <algorithm>
#include <vector>
#include "Analyzer/Formater.h"
#include "CppConstructs/CommonCpp.h"
using namespace std;
using namespace CppConstructs;

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
    size_t Size();
    vector<string> PrintDecl(bool withEnumText = true);
    void SetName(string name);
    string GetName() const;
    vector<FieldDataEnum> fields;
    void SetPrefix(string prefix);
    string GetPrefix();
private:
    string name;
    string _prefix;
    string PrintType();
};

#endif // ENUMDESCRIPTION_H
