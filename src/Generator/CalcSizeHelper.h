#ifndef CALCSIZELOOP_H
#define CALCSIZELOOP_H

#include <vector>
#include <string>
#include "Analyzer/Formater.h"
#include "SizeExpr.h"
#include "CppConstructs/ForLoopCpp.h"
#include "CppConstructs/FunctionCpp.h"
#include "CppConstructs/StructCpp.h"


using namespace std;
using namespace CppConstructs;

enum FunType
{
    Ser,
    Des
};

enum class fieldType
{
    std,
    Enum,
    Code,
    Type,
    Struct
};

struct ComplexField
{
    string name;
    pair<string, fieldType> type;
    SizeExprPtr bitSize = 0_lit;
    SizeExprPtr arrayElementSize = 0_lit;  // for array fields
    SizeExprPtr arrayElementCount = 0_lit; // for array fields

    FieldInfo info;
};

class CalcSizeHelper
{
public:
    CalcSizeHelper() = delete;

    static Strings CalcSizeLoop(string typePrefix, ComplexField field,
                                FunType type);

    static Strings CalcStructSize(string typePrefix,
                                  ComplexField field, FunType type,
                                  bool isInLoop = false);

    static string CalcSizeFunName(string name, FunType type);

    // Calculate serialization size for simple fieldTypes: Enum, Code, Type, std
    static string CalcSimpeTypeSize(ComplexField field, FunType type);

    static string CalcDesSimpleArrTypeSize(string typePrefix,
                                           ComplexField field);

    static Function CalcSizeFunDecl(string name, FunType type, bool hasStatic);

    static Strings CSizeDef();

private:
    static ForLoopCpp CalcSizeLoopDecl(ComplexField field, FunType type);
    static vector<string> AccumulateSize(ComplexField field);
    static string FieldSize(ComplexField field, FunType type);
};

#endif // DESCRIPTIONHELPER_H
