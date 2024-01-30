#ifndef CONTAINERUTILS_H
#define CONTAINERUTILS_H

namespace Utils
{
    template<typename T, template<class...> class Container>
    Container<T>& operator +=(Container<T>& c, const T& element)
    {
        c.push_back(element);
        return c;
    }

    template<typename Container>
    Container& operator +=(Container& l, const Container& r)
    {
        std::copy(r.begin(), r.end(), std::back_inserter(l));
        return l;
    }

    template<typename T, template<class...> class Container>
    bool contains(const Container<T>& c, T val)
    {
        return std::find(c.begin(), c.end(), val) != c.end();
    }

}
#endif // CONTAINERUTILS_H
