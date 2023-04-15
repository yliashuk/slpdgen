#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <string>
#include <vector>

using namespace std;

class Polynomial
{
public:
    Polynomial();
    Polynomial(int &&constPart);
    Polynomial(string dynamicPart);
    Polynomial(int constPart, string dynamicPart);
    Polynomial(Polynomial &&polinomial);

    Polynomial(const Polynomial &polinomial);
    Polynomial(const int &&value);

    void operator=(Polynomial &polinomial);
    void operator=(Polynomial &&polinomial);
    void operator=(int value);
    void operator=(string var);

    void operator+=(Polynomial &polinomial);
    void operator+=(Polynomial &&polinomial);
    void operator+=(int value);
    void operator+=(string var);

    Polynomial operator+(Polynomial polinomial);

    Polynomial operator *(int value);
    Polynomial operator *(string var);

    string Get() const;
    int GetConstPart() const;
    string GetDynamicPart() const;
private:
    int _constPart{};
    string _dynamicPart{};
};

#endif // POLYNOMIAL_H
