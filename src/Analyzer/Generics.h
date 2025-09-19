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
typename Container::iterator findByName(Container& container, const string& name)
{
    auto it = std::find_if(container.begin(), container.end(),
                           [name](auto type) { return type.getName() == name; });
    return it;
}

template<typename Container>
typename std::optional<typename Container::value_type>
findByNameOpt(const Container& container, const string& name)
{
    auto it = std::find_if(container.begin(), container.end(),
                           [name](auto type) { return type.getName() == name; });

    return it != container.end()
            ? std::optional<typename Container::value_type>(*it)
            : std::nullopt;
}

template<typename T>
typename vector<T>::iterator findInVector(vector<T>& DataStruct, T content)
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
findInVector(vector<pair<T,B>>& DataStruct, T content)
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
findInVector(vector<pair<T,B>>& DataStruct, B content)
{
    auto it = std::find_if(DataStruct.begin(),DataStruct.end(),
                           [content](const pair<T,B>& p)
                            {
                                return p.second == content ;
                            });
    return it;
}

template<typename T>
bool vectorIsContained(const vector<T>& DataStruct, T content)
{
    return findInVector(DataStruct, content) != DataStruct.end();
}

template<typename T,typename B>
bool vectorIsContained(const vector<pair<T,B>>& DataStruct, T content)
{
    return findInVector(DataStruct, content) != DataStruct.end();
}


template<typename T>
bool dataStructIsNotContains(const T& dataStruct, string structFieldName)
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
getListDataStruct(vector<T>& structures, string structName)
{
    auto it = std::find_if(structures.begin(),structures.end(),
                           [structName](const T& p)
                            {
                                return p.name == structName ;
                            });
    return it ;
}

template<typename T>
Status checkContent(vector<T>& structures,
                    typename vector<T>::iterator& it,
                    string structName, string structFieldName)
{
    it = getListDataStruct(structures, structName);
    if(it == structures.end())
        return Status::StructNameNotContained;
    if(dataStructIsNotContains(*it, structFieldName))
        return Status::Ok;
    else {
        return Status::DuplField;
    }
}

template<typename T>
static Status checkFieldValueRange(const T& field, uint8_t bitWidth)
{
    auto maxValue = pow(2, bitWidth) - 1;
    return field.second > maxValue ? ValueOutOfRange : Ok;
}

#endif // GENERICS_H
