#include "ComplexTypeDescription.h"
#include <Utils/StringUtils.h>
#include "StdTypeHandler.h"

ComplexTypeDescription::ComplexTypeDescription(){}

void ComplexTypeDescription::SetOption(AppOptions options)
{
    _options = options;
}

void ComplexTypeDescription::SetBlockType(ComplexType type)
{
    this->_blockType = type;
}

void ComplexTypeDescription::SetName(string name)
{
    this->name = name;
}

string ComplexTypeDescription::GetName() const
{
    return name;
}

string ComplexTypeDescription::GetCodeName() const
{
    return BlType() + "_" + name;
}

SizeExprPtr ComplexTypeDescription::Size() const
{   SizeExprPtr size = 0_lit;
    for(auto var: _fields) {
        size += var.bitSize;
    }
    return size;
}

void ComplexTypeDescription::AddContextOffset(SizeExprPtr offset)
{
    _contextOffsets.push_back(offset);
}

void ComplexTypeDescription::addField(FieldDataStruct data,
                                      pair<string, fieldType> type,
                                      SizeExprPtr fieldSize)
{
    StructField field;
    field.data = data.second;
    field.name = data.first;
    field.type.first = type.first;
    field.type.second = type.second;

    if(data.second.isArrayField) {
        if(data.second.hasDynamicSize) {
            field.arrayElementCount = Variable::Create("str->" + data.second.lenDefiningVar);
        } else {
            field.arrayElementCount = Literal::Create(data.second.value);
        }
        field.arrayElementSize = fieldSize;
        field.bitSize = field.arrayElementSize * field.arrayElementCount;
    }
    else { field.bitSize = fieldSize; }

    _fields.push_back(field);
}

vector<StructField> ComplexTypeDescription::GetFields() const
{
    return _fields;
}

vector<string> ComplexTypeDescription::Declaration()
{
    auto blockName = fmt("%s_%s", {BlType(), name});
    StructCpp structCpp;
    structCpp.SetName(blockName);
    structCpp.SetTypeDef(!_options.isCpp);

    StructCpp::Fields fields;
    for(auto var:_fields)
    {
        string dynSizeVar = var.data.lenDefiningVar;
        string fieldName = var.name;
        string type = var.type.first;

        bool isArray = var.data.isArrayField;
        bool isDynamicArray = isArray && var.data.hasDynamicSize;
        bool isStaticArray = isArray && !isDynamicArray;
        bool isArrayVarLen = IsArrayVarLen(fieldName);


        bool isNotUsedVarLen = _options.isCpp && isArrayVarLen;
        bool isPointerToArr = !_options.isCpp && isDynamicArray;

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

        if(_options.isCpp && isDynamicArray)
        {
            fields += {fmt("std::vector<%s>", {fieldT}), fieldName};
        }
        else if(_options.isCpp && isStaticArray)
        {
            fields += {fmt("std::array<%s, %s>", {fieldT, len}), fieldName};
        }
        else
        {
            string bitWidth =  IsSimpleBitField(var) ? var.bitSize->ToString() : "";
            string p = isDynamicArray ? "*" : "";
            string var = isStaticArray ? fmt("%s[%s]", {fieldName, len}) : fieldName;
            fields += {fieldT, p + var, bitWidth, comment};
        }
    }

    structCpp.AddFields(fields);
    return structCpp.Declaration();
}

vector<string> ComplexTypeDescription::SerDesDefinition(FunType funType, bool hasStatic)
{
    Strings body;
    body << "size_t pos = offset;";

    for(auto var = _fields.begin(); var !=_fields.end(); var++)
    {
        bool isArray = var->data.isArrayField;

        if(funType == Ser)
        {
            body << (isArray ? SerArrayField(*var) : SerSimpleField(*var));
        }
        else if(funType == Des)
        {
            body << (isArray ? DesArrayField(*var) : DesSimpleField(*var));

            if(var->data.hasInitValue)
            {
                body << DesCheckInitValue(*var);
            }
            if(var->data.valueRange)
            {
                body << DesCheckValueRange(*var);
            }
        }
    }

    if(funType == Des) { body << "*op_status = 1;"; }
    body << "return pos - offset;";

    Function funConstruct;
    SetSerDesDeclaration(funConstruct, funType);
    funConstruct.SetStatic(hasStatic);
    funConstruct.SetBody(body);

    return funConstruct.Definition();
}

string ComplexTypeDescription::SerCall(string pointerName, string offset, string paramName) const
{
    return fmt("%s%s_%s(%s, %s, %s)", {_pSer, BlType(), name, pointerName, offset, paramName});
}

string ComplexTypeDescription::DesCall(string pointerName, string offset,
                                       string paramName, string op_status) const
{
    return fmt("%s%s_%s(%s, %s, %s, %s)",
    {_pDes, BlType(), name, pointerName, offset, paramName, op_status});
}

