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
    virtual std::string ToString() const = 0;
    virtual int64_t Evaluate(const std::map<std::string, int64_t>&) const  = 0;
    virtual SizeExprPtr Clone() const = 0;
    virtual SizeExprPtr Expand() const = 0;
    virtual bool IsZero() const = 0;
    virtual bool IsSymbol() const = 0;
    virtual bool IsMultipleOf(int64_t value) const = 0;
};

class Literal : public SizeExpr {
public:
    Literal(int64_t value);
    int64_t value() const;
    std::string ToString() const override;
    int64_t Evaluate(const std::map<std::string, int64_t>& /*vars*/) const override;
    SizeExprPtr Clone() const override;
    SizeExprPtr Expand() const override;
    bool IsZero() const override;
    bool IsSymbol() const override;
    bool IsMultipleOf(int64_t value) const override;

    static SizeExprPtr Create(int64_t value);

private:
    int64_t _value;
};

class Variable : public SizeExpr {
public:
    Variable(std::string variable);
    std::string ToString() const override;
    int64_t Evaluate(const std::map<std::string, int64_t>& vars) const override;
    SizeExprPtr Clone() const override;
    SizeExprPtr Expand() const override;
    bool IsZero() const override;
    bool IsSymbol() const override;
    bool IsMultipleOf(int64_t) const override;

    static SizeExprPtr Create(const std::string& variable);

private:
    std::string _variable;
};

class BinaryOp : public SizeExpr {
public:
    enum Op { Add, Mul };

    BinaryOp(Op op, SizeExprPtr left, SizeExprPtr right);
    std::string ToString() const override;
    int64_t Evaluate(const std::map<std::string, int64_t>& vars) const override;
    SizeExprPtr Clone() const override;
    SizeExprPtr Expand() const override;
    bool IsZero() const override;
    bool IsSymbol() const override;
    bool IsMultipleOf(int64_t value) const override;

    static SizeExprPtr Create(Op op, SizeExprPtr left, SizeExprPtr right);

private:
    using BinOp = std::shared_ptr<BinaryOp>;

    Op _op;
    SizeExprPtr _left, _right;

    SizeExprPtr ExpandAdd(SizeExprPtr left, SizeExprPtr right) const;
    SizeExprPtr ExpandMul(SizeExprPtr left, SizeExprPtr right) const;

    static SizeExprPtr ExpandAdd2Add(BinOp left, BinOp right);
    static SizeExprPtr ExpandAddAddSymbol(BinOp op, SizeExprPtr symbol);
    static SizeExprPtr ExpandAddSymbolSymbol(SizeExprPtr left, SizeExprPtr right);

    static SizeExprPtr ExpandMul2Add(BinOp left, BinOp right);
    static SizeExprPtr ExpandMul2Mul(BinOp left, BinOp right);
    static SizeExprPtr ExpandMulAddMul(BinOp left, BinOp right);
    static SizeExprPtr ExpandMulMulSymbol(BinOp op, SizeExprPtr symbol);
    static SizeExprPtr ExpandMulAddSymbol(BinOp op, SizeExprPtr symbol);
    static SizeExprPtr ExpandMulSymbolSymbol(SizeExprPtr left, SizeExprPtr right);

    static bool IsAddOp (BinOp Op);
    static bool IsMulOp(BinOp Op);
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
