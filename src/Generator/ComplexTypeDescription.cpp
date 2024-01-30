#include "ComplexTypeDescription.h"
#include <Utils/StringUtils.h>

ComplexTypeDescription::ComplexTypeDescription()
{

}

void ComplexTypeDescription::SetBlockType(ComplexType type)
{
    this->blockType = type;
}

void ComplexTypeDescription::SetName(string name)
{
    this->name = name;
}

string ComplexTypeDescription::GetName()
{
    return name;
}

string ComplexTypeDescription::GetCodeName()
{
    return BlType() + b_und + name;
}

Polynomial ComplexTypeDescription::Size()
{   Polynomial size = 0;
    for(auto var: _fields)
        size += var.fieldSize;
    return size;
}

void ComplexTypeDescription::addField(FieldDataStruct data,
                                      pair<string, fieldType> type,
                                      Polynomial fieldSize)
{
    StructField field;
    field.data = data.second;
    field.fieldName = data.first;
    field.type.first = type.first;
    field.type.second = type.second;

    if(data.second.isArrayField) {
        if(data.second.hasDynamicSize) {
            field.fieldSize = ("str->" + data.second.lenDefiningVar);
        } else {
            field.fieldSize =  data.second.value;
        }
        field.arrayTypeSize = fieldSize;
    }
    else field.fieldSize = fieldSize;

    field.fieldOffset = Size();
    _fields.push_back(field);

    if(data.second.hasDynamicSize)
    {
        this->hasDynamicFields = true;
        VarArray arr;
        arr.varArrayType = data.first;
        arr.varArraySize = fieldSize;
        string str;

        if(!FieldTypePrefix(type.second).empty())
            str = FieldTypePrefix(type.second) + b_und;

        arr.varArrayTypeName = str + type.first;
        arr.varArrayLen = data.second.lenDefiningVar;
        varArrays.push_back(arr);
    }
}

vector<string> ComplexTypeDescription::Declaration()
{
    auto blockName = BlType() + b_und + name;
    StructCpp structCpp;
    structCpp.SetName(blockName);
    structCpp.SetTypeDef(!qtCppOption);

    StructCpp::Fields fields;
    for(auto var:_fields)
    {
        string dynSizeVar = var.data.lenDefiningVar;
        string fieldName = var.fieldName;
        string type = var.type.first;

        bool isArray = var.data.isArrayField;
        bool isDynamicArray = isArray && var.data.hasDynamicSize;
        bool isStaticArray = isArray && !isDynamicArray;
        bool isArrayVarLen = IsArrayVarLen(fieldName);


        bool isNotUsedVarLen = qtCppOption && isArrayVarLen;
        bool isPointerToArr = !qtCppOption && isDynamicArray;

        string comment = {};
        if(isNotUsedVarLen || isPointerToArr)
        {
            string helpComm = fmt("%s[%s]", {fieldName, dynSizeVar});
            string notUsedComm = "not used";
            comment = isNotUsedVarLen ? notUsedComm : helpComm;
        }

        string typePrefix = FieldTypePrefix(var.type.second);
        bool isStd = var.type.second == fieldType::std;

        string fieldT = isStd ? type : typePrefix + "_" + type;
        string len = to_string(var.data.value);

        if(qtCppOption && isDynamicArray)
        {
            fields += {fmt("std::vector<%s>", {fieldT}), fieldName};
        }
        else if(qtCppOption && isStaticArray)
        {
            fields += {fmt("std::array<%s, %s>", {fieldT, len}), fieldName};
        }
        else
        {
            string p = isDynamicArray ? "*" : "";
            string var = isStaticArray ? fmt("%s[%s]", {fieldName, len}) : fieldName;
            fields += {fieldT, p + var, comment};
        }
    }

    structCpp.AddFields(fields);
    return structCpp.Declaration();
}

