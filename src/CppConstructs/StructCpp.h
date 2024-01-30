#ifndef STRUCTCPP_H
#define STRUCTCPP_H
#include <vector>
#include "CommonCpp.h"
#include "Utils/StringUtils.h"

namespace CppConstructs
{
    class StructCpp
    {
    public:
        struct Field
        {
            string type;
            string name;
            string comment;

            Field(const string& t, const string& n, const string& c = "")
                : type(t), name(n), comment(c){}

            Strings toStrings() const {
                return {type, name, comment};
            }
        };

        using Fields = std::vector<Field>;

        StructCpp() = default;
        StructCpp(const std::string& structName);
        void SetTypeDef(bool state);

        void SetName(const std::string& structName);
        string GetName() const;

        void AddField(const Field& field);
        void AddFields(const Fields& fields);

        Strings Declaration() const;

    private:
        bool _hasTypeDef = false;
        string _structName;
        Fields _fields;
    };
}
#endif // STRUCTCPP_H
