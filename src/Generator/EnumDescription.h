#ifndef ENUMDESCRIPTION_H
#define ENUMDESCRIPTION_H

#include <string>
#include <algorithm>
#include <vector>
#include "Analyzer/Formater.h"
#include "SizeExpr.h"

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

    void SetName(string _name);
    string GetName() const;

    void SetPrefix(string prefix);
    string GetPrefix();

    void SetBitWidth(size_t size);
    SizeExprPtr Size() const;

    vector<string> Declaration(bool withEnumText = true);

    vector<EnumFieldInfo> fields;

private:
    string _name;
    string _prefix;
    size_t _bitWidth = 0;
    string PrintType();
};

#endif // ENUMDESCRIPTION_H