vector<string> ComplexTypeDescription::SerDesDefinition(FunType funType, bool hasStatic)
{
    auto blockName =  BlType() + b_und + name;
    Strings strings, funcBody;

    Function funConstruct;
    funConstruct.SetStaticDeclaration(hasStatic);

    funcBody += "uint32_t size = 0;";

    SetSerDesDeclaration(funConstruct, funType);

    if(funType == Des)
    {
        //funcBody.push_back(PrintVarDeclaration(BlockName,"str"));
        IfElseStatementCpp statement;
        statement.AddCase("*op_status == 0", "return size" + smcln);
        auto var = statement.GetDefinition();
        funcBody.insert(funcBody.end(), var.begin(), var.end());
    }

    //string addOffS, addOffD;
    for(auto var = _fields.begin(); var !=_fields.end(); var++)
    {
        bool isArray = var->data.isArrayField;
        bool isDynamicArray = var->data.hasDynamicSize;
        bool isStaticArray = !isDynamicArray && isArray;

        String fieldSize = var->fieldSize.Get();
        String arrayElementSize = var->arrayTypeSize.Get();

        String type = var->type.first;
        String fieldName = var->fieldName;
        String value = to_string(var->data.value);

        if(var->type.second != fieldType::Struct)
        {
            if(funType == Ser)
            {
                if(isArray)
                {
                    funcBody += SerArrayField(*var);
                }
                else
                {
                    if(var->data.initValue) {
                        funcBody += fmt("str->%s = %s;", {fieldName, value});
                    }
                    string copyFmt = "memcpy(p + size, &str->%s, %s);";
                    funcBody += fmt(copyFmt, {fieldName, fieldSize});
                    funcBody += fmt("size += %s;", {fieldSize});
                }
            } else if(funType == Des)
            {
                auto typePrefix = FieldTypePrefix(var->type.second);
                if(isArray)
                {
                    ForLoopCpp loop;
                    Strings body;

                    if(qtCppOption && isDynamicArray) {
                        funcBody += fmt("str->%s.resize(str->$s);", {fieldName, fieldSize});
                    }
                    else if(isDynamicArray)
                    {
                        String elementType = fmt("%{%s_}%s",{typePrefix, var->type.first});

                        String allocFmt = "str->%s =(%s*)allocate(%s*sizeof(%s));";
                        Strings allocFmtArgs = {fieldName, elementType, fieldSize, elementType};
                        funcBody += fmt(allocFmt, allocFmtArgs);
                    }

                    loop.SetDeclaration("int i = 0","i < " + fieldSize, "i++");
                    string copyFmt = "memcpy(&str->%s[i], p + size, %s);";

                    body += fmt(copyFmt, {fieldName, arrayElementSize});
                    body += fmt("size += %s;", {arrayElementSize});

                    loop.SetBody(body);
                    funcBody += loop.GetDefinition();
                }
                else
                {
                    if(var->type.second == fieldType::Enum ||
                            var->type.second == fieldType::Type ||
                            var->type.second == fieldType::Code)
                    {
                        funcBody += fmt("str->%s =(%s_%s)0;", {fieldName, typePrefix, type});
                    }

                    string copyFmt = "memcpy(&str->%s, p + size, %s);";
                    funcBody += fmt(copyFmt, {fieldName, fieldSize});
                    funcBody += fmt("size += %s;", {fieldSize});

                    if(var->data.initValue)
                    {
                        IfElseStatementCpp statement;
                        statement.AddCase("str->" + var->fieldName + neql +
                                          to_string(var->data.value), "*op_status = 0" +
                                          smcln + " return size" + smcln);
                        auto var = statement.GetDefinition();
                        funcBody.insert(funcBody.end(),var.begin(),var.end());
                    }
                    if(var->data.valueRange)
                    {
                        IfElseStatementCpp statement;
                        auto st1 = "str->" + var->fieldName + " < " +
                                to_string(var->data.min);
                        auto st2 = "str->" + var->fieldName + " > " +
                                to_string(var->data.max);
                        if(var->data.min == 0){
                            statement.AddCase(st2,"*op_status = 0" + smcln + +
                                              " return size" + smcln);
                        }
                        else {
                            statement.AddCase(st1 + orS + st2,"*op_status = 0" +
                                              smcln + + " return size" + smcln);
                        }
                        auto var = statement.GetDefinition();
                        funcBody.insert(funcBody.end(),var.begin(),var.end());
                    }
                }
            }
        }else if(var->type.second == fieldType::Struct)
        {
            Function localFunc;
            if(funType == Ser)
            {
                if(isArray)
                {
                    funcBody += SerArrayField(*var);
                }
                else
                {
                    funcBody.push_back("size += " + _pSer + FieldTypePrefix(var->type.second) +
                                       b_und + var->type.first + lsb +
                                       string("p + size") +
                                       com + "&str->" + var->fieldName + rsb + smcln);
                }
            }
            else if(funType == Des)
            {
                if(var->data.isArrayField)
                {
                    ForLoopCpp loop;

                    if(qtCppOption && var->data.hasDynamicSize)
                    {
                        funcBody.push_back(string("str->") + var->fieldName +
                                           ".resize" + "(str->" +
                                           var->data.lenDefiningVar + ")" + smcln);
                    }
                    else if(var->data.hasDynamicSize)
                    {
                        string numOfElements = var->data.lenDefiningVar;
                        string typePrefix = FieldTypePrefix(var->type.second);
                        string elementType = typePrefix + b_und + var->type.first;
                        string elementSize = PrintSizeOf(elementType);
                        string typeConvers = "(" + elementType + "*)";

                        funcBody.push_back(string("str->") + var->fieldName +
                                           " =" + typeConvers + "allocate" + "(str->" +
                                           numOfElements + "*" + elementSize + ")" + smcln);
                    }

                    loop.SetDeclaration("int i = 0","i < " + var->fieldSize.Get() /*+
                                        " / " + var->baseTypeSize.Get()*/,"i++");
                    loop.SetBody("size += " + _pDes + FieldTypePrefix(var->type.second) + b_und +
                                 var->type.first + lsb +
                                 string("p + size") +
                                 com + "&str->" + var->fieldName +
                                 PutInSqBraces("i") + com + string("op_status") +
                                 rsb + smcln);

                    auto var = loop.GetDefinition();
                    funcBody.insert(funcBody.end(),var.begin(),var.end());

                }else
                funcBody.push_back("size += " + _pDes + FieldTypePrefix(var->type.second) +
                                   b_und +var->type.first + lsb +
                                   string("p + size") +
                                   com + string("&str->")+var->fieldName + com +
                                   string("op_status")+ rsb + smcln);
            }
        }
    }
    funcBody.push_back("return size" + smcln);
    funConstruct.SetBody(funcBody);
    return funConstruct.GetDefinition();
}
string ComplexTypeDescription::PrintSerDesCall(FunType type, string structName)
{
    auto BlockName =  BlType() + b_und + name;
    vector<string> strings, funcBody;
    Function funConstruct;
    if(type == Ser)
    {
        vector<Parameter> params;
        params.push_back({"char*","l_p"});
        params.push_back({BlockName,"str"});
        funConstruct.SetDeclaration(_pSer + BlType() + b_und + name,"void",params);
    }
    else
    {
        vector<Parameter> params;
        if(blockType == ComplexType::Header && !qtOption)
            params.push_back({cObjType + "*","obj"});
        params.push_back({"char*","l_p"});
        params.push_back({BlockName,structName});
        params.push_back({"uint8_t*","op_status"});
        funConstruct.SetDeclaration(_pDes + BlType() + b_und + name,"void",params);
    }
    return funConstruct.GetCall();
}

