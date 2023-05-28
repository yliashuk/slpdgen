#include "ComplexTypeDescription.h"

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
    for(auto var: fields)
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
        if(data.second.withLenDefiningVar) {
            field.fieldSize = ("str->" + data.second.lenDefiningVar);
        } else {
            field.fieldSize =  data.second.value;
        }
        field.arrayTypeSize = fieldSize;
    }
    else field.fieldSize = fieldSize;

    field.fieldOffset = Size();
    fields.push_back(field);

    if(data.second.withLenDefiningVar)
    {
        this->hasDynamicFields = true;
        VarArray arr;
        arr.varArrayType = data.first;
        arr.varArraySize = fieldSize;
        string str;

        if(!FieldMetaType(type.second).empty())
            str = FieldMetaType(type.second) + b_und;

        arr.varArrayTypeName = str + type.first;
        arr.varArrayLen = data.second.lenDefiningVar;
        varArrays.push_back(arr);
    }
}

vector<string> ComplexTypeDescription::PrintDecl()
{
    vector<string> strings;
    auto BlockName =  BlType() + b_und + name;

    if(qtCppOption)
        strings.push_back("struct " + BlockName);
    else
        strings.push_back("typedef struct");

    strings.push_back(lb);
    for(auto var:fields)
    {
        string p, e, s, qtCppNotUsedComment;
        if(var.data.withLenDefiningVar) {
            p = "*";
            s = " //" + var.fieldName + PutInSqBraces(var.data.lenDefiningVar);
        }
        else if(var.data.isArrayField)
            e = PutInSqBraces(to_string(var.data.value));

        if(qtCppOption) {
            for(auto a:fields) {
                if(a.data.lenDefiningVar == var.fieldName)
                    qtCppNotUsedComment = " // not used";
            }
        }

        if(qtCppOption && var.data.withLenDefiningVar)
        {
            if(var.type.second == fieldType::std) {
                strings.push_back(tab + "std::vector<" + var.type.first +">" +
                                  spc + var.fieldName + smcln);
            }
            else
            {
                strings.push_back(tab + "std::vector<" +
                                  FieldMetaType(var.type.second) + b_und +
                                  var.type.first + ">"+ spc + var.fieldName + smcln);
            }
        }
        else if(qtCppOption && var.data.isArrayField)
        {
            auto customType = FieldMetaType(var.type.second) + b_und + var.type.first;
            auto arary_type = (var.type.second == fieldType::std) ?
                        var.type.first : customType;
            strings.push_back(tab + "std::array<" + arary_type + ", " +
                              to_string(var.data.value) + ">" + spc + var.fieldName +
                              smcln);
        }
        else if(var.type.second == fieldType::std) {
            strings.push_back(tab + var.type.first + spc + p + var.fieldName + e +
                              smcln + s + qtCppNotUsedComment);
        } else {
            strings.push_back(tab + FieldMetaType(var.type.second)+ b_und +
                              var.type.first + spc + p + var.fieldName + e + smcln + s +
                              qtCppNotUsedComment);
        }
    }

    if(!qtCppOption)
        strings.push_back(rb + BlockName + smcln);
    else
        strings.push_back(rb + smcln);

    strings.push_back(es);
    return strings;
}

