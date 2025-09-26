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

struct FieldData
{
    FieldData(){}
    FieldData(FieldInfo data)
        :isArrayField(data.isNumOfCeils),
         hasDynamicSize(data.withLenDefiningVar),
         lenDefiningVar(data.lenDefiningVar),
         type(data.type),
         hasInitValue(data.defaultValue),
         valueRange(data.valueRange),
         value(data.defaultVal),
         min(data.fromVal),
         max(data.toVal),
         isWithInitType(data.isWithSpecialType),
         initType(data.specialType)
    {
        if(isArrayField)
            value = data.constLenDefiningVar;
    }

    bool   isArrayField = false;
    bool   hasDynamicSize = false;
    string lenDefiningVar;
    string type;
    bool hasInitValue   = false;
    bool valueRange     = false;
    uint64_t value;     // defaultValue = true valueRange = false
    uint64_t min;       // defaultValue = true valueRange = true
    uint64_t max;       // defaultValue = true valueRange = true
    bool isWithInitType = false;
    string initType;
};

typedef pair<string, uint32_t> FieldDataEnum;
typedef pair<string, FieldData> FieldDataStruct;

struct Enum
{
    string name;
    vector<FieldDataEnum> fields;
    std::optional<size_t> bitWidth;
};

struct Struct
{
    string name;
    vector<FieldDataStruct> fields;
};

class Formater
{
public:
    Formater();
    ComplexStatus AddEnumDeclaration(EnumType eTp, string enumName,
                                     std::optional<size_t> bitWidth = {});
    ComplexStatus AddEnumField(EnumType eTp,string enumName, string fieldName);
    ComplexStatus AddEnumField(EnumType eTp,string enumName, string fieldName, int code);
    ComplexStatus AddStructDeclaration(DataStructType strTp, string structName);
    ComplexStatus AddStructField(DataStructType strTp, string currBlockName,
                                 FieldInfo dataStruct);
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
