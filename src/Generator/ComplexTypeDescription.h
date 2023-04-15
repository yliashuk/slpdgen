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
using namespace std;
using namespace CppConstructs;

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
    Polynomial baseTypeSize; // for array fields
    FieldData data;
};

struct EnumField
{
    string fieldName;
    uint32_t value;
};
enum FunType
{
    Ser,
    Des
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
    string BlType();
    Function GetCompareFun();
    bool WithVarArray = false;
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
};

#endif // COMPLEXTYPEDESCRIPTION_H
