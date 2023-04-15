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

    if(data.second.isNumOfCeils)
        field.baseTypeSize = fieldSize;
    if(data.second.isNumOfCeils && data.second.withLenDefiningVar == false)
        field.fieldSize = fieldSize * data.second.value;
    else if(data.second.isNumOfCeils && data.second.withLenDefiningVar == true)
        field.fieldSize = fieldSize * ("str->" + data.second.lenDefiningVar);
    else field.fieldSize = fieldSize;

    field.fieldOffset = Size();
    fields.push_back(field);

    if(data.second.withLenDefiningVar)
    {
        this->WithVarArray = true;
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
        else if(var.data.isNumOfCeils)
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
        else if(qtCppOption && var.data.isNumOfCeils)
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

    if(type == Ser)
    {
        vector<Parameter> params;
        params.push_back({"char*", "p"});
        params.push_back({BlockName, "*str"});
        funConstruct.SetDeclaration(_pSer + BlType() + b_und + name,preStatic + "void",
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
        funConstruct.SetDeclaration(_pDes + BlType() + b_und + name,preStatic + "void",
                                    params);
    }

    if(type == Des)
    {
        //funcBody.push_back(PrintVarDeclaration(BlockName,"str"));
        IfElseStatementCpp statement;
        statement.AddCase("*op_status == 0", "return" + smcln);
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
                if(var->data.isNumOfCeils)
                {                    
                    ForLoopCpp loop;
                    loop.SetDeclaration("int i = 0","i < " + var->fieldSize.Get() +
                                        " / " + var->baseTypeSize.Get() ,"i++");
                    loop.SetBody(PrintMemcpy((Polynomial(string("p")) +
                                              var->fieldOffset +
                                              var->baseTypeSize * "i").Get(),
                                             string("&str->") + var->fieldName +
                                             PutInSqBraces("i"),
                                             var->baseTypeSize.Get()));

                    auto var = loop.GetDefinition();
                    funcBody.insert(funcBody.end(),var.begin(),var.end());

                }
                else
                {
                    if(var->data.defaultValue)
                        funcBody.push_back(PrintVarDefinition("str->" +
                                                              var->fieldName,
                                                              to_string(var->data.value)));
                    funcBody.push_back(PrintMemcpy((Polynomial(string("p")) +
                                                    var->fieldOffset).Get(),
                                                   string("&str->")+var->fieldName,
                                                   var->fieldSize.Get()));
                }
            }
            else
            {
                if(var->data.isNumOfCeils)
                {
                    ForLoopCpp loop;

                    if(qtCppOption && var->data.withLenDefiningVar)
                    {
                        funcBody.push_back(string("str->") + var->fieldName +
                                           ".resize" + "(str->" +
                                           var->data.lenDefiningVar + ")" + smcln);
                    }

                    loop.SetDeclaration("int i = 0","i < " + var->fieldSize.Get() +
                                        " / " + var->baseTypeSize.Get(),"i++");
                    loop.SetBody(PrintMemcpy(string("&str->")+var->fieldName +
                                             PutInSqBraces("i"),
                                             (Polynomial(string("p")) +
                                              var->fieldOffset +
                                              var->baseTypeSize * "i").Get(),
                                             var->baseTypeSize.Get()));

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
                                                   (Polynomial(string("p")) +
                                                    var->fieldOffset).Get(),
                                                   var->fieldSize.Get()));
                    if(var->data.defaultValue)
                    {
                        IfElseStatementCpp statement;
                        statement.AddCase("str->" + var->fieldName + neql +
                                          to_string(var->data.value), "*op_status = 0" +
                                          smcln + " return"+smcln);
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
                                              " return" + smcln);
                        }
                        else {
                            statement.AddCase(st1 + orS + st2,"*op_status = 0" +
                                              smcln + + " return" + smcln);
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
                if(var->data.isNumOfCeils)
                {
                    ForLoopCpp loop;

                    loop.SetDeclaration("int i = 0","i < " + var->fieldSize.Get() +
                                        " / " + var->baseTypeSize.Get(),"i++");
                    loop.SetBody(_pSer + FieldMetaType(var->type.second) + b_und +
                                 var->type.first + lsb + (Polynomial(string("p")) +
                                                          var->fieldOffset +
                                                          var->baseTypeSize * "i").Get() +
                                 com + "&str->" + var->fieldName + PutInSqBraces("i") +
                                 rsb + smcln);

                    auto var = loop.GetDefinition();
                    funcBody.insert(funcBody.end(), var.begin(), var.end());

                }
                else
                {
                    funcBody.push_back(_pSer + FieldMetaType(var->type.second) +
                                       b_und + var->type.first + lsb +
                                       (Polynomial(string("p")) + var->fieldOffset).Get() +
                                       com + "&str->" + var->fieldName + rsb + smcln);
                }
            }
            else
            {
                if(var->data.isNumOfCeils)
                {
                    ForLoopCpp loop;

                    if(qtCppOption && var->data.withLenDefiningVar)
                    {
                        funcBody.push_back(string("str->") + var->fieldName +
                                           ".resize" + "(str->" +
                                           var->data.lenDefiningVar + ")" + smcln);
                    }

                    loop.SetDeclaration("int i = 0","i < " + var->fieldSize.Get() +
                                        " / " + var->baseTypeSize.Get(),"i++");
                    loop.SetBody(_pDes + FieldMetaType(var->type.second) + b_und +
                                 var->type.first + lsb +
                                 (Polynomial(string("p")) + var->fieldOffset +
                                  var->baseTypeSize * "i").Get() +
                                 com + "&str->" + var->fieldName +
                                 PutInSqBraces("i") + com + string("op_status") +
                                 rsb + smcln);

                    auto var = loop.GetDefinition();
                    funcBody.insert(funcBody.end(),var.begin(),var.end());

                }else
                funcBody.push_back(_pDes + FieldMetaType(var->type.second) +
                                   b_und +var->type.first + lsb +
                                   (Polynomial(string("p")) + var->fieldOffset).Get() +
                                   com + string("&str->")+var->fieldName + com +
                                   string("op_status")+ rsb + smcln);
            }
        }
    }
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
