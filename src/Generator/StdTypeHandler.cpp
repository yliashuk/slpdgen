#include "StdTypeHandler.h"
#include <Utils/StringUtils.h>
#include <CppConstructs/FunctionsSrc.h>

using namespace std;
using namespace Utils;

StdTypeHandler::StdTypeHandler(AppOptions options) : _options(options){}

optional<pair<string, SizeExprPtr> > StdTypeHandler::CheckType(string slpdType, bool isArray)
{
    if(slpdType == "char") return {{"char", 8_lit}};
    if(slpdType == "bool") return {{"bool", 1_lit}};
    if(slpdType == "f32")  return {{"float", 32_lit}};
    if(slpdType == "f64")  return {{"double", 64_lit}};

    string kind = {slpdType[0]};
    if(contains(Strings{"s", "i", "u"}, kind))
    {
        size_t size = strtol(slpdType.substr(1, slpdType.size()).data(), NULL, 10);
        string prefixT = kind == "u" ? "u" : "";

        if(size % 8 && isArray) // use slpd_bit_field type for bit array elements
        {
            string type = SlpdType(kind, size);
            BitType bitType = {type, IntType(prefixT, RoundUp(size)), size};
            if(!contains(_bitFieldTypes, bitType)){ _bitFieldTypes += bitType; }

            return {{type, Literal::Create(size)}};
        } else {
            return {{IntType(prefixT, RoundUp(size)), Literal::Create(size)}};
        }
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
    return fmt("%sint%s_t", {prefixT, to_string(size)});
}

string StdTypeHandler::SlpdType(string prefixT, size_t size)
{
    return fmt("slpd_%s%s", {prefixT, to_string(size)});
}

std::vector<string> StdTypeHandler::BitFieldTypes()
{
    vector<string> body = {};
   if(!_bitFieldTypes.empty())
   {
       body << (_options.isCpp ? BitFieldTemplate : BitFieldTemplateC);

       string cppFmt = "using %s = slpd_bit_field<%s, %s>;";

       for(auto type : _bitFieldTypes)
       {
           if(_options.isCpp) {
               body << fmt(cppFmt, {type.name, type.cppType, to_string(type.size)});
           }
           else {
               string uName = type.name;
               toUpper(uName);
               body << fmt(BitFieldCFmt, {uName, uName, type.name, type.cppType,
                                          to_string(type.size), uName});
           }
       }
   }

   return body;
}

string StdTypeHandler::SlpdToAlignedCppType(string stdSlpdType)
{
    string kind = {stdSlpdType.at(5)};
    if(contains(Strings{"i", "u"}, kind))
    {
        string prefixT = kind == "u" ? "u" : "";
        int size = strtol(stdSlpdType.substr(6, stdSlpdType.size()).data(), NULL, 10);
        return IntType(prefixT, RoundUp(size));
    }
    return "";
}

bool StdTypeHandler::BitType::operator==(const StdTypeHandler::BitType& type) const
{
    return name == type.name &&
           cppType == type.cppType &&
           size == type.size;
}
