#include "Formater.h"

Formater::Formater(){}

ComplexStatus Formater::AddEnumDeclaration(EnumType eTp, string enumName)
{
    ComplexStatus complexStatus;
    vector<Enum>::iterator it;
    switch (eTp) {
    case EnumType::SmplEnm: it = GetListDataStruct(enumList, enumName); break;
    case EnumType::Tp: it = GetListDataStruct(typeList, enumName); break;
    case EnumType::Cd: it = GetListDataStruct(codeList, enumName); break;
    }
   if(it == enumList.end())
   {
        Enum _Enum;
        _Enum.first = enumName;
        switch (eTp) {
        case EnumType::SmplEnm: enumList.push_back(_Enum); break;
        case EnumType::Tp:      typeList.push_back(_Enum); break;
        case EnumType::Cd:      codeList.push_back(_Enum); break;
        }
        complexStatus = {Ok, enumName};
   }else
    complexStatus = {DuplStruct, enumName};
   return complexStatus;
}

ComplexStatus Formater::AddEnumField(EnumType eTp, string enumName, string fieldName)
{
    Status status {};
    vector<Enum>::iterator it;
    switch (eTp) {
    case EnumType::SmplEnm: status = ContentCheck(enumList, it, enumName, fieldName);
        break;
    case EnumType::Tp: status  = ContentCheck(typeList, it, enumName, fieldName); break;
    case EnumType::Cd: status  = ContentCheck(codeList, it, enumName, fieldName); break;
    }
    if(status == Ok)
    {
        if(!(*it).second.empty())
            it->second.push_back({fieldName,(*it).second.back().second + 1});
        else
            it->second.push_back({fieldName, 0});
    }
    ComplexStatus complexStatus = {status, fieldName};
    return complexStatus;
}

ComplexStatus Formater::AddEnumField(EnumType eTp, string enumName, string fieldName,
                                     int code)
{
    Status status {};
    vector<Enum>::iterator it;
    switch (eTp) {
    case EnumType::SmplEnm: status = ContentCheck(enumList, it, enumName, fieldName);
        break;
    case EnumType::Tp: status = ContentCheck(typeList, it, enumName, fieldName); break;
    case EnumType::Cd: status = ContentCheck(codeList, it, enumName, fieldName); break;
    }
    if(status == Ok)
        it->second.push_back({fieldName, code});
    ComplexStatus complexStatus = {status, fieldName};
    return complexStatus;
}

ComplexStatus Formater::AddStructDeclaration(DataStructType strTp, string structName)
{
    Status status {};
    switch (strTp) {
    case DataStructType::SmplStr: status = AddStructToList(structList,structName);break;
    case DataStructType::Pckt: status = AddStructToList(packetList,structName);break;
    case DataStructType::Hdr: status = AddStructToList(headerList,structName);break;
    }
    ComplexStatus complexStatus = {status,structName};
    return complexStatus;
}

ComplexStatus Formater::AddStructField(DataStructType strTp, string currBlockName,
                                       FieldInfo dataStruct)
{
    Status status {};
    switch (strTp) {
    case DataStructType::SmplStr: status = AddFieldToStruct(structList, currBlockName,
                                                            dataStruct);
        break;
    case DataStructType::Pckt: status = AddFieldToStruct(packetList, currBlockName,
                                                         dataStruct);
        break;
    case DataStructType::Hdr: status = AddFieldToStruct(headerList, currBlockName,
                                                        dataStruct);
        break;
    }
    ComplexStatus complexStatus = {status, dataStruct.varName};
    return complexStatus;
}

ComplexStatus Formater::AddRule(string *command, string *sendType, string *sendPacket,
                                string *responseType, string *responsePacket,
                                bool isReverse)
{
    auto error = RuleCheck(command, sendType, sendPacket, responseType, responsePacket);
    if(error.status == Ok)
    {
        Rule rule(command,sendType, sendPacket, responseType, responsePacket, isReverse);
        rules.push_back(rule);
        ComplexStatus complexStatus = {Ok, "nullptr"};
        return complexStatus;
    }

    if(command != nullptr)
        delete command;
    if(sendType != nullptr)
        delete sendType;
    if(sendPacket != nullptr)
        delete sendPacket;
    if(responseType != nullptr)
        delete responseType;
    if(responsePacket != nullptr)
        delete responsePacket;

    return error;
}

ComplexStatus Formater::RuleCheck(string *command, string *sendType, string *sendPacket,
                                  string *responseType, string *responsePacket)
{
    if(command != nullptr && DataStructIsNotContains(codeList.at(0), *command))
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *command};
        return complexStatus;
    }
    if(sendType != nullptr && DataStructIsNotContains(typeList.at(0), *sendType))
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *sendType};
        return complexStatus;
    }
    if(sendPacket != nullptr && GetListDataStruct(packetList, *sendPacket) ==
            packetList.end())
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *sendPacket};
        return complexStatus;
    }
    if(responseType != nullptr && DataStructIsNotContains(typeList.at(0), *responseType))
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *sendType};
        return complexStatus;
    }
    if(responsePacket != nullptr && GetListDataStruct(packetList, *responsePacket) == packetList.end())
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *responsePacket};
        return complexStatus;
    }
    ComplexStatus complexStatus = {Ok, "nullptr"};
    return complexStatus;
}

Status Formater::AddStructToList(vector<Struct> &list, string structName)
{
    if(GetListDataStruct(list, structName) == list.end())
    {
        Struct _Struct;
        _Struct.first = structName;
        list.push_back(_Struct);
        return Ok;
    }
    return DuplStruct;
}

Status Formater::AddFieldToStruct(vector<Struct> &list, string structName,
                                  FieldInfo dataStruct)
{
    vector<Struct>::iterator it;
    auto status = ContentCheck(list,it,structName, dataStruct.varName);
    if(status == Ok)
    {
        auto isStd = VectorIsContained(GetStandartTypes(), dataStruct.type);
        FieldData data(dataStruct, isStd);
        it->second.push_back({dataStruct.varName, data});
    }
    return status;
}

vector<string> Formater::GetStandartTypes()
{
    vector<string> strings;

    strings.push_back("char");
    strings.push_back("s8");
    strings.push_back("s16");
    strings.push_back("s32");
    strings.push_back("s64");
    strings.push_back("i8");
    strings.push_back("i16");
    strings.push_back("i32");
    strings.push_back("i64");
    strings.push_back("u8");
    strings.push_back("u16");
    strings.push_back("u32");
    strings.push_back("u64");
    strings.push_back("f32");
    strings.push_back("f64");
    return strings;
}
