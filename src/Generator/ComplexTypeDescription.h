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
    void setOption(AppOptions options);

    void setBlockType(ComplexType type);
    void setPrefix(string prefix);

    void setName(string _name);
    string getName() const;

    string getCodeName() const;

    SizeExprPtr size() const;

    void addContextOffset(SizeExprPtr offset);

    void addField(StructFieldInfo data,
                  pair<string, FieldType> type,
                  SizeExprPtr fieldSize);

    vector<ComplexField> fields() const;

    vector<string> declaration();

    vector<string> serdesDefinition(FunType funType, bool hasStatic = true);

    string serCall(string pointerName, string offset, string paramName) const;
    string desCall(string pointerName, string offset, string paramName,
                   string op_status) const;

    string asVarDecl() const;
    vector<string> sizeCalcFun(FunType type, bool hasStatic = true);
    string sizeCalcFunCall(FunType type) const;
    string blType() const;
    Function compareFun();

    void setAlignedCopyPreferred(bool state);
    bool shouldUseAlignedCopy(const ComplexField &field) const;

private:
    string _name;
    vector<ComplexField> _fields;
    ComplexType _blockType;

    AppOptions _options{};

    string _prefix;
    string _pSer = "Ser_";
    string _pDes = "Des_";

    bool _isAlignedCopyPreferred = true;

    vector<SizeExprPtr> _contextOffsets;

    string fieldTypePrefix(FieldType type);

    bool isArrayVarLen(string name) const;

    void setSerdesDeclaration(Function& function, FunType type);

    Strings serArrayField(const ComplexField &field);
    Strings desArrayField(const ComplexField &field);
    Strings arrayFieldAllocation(const ComplexField &field);

    Strings serSimpleField(const ComplexField &field);
    Strings desSimpleField(const ComplexField &field);

    Strings desCheckInitValue(const ComplexField &field);
    Strings desCheckValueRange(const ComplexField &field);

    String usedCopyMethod(const ComplexField &field);

    bool isCppBoolDynamicArray(const ComplexField &field);

    static Strings checkOpStatus();
    static bool isSimpleBitField(const ComplexField &field);
    static bool isArrayBitField(const ComplexField &field);
    static bool isArrayAlignedField(const ComplexField &field);
    static bool isNonStandardSize(const SizeExprPtr& bitSize);


};

#endif // COMPLEXTYPEDESCRIPTION_H
