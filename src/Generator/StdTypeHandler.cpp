#include "StdTypeHandler.h"
#include <Utils/StringUtils.h>
#include <CppConstructs/FunctionsSrc.h>

using namespace std;
using namespace Utils;

StdTypeHandler::StdTypeHandler(AppOptions options) : _options(options){}

optional<pair<string, SizeExprPtr> > StdTypeHandler::checkType(string slpdType, bool isArray)
{
    if(slpdType == "char") return {{"char", 8_lit}};
    if(slpdType == "bool") return {{"bool", 8_lit}};
    if(slpdType == "f32")  return {{"float", 32_lit}};
    if(slpdType == "f64")  return {{"double", 64_lit}};

    if(isSlpdType(slpdType))
    {
        size_t size = strtol(slpdType.substr(1, slpdType.size()).data(), NULL, 10);
        string prefixT = slpdType[0] == 'u' ? "u" : "";

        if(size % 8 && isArray) // use slpd_bit_field type for bit array elements
        {
            string type = bitfieldArrayType({slpdType[0]}, size);
            BitType bitType = {type, intType(prefixT, roundUp(size)), size};
            if(!contains(_bitFieldTypes, bitType)){ _bitFieldTypes += bitType; }

            return {{type, Literal::create(size)}};
        } else {
            return {{intType(prefixT, roundUp(size)), Literal::create(size)}};
        }
    }
    return {};
}

int StdTypeHandler::roundUp(int number)
{
    std::set<uint8_t> values = {8, 16, 32, 64};
    auto it = values.lower_bound(number);
    return it != values.end() ? *it : number;
}

string StdTypeHandler::intType(string prefixT, size_t size)
{
    return fmt("%sint%s_t", {prefixT, to_string(size)});
}

string StdTypeHandler::bitfieldArrayType(string prefixT, size_t size)
{
    return fmt("slpd_%s%s", {prefixT, to_string(size)});
}

std::vector<string> StdTypeHandler::bitfieldArrayTypes()
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
               string uName = toUpper(type.name);
               body << fmt(BitFieldCFmt, {uName, uName, type.name, type.cppType,
                                          to_string(type.size), uName});
           }
       }
   }

   return body;
}

string StdTypeHandler::slpdToAlignedCppType(string stdSlpdType)
{
   if(stdSlpdType == "bool") {return "bool";}

    string kind = {stdSlpdType.at(5)};
    if(contains(Strings{"i", "u"}, kind))
    {
        string prefixT = kind == "u" ? "u" : "";
        int size = strtol(stdSlpdType.substr(6, stdSlpdType.size()).data(), NULL, 10);
        return intType(prefixT, roundUp(size));
    }
    return "";
}

bool StdTypeHandler::isSlpdType(string type)
{
    if(type.size() < 2 && type.size() > 3){ return false;}
    if(!contains(Strings{"s", "i", "u"}, {type[0]})) {return false;}
    for(auto it = type.begin() + 1; it != type.end(); ++it)
    {
        if(!isdigit(*it)) {return false;}
    }
    return true;
}

bool StdTypeHandler::BitType::operator==(const StdTypeHandler::BitType& type) const
{
    return name == type.name &&
           cppType == type.cppType &&
           size == type.size;
}
