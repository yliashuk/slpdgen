#include "Formater.h"

Formater::Formater(){}

ComplexStatus Formater::AddEnumDeclaration(EnumType eTp, string enumName,
                                           std::optional<size_t> bitWidth)
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
        Enum _enum;
        _enum.name = enumName;
        _enum.bitWidth = bitWidth;

        switch (eTp) {
        case EnumType::SmplEnm: enumList.push_back(_enum); break;
        case EnumType::Tp:      typeList.push_back(_enum); break;
        case EnumType::Cd:      codeList.push_back(_enum); break;
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
    case EnumType::SmplEnm: status = ContentCheck(enumList, it, enumName, fieldName); break;
    case EnumType::Tp: status  = ContentCheck(typeList, it, enumName, fieldName); break;
    case EnumType::Cd: status  = ContentCheck(codeList, it, enumName, fieldName); break;
    }
    if(status == Ok)
    {
        if(!(*it).fields.empty()) {
            it->fields.push_back({fieldName,(*it).fields.back().second + 1});
        } else {
            it->fields.push_back({fieldName, 0});
        }
    }

    if(it->bitWidth.has_value()){
        status = CheckFieldValueRange(it->fields.back(), *it->bitWidth);
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
    if(status == Ok) {
        it->fields.push_back({fieldName, code});
    }

    if(it->bitWidth.has_value()){
        status = CheckFieldValueRange(it->fields.back(), *it->bitWidth);
    }

    ComplexStatus complexStatus = {status, fieldName};
    return complexStatus;
}

ComplexStatus Formater::AddStructDeclaration(StructType strTp, string structName)
{
    Status status {};
    switch (strTp) {
    case StructType::SmplStr: status = AddStructToList(structList,structName);break;
    case StructType::Pckt: status = AddStructToList(packetList,structName);break;
    case StructType::Hdr: status = AddStructToList(headerList,structName);break;
    }
    ComplexStatus complexStatus = {status,structName};
    return complexStatus;
}

ComplexStatus Formater::AddStructField(StructType strTp, string currBlockName,
                                       FieldInfo fieldInfo)
{
    Status status {};
    switch (strTp) {
    case StructType::SmplStr: status = AddFieldToStruct(structList, currBlockName,
                                                            fieldInfo);
        break;
    case StructType::Pckt: status = AddFieldToStruct(packetList, currBlockName,
                                                         fieldInfo);
        break;
    case StructType::Hdr: status = AddFieldToStruct(headerList, currBlockName,
                                                        fieldInfo);
        break;
    }
    ComplexStatus complexStatus = {status, fieldInfo.name};
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
        _Struct.name = structName;
        list.push_back(_Struct);
        return Ok;
    }
    return DuplStruct;
}

Status Formater::AddFieldToStruct(vector<Struct> &list, string structName,
                                  FieldInfo dataStruct)
{
    vector<Struct>::iterator it;
    auto status = ContentCheck(list,it,structName, dataStruct.name);
    if(status == Ok)
    {
        it->fields.push_back({dataStruct.name, dataStruct});
    }
    return status;
}
