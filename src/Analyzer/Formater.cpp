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

ComplexStatus Formater::AddRule(Rule rule, bool hasReverse)
{
    auto error = RuleCheck(rule);

    if(error.status == Ok)
    {
        rules.push_back(rule);
        if(hasReverse) {rules.push_back(Rule::Reverse(rule));}

        ComplexStatus complexStatus = {Ok, "nullptr"};
        return complexStatus;
    }
    return error;
}

ComplexStatus Formater::RuleCheck(Rule rule)
{
    if(DataStructIsNotContains(codeList.at(0), *rule.command))
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *rule.command};
        return complexStatus;
    }
    if(DataStructIsNotContains(typeList.at(0), *rule.sendType))
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *rule.sendType};
        return complexStatus;
    }
    if(!rule.isEmptySend() && GetListDataStruct(packetList, *rule.sendPacket) ==
            packetList.end())
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *rule.sendPacket};
        return complexStatus;
    }
    if(rule.hasResponse() && DataStructIsNotContains(typeList.at(0), *rule.responseType))
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *rule.sendType};
        return complexStatus;
    }
    if(rule.hasResponseData() && GetListDataStruct(packetList, *rule.responsePacket) ==
            packetList.end())
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *rule.responsePacket};
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
        FieldData data(dataStruct);
        it->second.push_back({dataStruct.varName, data});
    }
    return status;
}
