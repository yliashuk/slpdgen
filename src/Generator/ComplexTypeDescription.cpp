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
    this->_name = name;
}

string ComplexTypeDescription::GetName() const
{
    return _name;
}

string ComplexTypeDescription::GetCodeName() const
{
    return BlType() + "_" + _name;
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

void ComplexTypeDescription::addField(StructFieldInfo info,
                                      pair<string, fieldType> type,
                                      SizeExprPtr fieldSize)
{
    ComplexField field;
    field.info = info.second;
    field.name = info.first;
    field.type.first = type.first;
    field.type.second = type.second;

    if(info.second.IsArray()) {
        if(info.second.HasDynamicSize()) {
            field.arrayElementCount = Variable::Create("str->" + *info.second.sizeVar);
        } else {
            field.arrayElementCount = Literal::Create(*info.second.constantSize);
        }
        field.arrayElementSize = fieldSize;
        field.bitSize = field.arrayElementSize * field.arrayElementCount;
    }
    else { field.bitSize = fieldSize; }

    _fields.push_back(field);
}

vector<ComplexField> ComplexTypeDescription::GetFields() const
{
    return _fields;
}

vector<string> ComplexTypeDescription::Declaration()
{
    auto blockName = fmt("%s_%s", {BlType(), _name});
    StructCpp structCpp;
    structCpp.SetName(blockName);
    structCpp.SetTypeDef(!_options.isCpp);

    StructCpp::Fields fields;
    for(auto field : _fields)
    {
        string sizeVar = field.info.sizeVar.value_or("");
        string fieldName = field.name;
        string type = field.type.first;

        bool isArray = field.info.IsArray();
        bool isDynamicArray = isArray && field.info.HasDynamicSize();
        bool isStaticArray = isArray && !isDynamicArray;
        bool isArrayVarLen = IsArrayVarLen(fieldName);


        bool isNotUsedVarLen = _options.isCpp && isArrayVarLen;
        bool isPointerToArr = !_options.isCpp && isDynamicArray;

        string comment = {};
        if(isNotUsedVarLen || isPointerToArr)
        {
            string helpComm = fmt("%s[%s]", {fieldName, sizeVar});
            string notUsedComm = "not used";
            comment = isNotUsedVarLen ? notUsedComm : helpComm;
        }

        string typePrefix = FieldTypePrefix(field.type.second);
        bool isStd = field.type.second == fieldType::std;

        string fieldT = isStd ? type : typePrefix + "_" + type;
        string len = to_string(*field.info.constantSize);

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
            string bitWidth =  IsSimpleBitField(field) ? field.bitSize->ToString() : "";
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
        bool isArray = var->info.IsArray();

        if(funType == Ser)
        {
            body << (isArray ? SerArrayField(*var) : SerSimpleField(*var));
        }
        else if(funType == Des)
        {
            body << (isArray ? DesArrayField(*var) : DesSimpleField(*var));

            if(var->info.initValue)
            {
                body << DesCheckInitValue(*var);
            }
            if(var->info.fromVal && var->info.toVal)
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
    return fmt("%s%s_%s(%s, %s, %s)", {_pSer, BlType(), _name, pointerName, offset, paramName});
}

string ComplexTypeDescription::DesCall(string pointerName, string offset,
                                       string paramName, string op_status) const
{
    return fmt("%s%s_%s(%s, %s, %s, %s)",
    {_pDes, BlType(), _name, pointerName, offset, paramName, op_status});
}

string ComplexTypeDescription::AsVarDecl() const
{
    return fmt("%s_%s header", {BlType(), _name});
}

vector<string> ComplexTypeDescription::SizeCalcFun(FunType type, bool hasStatic)
{
    auto fun = CalcSizeHelper::CalcSizeFunDecl(BlType() + "_" + _name, type, hasStatic);

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
        string structName = BlType() + '_' + _name;
        body << "(void)p;";

        string initVal = fmt("{sizeof(%s), 0, 0}", {structName});
        body << fmt("c_size_t c_size = %s;", {initVal});
    }

    for(auto field : _fields)
    {
        string prefix = FieldTypePrefix(field.type.second) + '_';
        fieldType fldType = field.type.second;
        bool isArray  = field.info.IsArray();
        bool isDynamicArray = field.info.HasDynamicSize();

        if(fldType != fieldType::Struct)
        {
            if(type == Des && IsArrayVarLen(field.name))
            {
                string name = field.name;
                string size = field.bitSize->ToString();
                string fType = field.type.first;
                string copyName = UsedCopyMethod(field);
                body << fmt("%s %s = 0;", {fType, name});
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
    auto funName = CalcSizeHelper::CalcSizeFunName(BlType() + "_" + _name, type);
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
        if (field.info.sizeVar == name) { return true; }
    }
    return false;
}

std::vector<std::pair<String, String>> ComplexTypeDescription::ArrNameSizeVarPairs()
{
    std::vector<std::pair<String, String>> pairs;
    for(const auto& field : _fields) {
        if (field.info.sizeVar) {
            pairs.push_back({field.name, *field.info.sizeVar});
        }
    }
    return pairs;
}

void ComplexTypeDescription::SetSerDesDeclaration(Function &function, FunType type)
{
    auto blockName =  BlType() + '_' + _name;

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

Strings ComplexTypeDescription::SerArrayField(const ComplexField& field)
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

Strings ComplexTypeDescription::DesArrayField(const ComplexField &field)
{
    const String& arrSize = field.arrayElementCount->ToString();
    const String& arrName = field.name;
    const String& elementSize = field.arrayElementSize->ToString();
    auto type = field.type.first;
    auto metaType = field.type.second;
    auto copyName = UsedCopyMethod(field);

    bool isDynamicArray = field.info.HasDynamicSize();

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

Strings ComplexTypeDescription::ArrayFieldAllocation(const ComplexField &field)
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

Strings ComplexTypeDescription::SerSimpleField(const ComplexField &field)
{
    auto metaType = field.type.second;
    auto isBF = IsSimpleBitField(field);
    auto copyName = UsedCopyMethod(field);
    auto type = field.type.first;
    const String& fieldName = field.name;
    const String& fieldSize = field.bitSize->ToString();

    Strings body;

    if(field.info.initValue) {
        String value = to_string(*field.info.initValue);
        body << fmt("str->%s = %s;", {fieldName, value});
    }

    if(metaType != fieldType::Struct)
    {
        if(isBF){body << fmt("%s %s = str->%s;", {type, fieldName, fieldName});}
        string var = sc(isBF, {"%s", "str->%s"}, fieldName);
        string copyFmt = "pos += %s(p, pos, &%s, 0, %s);";
        body << fmt(copyFmt, {copyName, var, fieldSize});
    } else if(metaType == fieldType::Struct) {
        type = fmt("%s%s_%s",{_pSer, FieldTypePrefix(metaType), type});
        body << fmt("pos += %s(p, pos, &str->%s);", {type, fieldName});
    }
    return body;
}

Strings ComplexTypeDescription::DesSimpleField(const ComplexField &field)
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

Strings ComplexTypeDescription::DesCheckInitValue(const ComplexField &field)
{
    IfElseStatementCpp statement;

    String value = to_string(*field.info.initValue);
    auto condition = fmt("str->%s != %s", {field.name, value});
    statement.AddCase(condition, "*op_status = 0; return pos - offset;");

    return statement.GetDefinition();
}

Strings ComplexTypeDescription::DesCheckValueRange(const ComplexField &field)
{
    IfElseStatementCpp statement;
    auto st1 = fmt("str->%s < %s", {field.name, to_string(*field.info.fromVal)});
    auto st2 = fmt("str->%s > %s", {field.name, to_string(*field.info.toVal)});

    if(*field.info.fromVal == 0) {
        statement.AddCase(st2, "*op_status = 0; return pos - offset;");
    } else {
        String condition = fmt("%s || %s", {st1, st2});
        statement.AddCase(condition, "*op_status = 0; return pos - offset;");
    }

    return statement.GetDefinition();
}

String ComplexTypeDescription::UsedCopyMethod(const ComplexField &field)
{
    return ShouldUseAlignedCopy(field) ? "bitcpya" : "bitcpy";
}

Function ComplexTypeDescription::CompareFun()
{
    Function fun;
    vector<Parameter> parameters;

    auto structName = BlType() + '_' + _name;
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

bool ComplexTypeDescription::ShouldUseAlignedCopy(const ComplexField &field) const
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

bool ComplexTypeDescription::IsSimpleBitField(const ComplexField &field)
{
    return field.type.second == fieldType::std &&
            !field.info.IsArray() &&
            !field.bitSize->IsMultipleOf(8);
}

bool ComplexTypeDescription::IsArrayBitField(const ComplexField &field)
{
    return field.type.second == fieldType::std &&
            field.info.IsArray() &&
            !field.arrayElementSize->IsMultipleOf(8);
}

bool ComplexTypeDescription::IsArrayAlignedField(const ComplexField &field)
{
    return field.type.second == fieldType::std &&
            field.info.IsArray() &&
            field.arrayElementSize->IsMultipleOf(8);
}
