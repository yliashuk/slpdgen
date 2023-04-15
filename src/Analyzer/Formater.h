#ifndef FORMATER_H
#define FORMATER_H

#include <iostream>
#include <string>
#include <vector>
#include "StructFieldData.h"
#include "Generics.h"

using namespace std;

struct ComplexStatus
{
    Status status;
    string wrongName;
};

struct Rule
{
    Rule(string* Command, string* SendType, string* SendPacket, string* ResponseType,
         string* ResponsePacket, bool isReverse = false)
    {
        if(Command != nullptr)
            this->command = *Command;

        if(SendType != nullptr)
            this->sendType = *SendType;

        if(SendPacket != nullptr){
            this->sendPacket = *SendPacket;
            this->isEmptySend = false;
        }
        else
            this->isEmptySend = true;

        if(ResponseType != nullptr){
            this->responseType = *ResponseType;
            this->isWithResponse = true;
        }
        else
            this->isWithResponse = false;

        if(ResponsePacket != nullptr){
            this->responsePacket = *ResponsePacket;
            this->isEmptyResponse = false;
        }
        else
            this->isEmptyResponse = true;

        this->isReverse = isReverse;
    }

    string command;
    string sendType;
    string sendPacket;
    bool isEmptySend;
    bool isWithResponse;
    bool isEmptyResponse;
    string responseType;
    string responsePacket;
    bool isReverse;
};

struct FieldData
{
    FieldData(){}
    FieldData(FieldInfo data, bool isStdType)
        :isStdType(isStdType),
         isNumOfCeils(data.isNumOfCeils),
         withLenDefiningVar(data.withLenDefiningVar),
         lenDefiningVar(data.lenDefiningVar),
         type(data.type),
         defaultValue(data.defaultValue),
         valueRange(data.valueRange),
         value(data.defaultVal),
         min(data.fromVal),
         max(data.toVal),
         isWithInitType(data.isWithSpecialType),
         initType(data.specialType)
    {
        if(isNumOfCeils)
            value = data.constLenDefiningVar;
    }

    bool   isStdType    = false;
    bool   isNumOfCeils = false;
    bool   withLenDefiningVar = false;
    string lenDefiningVar;
    string type;
    bool defaultValue   = false;
    bool valueRange     = false;
    uint64_t value;     // defaultValue = true valueRange = false
    uint64_t min;       // defaultValue = true valueRange = true
    uint64_t max;       // defaultValue = true valueRange = true
    bool isWithInitType = false;
    string initType;
};

typedef pair<string, uint32_t> FieldDataEnum;
typedef pair<string, vector<FieldDataEnum>> Enum;
typedef pair<string, FieldData> FieldDataStruct;
typedef pair<string, vector<FieldDataStruct>> Struct;

class Formater
{
public:
    Formater();
    ComplexStatus AddEnumDeclaration(EnumType eTp, string enumName);
    ComplexStatus AddEnumField(EnumType eTp,string enumName, string fieldName);
    ComplexStatus AddEnumField(EnumType eTp,string enumName, string fieldName, int code);
    ComplexStatus AddStructDeclaration(DataStructType strTp, string structName);
    ComplexStatus AddStructField(DataStructType strTp, string currBlockName,
                                 FieldInfo dataStruct);
    ComplexStatus AddRule(string* command = nullptr, string* sendType = nullptr,
                          string* sendPacket = nullptr, string* responseType = nullptr,
                          string* responsePacket = nullptr, bool isReverse = false);

    vector<Enum> enumList, codeList, typeList;
    vector<Struct> structList, packetList ,headerList;
    vector<Rule> rules;

private:
    ComplexStatus RuleCheck(string* command = nullptr, string* sendType = nullptr,
                            string* sendPacket = nullptr, string* responseType = nullptr,
                            string* responsePacket = nullptr);

    Status AddStructToList(vector<Struct>& list, string structName);

    Status AddFieldToStruct(vector<Struct>& list, string structName,
                            FieldInfo dataStruct);

    vector<string> GetStandartTypes();
};

#endif // FORMATER_H
