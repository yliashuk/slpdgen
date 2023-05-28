#ifndef COMPLEXTYPEDESCRIPTION_H
#define COMPLEXTYPEDESCRIPTION_H

#include <string>
#include <algorithm>
#include <vector>
#include "Analyzer/Formater.h"
#include "CppConstructs/CommonCpp.h"
#include "CppConstructs/IfElseStatementCpp.h"
#include "CppConstructs/FunctionCpp.h"
#include "CppConstructs/ForLoopCpp.h"

#include "Polynomial.h"

#include "DescriptionHelper.h"

using namespace std;
using namespace CppConstructs;


struct EnumField
{
    string fieldName;
    uint32_t value;
};

enum class ComplexType
{
    Struct,
    Message,
    Header
};

struct VarArray
{
    string varArrayType;
    Polynomial varArraySize;
    string varArrayLen;
    string varArrayTypeName;
};

class ComplexTypeDescription
{
public:
    ComplexTypeDescription();
    void SetBlockType(ComplexType type);
    void SetName(string name);
    string GetName();
    string GetCodeName();
    string name;
    ComplexType blockType;
    vector<StructField> fields;
    Polynomial Size();
    void addField(FieldDataStruct data,
                  pair<string, fieldType> type,
                  Polynomial fieldSize);

    vector<string> PrintDecl();
    vector<string>
    PrintSerDesDeclaration(FunType type, bool hasStatic = true);

    string PrintSerDesCall(FunType type, string structName);
    string PrintSerCall();
    string PrintSerCall(string pointerName, string paramName);
    string PrintDesCall(string pointerName, string paramName);
    string PrintDesCall();
    string PrintVarDecl();
    vector<string> PrintSizeCalcFun(FunType type, bool hasStatic = true);
    string PrintSizeCalcFunCall(FunType type);
    string BlType();
    Function GetCompareFun();
    bool hasDynamicFields = false;
    vector<VarArray> varArrays;
    //string varArrayTypeName;
    void SetPrefix(string prefix);
    bool qtOption = false;
    bool qtCppOption = false;
    string cObjType;
private:
    string _prefix;
    string _pSer = "Ser_";
    string _pDes = "Des_";

    string FieldMetaType(fieldType type);

    bool IsArrayVarLen(string name);
};

#endif // COMPLEXTYPEDESCRIPTION_H
