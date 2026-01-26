#include "ComplexTypeDescription.h"
#include <Utils/StringUtils.h>
#include "StdTypeHandler.h"

ComplexTypeDescription::ComplexTypeDescription(){}

void ComplexTypeDescription::setOption(AppOptions options)
{
    _options = options;
}

void ComplexTypeDescription::setBlockType(ComplexType type)
{
    this->_blockType = type;
}

void ComplexTypeDescription::setName(string name)
{
    this->_name = name;
}

string ComplexTypeDescription::getName() const
{
    return _name;
}

string ComplexTypeDescription::getCodeName() const
{
    return blType() + "_" + _name;
}

SizeExprPtr ComplexTypeDescription::size() const
{   SizeExprPtr size = 0_lit;
    for(auto var: _fields) {
        size += var.bitSize;
    }
    return size;
}

void ComplexTypeDescription::addContextOffset(SizeExprPtr offset)
{
    _contextOffsets.push_back(offset);
}

void ComplexTypeDescription::addField(StructFieldInfo info,
                                      pair<string, FieldType> type,
                                      SizeExprPtr fieldSize)
{
    ComplexField field;
    field.info = info.second;
    field.name = info.first;
    field.type.first = type.first;
    field.type.second = type.second;

    if(info.second.isArray()) {
        if(info.second.hasDynamicSize()) {
            field.arrayElementCount = Variable::create("str->" + *info.second.sizeVar);
        } else {
            field.arrayElementCount = Literal::create(*info.second.constantSize);
        }
        field.arrayElementSize = fieldSize;
        field.bitSize = field.arrayElementSize * field.arrayElementCount;
    }
    else { field.bitSize = fieldSize; }

    _fields.push_back(field);
}

vector<ComplexField> ComplexTypeDescription::fields() const
{
    return _fields;
}

vector<string> ComplexTypeDescription::declaration()
{
    auto blockName = fmt("%s_%s", {blType(), _name});
    StructCpp structCpp;
    structCpp.setName(blockName);
    structCpp.setTypeDef(!_options.isCpp);

    StructCpp::Fields fields;
    for(auto field : _fields)
    {
        string sizeVar = field.info.sizeVar.value_or("");
        string fieldName = field.name;
        string type = field.type.first;

        bool isArray = field.info.isArray();
        bool isDynamicArray = isArray && field.info.hasDynamicSize();
        bool isStaticArray = isArray && !isDynamicArray;
        bool isArrayVarLen = this->isArrayVarLen(fieldName);


        bool isNotUsedVarLen = _options.isCpp && isArrayVarLen;
        bool isPointerToArr = !_options.isCpp && isDynamicArray;

        string comment = {};
        if(isNotUsedVarLen || isPointerToArr)
        {
            string helpComm = fmt("%s[%s]", {fieldName, sizeVar});
            string notUsedComm = "not used";
            comment = isNotUsedVarLen ? notUsedComm : helpComm;
        }

        string typePrefix = fieldTypePrefix(field.type.second);
        bool isStd = field.type.second == FieldType::std;

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
            string bitWidth =  isSimpleBitField(field) ? field.bitSize->toString() : "";
            string p = isDynamicArray ? "*" : "";
            string var = isStaticArray ? fmt("%s[%s]", {fieldName, len}) : fieldName;
            fields += {fieldT, p + var, bitWidth, comment};
        }
    }

    structCpp.addFields(fields);
    return structCpp.declaration();
}

vector<string> ComplexTypeDescription::serdesDefinition(FunType funType, bool hasStatic)
{
    Strings body;
    body << "size_t pos = offset;";

    for(auto var = _fields.begin(); var !=_fields.end(); var++)
    {
        bool isArray = var->info.isArray();

        if(funType == Ser)
        {
            body << (isArray ? serArrayField(*var) : serSimpleField(*var));
        }
        else if(funType == Des)
        {
            body << (isArray ? desArrayField(*var) : desSimpleField(*var));

            if(var->info.initValue)
            {
                body << desCheckInitValue(*var);
            }
            if(var->info.fromVal && var->info.toVal)
            {
                body << desCheckValueRange(*var);
            }
        }
    }

    if(funType == Des) { body << "*op_status = 1;"; }
    body << "return pos - offset;";

    Function funConstruct;
    setSerdesDeclaration(funConstruct, funType);
    funConstruct.setStatic(hasStatic);
    funConstruct.setBody(body);

    return funConstruct.definition();
}

