#ifndef STRUCTCPP_H
#define STRUCTCPP_H
#include <vector>
#include "Utils/StringUtils.h"
#include "Utils/ContainerUtils.h"

namespace CppConstructs
{
    class StructCpp
    {
    public:
        struct Field
        {
            String type;
            String name;
            String bitWidth;
            String comment;

            Field(String t, String n, String bw = "", String c = "")
                : type(t), name(n), bitWidth(bw), comment(c){}

            Strings toStrings() const {
                return {type, name, bitWidth, comment};
            }
        };

        using Fields = std::vector<Field>;

        StructCpp() = default;
        StructCpp(const String &structName);
        void setTypeDef(bool state);

        void setName(const String& structName);
        String getName() const;

        void addField(const Field& field);
        void addFields(const Fields& fields);

        Fields getFields();

        Strings declaration() const;

    private:
        bool _hasTypeDef = false;
        std::string _structName;
        Fields _fields;
    };
}
#endif // STRUCTCPP_H
