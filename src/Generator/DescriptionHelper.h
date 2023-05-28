#ifndef DESCRIPTIONHELPER_H
#define DESCRIPTIONHELPER_H

#include <vector>
#include <string>
#include "Analyzer/Formater.h"
#include "Polynomial.h"
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

struct StructField
{
    pair<string,fieldType> type;
    string fieldName;
    Polynomial fieldSize;
    Polynomial fieldOffset;
    Polynomial arrayTypeSize; // for array fields
    FieldData data;
};

using strings = std::vector<string>;

template<typename T, template<class> class Container>
bool Contain(const Container<T>& c, T val) {
    return std::find(c.begin(), c.end(), val) != c.end();
}

class DescHelper
{
public:
    DescHelper() = delete;

    static strings CalcSizeLoop(string typePrefix, StructField field,
                                FunType type);

    static strings CalcStructSize(string typePrefix,
                                  StructField field, FunType type,
                                  bool isInLoop = false);

    static string CalcSizeFunName(string name, FunType type);

    // Calculate serialization size for simple fieldTypes: Enum, Code, Type, std
    static string CalcSimpeTypeSize(StructField field, FunType type);

    static string CalcDesSimpleArrTypeSize(string typePrefix,
                                           StructField field);

    static void AppendStrings(vector<string>& dst, const vector<string> &src);

    static Function CalcSizeFunDecl(string name, FunType type, bool hasStatic);

    static strings CSizeDef();

private:
    static ForLoopCpp CalcSizeLoopDecl(StructField field, FunType type);
    static vector<string> AccumulateSize(StructField field);
    static string FieldSize(StructField field, FunType type);
};

#endif // DESCRIPTIONHELPER_H
