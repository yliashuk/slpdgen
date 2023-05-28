#include "DescriptionHelper.h"

strings DescHelper::CalcSizeLoop(string typePrefix, StructField field,
                                 FunType type)
{
    ForLoopCpp loop = CalcSizeLoopDecl(field, type);
    loop.SetBody(CalcStructSize(typePrefix, field, type, true));
    return loop.GetDefinition();
}

strings
DescHelper::CalcStructSize(string typePrefix, StructField field,
                           FunType type, bool isInLoop)
{
    vector<string> body;
    string calcFunName;
    string callCalcFun;

    calcFunName = CalcSizeFunName(typePrefix + field.type.first, type);

    // Generate call calculation function for ser/des size
    if(type == Ser)
    {
        string arrIndex = isInLoop ? PutInSqBraces("i") : string{};
        string funParam = "&str->" + field.fieldName + arrIndex;
        callCalcFun = "size += " + calcFunName + "(" + funParam + ")" + smcln;
    }
    else if (type == Des)
    {
        string funParams = "p + c_size.r";
        callCalcFun = "c_size_t c_tmp_size = " + calcFunName +
                "(" + funParams + ")" + smcln;
    }
    AppendStrings(body, {callCalcFun});

    if(type == Des)
    {
        // accumulate result of calculation function in local size var
        AppendStrings(body, AccumulateSize(field));

        // out of loop put code in {} braces to limit visibility tmp_size var
        body = !isInLoop ? PutInBlock(body) : body;
    }

    return body;
}

string DescHelper::CalcSizeFunName(string name, FunType type)
{
    string funLabel = (type == Ser) ? "ser" : "des"; // send and receive
    return "calculate" + b_und + name + b_und + funLabel +
            b_und + "size";
}

string
DescHelper::CalcSimpeTypeSize(StructField field, FunType type)
{
    string sumVar = type == Ser ? "size" : "c_size.r";
    string res = sumVar + " += " + FieldSize(field, type);
    if(field.data.isArrayField)
    {
        res += " * " + field.arrayTypeSize.Get();
    }
    return res + smcln + " //" + field.fieldName;
}

string DescHelper::CalcDesSimpleArrTypeSize(string typePrefix,
                                            StructField field)
{
    string numOfElements = FieldSize(field, Des);
    string prefix = field.data.isStdType ? string{} : typePrefix;

    // type size after deserialization
    string typeSize = PrintSizeOf(prefix + field.type.first);
    auto dynArrSize = typeSize + " * " + numOfElements;

    string comment = " //" + field.fieldName;

    // calculate dynamic size
    return "c_size.d += " + dynArrSize + smcln + comment;
}

void
DescHelper::AppendStrings(vector<string> &dst, const vector<string> &src)
{
    std::copy(src.begin(), src.end(), std::back_inserter(dst));
}

Function DescHelper::CalcSizeFunDecl(string name, FunType type, bool hasStatic)
{
    Parameter funParam = type == Ser ? Parameter{name + "*", "str"} :
                                       Parameter{"char*","p"};
    string returnParam = type == Ser ? string{"size_t"} : string{"c_size_t"};
    Function fun;

    string staticPrefix = hasStatic ? "static " : "";

    fun.SetDeclaration(DescHelper::CalcSizeFunName(name, type),
                       staticPrefix + returnParam,
                       funParam);
    return fun;
}

strings DescHelper::CSizeDef()
{
    strings body;

    body.push_back("size_t s; // static part");
    body.push_back("size_t d; // dynamic part");
    body.push_back("size_t r; // size in raw memory");

    StructCpp str;
    str.SetName("c_size_t");
    str.SetBody(body);

    return str.GetDeclaration();
}

ForLoopCpp DescHelper::CalcSizeLoopDecl(StructField field, FunType type)
{
    string init =  "size_t i = 0";
    string condition = " i < " + FieldSize(field, type);
    string increment = "++i";

    ForLoopCpp loop;
    loop.SetDeclaration(init, condition, increment);

    return loop;
}

vector<string> DescHelper::AccumulateSize(StructField field)
{
    vector<string> body;
    if(field.data.withLenDefiningVar)
    {
        AppendStrings(body, {"c_size.d += c_tmp_size.s" + smcln});
    }
    AppendStrings(body, {"c_size.d += c_tmp_size.d" + smcln});
    AppendStrings(body, {"c_size.r += c_tmp_size.r" + smcln});

    return body;
}

string DescHelper::FieldSize(StructField field, FunType type)
{
    bool isDynamic = field.data.withLenDefiningVar;

    // write the fix size for static array field or var names for dynamic
    auto size = isDynamic ? field.data.lenDefiningVar : field.fieldSize.Get();

    // generate like
    if(isDynamic && type == Ser) {
        size = "str->" + size;
    }

    return size;
}
