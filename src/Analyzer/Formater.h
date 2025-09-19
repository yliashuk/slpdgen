#ifndef FORMATER_H
#define FORMATER_H

#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include "StructFieldData.h"
#include "Generics.h"
#include "Rule.h"

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
    ComplexStatus addEnumDeclaration(EnumType eTp, string enumName,
                                     std::optional<size_t> bitWidth = {});
    ComplexStatus addEnumField(EnumType eTp,string enumName, string fieldName);
    ComplexStatus addEnumField(EnumType eTp,string enumName, string fieldName, int code);
    ComplexStatus addStructDeclaration(StructType strTp, string structName);
    ComplexStatus addStructField(StructType strTp, string currBlockName, FieldInfo fieldInfo);
    ComplexStatus addRule(Rule rule, bool hasReverse = false);

    string toJson() const;

    vector<Enum> enumList, codeList, typeList;
    vector<Struct> structList, packetList ,headerList;
    vector<Rule> rules;

private:
    ComplexStatus checkRule(Rule rule);

    Status addStructToList(vector<Struct>& list, string structName);

    Status addFieldToStruct(vector<Struct>& list, string structName,
                            FieldInfo dataStruct);
};

#endif // FORMATER_H
