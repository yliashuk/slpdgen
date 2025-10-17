#include "CalcSizeHelper.h"

Strings CalcSizeHelper::CalcSizeLoop(string typePrefix, ComplexField field,
                                 FunType type)
{
    ForLoopCpp loop = CalcSizeLoopDecl(field, type);
    loop.SetBody(CalcStructSize(typePrefix, field, type, true));
    return loop.Definition();
}

Strings
CalcSizeHelper::CalcStructSize(string typePrefix, ComplexField field,
                           FunType type, bool isInLoop)
{
    Strings body;
    string calcFunName;
    string callCalcFun;

    calcFunName = CalcSizeFunName(typePrefix + field.type.first, type);

    // Generate call calculation function for ser/des size
    if(type == Ser)
    {
        string arrIndex = isInLoop ? "[i]" : string{};
        string funParam = "&str->" + field.name + arrIndex;
        callCalcFun = fmt("size += %s(%s);", {calcFunName, funParam});
    }
    else if (type == Des)
    {
        callCalcFun = fmt("c_size_t c_tmp_size = %s(p + c_size.r);", {calcFunName});
    }
    body << callCalcFun;

    if(type == Des)
    {
        // accumulate result of calculation function in local size var
        body << AccumulateSize(field);

        // out of loop put code in {} braces to limit visibility tmp_size var
        body = !isInLoop ? Strings() << "{" << fmt("\t%s", _d(body)) << "}" : body;
    }

    return body;
}

string CalcSizeHelper::CalcSizeFunName(string name, FunType type)
{
    string funLabel = (type == Ser) ? "ser" : "des"; // send and receive
    return fmt("calculate_%s_%s_size",{name, funLabel});
}

string
CalcSizeHelper::CalcSimpeTypeSize(ComplexField field, FunType type)
{
    string sumVar = type == Ser ? "size" : "c_size.r";
    string res = sumVar + " += " + FieldSize(field, type);
    if(field.info.IsArray())
    {
        res += " * " + field.arrayElementSize->ToString();
    }
    return res + "; //" + field.name;
}

string CalcSizeHelper::CalcDesSimpleArrTypeSize(string typePrefix,
                                            ComplexField field)
{
    string numOfElements = FieldSize(field, Des);
    string prefix = field.type.second == fieldType::std ? string{} : typePrefix;

    // type size after deserialization
    string typeSize = fmt("sizeof(%s%s) * 8",{prefix, field.type.first});

    auto dynArrSize = typeSize + " * " + numOfElements;

    // calculate dynamic size
    return fmt("c_size.d += %s; //%s",{dynArrSize, field.name});
}

Function CalcSizeHelper::CalcSizeFunDecl(string name, FunType type, bool hasStatic)
{
    Parameter funParam = type == Ser ? Parameter{name + "*", "str"} :
                                       Parameter{"char*","p"};
    string returnParam = type == Ser ? string{"size_t"} : string{"c_size_t"};
    Function fun;

    string staticPrefix = hasStatic ? "static " : "";

    fun.SetDeclaration(CalcSizeHelper::CalcSizeFunName(name, type),
                       staticPrefix + returnParam,
                       funParam);
    return fun;
}

Strings CalcSizeHelper::CSizeDef()
{
    StructCpp::Fields fields {
        {"size_t", "s", "", "static part"},
        {"size_t", "d", "", "dynamic part"},
        {"size_t", "r", "", "size in raw memory"}
    };

    StructCpp str;
    str.SetName("c_size_t");
    str.SetTypeDef(true);
    str.AddFields(fields);

    return str.Declaration();
}

ForLoopCpp CalcSizeHelper::CalcSizeLoopDecl(ComplexField field, FunType type)
{
    string init =  "size_t i = 0";
    string condition = " i < " + FieldSize(field, type);
    string increment = "++i";

    ForLoopCpp loop;
    loop.SetDeclaration(init, condition, increment);

    return loop;
}

vector<string> CalcSizeHelper::AccumulateSize(ComplexField field)
{
    vector<string> body;
    if(field.info.HasDynamicSize())
    {
        body << "c_size.d += c_tmp_size.s;";
    }
    body << "c_size.d += c_tmp_size.d;";
    body << "c_size.r += c_tmp_size.r;";

    return body;
}

string CalcSizeHelper::FieldSize(ComplexField field, FunType type)
{
    bool isArray = field.info.IsArray();
    bool isDynamic = field.info.HasDynamicSize();

    auto bitSize = field.bitSize->ToString();
    auto elementCount = field.arrayElementCount->ToString();
    // write the fix size for static array field or var names for dynamic
    auto size = isArray ? (isDynamic ? *field.info.sizeVar : elementCount) : bitSize;

    // generate like
    if(isDynamic && type == Ser) {
        size = "str->" + size;
    }

    return size;
}
