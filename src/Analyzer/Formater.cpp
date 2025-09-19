#include "Formater.h"
#include "Utils/StringUtils.h"

using namespace Utils;

Formater::Formater(){}

ComplexStatus Formater::addEnumDeclaration(EnumType eTp, string enumName,
                                           std::optional<size_t> bitWidth)
{
    ComplexStatus complexStatus;
    vector<Enum>::iterator it;
    switch (eTp) {
    case EnumType::SmplEnm: it = getListDataStruct(enumList, enumName); break;
    case EnumType::Tp: it = getListDataStruct(typeList, enumName); break;
    case EnumType::Cd: it = getListDataStruct(codeList, enumName); break;
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

ComplexStatus Formater::addEnumField(EnumType eTp, string enumName, string fieldName)
{
    Status status {};
    vector<Enum>::iterator it;
    switch (eTp) {
    case EnumType::SmplEnm: status = checkContent(enumList, it, enumName, fieldName); break;
    case EnumType::Tp: status  = checkContent(typeList, it, enumName, fieldName); break;
    case EnumType::Cd: status  = checkContent(codeList, it, enumName, fieldName); break;
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
        status = checkFieldValueRange(it->fields.back(), *it->bitWidth);
    }

    ComplexStatus complexStatus = {status, fieldName};
    return complexStatus;
}

ComplexStatus Formater::addEnumField(EnumType eTp, string enumName, string fieldName,
                                     int code)
{
    Status status {};
    vector<Enum>::iterator it;
    switch (eTp) {
    case EnumType::SmplEnm: status = checkContent(enumList, it, enumName, fieldName);
        break;
    case EnumType::Tp: status = checkContent(typeList, it, enumName, fieldName); break;
    case EnumType::Cd: status = checkContent(codeList, it, enumName, fieldName); break;
    }
    if(status == Ok) {
        it->fields.push_back({fieldName, code});
    }

    if(it->bitWidth.has_value()){
        status = checkFieldValueRange(it->fields.back(), *it->bitWidth);
    }

    ComplexStatus complexStatus = {status, fieldName};
    return complexStatus;
}

ComplexStatus Formater::addStructDeclaration(StructType strTp, string structName)
{
    Status status {};
    switch (strTp) {
    case StructType::SmplStr: status = addStructToList(structList,structName);break;
    case StructType::Pckt: status = addStructToList(packetList,structName);break;
    case StructType::Hdr: status = addStructToList(headerList,structName);break;
    }
    ComplexStatus complexStatus = {status,structName};
    return complexStatus;
}

ComplexStatus Formater::addStructField(StructType strTp, string currBlockName,
                                       FieldInfo fieldInfo)
{
    Status status {};
    switch (strTp) {
    case StructType::SmplStr: status = addFieldToStruct(structList, currBlockName,
                                                            fieldInfo);
        break;
    case StructType::Pckt: status = addFieldToStruct(packetList, currBlockName,
                                                         fieldInfo);
        break;
    case StructType::Hdr: status = addFieldToStruct(headerList, currBlockName,
                                                        fieldInfo);
        break;
    }
    ComplexStatus complexStatus = {status, fieldInfo.name};
    return complexStatus;
}

ComplexStatus Formater::addRule(Rule rule, bool hasReverse)
{
    auto error = checkRule(rule);

    if(error.status == Ok)
    {
        rules.push_back(rule);
        if(hasReverse) {rules.push_back(Rule::reverse(rule));}

        ComplexStatus complexStatus = {Ok, "nullptr"};
        return complexStatus;
    }
    return error;
}

string Formater::toJson() const
{
    auto jStr = [](auto opt){ return opt ? fmt("\"%s\"", {*opt}) : "null"; };
    auto jNum = [](auto opt){ return opt ? to_string(*opt) : "null"; };
    auto checkAndAddCommas = [](string& s){s = s.empty() ? s : s + ",\n";};

    auto jEnums = [&jNum, &checkAndAddCommas](const auto& slpdEnums) {
        string jObj;
        for(const auto& enumR : slpdEnums)
        {
            string jEnum = fmt("\"name\": \"%s\",\n", {enumR.name});
            jEnum += fmt("\"size\": \"%s\",\n", {jNum(enumR.bitWidth)});
            string jFields;
            for(const auto& field : enumR.fields)
            {
                string jField;
                jField += fmt("\t\"name\": \"%s\",\n", {field.first});
                jField += fmt("\t\"value\": %s\n", {to_string(field.second)});

                checkAndAddCommas(jFields);
                jFields += (fmt("{\n%s}", {jField}));
            }
            jEnum += fmt("\"fields\": [\n%s]\n", {jFields});
            checkAndAddCommas(jObj);
            jObj += fmt("{\n%s}", {jEnum});
        }
        return jObj;
    };

    auto jStructs = [&jStr, &jNum, &checkAndAddCommas](const auto& slpdStructs) {
        string jObj;
        for(const auto& structR : slpdStructs)
        {
            string jStruct = fmt("\"name\": \"%s\",\n", {structR.name});
            string jFields;
            for(const auto& field : structR.fields)
            {
                const auto& info = field.second;
                string jField;
                jField += fmt("\t\"name\": \"%s\",\n", {info.name});
                jField += fmt("\t\"type\": \"%s\",\n", {info.type});
                jField += fmt("\t\"sizeVar\": %s,\n", {jStr(info.sizeVar)});
                jField += fmt("\t\"constantSize\": %s,\n", {jNum(info.constantSize)});
                jField += fmt("\t\"specialType\": %s,\n", {jStr(info.specialType)});
                jField += fmt("\t\"initValue\": %s,\n", {jNum(info.initValue)});
                jField += fmt("\t\"fromVal\": %s,\n", {jNum(info.fromVal)});
                jField += fmt("\t\"toVal\": %s\n", {jNum(info.toVal)});

                checkAndAddCommas(jFields);
                jFields += (fmt("{\n%s}", {jField}));
            }
            jStruct += fmt("\"fields\": [\n%s]\n", {jFields});
            checkAndAddCommas(jObj);
            jObj += fmt("{\n%s}", {jStruct});
        }
        return jObj;
    };

    auto jRules = [&jStr, &checkAndAddCommas](const auto& slpdRules) {
        string jObj;
        for(const auto& rule : slpdRules)
        {
            string jRule;
            jRule += fmt("\t\"command\": %s,\n", {jStr(rule.command)});
            jRule += fmt("\t\"sendType\": %s,\n", {jStr(rule.sendType)});
            jRule += fmt("\t\"sendPacket\": %s,\n", {jStr(rule.sendPacket)});
            jRule += fmt("\t\"responseType\": %s,\n", {jStr(rule.responseType)});
            jRule += fmt("\t\"responsePacket\": %s\n", {jStr(rule.responsePacket)});

            checkAndAddCommas(jObj);
            jObj += fmt("{\n%s}", {jRule});
        }
        return jObj;
    };

    string json;
    json += fmt("\n\"type\": [\n%s],", {jEnums(typeList)});
    json += fmt("\n\"code\": [\n%s],", {jEnums(codeList)});
    json += fmt("\n\"enums\": [\n%s],", {jEnums(enumList)});
    json += fmt("\n\"header\": [\n%s],", {jStructs(headerList)});
    json += fmt("\n\"structs\": [\n%s],", {jStructs(structList)});
    json += fmt("\n\"messages\": [\n%s],", {jStructs(packetList)});
    json += fmt("\n\"rules\": [\n%s]", {jRules(rules)});
    return fmt("{%s}", {json});
}

ComplexStatus Formater::checkRule(Rule rule)
{
    if(dataStructIsNotContains(codeList.at(0), *rule.command))
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *rule.command};
        return complexStatus;
    }
    if(dataStructIsNotContains(typeList.at(0), *rule.sendType))
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *rule.sendType};
        return complexStatus;
    }
    if(!rule.isEmptySend() && getListDataStruct(packetList, *rule.sendPacket) ==
            packetList.end())
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *rule.sendPacket};
        return complexStatus;
    }
    if(rule.hasResponse() && dataStructIsNotContains(typeList.at(0), *rule.responseType))
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *rule.responseType};
        return complexStatus;
    }
    if(rule.hasResponseData() && getListDataStruct(packetList, *rule.responsePacket) ==
            packetList.end())
    {
        ComplexStatus complexStatus = {FieldNameNotContained, *rule.responsePacket};
        return complexStatus;
    }
    ComplexStatus complexStatus = {Ok, "nullptr"};
    return complexStatus;
}

Status Formater::addStructToList(vector<Struct> &list, string structName)
{
    if(getListDataStruct(list, structName) == list.end())
    {
        Struct _Struct;
        _Struct.name = structName;
        list.push_back(_Struct);
        return Ok;
    }
    return DuplStruct;
}

Status Formater::addFieldToStruct(vector<Struct> &list, string structName,
                                  FieldInfo dataStruct)
{
    vector<Struct>::iterator it;
    auto status = checkContent(list,it,structName, dataStruct.name);
    if(status == Ok)
    {
        it->fields.push_back({dataStruct.name, dataStruct});
    }
    return status;
}
