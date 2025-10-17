#ifndef GENERICS_H
#define GENERICS_H

#include <cmath>
#include <vector>
#include <optional>
#include <algorithm>

using namespace std;

enum StructType
{
    SmplStr,
    Hdr,
    Pckt
};

enum EnumType
{
    SmplEnm,
    Tp,
    Cd
};

typedef enum
{
    DuplStruct,
    DuplField,
    EnumFieldNotContained,
    StructNameNotContained,
    FieldNameNotContained,
    DefiningVarNotContained,
    ValueOutOfRange,
    Ok
} Status;

template<typename Container>
typename Container::iterator FindByName(Container& container, const string& name)
{
    auto it = std::find_if(container.begin(), container.end(),
                           [name](auto type) { return type.GetName() == name; });
    return it;
}

template<typename Container>
typename std::optional<typename Container::value_type>
FindByNameOpt(const Container& container, const string& name)
{
    auto it = std::find_if(container.begin(), container.end(),
                           [name](auto type) { return type.GetName() == name; });

    return it != container.end()
            ? std::optional<typename Container::value_type>(*it)
            : std::nullopt;
}

template<typename T>
typename vector<T>::iterator FindInVector(vector<T>& DataStruct, T content)
{
    auto it = std::find_if(DataStruct.begin(), DataStruct.end(),
                           [content](const T& p)
                            {
                                return p == content ;
                            });
    return it;
}

template<typename T,typename B>
typename vector<pair<T,B>>::iterator
FindInVector(vector<pair<T,B>>& DataStruct, T content)
{
    auto it = std::find_if(DataStruct.begin(),DataStruct.end(),
                           [content](const pair<T,B>& p)
                            {
                                return p.first == content ;
                            });
    return it;
}

template<typename T,typename B>
typename vector<pair<T,B>>::iterator
FindInVector(vector<pair<T,B>>& DataStruct, B content)
{
    auto it = std::find_if(DataStruct.begin(),DataStruct.end(),
                           [content](const pair<T,B>& p)
                            {
                                return p.second == content ;
                            });
    return it;
}

template<typename T>
bool VectorIsContained(const vector<T>& DataStruct, T content)
{
    return FindInVector(DataStruct, content) != DataStruct.end();
}

template<typename T,typename B>
bool VectorIsContained(const vector<pair<T,B>>& DataStruct, T content)
{
    return FindInVector(DataStruct, content) != DataStruct.end();
}


template<typename T>
bool DataStructIsNotContains(const T& dataStruct, string structFieldName)
{
    auto it = std::find_if(dataStruct.fields.begin(),dataStruct.fields.end(),
                           [structFieldName](const auto& p)
                            {
                                return p.first == structFieldName ;
                            });
    return dataStruct.fields.end() == it ;
}

template<typename T>
typename vector<T>::iterator
GetListDataStruct(vector<T>& structures, string structName)
{
    auto it = std::find_if(structures.begin(),structures.end(),
                           [structName](const T& p)
                            {
                                return p.name == structName ;
                            });
    return it ;
}

template<typename T>
Status ContentCheck(vector<T>& structures,
                    typename vector<T>::iterator& it,
                    string structName, string structFieldName)
{
    it = GetListDataStruct(structures, structName);
    if(it == structures.end())
        return Status::StructNameNotContained;
    if(DataStructIsNotContains(*it, structFieldName))
        return Status::Ok;
    else {
        return Status::DuplField;
    }
}

template<typename T>
static Status CheckFieldValueRange(const T& field, uint8_t bitWidth)
{
    auto maxValue = pow(2, bitWidth) - 1;
    return field.second > maxValue ? ValueOutOfRange : Ok;
}

#endif // GENERICS_H
