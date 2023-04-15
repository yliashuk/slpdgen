#ifndef GENERICS_H
#define GENERICS_H

#include <vector>
#include <algorithm>

using namespace std;

enum DataStructType
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
    Ok
} Status;



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
typename vector<pair<T,B>>::iterator FindInVector(vector<pair<T,B>>& DataStruct,
                                                  T content)
{
    auto it = std::find_if(DataStruct.begin(),DataStruct.end(),
                           [content](const pair<T,B>& p)
                            {
                                return p.first == content ;
                            });
    return it;
}

template<typename T,typename B>
typename vector<pair<T,B>>::iterator FindInVector(vector<pair<T,B>>& DataStruct,
                                                  B content)
{
    auto it = std::find_if(DataStruct.begin(),DataStruct.end(),
                           [content](const pair<T,B>& p)
                            {
                                return p.second == content ;
                            });
    return it;
}

template<typename T>
bool VectorIsContained(vector<T> DataStruct, T content)
{
    return FindInVector(DataStruct, content) != DataStruct.end();
}

template<typename T,typename B>
bool VectorIsContained(vector<pair<T,B>> DataStruct, T content)
{
    return FindInVector(DataStruct, content) != DataStruct.end();
}


template<typename T>
bool DataStructIsNotContains(pair<string, vector<pair<string,T>>>& DataStruct,
                             string contentFieldName)
{
    auto it = std::find_if(DataStruct.second.begin(),DataStruct.second.end(),
                           [contentFieldName](const pair<string,T>& p)
                            {
                                return p.first == contentFieldName ;
                            });
    return DataStruct.second.end() == it ;
}

template<typename T>
typename vector<pair<string,T>>::iterator
GetListDataStruct(vector<pair<string,T>>& structuresList, string contentDataStructName)
{
    auto it = std::find_if(structuresList.begin(),structuresList.end(),
                           [contentDataStructName](const pair<string,T>& p)
                            {
                                return p.first == contentDataStructName ;
                            });
    return it ;
}

template<typename T>
Status ContentCheck(vector<pair<string,T>>& structuresList,
                    typename vector<pair<string,T>>::iterator& it,
                    string dataStructName, string contentFieldName)
{
    it = GetListDataStruct(structuresList, dataStructName);
    if(it == structuresList.end())
        return Status::StructNameNotContained;
    if(DataStructIsNotContains(*it, contentFieldName))
        return Status::Ok;
    else {
        return Status::DuplField;
    }
}

#endif // GENERICS_H