string ComplexTypeDescription::serCall(string pointerName, string offset, string paramName) const
{
    return fmt("%s%s_%s(%s, %s, %s)", {_pSer, blType(), _name, pointerName, offset, paramName});
}

string ComplexTypeDescription::desCall(string pointerName, string offset,
                                       string paramName, string op_status) const
{
    return fmt("%s%s_%s(%s, %s, %s, %s)",
    {_pDes, blType(), _name, pointerName, offset, paramName, op_status});
}

string ComplexTypeDescription::asVarDecl() const
{
    return fmt("%s_%s header", {blType(), _name});
}

vector<string> ComplexTypeDescription::sizeCalcFun(FunType type, bool hasStatic)
{
    auto fun = CalcSizeHelper::calcSizeFunDecl(blType() + "_" + _name, type, hasStatic);

    vector<string> body;

    if(type == Ser)
    {
        // for suppress unused warning
        body << "(void)str;";

        if(_options.isCpp)
        {
            for(auto field : fields())
            {
                if(field.info.hasDynamicSize())
                {
                    body << fmt("str->%s = str->%s.size();",
                    {*field.info.sizeVar, field.name});
                }
            }
        }

        body << "size_t size = 0;";
    }
    else
    {
        string structName = blType() + '_' + _name;
        body << "(void)p;";
        body << "(void)offset;";

        string initVal = fmt("{sizeof(%s) * 8, 0, 0}", {structName});
        body << fmt("c_size_t c_size = %s;", {initVal});
    }

    for(auto field : _fields)
    {
        string prefix = fieldTypePrefix(field.type.second) + '_';
        FieldType fldType = field.type.second;
        bool isArray  = field.info.isArray();
        bool isDynamicArray = field.info.hasDynamicSize();

        if(fldType != FieldType::Struct)
        {
            if(type == Des && isArrayVarLen(field.name))
            {
                string name = field.name;
                string size = field.bitSize->toString();
                string fType = field.type.first;
                string copyName = usedCopyMethod(field);
                body << fmt("%s %s = 0;", {fType, name});
                body << fmt("%s(&%s, 0, p, offset + c_size.r, %s);",
                {copyName, name, size});
            }
            if(type == Des && isDynamicArray)
            {
                body << CalcSizeHelper::calcDesSimpleArrTypeSize(prefix, field);
            }
            body << CalcSizeHelper::calcSimpeTypeSize(field, type);
        }
        else if(isArray && fldType == FieldType::Struct)
        {
            body << CalcSizeHelper::calcSizeLoop(prefix, field, type);
        }
        else if(fldType == FieldType::Struct)
        {
            body << CalcSizeHelper::calcStructSize(prefix, field, type);
        }
    }

    string returnVarName = type == Ser ? "size" : "c_size";
    body << fmt("return %s;", {returnVarName});

    fun.setBody(body);
    return fun.definition();
}

string ComplexTypeDescription::sizeCalcFunCall(FunType type) const
{
    auto funName = CalcSizeHelper::calcSizeFunName(blType() + "_" + _name, type);
    auto vars = type == Ser ? "str" : "l_p, offset";
    return funName + "(" + vars + ")";
}

string ComplexTypeDescription::blType() const
{
    if(_blockType == ComplexType::Message) return  _prefix + "message";
    if(_blockType == ComplexType::Header) return  _prefix + "HEADER";
    else return  _prefix + "struct";
}

void ComplexTypeDescription::setPrefix(string prefix)
{
    _prefix = prefix + '_';
}

string ComplexTypeDescription::fieldTypePrefix(FieldType type)
{
    switch (type)
    {
    case FieldType::Enum: return _prefix + "enum";
    case FieldType::Struct: return _prefix + "struct";
    case FieldType::Code: return _prefix + "CODE";
    case FieldType::Type: return _prefix + "TYPE";
    default: return "";
    }
}

