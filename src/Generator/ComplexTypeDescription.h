#ifndef COMPLEXTYPEDESCRIPTION_H
#define COMPLEXTYPEDESCRIPTION_H

#include <string>
#include <algorithm>
#include <vector>
#include "AppOptions.h"
#include "Analyzer/Formater.h"
#include "CppConstructs/IfElseStatementCpp.h"
#include "CppConstructs/FunctionCpp.h"
#include "CppConstructs/ForLoopCpp.h"
#include "Utils/StringUtils.h"

#include "CalcSizeHelper.h"

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

class ComplexTypeDescription
{
public:
    ComplexTypeDescription();
    void SetOption(AppOptions options);

    void SetBlockType(ComplexType type);
    void SetPrefix(string prefix);

    void SetName(string name);
    string GetName() const;

    string GetCodeName() const;

    SizeExprPtr Size() const;

    void AddContextOffset(SizeExprPtr offset);

    void addField(FieldDataStruct data,
                  pair<string, fieldType> type,
                  SizeExprPtr fieldSize);

    vector<StructField> GetFields() const;

    vector<string> Declaration();

    vector<string> SerDesDefinition(FunType funType, bool hasStatic = true);

    string SerCall(string pointerName, string offset, string paramName) const;
    string DesCall(string pointerName, string offset, string paramName,
                   string op_status) const;

    string AsVarDecl() const;
    vector<string> SizeCalcFun(FunType type, bool hasStatic = true);
    string SizeCalcFunCall(FunType type) const;
    string BlType() const;
    Function CompareFun();

    void SetAlignedCopyPreferred(bool state);

private:
    string name;
    vector<StructField> _fields;
    ComplexType _blockType;

    AppOptions _options{};

    string _prefix;
    string _pSer = "Ser_";
    string _pDes = "Des_";

    bool _isAlignedCopyPreferred = true;

    vector<SizeExprPtr> _contextOffsets;

    string FieldTypePrefix(fieldType type);

    bool IsArrayVarLen(string name) const;
    std::vector<std::pair<String, String>> ArrNameSizeVarPairs();

    void SetSerDesDeclaration(Function& function, FunType type);

    Strings SerArrayField(const StructField &field);
    Strings DesArrayField(const StructField &field);
    Strings ArrayFieldAllocation(const StructField &field);

    Strings SerSimpleField(const StructField &field);
    Strings DesSimpleField(const StructField &field);

    Strings DesCheckInitValue(const StructField &field);
    Strings DesCheckValueRange(const StructField &field);

    String UsedCopyMethod(const StructField &field);
    bool ShouldUseAlignedCopy(const StructField &field) const;

    static bool IsSimpleBitField(const StructField &field);
    static bool IsArrayBitField(const StructField &field);
    static bool IsArrayAlignedField(const StructField &field);


};

#endif // COMPLEXTYPEDESCRIPTION_H
