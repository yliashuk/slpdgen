#include "SizeExpr.h"
#include <iostream>

Literal::Literal(int64_t value) : _value(value) {}

int64_t Literal::value() const
{
    return _value;
}

SizeExprPtr Literal::create(int64_t value)
{
    return std::make_shared<Literal>(value);
}

std::string Literal::toString() const {
    return std::to_string(_value);
}

int64_t Literal::evaluate(const std::map<std::string, int64_t> &) const
{
    return _value;
}

SizeExprPtr Literal::clone() const {
    return std::make_shared<Literal>(_value);
}

SizeExprPtr Literal::expand() const
{
    return clone();
}

bool Literal::isZero() const
{
    return _value == 0;
}

bool Literal::isSymbol() const
{
    return true;
}

bool Literal::isMultipleOf(int64_t value) const
{
    return _value % value == 0;
}

Variable::Variable(std::string variable) : _variable(variable) {}

SizeExprPtr Variable::create(const std::string &variable)
{
    return std::make_shared<Variable>(variable);
}

std::string Variable::toString() const
{
    return _variable;
}

int64_t Variable::evaluate(const std::map<std::string, int64_t> &vars) const
{
    auto it = vars.find(_variable);
    if (it == vars.end()) {
        throw std::runtime_error("Variable '" + _variable + "' not defined");
    }
    return it->second;
}

SizeExprPtr Variable::clone() const
{
    return std::make_shared<Variable>(_variable);
}

SizeExprPtr Variable::expand() const
{
    return clone();
}

bool Variable::isZero() const
{
    return _variable == "0";
}

bool Variable::isSymbol() const
{
    return true;
}

bool Variable::isMultipleOf(int64_t) const
{
    return true;
}

BinaryOp::BinaryOp(Op op, SizeExprPtr left, SizeExprPtr right)
    : _op(op), _left(left), _right(right) {}

SizeExprPtr BinaryOp::create(Op op, SizeExprPtr left, SizeExprPtr right) {
    return std::make_shared<BinaryOp>(op, left, right);
}

std::string BinaryOp::toString() const {
    std::string op_str;
    switch (_op) {
    case Add: op_str = "+"; break;
    case Mul: op_str = "*"; break;
    }
    return "(" + _left->toString() + " " + op_str + " " + _right->toString() + ")";
}

int64_t BinaryOp::evaluate(const std::map<std::string, int64_t> &vars) const {
    int64_t lVal = _left->evaluate(vars);
    int64_t rVal = _right->evaluate(vars);
    switch (_op)
    {
        case Add: return lVal + rVal;
        case Mul: return lVal * rVal;
        default: throw std::runtime_error("Unknown operation");
    }
}

SizeExprPtr BinaryOp::clone() const {
    return std::make_shared<BinaryOp>(_op, _left->clone(), _right->clone());
}

SizeExprPtr BinaryOp::expand() const
{
    auto lExpanded = _left->expand();
    auto rExpanded= _right->expand();

    //std::cout << "Expand: " << lExpanded->ToString() <<'|' << _op << '|' << rExpanded->ToString() << std::endl;
    switch (_op)
    {
        case Add: return expandAdd(lExpanded, rExpanded);
        case Mul: return expandMul(lExpanded, rExpanded);
        default: throw std::runtime_error("Unknown operation");
    }

    return BinaryOp::create(_op, lExpanded, rExpanded);
}

bool BinaryOp::isZero() const
{
    if(_op == Mul && (_left->isZero() || _right->isZero())) {
        return true;
    }
    return _left->isZero() && _right->isZero();
}

bool BinaryOp::isSymbol() const
{
    return false;
}

bool BinaryOp::isMultipleOf(int64_t value) const
{
    auto expr = expand();
    auto op = std::dynamic_pointer_cast<BinaryOp>(expr);
    if(op) {
        return op->_left->isMultipleOf(value) && op->_right->isMultipleOf(value);
    }
    return expr->isMultipleOf(value);
}

SizeExprPtr BinaryOp::expandAdd(SizeExprPtr left, SizeExprPtr right) const
{
    if(left->isSymbol() && right->isSymbol()) {
        return expandAddSymbolSymbol(left, right);
    }

    auto lOp = std::dynamic_pointer_cast<BinaryOp>(left);
    auto rOp = std::dynamic_pointer_cast<BinaryOp>(right);

    auto hasSymbol = left->isSymbol() || right->isSymbol();

    if((isAddOp(lOp) || isAddOp(rOp)) && hasSymbol)
    {
        auto op = isAddOp(lOp) ? lOp : rOp;
        auto literal = isAddOp(lOp) ? right : left;
        return expandAddAddSymbol(op, literal);
    }

    if(isAddOp(lOp) && isAddOp(rOp))
    {
        return expandAdd2Add(lOp, rOp);
    }

    return BinaryOp::create(_op, left, right);
}