bool ComplexTypeDescription::isArrayVarLen(string name) const
{
    for(const auto& field : _fields) {
        if (field.info.sizeVar == name) { return true; }
    }
    return false;
}

void ComplexTypeDescription::setSerdesDeclaration(Function &function, FunType type)
{
    auto blockName =  blType() + '_' + _name;

    vector<Parameter> params;

    params.push_back({"char*", "p"});
    params.push_back({"size_t", "offset"});
    params.push_back({blockName, "*str"});

    if(type == Des) {
        params.push_back({"uint8_t*", "op_status"});
    }
    String serDesPrefix = type == Ser ? _pSer : _pDes;
    function.setDeclaration(serDesPrefix + blockName,"size_t", params);
}

Strings ComplexTypeDescription::serArrayField(const ComplexField& field)
{
    const String& arrSize = field.arrayElementCount->toString();
    const String& arrName = field.name;
    const String& elementSize = field.arrayElementSize->toString();
    auto type = field.type.first;
    auto metaType = field.type.second;
    auto copyName = usedCopyMethod(field);
    Strings body;
    Strings loopBody;

    if(metaType != FieldType::Struct)
    {
        if(isArrayBitField(field) || isCppBoolDynamicArray(field))
        {
            string varType = StdTypeHandler::slpdToAlignedCppType(type);
            body << fmt("%s %s_value = 0;", {varType, arrName});

            string valueField = isCppBoolDynamicArray(field) ? "" : ".value";
            loopBody << fmt("%s_value = str->%s[i]%{%s};", {arrName, arrName, valueField});
            string copyFmt = "pos += %s(p, pos, &%s_value, 0, %s);";
            loopBody << fmt(copyFmt, {copyName, arrName, elementSize});
        } else if(isArrayAlignedField(field)) {
            string copyFmt = "pos += %s(p, pos, &str->%s[0], 0, %s * %s);";
            body << fmt(copyFmt, {copyName, arrName, elementSize, arrSize});
        } else {
            string copyFmt = "pos += %s(p, pos, &str->%s[i], 0, %s);";
            loopBody << fmt(copyFmt, {copyName, arrName, elementSize});
        }
    } else if(metaType == FieldType::Struct)
    {
        type = fmt("%s%s_%s",{_pSer, fieldTypePrefix(metaType), type});
        loopBody << fmt("pos += %s(p, pos, &str->%s[i]);", {type, arrName});
    }

    if(!loopBody.empty())
    {
        ForLoopCpp loop;
        loop.setDeclaration("int i = 0","i < " + arrSize, "i++");
        loop.setBody(loopBody);
        body << loop.definition();
    }

    return body;
}

Strings ComplexTypeDescription::desArrayField(const ComplexField &field)
{
    const String& arrSize = field.arrayElementCount->toString();
    const String& arrName = field.name;
    const String& elementSize = field.arrayElementSize->toString();
    auto type = field.type.first;
    auto metaType = field.type.second;
    auto copyName = usedCopyMethod(field);

    bool isDynamicArray = field.info.hasDynamicSize();

    Strings body, loopBody;

    if(isDynamicArray) {
        body << arrayFieldAllocation(field);
    }

    if(metaType != FieldType::Struct)
    {
        if(isArrayBitField(field) || isCppBoolDynamicArray(field))
        {
            string varType = StdTypeHandler::slpdToAlignedCppType(type);
            body << fmt("%s %s_value = 0;", {varType, arrName});

            string copyFmt = "pos += %s(&%s_value, 0, p, pos, %s);";
            loopBody << fmt(copyFmt, {copyName, arrName, elementSize});

            string valueField = isCppBoolDynamicArray(field) ? "" : ".value";
            loopBody << fmt("str->%s[i]%{%s} = %s_value;", {arrName, valueField, arrName});
        } else if(isArrayAlignedField(field)) {
            string copyFmt = "pos += %s(&str->%s[0], 0, p, pos, %s * %s);";
            body << fmt(copyFmt, {copyName, arrName, elementSize, arrSize});
        } else {
            string copyFmt = "pos += %s(&str->%s[i], 0, p, pos, %s);";
            loopBody << fmt(copyFmt, {copyName, arrName, elementSize});
        }
    }
    else if(metaType == FieldType::Struct)
    {
        type = fmt("%s%s_%s",{_pDes, fieldTypePrefix(metaType), type});
        loopBody << fmt("pos += %s(p, pos, &str->%s[i], op_status);", {type, arrName});
        loopBody << checkOpStatus();
    }

    if(!loopBody.empty())
    {
        ForLoopCpp loop;
        loop.setDeclaration("int i = 0","i < " + arrSize, "i++");
        loop.setBody(loopBody);
        body << loop.definition();
    }

    return body;
}

