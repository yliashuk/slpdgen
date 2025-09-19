#include "StdTypeHandler.h"
#include <Utils/StringUtils.h>
#include <CppConstructs/FunctionsSrc.h>

using namespace std;
using namespace Utils;

optional<pair<string, size_t> > StdTypeHandler::CheckType(string slpdType)
{
    if(slpdType == "char") return {{"char", 8}};
    if(slpdType == "bool") return {{"bool", 1}};
    if(slpdType == "f32")  return {{"float", 32}};
    if(slpdType == "f64")  return {{"double", 64}};

    string kind = {slpdType[0]};
    if(contains(Strings{"s", "i", "u"}, kind))
    {
        size_t size = strtol(slpdType.substr(1, slpdType.size()).data(), NULL, 10);
        string prefixT = kind == "u" ? "u" : "";

        string type = IntType(prefixT, size);
        if(size % 8) { // bitfield
            BitType bitType = {type, IntType(prefixT, RoundUp(size)), size};
            if(!contains(_bitFieldTypes, bitType)){ _bitFieldTypes += bitType; }
        }

        return {{type, size}};
    }
    return {};
}

int StdTypeHandler::RoundUp(int number)
{
    std::set<uint8_t> values = {8, 16, 32, 64};
    auto it = values.lower_bound(number);
    return it != values.end() ? *it : number;
}

string StdTypeHandler::IntType(string prefixT, size_t size)
{
    if(size % 8 == 0) {
        return fmt("%sint%s_t", {prefixT, to_string(size)});
    } else {
        return fmt("slpd_%s%s", {prefixT, to_string(size)});
    }
}

std::vector<string> StdTypeHandler::BitFieldTypes()
{
    vector<string> body = {};
   if(!_bitFieldTypes.empty())
   {
        body << BitFieldTemplate;
        for(auto type : _bitFieldTypes)
        {
            body << fmt("using %s = slpd_bit_field<%s, %s>;",
            {type.name, type.cppType, to_string(type.size)});
        }
        body << "";
   }

   return body;
}

bool StdTypeHandler::BitType::operator==(const StdTypeHandler::BitType& type) const
{
    return name == type.name &&
           cppType == type.cppType &&
           size == type.size;
}
