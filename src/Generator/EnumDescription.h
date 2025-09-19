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

    void setName(string _name);
    string getName() const;

    void setPrefix(string prefix);
    string getPrefix();

    void setBitWidth(size_t size);
    SizeExprPtr size() const;

    vector<string> declaration(bool withEnumText = true);

    vector<EnumFieldInfo> fields;

private:
    string _name;
    string _prefix;
    size_t _bitWidth = 0;
    string printType();
};

#endif // ENUMDESCRIPTION_H
