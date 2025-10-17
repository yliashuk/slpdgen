#include "SizeExpr.h"
#include <iostream>

Literal::Literal(int64_t value) : _value(value) {}

int64_t Literal::value() const
{
    return _value;
}

SizeExprPtr Literal::Create(int64_t value)
{
    return std::make_shared<Literal>(value);
}

std::string Literal::ToString() const {
    return std::to_string(_value);
}

int64_t Literal::Evaluate(const std::map<std::string, int64_t> &) const
{
    return _value;
}

SizeExprPtr Literal::Clone() const {
    return std::make_shared<Literal>(_value);
}

SizeExprPtr Literal::Expand() const
{
    return Clone();
}

bool Literal::IsZero() const
{
    return _value == 0;
}

bool Literal::IsSymbol() const
{
    return true;
}

bool Literal::IsMultipleOf(int64_t value) const
{
    return _value % value == 0;
}

Variable::Variable(std::string variable) : _variable(variable) {}

SizeExprPtr Variable::Create(const std::string &variable)
{
    return std::make_shared<Variable>(variable);
}

std::string Variable::ToString() const
{
    return _variable;
}

int64_t Variable::Evaluate(const std::map<std::string, int64_t> &vars) const
{
    auto it = vars.find(_variable);
    if (it == vars.end()) {
        throw std::runtime_error("Variable '" + _variable + "' not defined");
    }
    return it->second;
}

SizeExprPtr Variable::Clone() const
{
    return std::make_shared<Variable>(_variable);
}

SizeExprPtr Variable::Expand() const
{
    return Clone();
}

bool Variable::IsZero() const
{
    return _variable == "0";
}

bool Variable::IsSymbol() const
{
    return true;
}

bool Variable::IsMultipleOf(int64_t) const
{
    return true;
}

BinaryOp::BinaryOp(Op op, SizeExprPtr left, SizeExprPtr right)
    : _op(op), _left(left), _right(right) {}

SizeExprPtr BinaryOp::Create(Op op, SizeExprPtr left, SizeExprPtr right) {
    return std::make_shared<BinaryOp>(op, left, right);
}

std::string BinaryOp::ToString() const {
    std::string op_str;
    switch (_op) {
    case Add: op_str = "+"; break;
    case Mul: op_str = "*"; break;
    }
    return "(" + _left->ToString() + " " + op_str + " " + _right->ToString() + ")";
}

int64_t BinaryOp::Evaluate(const std::map<std::string, int64_t> &vars) const {
    int64_t lVal = _left->Evaluate(vars);
    int64_t rVal = _right->Evaluate(vars);
    switch (_op)
    {
        case Add: return lVal + rVal;
        case Mul: return lVal * rVal;
        default: throw std::runtime_error("Unknown operation");
    }
}

SizeExprPtr BinaryOp::Clone() const {
    return std::make_shared<BinaryOp>(_op, _left->Clone(), _right->Clone());
}

SizeExprPtr BinaryOp::Expand() const
{
    auto lExpanded = _left->Expand();
    auto rExpanded= _right->Expand();

    //std::cout << "Expand: " << lExpanded->ToString() <<'|' << _op << '|' << rExpanded->ToString() << std::endl;
    switch (_op)
    {
        case Add: return ExpandAdd(lExpanded, rExpanded);
        case Mul: return ExpandMul(lExpanded, rExpanded);
        default: throw std::runtime_error("Unknown operation");
    }

    return BinaryOp::Create(_op, lExpanded, rExpanded);
}

bool BinaryOp::IsZero() const
{
    if(_op == Mul && (_left->IsZero() || _right->IsZero())) {
        return true;
    }
    return _left->IsZero() && _right->IsZero();
}

bool BinaryOp::IsSymbol() const
{
    return false;
}

bool BinaryOp::IsMultipleOf(int64_t value) const
{
    auto expr = Expand();
    auto op = std::dynamic_pointer_cast<BinaryOp>(expr);
    if(op) {
        return op->_left->IsMultipleOf(value) && op->_right->IsMultipleOf(value);
    }
    return expr->IsMultipleOf(value);
}

SizeExprPtr BinaryOp::ExpandAdd(SizeExprPtr left, SizeExprPtr right) const
{
    if(left->IsSymbol() && right->IsSymbol()) {
        return ExpandAddSymbolSymbol(left, right);
    }

    auto lOp = std::dynamic_pointer_cast<BinaryOp>(left);
    auto rOp = std::dynamic_pointer_cast<BinaryOp>(right);

    auto hasSymbol = left->IsSymbol() || right->IsSymbol();

    if((IsAddOp(lOp) || IsAddOp(rOp)) && hasSymbol)
    {
        auto op = IsAddOp(lOp) ? lOp : rOp;
        auto literal = IsAddOp(lOp) ? right : left;
        return ExpandAddAddSymbol(op, literal);
    }

    if(IsAddOp(lOp) && IsAddOp(rOp))
    {
        return ExpandAdd2Add(lOp, rOp);
    }

    return BinaryOp::Create(_op, left, right);
}

