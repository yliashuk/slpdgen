#include "CalcSizeHelper.h"

Strings CalcSizeHelper::calcSizeLoop(string typePrefix, ComplexField field,
                                 FunType type)
{
    ForLoopCpp loop = calcSizeLoopDecl(field, type);
    loop.setBody(calcStructSize(typePrefix, field, type, true));
    return loop.definition();
}

Strings
CalcSizeHelper::calcStructSize(string typePrefix, ComplexField field,
                           FunType type, bool isInLoop)
{
    Strings body;
    string calcFunName;
    string callCalcFun;

    calcFunName = calcSizeFunName(typePrefix + field.type.first, type);

    // Generate call calculation function for ser/des size
    if(type == Ser)
    {
        string arrIndex = isInLoop ? "[i]" : string{};
        string funParam = "&str->" + field.name + arrIndex;
        callCalcFun = fmt("size += %s(%s);", {calcFunName, funParam});
    }
    else if (type == Des)
    {
        callCalcFun = fmt("c_size_t c_tmp_size = %s(p, offset + c_size.r);",
        {calcFunName});
    }
    body << callCalcFun;

    if(type == Des)
    {
        // accumulate result of calculation function in local size var
        body << accumulateSize(field);

        // out of loop put code in {} braces to limit visibility tmp_size var
        body = !isInLoop ? Strings() << "{" << fmt("\t%s", _d(body)) << "}" : body;
    }

    return body;
}

string CalcSizeHelper::calcSizeFunName(string name, FunType type)
{
    string funLabel = (type == Ser) ? "ser" : "des"; // send and receive
    return fmt("calculate_%s_%s_size",{name, funLabel});
}

string
CalcSizeHelper::calcSimpeTypeSize(ComplexField field, FunType type)
{
    string sumVar = type == Ser ? "size" : "c_size.r";
    string res = sumVar + " += " + fieldSize(field, type);
    if(field.info.isArray())
    {
        res += " * " + field.arrayElementSize->toString();
    }
    return res + "; //" + field.name;
}

string CalcSizeHelper::calcDesSimpleArrTypeSize(string typePrefix,
                                            ComplexField field)
{
    string numOfElements = fieldSize(field, Des);
    string prefix = field.type.second == FieldType::std ? string{} : typePrefix;

    // type size after deserialization
    string typeSize = fmt("sizeof(%s%s) * 8",{prefix, field.type.first});

    auto dynArrSize = typeSize + " * " + numOfElements;

    // calculate dynamic size
    return fmt("c_size.d += %s; //%s",{dynArrSize, field.name});
}

Function CalcSizeHelper::calcSizeFunDecl(string name, FunType type, bool hasStatic)
{
    auto params = type == Ser
            ? std::vector<Parameter>{{name + "*", "str"}}
            : std::vector<Parameter>{{"char*", "p"}, {"size_t", "offset"}};

    string returnType = type == Ser ? "size_t" : "c_size_t";
    string funName = CalcSizeHelper::calcSizeFunName(name, type);

    Function fun;
    fun.setStatic(hasStatic);
    fun.setDeclaration(funName, returnType, params);

    return fun;
}

Strings CalcSizeHelper::cSizeDef()
{
    StructCpp::Fields fields {
        {"size_t", "s", "", "static part"},
        {"size_t", "d", "", "dynamic part"},
        {"size_t", "r", "", "size in raw memory"}
    };

    StructCpp str;
    str.setName("c_size_t");
    str.setTypeDef(true);
    str.addFields(fields);

    return str.declaration();
}

ForLoopCpp CalcSizeHelper::calcSizeLoopDecl(ComplexField field, FunType type)
{
    string init =  "size_t i = 0";
    string condition = " i < " + fieldSize(field, type);
    string increment = "++i";

    ForLoopCpp loop;
    loop.setDeclaration(init, condition, increment);

    return loop;
}

vector<string> CalcSizeHelper::accumulateSize(ComplexField field)
{
    vector<string> body;
    if(field.info.hasDynamicSize())
    {
        body << "c_size.d += c_tmp_size.s;";
    }
    body << "c_size.d += c_tmp_size.d;";
    body << "c_size.r += c_tmp_size.r;";

    return body;
}

string CalcSizeHelper::fieldSize(ComplexField field, FunType type)
{
    bool isArray = field.info.isArray();
    bool isDynamic = field.info.hasDynamicSize();

    auto bitSize = field.bitSize->toString();
    auto elementCount = field.arrayElementCount->toString();
    // write the fix size for static array field or var names for dynamic
    auto size = isArray ? (isDynamic ? *field.info.sizeVar : elementCount) : bitSize;

    // generate like
    if(isDynamic && type == Ser) {
        size = "str->" + size;
    }

    return size;
}
