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
    checkType(std::string slpdType, bool isArray);

    std::vector<std::string> bitfieldArrayTypes();
    static std::string slpdToAlignedCppType(std::string stdSlpdType);
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

    static bool isSlpdType(std::string type);
    static int roundUp(int number);
    static std::string intType(std::string prefixT, size_t size);
    static std::string bitfieldArrayType(std::string prefixT, size_t size);
};

#endif // STDTYPEHANDLER_H