Strings ComplexTypeDescription::arrayFieldAllocation(const ComplexField &field)
{
    auto metaType = field.type.second;
    const String& arrName = field.name;
    const String& arrSize = field.arrayElementCount->toString();

    Strings body;

    if(_options.isCpp) {
        body << fmt("str->%s.resize(%s);", {arrName, arrSize});
    }
    else
    {
        String typePrefix = fieldTypePrefix(metaType);
        String elementType = fmt("%{%s_}%s",{typePrefix, field.type.first});

        String allocFmt = "str->%s =(%s*)allocate(%s*sizeof(%s));";
        Strings allocFmtArgs = {arrName, elementType, arrSize, elementType};
        body << fmt(allocFmt, allocFmtArgs);
    }
    return body;
}

Strings ComplexTypeDescription::serSimpleField(const ComplexField &field)
{
    auto metaType = field.type.second;
    auto isBF = isSimpleBitField(field);
    auto copyName = usedCopyMethod(field);
    auto type = field.type.first;
    const String& fieldName = field.name;
    const String& fieldSize = field.bitSize->toString();

    Strings body;

    if(field.info.initValue) {
        String value = to_string(*field.info.initValue);
        body << fmt("str->%s = %s;", {fieldName, value});
    }

    if(metaType != FieldType::Struct)
    {
        if(isBF){body << fmt("%s %s = str->%s;", {type, fieldName, fieldName});}
        string var = sc(isBF, {"%s", "str->%s"}, fieldName);
        string copyFmt = "pos += %s(p, pos, &%s, 0, %s);";
        body << fmt(copyFmt, {copyName, var, fieldSize});
    } else if(metaType == FieldType::Struct) {
        type = fmt("%s%s_%s",{_pSer, fieldTypePrefix(metaType), type});
        body << fmt("pos += %s(p, pos, &str->%s);", {type, fieldName});
    }
    return body;
}

Strings ComplexTypeDescription::desSimpleField(const ComplexField &field)
{
    auto metaType = field.type.second;
    String type = field.type.first;
    auto typePrefix = fieldTypePrefix(metaType);
    auto copyName = usedCopyMethod(field);
    const String& fieldName = field.name;
    const String& fieldSize = field.bitSize->toString();
    Strings body;

    if(metaType != FieldType::Struct)
    {
        if(isSimpleBitField(field))
        {
            body << fmt("%s %s = 0;", {type, fieldName});
            string copyFmt = "pos += %s(&%s, 0, p, pos, %s);";
            body << fmt(copyFmt, {copyName, fieldName, fieldSize});
            body << fmt("str->%s = %s;", {fieldName, fieldName});
        } else {
            string copyFmt = "pos += %s(&str->%s, 0, p, pos, %s);";
            body << fmt(copyFmt, {copyName, fieldName, fieldSize});
        }
    } else if(metaType == FieldType::Struct) {
        type = fmt("%s%s_%s",{_pDes, typePrefix, type});
        body << fmt("pos += %s(p, pos, &str->%s, op_status);", {type, fieldName});
        body << checkOpStatus();
    }

    return body;
}

Strings ComplexTypeDescription::desCheckInitValue(const ComplexField &field)
{
    ConditionCpp statement;

    String value = to_string(*field.info.initValue);
    auto condition = fmt("str->%s != %s", {field.name, value});
    statement.addCase(condition, "*op_status = 0; return pos - offset;");

    return statement.definition();
}