string ComplexTypeDescription::PrintSerCall()
{
    return _pSer + BlType() + b_und + name + lsb + string("l_p") + com + BlType() +
            b_und + name + rsb + smcln;
}

string ComplexTypeDescription::PrintSerCall(string pointerName, string paramName)
{
    return _pSer + BlType() + b_und + name + lsb + pointerName + com + paramName + rsb +
            smcln;
}

string ComplexTypeDescription::PrintDesCall(string pointerName, string paramName)
{
    return _pDes + BlType() + b_und + name + lsb + pointerName + com + paramName + rsb +
            smcln;
}

string ComplexTypeDescription::PrintVarDecl()
{
    return BlType() + b_und + name + spc + "header";
}

vector<string> ComplexTypeDescription::PrintSizeCalcFun(FunType type, bool hasStatic)
{
    auto fun = DescHelper::CalcSizeFunDecl(BlType() + b_und + name, type, hasStatic);

    vector<string> body;

    if(type == Ser)
    {
        // set vector sizes to array length variables
        if(qtCppOption)
        {
            for(const auto& var : varArrays)
            {
                body.push_back("str->" + var.varArrayLen + " = str->" +
                               var.varArrayType + ".size()" + smcln);
            }
            if(varArrays.size()){ body.push_back(""); }
        }
        // for suppress unused warning
        body.push_back("(void)str" + smcln);

        body.push_back("size_t size = 0" + smcln);
    }
    else
    {
        string structName = BlType() + b_und + name;
        body.push_back("(void)p" + smcln);

        string initVal = "{" + PrintSizeOf(structName) + ", 0, 0}";;
        body.push_back("c_size_t c_size = "  + initVal + smcln);
    }

    for(auto field : _fields)
    {
        string prefix = FieldTypePrefix(field.type.second) + b_und;
        fieldType fldType = field.type.second;
        bool isArray  = field.data.isArrayField;
        bool isDynamicArray = field.data.hasDynamicSize;

        if(fldType != fieldType::Struct)
        {
            if(type == Des && IsArrayVarLen(field.fieldName))
            {
                string name = field.fieldName;
                string size = field.fieldSize.Get();
                string type = field.type.first;
                body.push_back(PrintVarDeclaration(type, name, "0"));
                body.push_back(PrintMemcpy("&" + name, "p + c_size.r", size));
            }
            if(type == Des && isDynamicArray)
            {
                string calc;
                calc = DescHelper::CalcDesSimpleArrTypeSize(prefix, field);
                body.push_back(calc);
            }
            body.push_back(DescHelper::CalcSimpeTypeSize(field, type));
        }
        else if(isArray && fldType == fieldType::Struct)
        {
            auto loopDef = DescHelper::CalcSizeLoop(prefix, field, type);
            DescHelper::AppendStrings(body, loopDef);
        }
        else if(fldType == fieldType::Struct)
        {
            auto calcSize = DescHelper::CalcStructSize(prefix, field, type);
            DescHelper::AppendStrings(body, calcSize);
        }
    }

    string returnVarName = type == Ser ? "size" : "c_size";
    body.push_back("return " + returnVarName + smcln);

    fun.SetBody(body);
    return fun.GetDefinition();
}

