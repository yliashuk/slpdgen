#ifndef SIZEEXPR_H
#define SIZEEXPR_H

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <cstdint>

class SizeExpr;
using SizeExprPtr = std::shared_ptr<SizeExpr>;

class SizeExpr {
public:
    SizeExpr() = default;
    virtual ~SizeExpr() = default;

    // Виртуальные методы
    virtual std::string toString() const = 0;
    virtual int64_t evaluate(const std::map<std::string, int64_t>&) const  = 0;
    virtual SizeExprPtr clone() const = 0;
    virtual SizeExprPtr expand() const = 0;
    virtual bool isZero() const = 0;
    virtual bool isSymbol() const = 0;
    virtual bool isMultipleOf(int64_t value) const = 0;
};

class Literal : public SizeExpr {
public:
    Literal(int64_t value);
    int64_t value() const;
    std::string toString() const override;
    int64_t evaluate(const std::map<std::string, int64_t>& /*vars*/) const override;
    SizeExprPtr clone() const override;
    SizeExprPtr expand() const override;
    bool isZero() const override;
    bool isSymbol() const override;
    bool isMultipleOf(int64_t value) const override;

    static SizeExprPtr create(int64_t value);

private:
    int64_t _value;
};

class Variable : public SizeExpr {
public:
    Variable(std::string variable);
    std::string toString() const override;
    int64_t evaluate(const std::map<std::string, int64_t>& vars) const override;
    SizeExprPtr clone() const override;
    SizeExprPtr expand() const override;
    bool isZero() const override;
    bool isSymbol() const override;
    bool isMultipleOf(int64_t) const override;

    static SizeExprPtr create(const std::string& variable);

private:
    std::string _variable;
};

class BinaryOp : public SizeExpr {
public:
    enum Op { Add, Mul };

    BinaryOp(Op op, SizeExprPtr left, SizeExprPtr right);
    std::string toString() const override;
    int64_t evaluate(const std::map<std::string, int64_t>& vars) const override;
    SizeExprPtr clone() const override;
    SizeExprPtr expand() const override;
    bool isZero() const override;
    bool isSymbol() const override;
    bool isMultipleOf(int64_t value) const override;

    static SizeExprPtr create(Op op, SizeExprPtr left, SizeExprPtr right);

private:
    using BinOp = std::shared_ptr<BinaryOp>;

    Op _op;
    SizeExprPtr _left, _right;

    SizeExprPtr expandAdd(SizeExprPtr left, SizeExprPtr right) const;
    SizeExprPtr expandMul(SizeExprPtr left, SizeExprPtr right) const;

    static SizeExprPtr expandAdd2Add(BinOp left, BinOp right);
    static SizeExprPtr expandAddAddSymbol(BinOp op, SizeExprPtr symbol);
    static SizeExprPtr expandAddSymbolSymbol(SizeExprPtr left, SizeExprPtr right);

    static SizeExprPtr expandMul2Add(BinOp left, BinOp right);
    static SizeExprPtr expandMul2Mul(BinOp left, BinOp right);
    static SizeExprPtr expandMulAddMul(BinOp left, BinOp right);
    static SizeExprPtr expandMulMulSymbol(BinOp op, SizeExprPtr symbol);
    static SizeExprPtr expandMulAddSymbol(BinOp op, SizeExprPtr symbol);
    static SizeExprPtr expandMulSymbolSymbol(SizeExprPtr left, SizeExprPtr right);

    static bool isAddOp(BinOp Op);
    static bool isMulOp(BinOp Op);
};

SizeExprPtr operator+(SizeExprPtr left, SizeExprPtr right);
SizeExprPtr operator+(int64_t left, const SizeExprPtr& right);
SizeExprPtr operator+(SizeExprPtr left, int64_t right);
SizeExprPtr operator+(const char* left, SizeExprPtr right);
SizeExprPtr operator+(SizeExprPtr left, const char* right);

SizeExprPtr operator*(SizeExprPtr left, SizeExprPtr right);
SizeExprPtr operator*(int64_t left, SizeExprPtr right);
SizeExprPtr operator*(SizeExprPtr left, int64_t right);
SizeExprPtr operator*(const char* left, SizeExprPtr right);
SizeExprPtr operator*(SizeExprPtr left, const char* right);

SizeExprPtr& operator+=(SizeExprPtr& left, SizeExprPtr right);
SizeExprPtr& operator+=(SizeExprPtr& left, int64_t right);
SizeExprPtr& operator+=(SizeExprPtr& left, const char* right);

SizeExprPtr& operator*=(SizeExprPtr& left, SizeExprPtr right);
SizeExprPtr& operator*=(SizeExprPtr& left, int64_t right);
SizeExprPtr& operator*=(SizeExprPtr& left, const char* right);

SizeExprPtr operator""_lit(unsigned long long value);
SizeExprPtr operator""_var(const char* name, std::size_t);

#endif // SIZEEXPR_H
