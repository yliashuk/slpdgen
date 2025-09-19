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

enum class FieldType
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
    pair<string, FieldType> type;
    SizeExprPtr bitSize = 0_lit;
    SizeExprPtr arrayElementSize = 0_lit;  // for array fields
    SizeExprPtr arrayElementCount = 0_lit; // for array fields

    FieldInfo info;
};

class CalcSizeHelper
{
public:
    CalcSizeHelper() = delete;

    static Strings calcSizeLoop(string typePrefix, ComplexField field,
                                FunType type);

    static Strings calcStructSize(string typePrefix,
                                  ComplexField field, FunType type,
                                  bool isInLoop = false);

    static string calcSizeFunName(string name, FunType type);

    // Calculate serialization size for simple fieldTypes: Enum, Code, Type, std
    static string calcSimpeTypeSize(ComplexField field, FunType type);

    static string calcDesSimpleArrTypeSize(string typePrefix,
                                           ComplexField field);

    static Function calcSizeFunDecl(string name, FunType type, bool hasStatic);

    static Strings cSizeDef();

private:
    static ForLoopCpp calcSizeLoopDecl(ComplexField field, FunType type);
    static vector<string> accumulateSize(ComplexField field);
    static string fieldSize(ComplexField field, FunType type);
};

#endif // DESCRIPTIONHELPER_H
