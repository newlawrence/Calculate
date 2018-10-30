/*
    Calculate - Version 2.1.1rc10
    Last modified 2018/09/04
    Released under MIT license
    Copyright (c) 2016-2018 Alberto Lorenzo <alorenzo.md@gmail.com>
*/


#ifndef __CALCULATE_SYMBOL_HPP__
#define __CALCULATE_SYMBOL_HPP__

#include "wrapper.hpp"


namespace calculate {

template<typename> class Function;
template<typename> class Operator;

template<typename Expression>
class Symbol : Wrapper<typename Expression::Type, Expression> {
    friend struct std::hash<Symbol>;

public:
    using Type = typename Expression::Type;

    enum class SymbolType { LEFT, RIGHT, SEPARATOR, CONSTANT, FUNCTION, OPERATOR, PREFIX, SUFFIX };

private:
    using WrapperConcept = calculate::WrapperConcept<Type, Expression>;
    using Wrapper = calculate::Wrapper<Type, Expression>;

    std::size_t _hash() const noexcept {
        if (type() == SymbolType::CONSTANT)
            return std::hash<Type>()(static_cast<const Wrapper&>(*this)());
        return std::hash<Wrapper>()(static_cast<const Wrapper&>(*this));
    }

    virtual bool _equal(const Symbol&) const noexcept = 0;

public:
    template<
        typename Callable,
        typename = std::enable_if_t<util::not_same_v<Callable, Symbol>>,
        typename = std::enable_if_t<!util::is_base_of_v<WrapperConcept, Callable>>
    >
    Symbol(Callable&& callable) :
            Wrapper{
                std::forward<Callable>(callable),
                [](const Expression& e) noexcept { return e._symbol->eval(e._nodes); }
            }
    {
        static_assert(
            util::not_same_v<Callable, Function<Expression>> || util::argc_v<Callable> == 0,
            "Functions must have one argument at least"
        );
        static_assert(
            util::not_same_v<Callable, Operator<Expression>> || util::argc_v<Callable> == 2,
            "Operators must have two arguments"
        );
    }

    template<
        typename Callable,
        typename = std::enable_if_t<util::is_base_of_v<WrapperConcept, Callable>>
    >
    Symbol(Callable&& callable) : Wrapper{std::forward<Callable>(callable)} {}

    virtual ~Symbol() = default;

    template<typename Class>
    bool operator==(const Class& other) const noexcept {
        auto& this_wrapper = static_cast<const Wrapper&>(*this);
        auto& other_wrapper = static_cast<const Wrapper&>(other);

        if (type() != other.type())
            return false;
        if (type() != SymbolType::CONSTANT && this_wrapper != other_wrapper)
            return false;
        return this->_equal(other);
    }

    template<typename Class>
    bool operator!=(const Class& other) const noexcept { return !operator==(other); }

    using Wrapper::operator();

    using Wrapper::eval;

    std::size_t arguments() const noexcept { return static_cast<const Wrapper*>(this)->argc(); }

    virtual SymbolType type() const noexcept = 0;

    virtual std::unique_ptr<Symbol> clone() const = 0;
};


template<typename Expression>
class Variable final : public Symbol<Expression> {
    using Symbol = calculate::Symbol<Expression>;
    using SymbolType = typename Symbol::SymbolType;

    bool _equal(const Symbol&) const noexcept override { return false; }

public:
    using Type = typename Expression::Type;

    Variable(Type& variable) :
            Symbol{[&variable]() noexcept { return variable; }}
    {}

    SymbolType type() const noexcept override { return SymbolType::CONSTANT; }

    std::unique_ptr<Symbol> clone() const override {
        return std::make_unique<Variable>(*this);
    }
};

template<typename Expression>
class Constant final : public Symbol<Expression> {
    using Symbol = calculate::Symbol<Expression>;
    using SymbolType = typename Symbol::SymbolType;

    bool _equal(const Symbol& other) const noexcept override { return Type{*this} == other(); }

public:
    using Type = typename Expression::Type;

    Constant(Type value) :
            Symbol{[value]() noexcept { return value; }}
    {}

    operator Type() const { return Symbol::operator()(); }

    template<typename U> bool operator==(U value) const { return Type{*this} == value; }
    template<typename U> bool operator!=(U value) const { return Type{*this} != value; }
    template<typename U> bool operator>(U value) const { return Type{*this} > value; }
    template<typename U> bool operator<(U value) const { return Type{*this} < value; }
    template<typename U> bool operator>=(U value) const { return Type{*this} >= value; }
    template<typename U> bool operator<=(U value) const { return Type{*this} <= value; }

    template<typename U> auto operator+(U value) const { return Type{*this} + value; }
    template<typename U> auto operator-(U value) const { return Type{*this} - value; }
    template<typename U> auto operator*(U value) const { return Type{*this} * value; }
    template<typename U> auto operator/(U value) const { return Type{*this} / value; }
    template<typename U> auto operator%(U value) const { return Type{*this} % value; }

    SymbolType type() const noexcept override { return SymbolType::CONSTANT; }

    std::unique_ptr<Symbol> clone() const override {
        return std::make_unique<Constant>(*this);
    }
};

template<typename Expression>
class Function final : public Symbol<Expression> {
    using Symbol = calculate::Symbol<Expression>;
    using SymbolType = typename Symbol::SymbolType;

    bool _equal(const Symbol&) const noexcept override { return true; }

public:
    using Type = typename Expression::Type;

    template<typename Callable>
    Function(Callable&& callable) :
            Symbol{std::forward<Callable>(callable)}
    {}

    template<typename... Args>
    Type operator()(Args&&... args) const {
        return Symbol::operator()(std::forward<Args>(args)...);
    }

    SymbolType type() const noexcept override { return SymbolType::FUNCTION; }

    std::unique_ptr<Symbol> clone() const override {
        return std::make_unique<Function>(*this);
    }
};

template<typename Expression>
class Operator final : public Symbol<Expression> {
    using Symbol = calculate::Symbol<Expression>;
    using SymbolType = typename Symbol::SymbolType;

public:
    using Type = typename Expression::Type;

    enum class Associativity {LEFT, RIGHT, FULL};

private:
    std::size_t _precedence;
    Associativity _associativity;

    bool _equal(const Symbol& other) const noexcept override {
        auto op = static_cast<const Operator&>(other);
        return _precedence == op._precedence && _associativity == op._associativity;
    }

public:
    template<typename Callable>
    Operator(
        Callable&& callable,
        std::size_t precedence,
        Associativity associativity
    ) :
            Symbol{std::forward<Callable>(callable)},
            _precedence{precedence},
            _associativity{associativity}
    {}

    template<typename... Args>
    Type operator()(Args&&... args) const {
        return Symbol::operator()(std::forward<Args>(args)...);
    }

    std::size_t precedence() const noexcept { return _precedence; }

    Associativity associativity() const noexcept { return _associativity; }

    SymbolType type() const noexcept override { return SymbolType::OPERATOR; }

    std::unique_ptr<Symbol> clone() const override {
        return std::make_unique<Operator>(*this);
    }
};

}


namespace std {

template<typename Expression>
struct hash<calculate::Symbol<Expression>> {
    size_t operator()(const calculate::Symbol<Expression>& symbol) const { return symbol._hash(); }
};

}

#endif
