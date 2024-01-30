#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <vector>

namespace Utils
{
    using String = std::string;
    using Strings = std::vector<String>;

    /// insert separator between strings
    String sep(String separator, Strings strs);

    String fmt(const String& format, const Strings& args);

    template<typename T, template<class...> class Container>
    Strings fmt(const String& format, const Container<T>& argsPacks)
    {
        Strings result;
        for(const auto& args : argsPacks)
        {
            result += fmt(format, args.toStrings());
        }
        return result;
    }

    template<template<class...> class Container>
    Strings fmt(const String& format, const Container<Strings>& argsPacks)
    {
        Strings result;
        for(const auto& args : argsPacks)
        {
            result += fmt(format, args);
        }
        return result;
    }


    template<typename T, template<class...> class Container>
    Container<T>& operator +=(Container<T>& c, const T& element)
    {
        c.push_back(element);
        return c;
    }

    inline Strings& operator +=(Strings& c, const char* str)
    {
        c.push_back(str);
        return c;
    }

    template<typename Container>
    Container& operator +=(Container& l, const Container& r)
    {
        std::copy(r.begin(), r.end(), std::back_inserter(l));
        return l;
    }

    inline Strings operator <<(const Strings& strs, const String& str)
    {
        Strings tmp = strs;
        tmp.push_back(str);
        return tmp;
    }

    inline Strings operator <<(const String& str, const Strings& strs)
    {
        Strings tmp = {str};
        std::copy(strs.begin(), strs.end(), std::back_inserter(tmp));
        return tmp;
    }

    inline Strings operator <<(const String& str1, const String& str2)
    {
        return {str1, str2};
    }

    inline Strings operator <<(const Strings& strs1, const Strings& strs2)
    {
        Strings tmp = strs1;
        std::copy(strs2.begin(), strs2.end(), std::back_inserter(tmp));
        return tmp;
    }
}

namespace CppConstructs
{
    using namespace Utils;
}

#endif // STRINGUTILS_H