Strings ComplexTypeDescription::desCheckValueRange(const ComplexField &field)
{
    ConditionCpp statement;
    auto st1 = fmt("str->%s < %s", {field.name, to_string(*field.info.fromVal)});
    auto st2 = fmt("str->%s > %s", {field.name, to_string(*field.info.toVal)});

    if(*field.info.fromVal == 0) {
        statement.addCase(st2, "*op_status = 0; return pos - offset;");
    } else {
        String condition = fmt("%s || %s", {st1, st2});
        statement.addCase(condition, "*op_status = 0; return pos - offset;");
    }

    return statement.definition();
}

String ComplexTypeDescription::usedCopyMethod(const ComplexField &field)
{
    return shouldUseAlignedCopy(field) ? "bitcpya" : "bitcpy";
}

Function ComplexTypeDescription::compareFun()
{
    Function fun;
    vector<Parameter> parameters;

    auto structName = blType() + '_' + _name;
    parameters.push_back({"const " + structName + "&", "obj1"});
    parameters.push_back({"const " + structName + "&", "obj2"});

    fun.setDeclaration("operator==", "bool", parameters);

    vector<string> body;

    ConditionCpp statement;
    for(auto field : _fields)
    {
        auto cond = fmt("(obj1.%s == obj2.%s) == false", {field.name,
                                                          field.name});
        statement.addCase(cond, "return false;");
    }

    body << statement.definition();
    body << "return true;";

    fun.setBody(body);

    return fun;
}

void ComplexTypeDescription::setAlignedCopyPreferred(bool state)
{
    _isAlignedCopyPreferred = state;
}

bool ComplexTypeDescription::shouldUseAlignedCopy(const ComplexField &field) const
{
    if(!_isAlignedCopyPreferred) { return false; }

    bool isAligned = field.info.isArray() ? field.arrayElementSize->isMultipleOf(8)
                                          : field.bitSize->isMultipleOf(8);

    if(!isAligned) { return false; }

    SizeExprPtr size = 0_lit;

    if(_contextOffsets.size() == 1)
    {
        size = _contextOffsets[0];
    }
    else
    {
        for(const auto& offset : _contextOffsets)
        {
            if(!offset->isMultipleOf(8)) {return false;}
        }
    }

    for(const auto& currentField : _fields)
    {
        if(currentField.name == field.name) { break; }
        size += currentField.bitSize;
    }

    return size->isMultipleOf(8);
}

bool ComplexTypeDescription::isCppBoolDynamicArray(const ComplexField &field)
{
    return field.type.first == "bool" &&
           field.info.hasDynamicSize() &&
           _options.isCpp;
}

Strings ComplexTypeDescription::checkOpStatus()
{
    return ConditionCpp()
            .addCase("*op_status == 0", "return pos - offset;")
            .definition();
}

bool ComplexTypeDescription::isSimpleBitField(const ComplexField &field)
{
    return field.type.second == FieldType::std &&
           !field.info.isArray() &&
           (!field.bitSize->isMultipleOf(8) ||
            isNonStandardSize(field.bitSize));
}

bool ComplexTypeDescription::isArrayBitField(const ComplexField &field)
{
    return field.type.second == FieldType::std &&
           field.info.isArray() &&
           isNonStandardSize(field.arrayElementSize);
}

bool ComplexTypeDescription::isArrayAlignedField(const ComplexField &field)
{
    return field.type.second == FieldType::std &&
           field.info.isArray() &&
           !isNonStandardSize(field.arrayElementSize);
}

bool ComplexTypeDescription::isNonStandardSize(const SizeExprPtr &bitSize)
{
    bool isNonStandard = {};
    bool isAligned = bitSize->isMultipleOf(8);

    if(isAligned)
    {
        isNonStandard = bitSize->isMultipleOf(24) ||
                        bitSize->isMultipleOf(40) ||
                        bitSize->isMultipleOf(48) ||
                        bitSize->isMultipleOf(56);
    }

    return !isAligned || isNonStandard;
}
