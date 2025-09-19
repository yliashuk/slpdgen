#include "ComplexTypeDescription.h"
#include <Utils/StringUtils.h>

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

Polynomial ComplexTypeDescription::Size() const
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
        VarArray arr;
        arr.varArrayType = data.first;
        arr.varArraySize = fieldSize;
        string str;

        if(!FieldTypePrefix(type.second).empty())
            str = FieldTypePrefix(type.second) + '_';

        arr.varArrayTypeName = str + type.first;
        arr.varArrayLen = data.second.lenDefiningVar;
        _varArrays.push_back(arr);
    }
}

vector<StructField> ComplexTypeDescription::GetFields() const
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
    for(auto var:_fields)
    {
        string dynSizeVar = var.data.lenDefiningVar;
        string fieldName = var.fieldName;
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
    Strings body;
    body << "size_t size = offset;";

    if(funType == Des)
    {
        IfElseStatementCpp statement;
        statement.AddCase("*op_status == 0", "return size;");
        body << statement.GetDefinition();
    }

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
    body << "return size - offset;";

    Function funConstruct;
    SetSerDesDeclaration(funConstruct, funType);
    funConstruct.SetStaticDeclaration(hasStatic);
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
            for(const auto& var : _varArrays)
            {
                body << fmt("str->%s = str->%s.size();", {var.varArrayLen,
                                                          var.varArrayType});
            }
            if(_varArrays.size()){ body.push_back(""); }
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
        bool isArray  = field.data.isArrayField;
        bool isDynamicArray = field.data.hasDynamicSize;

        if(fldType != fieldType::Struct)
        {
            if(type == Des && IsArrayVarLen(field.fieldName))
            {
                string name = field.fieldName;
                string size = field.fieldSize.Get();
                string type = field.type.first;
                body << fmt("%s %s = 0;", {type, name});
                body << fmt("bitcpy(&%s, 0, p, c_size.r, %s);", {name, size});
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

bool ComplexTypeDescription::IsArrayVarLen(string name)
{
    for(const auto& lenVar : _varArrays)
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
    auto blockName =  BlType() + '_' + _name;

    vector<Parameter> params;

    params.push_back({"char*", "p"});
    params.push_back({"size_t", "offset"});
    params.push_back({blockName, "*str"});

    if(type == Des)
    {
        if(_blockType == ComplexType::Header && !_options.isCpp) {
            string fName = _options.fileName;
            params.insert(params.begin(), {fmt("%s_Obj*", {toLower(fName)}), "obj"});
        }
        params.push_back({"uint8_t*", "op_status"});
    }
    String serDesPrefix = type == Ser ? _pSer : _pDes;
    function.SetDeclaration(serDesPrefix + blockName,"size_t", params);
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
        string copyFmt = "size += bitcpy(p, size, &str->%s[i], 0, %s);";

        loopBody << fmt(copyFmt, {fieldName, elementSize});

    } else if(metaType == fieldType::Struct)
    {
        String type = field.type.first;
        type = fmt("%s%s_%s",{_pSer, FieldTypePrefix(metaType), type});
        loopBody << fmt("size += %s(p, size, &str->%s[i]);", {type, fieldName});
    }

    ForLoopCpp loop;
    loop.SetDeclaration("int i = 0","i < " + fieldSize, "i++");
    loop.SetBody(loopBody);

    return loop.Definition();
}

Strings ComplexTypeDescription::DesArrayField(const StructField &field)
{
    const String& fieldSize = field.fieldSize.Get();
    const String& fieldName = field.fieldName;
    const String& elementSize = field.arrayTypeSize.Get();
    auto metaType = field.type.second;

    bool isDynamicArray = field.data.hasDynamicSize;

    Strings body, loopBody;

    if(isDynamicArray) {
        body << ArrayFieldAllocation(field);
    }

    {
        if(metaType != fieldType::Struct)
        {
            string copyFmt = "size += bitcpy(&str->%s[i], 0, p, size, %s);";
            loopBody << fmt(copyFmt, {fieldName, elementSize});
        }
        else if(metaType == fieldType::Struct)
        {
            String type = field.type.first;
            type = fmt("%s%s_%s",{_pDes, FieldTypePrefix(metaType), type});
            loopBody << fmt("size += %s(p, size, &str->%s[i], op_status);",
                            {type, fieldName});
        }

        ForLoopCpp loop;
        loop.SetDeclaration("int i = 0","i < " + fieldSize, "i++");
        loop.SetBody(loopBody);
        body << loop.Definition();
    }

    return body;
}

Strings ComplexTypeDescription::ArrayFieldAllocation(const StructField &field)
{
    auto metaType = field.type.second;
    const String& fieldName = field.fieldName;
    const String& fieldSize = field.fieldSize.Get();

    Strings body;

    if(_options.isCpp) {
        body << fmt("str->%s.resize(%s);", {fieldName, fieldSize});
    }
    else
    {
        String typePrefix = FieldTypePrefix(metaType);
        String elementType = fmt("%{%s_}%s",{typePrefix, field.type.first});

        String allocFmt = "str->%s =(%s*)allocate(%s*sizeof(%s));";
        Strings allocFmtArgs = {fieldName, elementType, fieldSize, elementType};
        body << fmt(allocFmt, allocFmtArgs);
    }
    return body;
}

Strings ComplexTypeDescription::SerSimpleField(const StructField &field)
{
    auto metaType = field.type.second;
    auto hasInitValue = field.data.hasInitValue;
    const String& fieldName = field.fieldName;
    const String& fieldSize = field.fieldSize.Get();

    Strings body;

    if(hasInitValue) {
        String value = to_string(field.data.value);
        body << fmt("str->%s = %s;", {fieldName, value});
    }

    if(field.type.second != fieldType::Struct) {
        string copyFmt = "size += bitcpy(p, size, &str->%s, 0, %s);";
        body << fmt(copyFmt, {fieldName, fieldSize});
    } else if(field.type.second == fieldType::Struct) {
        String type = field.type.first;
        type = fmt("%s%s_%s",{_pSer, FieldTypePrefix(metaType), type});
        body << fmt("size += %s(p, size, &str->%s);", {type, fieldName});
    }

    return body;
}

Strings ComplexTypeDescription::DesSimpleField(const StructField &field)
{
    auto metaType = field.type.second;
    String type = field.type.first;
    auto typePrefix = FieldTypePrefix(metaType);
    const String& fieldName = field.fieldName;
    const String& fieldSize = field.fieldSize.Get();
    Strings body;

    if(field.type.second != fieldType::Struct) {
        string copyFmt = "size += bitcpy(&str->%s, 0, p, size, %s);";
        body << fmt(copyFmt, {fieldName, fieldSize});
    } else if(field.type.second == fieldType::Struct) {
        type = fmt("%s%s_%s",{_pDes, typePrefix, type});
        body << fmt("size += %s(p, size, &str->%s, op_status);", {type, fieldName});
    }

    return body;
}

Strings ComplexTypeDescription::DesCheckInitValue(const StructField &field)
{
    IfElseStatementCpp statement;

    String value = to_string(field.data.value);
    auto condition = fmt("str->%s != %s", {field.fieldName, value});
    statement.AddCase(condition, "*op_status = 0; return size;");

    return statement.GetDefinition();
}

Strings ComplexTypeDescription::DesCheckValueRange(const StructField &field)
{
    IfElseStatementCpp statement;
    auto st1 = fmt("str->%s < %s", {field.fieldName, to_string(field.data.min)});
    auto st2 = fmt("str->%s > %s", {field.fieldName, to_string(field.data.max)});

    if(field.data.min == 0) {
        statement.AddCase(st2, "*op_status = 0; return size;");
    }
    else {
        String condition = fmt("%s || %s", {st1, st2});
        statement.AddCase(condition, "*op_status = 0; return size;");
    }

    return statement.GetDefinition();
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
        if(IsBitField(field)){ field.fieldName += ".value"; }
        auto cond = fmt("(obj1.%s == obj2.%s) == false", {field.fieldName,
                                                          field.fieldName});
        statement.AddCase(cond, "return false;");
    }

    body << statement.GetDefinition();
    body << "return true;";

    fun.SetBody(body);

    return fun;
}

bool ComplexTypeDescription::IsBitField(const StructField &field)
{
    return field.type.second == fieldType::std &&
           ((field.fieldSize.GetConstPart() % 8) > 0);
}