SizeExprPtr BinaryOp::ExpandMul(SizeExprPtr left, SizeExprPtr right) const
{
    //std::cout << "ExpandMul: " << _left->ToString() << " " << right->ToString() << std::endl;
    if(left->IsSymbol() && right->IsSymbol()) {
        return ExpandMulSymbolSymbol(left, right);
    }

    auto lOp = std::dynamic_pointer_cast<BinaryOp>(left);
    auto rOp = std::dynamic_pointer_cast<BinaryOp>(right);

    if(IsAddOp(lOp) && IsAddOp(rOp)) {
        return ExpandMul2Add(lOp, rOp);
    }

    if(IsMulOp(lOp) && IsMulOp(rOp)) {
        return ExpandMul2Mul(lOp, rOp);
    }

    if((IsAddOp(lOp) || IsAddOp(rOp)) && (IsMulOp(lOp) || IsMulOp(rOp))) {
        return ExpandMulAddMul(lOp, rOp);
    }

    auto hasSymbol = left->IsSymbol() || right->IsSymbol();

    if((IsAddOp(lOp) || IsAddOp(rOp)) && hasSymbol)
    {
        auto op = IsAddOp(lOp) ? lOp : rOp;
        auto symbol = IsAddOp(lOp) ? right : left;
        return ExpandMulAddSymbol(op, symbol);
    }

    if((IsMulOp(lOp) || IsMulOp(rOp)) && hasSymbol)
    {
        auto op = IsMulOp(lOp) ? lOp : rOp;
        auto symbol = IsMulOp(lOp) ? right : left;
        return ExpandMulMulSymbol(op, symbol);
    }

    return BinaryOp::Create(_op, left, right);
}

SizeExprPtr BinaryOp::ExpandAdd2Add(BinOp left, BinaryOp::BinOp right)
{
    SizeExprPtr sum;

    auto rLOp = std::dynamic_pointer_cast<BinaryOp>(right->_left);
    if(IsAddOp(rLOp)) { sum = ExpandAdd2Add(left, rLOp); }
    else { sum = ExpandAddAddSymbol(left, right->_left); }

    auto rROp = std::dynamic_pointer_cast<BinaryOp>(right->_right);
    auto sumOp = std::dynamic_pointer_cast<BinaryOp>(sum);
    if(sumOp && IsAddOp(rROp)) { return ExpandAdd2Add(sumOp, rROp); }
    else if(sumOp) { return ExpandAddAddSymbol(sumOp, right->_right); }

    return BinaryOp::Create(Add, sum, right->_right);
}

SizeExprPtr BinaryOp::ExpandAddAddSymbol(BinOp op, SizeExprPtr symbol)
{
    auto lLit = std::dynamic_pointer_cast<Literal>(op->_left);
    auto rLit = std::dynamic_pointer_cast<Literal>(op->_right);
    auto literal = std::dynamic_pointer_cast<Literal>(symbol);

    if(literal && (lLit || rLit))
    {
        auto lit = lLit ? lLit : rLit;
        auto op1 = ExpandAddSymbolSymbol(lit, literal);
        auto op2 = lLit ? op->_right : op->_left;
        return BinaryOp::Create(Add, op1, op2);
    }

    auto lOp = std::dynamic_pointer_cast<BinaryOp>(op->_left);
    auto rOp = std::dynamic_pointer_cast<BinaryOp>(op->_right);
    if(literal && (IsAddOp(lOp) || IsAddOp(rOp)))
    {
        auto bOp = IsAddOp(lOp) ? lOp : rOp;
        auto& editableOp = IsAddOp(lOp) ? op->_left : op->_right;
        editableOp = ExpandAddAddSymbol(bOp, literal);
        return op;
    }

    return BinaryOp::Create(Add, op, symbol);
}

SizeExprPtr BinaryOp::ExpandAddSymbolSymbol(SizeExprPtr left, SizeExprPtr right)
{
    auto lLit = std::dynamic_pointer_cast<Literal>(left);
    auto rLit = std::dynamic_pointer_cast<Literal>(right);

    if(lLit && rLit) {
        return Literal::Create(lLit->value() + rLit->value());
    }
    return BinaryOp::Create(Add, left, right);
}

