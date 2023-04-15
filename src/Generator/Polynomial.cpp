#include "Polynomial.h"


Polynomial::Polynomial(){}

Polynomial::Polynomial(int &&constPart): _constPart(constPart){}

Polynomial::Polynomial(string dynamicPart): _dynamicPart(dynamicPart){}

Polynomial::Polynomial(int constPart, string dynamicPart)
    :_constPart(constPart), _dynamicPart(dynamicPart){}

Polynomial::Polynomial(const Polynomial &polinomial)
{
    this->_constPart = polinomial.GetConstPart();
    this->_dynamicPart = polinomial.GetDynamicPart();
}

Polynomial::Polynomial(Polynomial &&polinomial)
{
    this->_constPart = polinomial.GetConstPart();
    this->_dynamicPart = polinomial.GetDynamicPart();
}

Polynomial::Polynomial(const int &&value): _constPart(value){}


void Polynomial::operator=(Polynomial &polinomial)
{
    this->_constPart = polinomial.GetConstPart();
    this->_dynamicPart = polinomial.GetDynamicPart();
}

void Polynomial::operator=(Polynomial &&polinomial)
{
    this->_constPart = polinomial.GetConstPart();
    this->_dynamicPart = polinomial.GetDynamicPart();
}

void Polynomial::operator=(int value)
{
    this->_constPart = value;
}

void Polynomial::operator=(string var)
{
    this->_dynamicPart = var;
}

void Polynomial::operator+=(Polynomial &polinomial)
{
    this->_constPart += polinomial.GetConstPart();
    if(!this->_dynamicPart.empty() && !polinomial.GetDynamicPart().empty())
        this->_dynamicPart += " + ";
    this->_dynamicPart += polinomial.GetDynamicPart();
}

Polynomial Polynomial::operator+(Polynomial polinomial)
{
    Polynomial buf;
    buf += *this;
    buf += polinomial;
    return buf;
}

void Polynomial::operator+=(Polynomial &&polinomial)
{
    this->_constPart += polinomial.GetConstPart();
    this->_dynamicPart+= polinomial.GetDynamicPart();
}

Polynomial Polynomial::operator*(int value)
{
    if(!this->GetDynamicPart().empty()) {
        return Polynomial(this->GetConstPart() * value, "(" + this->GetDynamicPart() +
                          ") * " + to_string(value));
    } else
        return Polynomial(this->GetConstPart() * value);
}

Polynomial Polynomial::operator*(string var)
{
    if(this->GetDynamicPart().empty() && this->GetConstPart() == 0)
        return Polynomial();

    if(this->GetDynamicPart().empty() && this->GetConstPart() != 0)
        return Polynomial(to_string(this->GetConstPart()) + " * " + var);

    if(!this->GetDynamicPart().empty() && this->GetConstPart() == 0)
        return Polynomial("(" + this->GetDynamicPart() + ") * " + var);

    else {
        return Polynomial("(" +to_string(this->GetConstPart()) + " + " +
                          this->GetDynamicPart() + ") * " + var);
    }
}

void Polynomial::operator+=(int value)
{
    this->_constPart += value;
}

void Polynomial::operator+=(string var)
{
    if(!_dynamicPart.empty())
        _dynamicPart += " + ";
    _dynamicPart += var;
}


string Polynomial::Get() const
{
    string polynomial;
    if(_constPart || _dynamicPart.empty())
        polynomial = to_string(_constPart);
    if(!polynomial.empty() && !_dynamicPart.empty())
        polynomial += " + ";

    polynomial += _dynamicPart;
    return polynomial;
}

int Polynomial::GetConstPart() const
{
    return _constPart;
}

string Polynomial::GetDynamicPart() const
{
    return _dynamicPart;
}