vector<string> ComplexTypeDescription::PrintSerDesDeclaration(FunType type,
                                                              bool hasStatic)
{
    auto BlockName =  BlType() + b_und + name;
    vector<string> strings, funcBody;
    Function funConstruct;
    funConstruct.SetStaticDeclaration(true);

    string preStatic;
    if(hasStatic)
        preStatic = "static ";

    funcBody.push_back("uint32_t size = 0" + smcln);

    if(type == Ser)
    {
        vector<Parameter> params;
        params.push_back({"char*", "p"});
        params.push_back({BlockName, "*str"});
        funConstruct.SetDeclaration(_pSer + BlType() + b_und + name,preStatic + "uint32_t",
                                    params);
    }
    else
    {
        vector<Parameter> params;
        if(blockType == ComplexType::Header && !qtOption)
            params.push_back({cObjType + "*", "obj"});

        params.push_back({"char*", "p"});
        params.push_back({BlockName, "*str"});
        params.push_back({"uint8_t*", "op_status"});
        funConstruct.SetDeclaration(_pDes + BlType() + b_und + name,preStatic + "uint32_t",
                                    params);
    }

    if(type == Des)
    {
        IfElseStatementCpp statement;
        statement.AddCase("*op_status == 0", "return size" + smcln);
        auto var = statement.GetDefinition();
        funcBody.insert(funcBody.end(), var.begin(), var.end());
    }
    //string addOffS, addOffD;
    for(auto var = fields.begin(); var !=fields.end(); var++)
    {
        if(var->type.second != fieldType::Struct)
        {
            if(type == Ser)
            {
                if(var->data.isArrayField)
                {                    
                    ForLoopCpp loop;
                    vector<string> body;

                    loop.SetDeclaration("int i = 0","i < " + var->fieldSize.Get() /*+
                                        " / " + var->baseTypeSize.Get()*/ ,"i++");

                    body.push_back(PrintMemcpy(string("p + size"),
                                               string("&str->") + var->fieldName +
                                               PutInSqBraces("i"),
                                               var->arrayTypeSize.Get()));
                    body.push_back("size += " + var->arrayTypeSize.Get() + smcln);
                    loop.SetBody(body);


                    auto def = loop.GetDefinition();
                    funcBody.insert(funcBody.end(),def.begin(),def.end());
                }
                else
                {
                    if(var->data.defaultValue)
                        funcBody.push_back(PrintVarDefinition("str->" +
                                                              var->fieldName,
                                                              to_string(var->data.value)));
                    funcBody.push_back(PrintMemcpy(string("p + size"),
                                                   string("&str->")+var->fieldName,
                                                   var->fieldSize.Get()));
                    funcBody.push_back("size += " + var->fieldSize.Get() + smcln);
                }
            }
            else
            {
                if(var->data.isArrayField)
                {
                    ForLoopCpp loop;
                    vector<string> body;

                    if(qtCppOption && var->data.withLenDefiningVar)
                    {
                        funcBody.push_back(string("str->") + var->fieldName +
                                           ".resize" + "(str->" +
                                           var->data.lenDefiningVar + ")" + smcln);
                    }
                    else if(var->data.withLenDefiningVar)
                    {
                        string numOfElements = var->data.lenDefiningVar;

                        string typePrefix = FieldMetaType(var->type.second);
                        if(typePrefix.size() > 0) typePrefix += b_und;

                        string elementType = typePrefix + var->type.first;
                        string elementSize = PrintSizeOf(elementType);
                        string typeConvers = "(" + elementType + "*)";

                        funcBody.push_back(string("str->") + var->fieldName +
                                           " =" + typeConvers + "allocate" + "(str->" +
                                           numOfElements + "*" + elementSize + ")" + smcln);
                    }

                    loop.SetDeclaration("int i = 0","i < " + var->fieldSize.Get() /*+
                                        " / " + var->baseTypeSize.Get()*/,"i++");

                    body.push_back(PrintMemcpy(string("&str->") + var->fieldName +
                                               PutInSqBraces("i"),
                                               string("p + size"),
                                               var->arrayTypeSize.Get()));
                    body.push_back("size += " + var->arrayTypeSize.Get() + smcln);
                    loop.SetBody(body);

                    auto var = loop.GetDefinition();
                    funcBody.insert(funcBody.end(),var.begin(),var.end());

                }
                else
                {
                    if(var->type.second == fieldType::Enum ||
                            var->type.second == fieldType::Type ||
                            var->type.second == fieldType::Code)
                    {
                        funcBody.push_back(string("str->")+var->fieldName + " =" +
                                           lsb + FieldMetaType(var->type.second) +
                                           "_" + var->type.first + rsb + "0" + smcln);
                    }

                    funcBody.push_back(PrintMemcpy(string("&str->") + var->fieldName,
                                                   string("p + size"),
                                                   var->fieldSize.Get()));
                    funcBody.push_back("size += " + var->fieldSize.Get() + smcln);
                    if(var->data.defaultValue)
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
        }else
        {
            Function localFunc;
            if(type == Ser)
            {
                if(var->data.isArrayField)
                {
                    ForLoopCpp loop;

                    loop.SetDeclaration("int i = 0","i < " + var->fieldSize.Get() /*+
                                        " / " + var->baseTypeSize.Get()*/,"i++");
                    loop.SetBody("size += " + _pSer + FieldMetaType(var->type.second) + b_und +
                                 var->type.first + lsb + string("p + size")  +
                                 com + "&str->" + var->fieldName + PutInSqBraces("i") +
                                 rsb + smcln);

                    auto var = loop.GetDefinition();
                    funcBody.insert(funcBody.end(), var.begin(), var.end());

                }
                else
                {
                    funcBody.push_back("size += " + _pSer + FieldMetaType(var->type.second) +
                                       b_und + var->type.first + lsb +
                                       string("p + size") +
                                       com + "&str->" + var->fieldName + rsb + smcln);
                }
            }
            else
            {
                if(var->data.isArrayField)
                {
                    ForLoopCpp loop;

                    if(qtCppOption && var->data.withLenDefiningVar)
                    {
                        funcBody.push_back(string("str->") + var->fieldName +
                                           ".resize" + "(str->" +
                                           var->data.lenDefiningVar + ")" + smcln);
                    }
                    else if(var->data.withLenDefiningVar)
                    {
                        string numOfElements = var->data.lenDefiningVar;
                        string typePrefix = FieldMetaType(var->type.second);
                        string elementType = typePrefix + b_und + var->type.first;
                        string elementSize = PrintSizeOf(elementType);
                        string typeConvers = "(" + elementType + "*)";

                        funcBody.push_back(string("str->") + var->fieldName +
                                           " =" + typeConvers + "allocate" + "(str->" +
                                           numOfElements + "*" + elementSize + ")" + smcln);
                    }

                    loop.SetDeclaration("int i = 0","i < " + var->fieldSize.Get() /*+
                                        " / " + var->baseTypeSize.Get()*/,"i++");
                    loop.SetBody("size += " + _pDes + FieldMetaType(var->type.second) + b_und +
                                 var->type.first + lsb +
                                 string("p + size") +
                                 com + "&str->" + var->fieldName +
                                 PutInSqBraces("i") + com + string("op_status") +
                                 rsb + smcln);

                    auto var = loop.GetDefinition();
                    funcBody.insert(funcBody.end(),var.begin(),var.end());

                }else
                funcBody.push_back("size += " + _pDes + FieldMetaType(var->type.second) +
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

    for(auto field : fields)
    {
        string prefix = FieldMetaType(field.type.second) + b_und;
        fieldType fldType = field.type.second;
        bool isArray  = field.data.isArrayField;
        bool isDynamicArray = field.data.withLenDefiningVar;

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

string ComplexTypeDescription::FieldMetaType(fieldType type)
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
    for(const auto& field : fields)
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
