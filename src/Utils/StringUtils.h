#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <type_traits>
#include "ContainerUtils.h"

namespace Utils
{
    using String = std::string;
    using Strings = std::vector<String>;

    /// insert separator between strings
    String sep(String separator, Strings strs);

    String fmt(const String& format, const Strings& args);

    String& toLower(String& str);
    String& toUpper(String& str);

    template<typename T> Strings Stringifier(const T& t) {return t.toStrings();}

    template<typename T, template<class...> class Container>
    Strings fmt(const String& format, const Container<T>& argsPacks,
                std::function<Strings(T)> toStrings = Stringifier<T>)
    {
        Strings result;
        for (const auto& args : argsPacks)
        {
            result += fmt(format, toStrings(args));
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

    // Dummy fuction, used to select fmt with argsPacks specialization
    std::vector<Strings> _d(const Strings& strs);

    std::ostream& operator<<(std::ostream& os, const Strings& strs);

    Strings&  operator+=(Strings& strs, const String& str);

    Strings   operator<<(const String& str1, const String& str2);

    Strings&  operator<<(Strings& strs, const String& str);
    Strings&& operator<<(Strings&& strs, const String& str);

    Strings&  operator<<(Strings& strs1, const Strings& strs2);
    Strings&& operator<<(Strings&& strs1, const Strings& strs2);
}

namespace CppConstructs
{
    using namespace Utils;
}

#endif // STRINGUTILS_H
