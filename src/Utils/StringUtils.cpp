#include "StringUtils.h"
#include <numeric>
#include <algorithm>

using namespace Utils;

String Utils::sep(String separator, Strings strs)
{
    if(strs.empty()) {
        return "";
    }
    return std::accumulate(strs.begin() + 1, strs.end(), strs.front(),
                           [separator](String acc, String s) {
                                return acc + separator + s;
                           });
}

String Utils::fmt(const String &format, const Strings &args)
{
    std::string result;
    size_t arg_index = 0;

    for (size_t i = 0; i < format.length(); ++i)
    {
        if (format[i] == '%' && i + 1 < format.length())
        {
            if (format[i + 1] == 's')
            {
                if (arg_index < args.size())
                {
                    result += args[arg_index++];
                }
                i++; // Skip 's'
            } else if (format[i + 1] == '{')
            {
                size_t end = format.find('}', i + 2);
                if (end != std::string::npos) {
                    std::string block = format.substr(i + 2, end - (i + 2));
                    size_t pos = block.find("%s");

                    if (pos != std::string::npos && arg_index < args.size())
                    {
                        if (!args[arg_index].empty())
                        {
                            block.replace(pos, 2, args[arg_index]);
                            result += block;
                        }
                        arg_index++;
                    }
                    i = end;
                }
            }
        } else
        {
            result += format[i];
        }
    }
    return result;
}

String& Utils::toLower(String& str)
{
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return str;
}

String& Utils::toUpper(String& str)
{
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return str;
}

std::vector<Strings> Utils::_d(const Strings &strs)
{
    std::vector<Strings> tmp;
    for(const auto& str : strs)
    {
        tmp += {str};
    }
    return tmp;
}

Strings&& Utils::operator<<(Strings &&strs, const String &str)
{
    return std::move(strs += str);
}

Strings&& Utils::operator<<(Strings &&strs1, const Strings &strs2)
{
    return std::move(strs1 += strs2);
}

std::ostream& Utils::operator<<(std::ostream &os, const Strings &strs)
{
    for(auto &str : strs)
    {
        os << str << std::endl;
    }
    return os;
}

Strings &Utils::operator+=(Strings &strs, const String &str)
{
    strs.push_back(str);
    return strs;
}

Strings Utils::operator<<(const String &str1, const String &str2)
{
    return {str1, str2};
}

Strings& Utils::operator<<(Strings &strs1, const Strings &strs2)
{
    return strs1 += strs2;
}

Strings &Utils::operator<<(Strings &strs, const String &str)
{
    return strs += str;
}