string ComplexTypeDescription::PrintSizeCalcFunCall(FunType type)
{
    auto funName = DescHelper::CalcSizeFunName(BlType() + b_und + name, type);
    string varName = type == Ser ? "str" : "l_p";
    return funName + "(" + varName + ")";
}

string ComplexTypeDescription::BlType()
{
    if(blockType == ComplexType::Message) return  _prefix + "message";
    if(blockType == ComplexType::Header) return  _prefix + "HEADER";
    else return  _prefix + "struct";
}

void ComplexTypeDescription::SetPrefix(string prefix)
{
    _prefix = prefix + b_und;
}

string ComplexTypeDescription::FieldTypePrefix(fieldType type)
{
    switch (type)
    {
    case fieldType::Enum: return _prefix + "enum";
    case fieldType::Struct: return _prefix + "struct";
    case fieldType::Code: return _prefix + "CODE";
    case fieldType::Type: return _prefix + "TYPE";
    default: return "";
    }
}

bool ComplexTypeDescription::IsArrayVarLen(string name)
{
    for(const auto& lenVar : varArrays)
    {
        if(lenVar.varArrayLen == name)
        {
            return true;
        }
    }
    return false;
}

void ComplexTypeDescription::SetSerDesDeclaration(Function &function, FunType type)
{
    auto blockName =  BlType() + b_und + name;

    vector<Parameter> params;
    params.push_back({"char*", "p"});
    params.push_back({blockName, "*str"});

    if(type == Des)
    {
        if(blockType == ComplexType::Header && !qtOption) {
            params.push_back({cObjType + "*", "obj"});
        }
        params.push_back({"uint8_t*", "op_status"});
    }
    String serDesPrefix = type == Ser ? _pSer : _pDes;
    function.SetDeclaration(serDesPrefix + blockName,"uint32_t", params);
}

Strings ComplexTypeDescription::SerArrayField(const StructField& field)
{
    const String& fieldSize = field.fieldSize.Get();
    const String& fieldName = field.fieldName;
    const String& elementSize = field.arrayTypeSize.Get();
    auto metaType = field.type.second;
    Strings loopBody;

    if(metaType != fieldType::Struct)
    {
        string copyFmt = "memcpy(p + size, &str->%s[i], %s);";

        loopBody += fmt(copyFmt, {fieldName, elementSize});
        loopBody += fmt("size += %s;",{elementSize});

    } else if(metaType == fieldType::Struct)
    {
        String type = field.type.first;
        type = fmt("%s%s_%s",{_pSer, FieldTypePrefix(metaType), type});
        loopBody += fmt("size += %s(p + size, &str->%s[i]);", {type, fieldName});
    }

    ForLoopCpp loop;
    loop.SetDeclaration("int i = 0","i < " + fieldSize, "i++");
    loop.SetBody(loopBody);

    return loop.GetDefinition();
}

Function ComplexTypeDescription::GetCompareFun()
{
    Function fun;
    vector<Parameter> parameters;

    auto structName = BlType() + b_und + name;
    parameters.push_back({"const " + structName + "&", "obj1"});
    parameters.push_back({"const " + structName + "&", "obj2"});

    fun.SetDeclaration("operator==", "bool", parameters);

    vector<string> body;

    IfElseStatementCpp statement;
    for(const auto& field : _fields)
    {
        statement.AddCase(lsb + "obj1." + field.fieldName + eql + "obj2." +
                          field.fieldName + rsb + eql + "false",  "return false" +
                          smcln);
    }
    auto statementDefinition = statement.GetDefinition();
    body.insert(body.end(), statementDefinition.begin(), statementDefinition.end());

    body.push_back("return true" + smcln);

    fun.SetBody(body);

    return fun;
}
