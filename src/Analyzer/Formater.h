#ifndef FORMATER_H
#define FORMATER_H

#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include "StructFieldData.h"
#include "Generics.h"
#include "Rule.h"

using namespace std;

struct ComplexStatus
{
    Status status;
    string wrongName;
};

typedef pair<string, uint32_t> EnumFieldInfo;
typedef pair<string, FieldInfo> StructFieldInfo;

struct Enum
{
    string name;
    vector<EnumFieldInfo> fields;
    std::optional<size_t> bitWidth;
};

struct Struct
{
    string name;
    vector<StructFieldInfo> fields;
};

class Formater
{
public:
    Formater();
    ComplexStatus AddEnumDeclaration(EnumType eTp, string enumName,
                                     std::optional<size_t> bitWidth = {});
    ComplexStatus AddEnumField(EnumType eTp,string enumName, string fieldName);
    ComplexStatus AddEnumField(EnumType eTp,string enumName, string fieldName, int code);
    ComplexStatus AddStructDeclaration(StructType strTp, string structName);
    ComplexStatus AddStructField(StructType strTp, string currBlockName, FieldInfo fieldInfo);
    ComplexStatus AddRule(Rule rule, bool hasReverse = false);

    vector<Enum> enumList, codeList, typeList;
    vector<Struct> structList, packetList ,headerList;
    vector<Rule> rules;

private:
    ComplexStatus RuleCheck(Rule rule);

    Status AddStructToList(vector<Struct>& list, string structName);

    Status AddFieldToStruct(vector<Struct>& list, string structName,
                            FieldInfo dataStruct);
};

#endif // FORMATER_H