string ComplexTypeDescription::AsVarDecl() const
{
    return fmt("%s_%s header", {BlType(), name});
}

vector<string> ComplexTypeDescription::SizeCalcFun(FunType type, bool hasStatic)
{
    auto fun = CalcSizeHelper::CalcSizeFunDecl(BlType() + "_" + name, type, hasStatic);

    vector<string> body;

    if(type == Ser)
    {
        // set vector sizes to array length variables
        if(_options.isCpp)
        {
            auto pairs = ArrNameSizeVarPairs();
            for(auto [arrName, sizeVar] : pairs)
            {
                body << fmt("str->%s = str->%s.size();", {sizeVar, arrName});
            }
            if(pairs.size()){ body.push_back(""); }
        }
        // for suppress unused warning
        body << "(void)str;";

        body << "size_t size = 0;";
    }
    else
    {
        string structName = BlType() + '_' + name;
        body << "(void)p;";

        string initVal = fmt("{sizeof(%s), 0, 0}", {structName});
        body << fmt("c_size_t c_size = %s;", {initVal});
    }

    for(auto field : _fields)
    {
        string prefix = FieldTypePrefix(field.type.second) + '_';
        fieldType fldType = field.type.second;
        bool isArray  = field.data.isArrayField;
        bool isDynamicArray = field.data.hasDynamicSize;

        if(fldType != fieldType::Struct)
        {
            if(type == Des && IsArrayVarLen(field.name))
            {
                string name = field.name;
                string size = field.bitSize->ToString();
                string type = field.type.first;
                string copyName = UsedCopyMethod(field);
                body << fmt("%s %s = 0;", {type, name});
                body << fmt("%s(&%s, 0, p, c_size.r, %s);", {copyName, name, size});
            }
            if(type == Des && isDynamicArray)
            {
                body << CalcSizeHelper::CalcDesSimpleArrTypeSize(prefix, field);
            }
            body << CalcSizeHelper::CalcSimpeTypeSize(field, type);
        }
        else if(isArray && fldType == fieldType::Struct)
        {
            body << CalcSizeHelper::CalcSizeLoop(prefix, field, type);
        }
        else if(fldType == fieldType::Struct)
        {
            body << CalcSizeHelper::CalcStructSize(prefix, field, type);
        }
    }

    string returnVarName = type == Ser ? "size" : "c_size";
    body << fmt("return %s;", {returnVarName});

    fun.SetBody(body);
    return fun.Definition();
}

string ComplexTypeDescription::SizeCalcFunCall(FunType type) const
{
    auto funName = CalcSizeHelper::CalcSizeFunName(BlType() + "_" + name, type);
    string varName = type == Ser ? "str" : "l_p";
    return funName + "(" + varName + ")";
}

string ComplexTypeDescription::BlType() const
{
    if(_blockType == ComplexType::Message) return  _prefix + "message";
    if(_blockType == ComplexType::Header) return  _prefix + "HEADER";
    else return  _prefix + "struct";
}