SizeExprPtr BinaryOp::ExpandMul2Add(BinOp left, BinOp right)
{
    auto sum1 = ExpandMulAddSymbol(left, right->_left);
    auto sum2 = ExpandMulAddSymbol(left, right->_right);
    return BinaryOp::Create(Add, sum1, sum2);
}

SizeExprPtr BinaryOp::ExpandMul2Mul(BinOp left, BinOp right)
{
    auto mul = ExpandMulMulSymbol(left, right->_left);

    auto lOp = std::dynamic_pointer_cast<BinaryOp>(mul);
    if(lOp) {
        return ExpandMulMulSymbol(lOp, right->_right);
    }
    return BinaryOp::Create(Mul, mul, right->_right);
}

SizeExprPtr BinaryOp::ExpandMulAddMul(BinOp left, BinOp right)
{
    auto addOp = IsAddOp(left) ? left : right;
    auto mulOp = IsMulOp(left) ? left : right;

    auto lMul = ExpandMulMulSymbol(mulOp, addOp->_left);
    auto rMul = ExpandMulMulSymbol(mulOp, addOp->_right);
    return BinaryOp::Create(Add, lMul, rMul);
}

SizeExprPtr BinaryOp::ExpandMulMulSymbol(BinOp op, SizeExprPtr symbol)
{
    auto lLit = std::dynamic_pointer_cast<Literal>(op->_left);
    auto lit = lLit ? op->_left : op->_right;
    auto mul = ExpandMulSymbolSymbol(lit, symbol);
    return BinaryOp::Create(Mul, mul, lLit ? op->_right : op->_left);
}

SizeExprPtr BinaryOp::ExpandMulAddSymbol(BinOp op, SizeExprPtr symbol)
{
    SizeExprPtr lMul;
    auto lOp = std::dynamic_pointer_cast<BinaryOp>(op->_left);
    if(IsAddOp(lOp)) { lMul = ExpandMulAddSymbol(lOp, symbol); }
    else { lMul = ExpandMulSymbolSymbol(op->_left, symbol); }

    SizeExprPtr rMul;
    auto rOp = std::dynamic_pointer_cast<BinaryOp>(op->_right);
    if(IsAddOp(rOp)) { rMul = ExpandMulAddSymbol(rOp, symbol); }
    else { rMul = ExpandMulSymbolSymbol(op->_right, symbol); }

    return BinaryOp::Create(Add, lMul, rMul);
}

SizeExprPtr BinaryOp::ExpandMulSymbolSymbol(SizeExprPtr left, SizeExprPtr right)
{
    if(left->IsZero() || right->IsZero()) {
        return Literal::Create(0);
    }

    auto lLit = std::dynamic_pointer_cast<Literal>(left);
    auto rLit = std::dynamic_pointer_cast<Literal>(right);

    if(lLit && rLit) {
        return Literal::Create(lLit->value() * rLit->value());
    }
    return BinaryOp::Create(Mul, left, right);
}

bool BinaryOp::IsAddOp(BinOp Op){return Op && Op->_op == Add;}
bool BinaryOp::IsMulOp(BinOp op){return op && op->_op == Mul;}

SizeExprPtr operator+(SizeExprPtr left, SizeExprPtr right) {
    return BinaryOp::Create(BinaryOp::Add, left->Clone(), right->Clone());
}

SizeExprPtr operator*(SizeExprPtr left, SizeExprPtr right) {
    return BinaryOp::Create(BinaryOp::Mul, left->Clone(), right->Clone());
}

SizeExprPtr operator+(int64_t left, SizeExprPtr right) {
    return Literal::Create(left) + right;
}

SizeExprPtr operator+(SizeExprPtr left, int64_t right) {
    return left + Literal::Create(right);
}

SizeExprPtr operator+(const char *left, SizeExprPtr right) {
    return Variable::Create(left) + right;
}

SizeExprPtr operator+(SizeExprPtr left, const char *right) {
    return left + Variable::Create(right);
}

SizeExprPtr operator*(int64_t left, SizeExprPtr right) {
    return Literal::Create(left) + right;
}

SizeExprPtr operator*(SizeExprPtr left, int64_t right) {
    return left + Literal::Create(right);
}

SizeExprPtr operator*(const char *left, SizeExprPtr right) {
    return Variable::Create(left) * right;
}

SizeExprPtr operator*(SizeExprPtr left, const char *right) {
    return left * Variable::Create(right);
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
    return Literal::Create(static_cast<int64_t>(value));
}

SizeExprPtr operator""_var(const char* name, std::size_t) {
    return Variable::Create(std::string(name));
}
