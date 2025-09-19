#ifndef STDTYPEHANDLER_H
#define STDTYPEHANDLER_H

#include <set>
#include <vector>
#include <string>
#include <optional>

class StdTypeHandler
{
public:
    std::optional<std::pair<std::string, size_t>> CheckType(std::string slpdType);
    std::vector<std::string> BitFieldTypes();

private:
    struct BitType
    {
        std::string name;
        std::string cppType;
        size_t size;

        bool operator==(const BitType& rhs) const;
    };

    std::vector<BitType> _bitFieldTypes;

    static int RoundUp(int number);
    static std::string IntType(std::string prefixT, size_t sizeT);
    std::vector<std::string> BitFieldTypeTemplate();
};

#endif // STDTYPEHANDLER_H