void ComplexTypeDescription::SetPrefix(string prefix)
{
    _prefix = prefix + '_';
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

bool ComplexTypeDescription::IsArrayVarLen(string name) const
{
    for(const auto& field : _fields) {
        if (field.data.lenDefiningVar == name) { return true; }
    }
    return false;
}

std::vector<std::pair<String, String>> ComplexTypeDescription::ArrNameSizeVarPairs()
{
    std::vector<std::pair<String, String>> pairs;
    for(const auto& field : _fields) {
        if (!field.data.lenDefiningVar.empty()) {
            pairs.push_back({field.name, field.data.lenDefiningVar});
        }
    }
    return pairs;
}

void ComplexTypeDescription::SetSerDesDeclaration(Function &function, FunType type)
{
    auto blockName =  BlType() + '_' + name;

    vector<Parameter> params;

    params.push_back({"char*", "p"});
    params.push_back({"size_t", "offset"});
    params.push_back({blockName, "*str"});

    if(type == Des) {
        params.push_back({"uint8_t*", "op_status"});
    }
    String serDesPrefix = type == Ser ? _pSer : _pDes;
    function.SetDeclaration(serDesPrefix + blockName,"size_t", params);
}

Strings ComplexTypeDescription::SerArrayField(const StructField& field)
{
    const String& arrSize = field.arrayElementCount->ToString();
    const String& arrName = field.name;
    const String& elementSize = field.arrayElementSize->ToString();
    auto type = field.type.first;
    auto metaType = field.type.second;
    auto copyName = UsedCopyMethod(field);
    Strings body;
    Strings loopBody;

    if(metaType != fieldType::Struct)
    {
        if(IsArrayBitField(field))
        {
            string varType = StdTypeHandler::SlpdToAlignedCppType(type);
            body << fmt("%s %s_value = 0;", {varType, arrName});

            loopBody << fmt("%s_value = str->%s[i].value;", {arrName, arrName});
            string copyFmt = "pos += %s(p, pos, &%s_value, 0, %s);";
            loopBody << fmt(copyFmt, {copyName, arrName, elementSize});
        } else if(IsArrayAlignedField(field)) {
            string copyFmt = "pos += %s(p, pos, &str->%s[0], 0, %s * %s);";
            body << fmt(copyFmt, {copyName, arrName, elementSize, arrSize});
        } else {
            string copyFmt = "pos += %s(p, pos, &str->%s[i], 0, %s);";
            loopBody << fmt(copyFmt, {copyName, arrName, elementSize});
        }
    } else if(metaType == fieldType::Struct)
    {
        type = fmt("%s%s_%s",{_pSer, FieldTypePrefix(metaType), type});
        loopBody << fmt("pos += %s(p, pos, &str->%s[i]);", {type, arrName});
    }

    if(!loopBody.empty())
    {
        ForLoopCpp loop;
        loop.SetDeclaration("int i = 0","i < " + arrSize, "i++");
        loop.SetBody(loopBody);
        body << loop.Definition();
    }

    return body;
}

Strings ComplexTypeDescription::DesArrayField(const StructField &field)
{
    const String& arrSize = field.arrayElementCount->ToString();
    const String& arrName = field.name;
    const String& elementSize = field.arrayElementSize->ToString();
    auto type = field.type.first;
    auto metaType = field.type.second;
    auto copyName = UsedCopyMethod(field);

    bool isDynamicArray = field.data.hasDynamicSize;

    Strings body, loopBody;

    if(isDynamicArray) {
        body << ArrayFieldAllocation(field);
    }

    if(metaType != fieldType::Struct)
    {
        if(IsArrayBitField(field))
        {
            string varType = StdTypeHandler::SlpdToAlignedCppType(type);
            body << fmt("%s %s_value = 0;", {varType, arrName});

            string copyFmt = "pos += %s(&%s_value, 0, p, pos, %s);";
            loopBody << fmt(copyFmt, {copyName, arrName, elementSize});
            loopBody << fmt("str->%s[i].value = %s_value;", {arrName, arrName});
        } else if(IsArrayAlignedField(field)) {
            string copyFmt = "pos += %s(&str->%s[0], 0, p, pos, %s * %s);";
            body << fmt(copyFmt, {copyName, arrName, elementSize, arrSize});
        } else {
            string copyFmt = "pos += %s(&str->%s[i], 0, p, pos, %s);";
            loopBody << fmt(copyFmt, {copyName, arrName, elementSize});
        }
    }
    else if(metaType == fieldType::Struct)
    {
        type = fmt("%s%s_%s",{_pDes, FieldTypePrefix(metaType), type});
        loopBody << fmt("pos += %s(p, pos, &str->%s[i], op_status);",
                        {type, arrName});
    }

    if(!loopBody.empty())
    {
        ForLoopCpp loop;
        loop.SetDeclaration("int i = 0","i < " + arrSize, "i++");
        loop.SetBody(loopBody);
        body << loop.Definition();
    }

    return body;
}

Strings ComplexTypeDescription::ArrayFieldAllocation(const StructField &field)
{
    auto metaType = field.type.second;
    const String& arrName = field.name;
    const String& arrSize = field.arrayElementCount->ToString();

    Strings body;

    if(_options.isCpp) {
        body << fmt("str->%s.resize(%s);", {arrName, arrSize});
    }
    else
    {
        String typePrefix = FieldTypePrefix(metaType);
        String elementType = fmt("%{%s_}%s",{typePrefix, field.type.first});

        String allocFmt = "str->%s =(%s*)allocate(%s*sizeof(%s));";
        Strings allocFmtArgs = {arrName, elementType, arrSize, elementType};
        body << fmt(allocFmt, allocFmtArgs);
    }
    return body;
}

Strings ComplexTypeDescription::SerSimpleField(const StructField &field)
{
    auto metaType = field.type.second;
    auto hasInitValue = field.data.hasInitValue;
    auto isBF = IsSimpleBitField(field);
    auto copyName = UsedCopyMethod(field);
    const String& type = field.type.first;
    const String& fieldName = field.name;
    const String& fieldSize = field.bitSize->ToString();

    Strings body;

    if(hasInitValue) {
        String value = to_string(field.data.value);
        body << fmt("str->%s = %s;", {fieldName, value});
    }

    if(metaType != fieldType::Struct)
    {
        if(isBF){body << fmt("%s %s = str->%s;", {type, fieldName, fieldName});}
        string var = sc(isBF, {"%s", "str->%s"}, fieldName);
        string copyFmt = "pos += %s(p, pos, &%s, 0, %s);";
        body << fmt(copyFmt, {copyName, var, fieldSize});
    } else if(metaType == fieldType::Struct) {
        String type = field.type.first;
        type = fmt("%s%s_%s",{_pSer, FieldTypePrefix(metaType), type});
        body << fmt("pos += %s(p, pos, &str->%s);", {type, fieldName});
    }
    return body;
}

Strings ComplexTypeDescription::DesSimpleField(const StructField &field)
{
    auto metaType = field.type.second;
    String type = field.type.first;
    auto typePrefix = FieldTypePrefix(metaType);
    auto copyName = UsedCopyMethod(field);
    const String& fieldName = field.name;
    const String& fieldSize = field.bitSize->ToString();
    Strings body;

    if(metaType != fieldType::Struct)
    {
        if(IsSimpleBitField(field))
        {
            body << fmt("%s %s = 0;", {type, fieldName});
            string copyFmt = "pos += %s(&%s, 0, p, pos, %s);";
            body << fmt(copyFmt, {copyName, fieldName, fieldSize});
            body << fmt("str->%s = %s;", {fieldName, fieldName});
        } else {
            string copyFmt = "pos += %s(&str->%s, 0, p, pos, %s);";
            body << fmt(copyFmt, {copyName, fieldName, fieldSize});
        }
    } else if(metaType == fieldType::Struct) {
        type = fmt("%s%s_%s",{_pDes, typePrefix, type});
        body << fmt("pos += %s(p, pos, &str->%s, op_status);", {type, fieldName});
    }

    return body;
}

Strings ComplexTypeDescription::DesCheckInitValue(const StructField &field)
{
    IfElseStatementCpp statement;

    String value = to_string(field.data.value);
    auto condition = fmt("str->%s != %s", {field.name, value});
    statement.AddCase(condition, "*op_status = 0; return pos - offset;");

    return statement.GetDefinition();
}

Strings ComplexTypeDescription::DesCheckValueRange(const StructField &field)
{
    IfElseStatementCpp statement;
    auto st1 = fmt("str->%s < %s", {field.name, to_string(field.data.min)});
    auto st2 = fmt("str->%s > %s", {field.name, to_string(field.data.max)});

    if(field.data.min == 0) {
        statement.AddCase(st2, "*op_status = 0; return pos - offset;");
    }
    else {
        String condition = fmt("%s || %s", {st1, st2});
        statement.AddCase(condition, "*op_status = 0; return pos - offset;");
    }

    return statement.GetDefinition();
}

String ComplexTypeDescription::UsedCopyMethod(const StructField &field)
{
    return ShouldUseAlignedCopy(field) ? "bitcpya" : "bitcpy";
}

Function ComplexTypeDescription::CompareFun()
{
    Function fun;
    vector<Parameter> parameters;

    auto structName = BlType() + '_' + name;
    parameters.push_back({"const " + structName + "&", "obj1"});
    parameters.push_back({"const " + structName + "&", "obj2"});

    fun.SetDeclaration("operator==", "bool", parameters);

    vector<string> body;

    IfElseStatementCpp statement;
    for(auto field : _fields)
    {
        auto cond = fmt("(obj1.%s == obj2.%s) == false", {field.name,
                                                          field.name});
        statement.AddCase(cond, "return false;");
    }

    body << statement.GetDefinition();
    body << "return true;";

    fun.SetBody(body);

    return fun;
}

void ComplexTypeDescription::SetAlignedCopyPreferred(bool state)
{
    _isAlignedCopyPreferred = state;
}

bool ComplexTypeDescription::ShouldUseAlignedCopy(const StructField &field) const
{
    if(!_isAlignedCopyPreferred || !field.bitSize->IsMultipleOf(8)) { return false; }

    SizeExprPtr size = 0_lit;

    if(_contextOffsets.size() == 1)
    {
        size = _contextOffsets[0];
    }
    else
    {
        for(const auto& offset : _contextOffsets)
        {
            if(!offset->IsMultipleOf(8)) {return false;}
        }
    }

    for(const auto& currentField : _fields)
    {
        if(currentField.name == field.name) { break; }
        size += currentField.bitSize;
    }

    return size->IsMultipleOf(8);
}

bool ComplexTypeDescription::IsSimpleBitField(const StructField &field)
{
    return field.type.second == fieldType::std &&
            !field.data.isArrayField &&
            !field.bitSize->IsMultipleOf(8);
}

bool ComplexTypeDescription::IsArrayBitField(const StructField &field)
{
    return field.type.second == fieldType::std &&
            field.data.isArrayField &&
            !field.arrayElementSize->IsMultipleOf(8);
}

bool ComplexTypeDescription::IsArrayAlignedField(const StructField &field)
{
    return field.type.second == fieldType::std &&
            field.data.isArrayField &&
            field.arrayElementSize->IsMultipleOf(8);
}
