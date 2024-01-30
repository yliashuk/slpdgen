#include "StringUtils.h"
#include <sstream>
#include <numeric>
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