SizeExprPtr BinaryOp::expandMul(SizeExprPtr left, SizeExprPtr right) const
{
    if(left->isSymbol() && right->isSymbol()) {
        return expandMulSymbolSymbol(left, right);
    }

    auto lOp = std::dynamic_pointer_cast<BinaryOp>(left);
    auto rOp = std::dynamic_pointer_cast<BinaryOp>(right);

    if(isAddOp(lOp) && isAddOp(rOp)) {
        return expandMul2Add(lOp, rOp);
    }

    if(isMulOp(lOp) && isMulOp(rOp)) {
        return expandMul2Mul(lOp, rOp);
    }

    if((isAddOp(lOp) || isAddOp(rOp)) && (isMulOp(lOp) || isMulOp(rOp))) {
        return expandMulAddMul(lOp, rOp);
    }

    auto hasSymbol = left->isSymbol() || right->isSymbol();

    if((isAddOp(lOp) || isAddOp(rOp)) && hasSymbol)
    {
        auto op = isAddOp(lOp) ? lOp : rOp;
        auto symbol = isAddOp(lOp) ? right : left;
        return expandMulAddSymbol(op, symbol);
    }

    if((isMulOp(lOp) || isMulOp(rOp)) && hasSymbol)
    {
        auto op = isMulOp(lOp) ? lOp : rOp;
        auto symbol = isMulOp(lOp) ? right : left;
        return expandMulMulSymbol(op, symbol);
    }

    return BinaryOp::create(_op, left, right);
}

SizeExprPtr BinaryOp::expandAdd2Add(BinOp left, BinaryOp::BinOp right)
{
    SizeExprPtr sum;

    auto rLOp = std::dynamic_pointer_cast<BinaryOp>(right->_left);
    if(isAddOp(rLOp)) { sum = expandAdd2Add(left, rLOp); }
    else { sum = expandAddAddSymbol(left, right->_left); }

    auto rROp = std::dynamic_pointer_cast<BinaryOp>(right->_right);
    auto sumOp = std::dynamic_pointer_cast<BinaryOp>(sum);
    if(sumOp && isAddOp(rROp)) { return expandAdd2Add(sumOp, rROp); }
    else if(sumOp) { return expandAddAddSymbol(sumOp, right->_right); }

    return BinaryOp::create(Add, sum, right->_right);
}

SizeExprPtr BinaryOp::expandAddAddSymbol(BinOp op, SizeExprPtr symbol)
{
    auto lLit = std::dynamic_pointer_cast<Literal>(op->_left);
    auto rLit = std::dynamic_pointer_cast<Literal>(op->_right);
    auto literal = std::dynamic_pointer_cast<Literal>(symbol);

    if(literal && (lLit || rLit))
    {
        auto lit = lLit ? lLit : rLit;
        auto op1 = expandAddSymbolSymbol(lit, literal);
        auto op2 = lLit ? op->_right : op->_left;
        return BinaryOp::create(Add, op1, op2);
    }

    auto lOp = std::dynamic_pointer_cast<BinaryOp>(op->_left);
    auto rOp = std::dynamic_pointer_cast<BinaryOp>(op->_right);
    if(literal && (isAddOp(lOp) || isAddOp(rOp)))
    {
        auto bOp = isAddOp(lOp) ? lOp : rOp;
        auto& editableOp = isAddOp(lOp) ? op->_left : op->_right;
        editableOp = expandAddAddSymbol(bOp, literal);
        return op;
    }

    return BinaryOp::create(Add, op, symbol);
}

SizeExprPtr BinaryOp::expandAddSymbolSymbol(SizeExprPtr left, SizeExprPtr right)
{
    auto lLit = std::dynamic_pointer_cast<Literal>(left);
    auto rLit = std::dynamic_pointer_cast<Literal>(right);

    if(lLit && rLit) {
        return Literal::create(lLit->value() + rLit->value());
    }
    return BinaryOp::create(Add, left, right);
}

SizeExprPtr BinaryOp::expandMul2Add(BinOp left, BinOp right)
{
    auto sum1 = expandMulAddSymbol(left, right->_left);
    auto sum2 = expandMulAddSymbol(left, right->_right);
    return BinaryOp::create(Add, sum1, sum2);
}

SizeExprPtr BinaryOp::expandMul2Mul(BinOp left, BinOp right)
{
    auto mul = expandMulMulSymbol(left, right->_left);

    auto lOp = std::dynamic_pointer_cast<BinaryOp>(mul);
    if(lOp) {
        return expandMulMulSymbol(lOp, right->_right);
    }
    return BinaryOp::create(Mul, mul, right->_right);
}

