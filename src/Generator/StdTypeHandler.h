#ifndef STDTYPEHANDLER_H
#define STDTYPEHANDLER_H

#include <set>
#include <vector>
#include <string>
#include <optional>
#include "SizeExpr.h"
#include "AppOptions.h"

class StdTypeHandler
{
public:
    StdTypeHandler(AppOptions options);

    std::optional<std::pair<std::string, SizeExprPtr>>
    CheckType(std::string slpdType, bool isArray);

    std::vector<std::string> BitFieldTypes();
    static std::string SlpdToAlignedCppType(std::string stdSlpdType);
private:
    AppOptions _options;

    struct BitType
    {
        std::string name;
        std::string cppType;
        size_t size;

        bool operator==(const BitType& rhs) const;
    };

    std::vector<BitType> _bitFieldTypes;

    static int RoundUp(int number);
    static std::string IntType(std::string prefixT, size_t size);
    static std::string SlpdType(std::string prefixT, size_t size);
    std::vector<std::string> BitFieldTypeTemplate();
};

#endif // STDTYPEHANDLER_H