SizeExprPtr BinaryOp::expandMulAddMul(BinOp left, BinOp right)
{
    auto addOp = isAddOp(left) ? left : right;
    auto mulOp = isMulOp(left) ? left : right;

    auto lMul = expandMulMulSymbol(mulOp, addOp->_left);
    auto rMul = expandMulMulSymbol(mulOp, addOp->_right);
    return BinaryOp::create(Add, lMul, rMul);
}

SizeExprPtr BinaryOp::expandMulMulSymbol(BinOp op, SizeExprPtr symbol)
{
    auto lLit = std::dynamic_pointer_cast<Literal>(op->_left);
    auto lit = lLit ? op->_left : op->_right;
    auto mul = expandMulSymbolSymbol(lit, symbol);
    return BinaryOp::create(Mul, mul, lLit ? op->_right : op->_left);
}

SizeExprPtr BinaryOp::expandMulAddSymbol(BinOp op, SizeExprPtr symbol)
{
    SizeExprPtr lMul;
    auto lOp = std::dynamic_pointer_cast<BinaryOp>(op->_left);
    if(isAddOp(lOp)) { lMul = expandMulAddSymbol(lOp, symbol); }
    else { lMul = expandMulSymbolSymbol(op->_left, symbol); }

    SizeExprPtr rMul;
    auto rOp = std::dynamic_pointer_cast<BinaryOp>(op->_right);
    if(isAddOp(rOp)) { rMul = expandMulAddSymbol(rOp, symbol); }
    else { rMul = expandMulSymbolSymbol(op->_right, symbol); }

    return BinaryOp::create(Add, lMul, rMul);
}

SizeExprPtr BinaryOp::expandMulSymbolSymbol(SizeExprPtr left, SizeExprPtr right)
{
    if(left->isZero() || right->isZero()) {
        return Literal::create(0);
    }

    auto lLit = std::dynamic_pointer_cast<Literal>(left);
    auto rLit = std::dynamic_pointer_cast<Literal>(right);

    if(lLit && rLit) {
        return Literal::create(lLit->value() * rLit->value());
    }
    return BinaryOp::create(Mul, left, right);
}

bool BinaryOp::isAddOp(BinOp Op){return Op && Op->_op == Add;}
bool BinaryOp::isMulOp(BinOp op){return op && op->_op == Mul;}

SizeExprPtr operator+(SizeExprPtr left, SizeExprPtr right) {
    return BinaryOp::create(BinaryOp::Add, left->clone(), right->clone());
}

SizeExprPtr operator*(SizeExprPtr left, SizeExprPtr right) {
    return BinaryOp::create(BinaryOp::Mul, left->clone(), right->clone());
}

SizeExprPtr operator+(int64_t left, SizeExprPtr right) {
    return Literal::create(left) + right;
}

SizeExprPtr operator+(SizeExprPtr left, int64_t right) {
    return left + Literal::create(right);
}

SizeExprPtr operator+(const char *left, SizeExprPtr right) {
    return Variable::create(left) + right;
}

SizeExprPtr operator+(SizeExprPtr left, const char *right) {
    return left + Variable::create(right);
}

SizeExprPtr operator*(int64_t left, SizeExprPtr right) {
    return Literal::create(left) + right;
}

SizeExprPtr operator*(SizeExprPtr left, int64_t right) {
    return left + Literal::create(right);
}

SizeExprPtr operator*(const char *left, SizeExprPtr right) {
    return Variable::create(left) * right;
}

SizeExprPtr operator*(SizeExprPtr left, const char *right) {
    return left * Variable::create(right);
}

SizeExprPtr &operator+=(SizeExprPtr &left, SizeExprPtr right) {
    left = left + right;
    return left;
}

SizeExprPtr &operator+=(SizeExprPtr &left, int64_t right) {
    left = left + right;
    return left;
}

SizeExprPtr &operator+=(SizeExprPtr &left, const char *right) {
    left = left + right;
    return left;
}

SizeExprPtr &operator*=(SizeExprPtr &left, SizeExprPtr right) {
    left = left * right;
    return left;
}

SizeExprPtr &operator*=(SizeExprPtr &left, int64_t right) {
    left = left * right;
    return left;
}

SizeExprPtr &operator*=(SizeExprPtr &left, const char *right) {
    left = left * right;
    return left;
}

SizeExprPtr operator""_lit(unsigned long long value) {
    return Literal::create(static_cast<int64_t>(value));
}

SizeExprPtr operator""_var(const char* name, std::size_t) {
    return Variable::create(std::string(name));
}
