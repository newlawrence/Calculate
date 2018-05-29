/*
    Calculate - Version 2.1.0dev0
    Last modified 2018/05/29
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
    friend Expression;

public:
    using Type = typename Expression::Type;

    enum class SymbolType : int {
        LEFT=0,
        RIGHT,
        SEPARATOR,
        CONSTANT,
        FUNCTION,
        OPERATOR,
        PREFIX,
        SUFFIX
    };

private:
    using WrapperConcept = calculate::WrapperConcept<Type, Expression>;
    using Wrapper = calculate::Wrapper<Type, Expression>;

    template<typename Callable>
    struct Inspect {
        static constexpr bool not_self =
            detail::NotSame<Callable, Symbol>::value;
        static constexpr bool is_model =
            std::is_base_of<WrapperConcept, Callable>::value;
    };

    std::size_t _hash() const noexcept {
        if (symbol() == SymbolType::CONSTANT)
            return std::hash<Type>()(static_cast<const Wrapper&>(*this)());
        return std::hash<Wrapper>()(static_cast<const Wrapper&>(*this));
    }

    virtual bool _equal(const Symbol&) const noexcept = 0;

public:
    template<
        typename Callable,
        typename = std::enable_if_t<Inspect<Callable>::not_self>,
        typename = std::enable_if_t<!Inspect<Callable>::is_model>
    >
    Symbol(Callable&& callable) :
            Wrapper{
                std::forward<Callable>(callable),
                [](const Expression& expression) { return Type{expression}; }
            }
    {
        static_assert(
            detail::NotSame<Callable, Function<Expression>>::value ||
            detail::Argc<Callable>::value > 0,
            "Functions must have at least one argument"
        );
        static_assert(
            detail::NotSame<Callable, Operator<Expression>>::value ||
            detail::Argc<Callable>::value == 2,
            "Operators must have two arguments"
        );
	}

    template<
        typename Callable,
        typename = std::enable_if_t<Inspect<Callable>::is_model>
    >
    Symbol(Callable&& callable) :
            Wrapper{std::forward<Callable>(callable)}
    {}

    virtual ~Symbol() = default;

    template<typename Class>
    bool operator==(const Class& other) const noexcept {
        auto& this_wrapper = static_cast<const Wrapper&>(*this);
        auto& other_wrapper = static_cast<const Wrapper&>(other);

        if (symbol() != other.symbol())
            return false;
        if (symbol() != SymbolType::CONSTANT && this_wrapper != other_wrapper)
            return false;
        return this->_equal(other);
    }

    template<typename Class>
    bool operator!=(const Class& other) const noexcept {
        return !operator==(other);
    }

    using Wrapper::operator();

    using Wrapper::eval;

    std::size_t arguments() const noexcept {
        return static_cast<const Wrapper*>(this)->argc();
    }

    virtual SymbolType symbol() const noexcept = 0;

    virtual std::unique_ptr<Symbol> clone() const noexcept = 0;
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

    SymbolType symbol() const noexcept override { return SymbolType::CONSTANT; }

    std::unique_ptr<Symbol> clone() const noexcept override {
        return std::make_unique<Variable>(*this);
    }
};

template<typename Expression>
class Constant final : public Symbol<Expression> {
    using Symbol = calculate::Symbol<Expression>;
    using SymbolType = typename Symbol::SymbolType;

    bool _equal(const Symbol& other) const noexcept override {
        return (*this)() == other();
    }

public:
    using Type = typename Expression::Type;

    Constant(Type value) :
            Symbol{[value]() noexcept { return value; }}
    {}

    SymbolType symbol() const noexcept override { return SymbolType::CONSTANT; }

    std::unique_ptr<Symbol> clone() const noexcept override {
        return std::make_unique<Constant>(*this);
    }

    operator Type() const { return Symbol::operator()(); }
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

    SymbolType symbol() const noexcept override { return SymbolType::FUNCTION; }

    std::unique_ptr<Symbol> clone() const noexcept override {
        return std::make_unique<Function>(*this);
    }

    template<typename... Args>
    Type operator()(Args&&... args) const {
        return Symbol::operator()(std::forward<Args>(args)...);
    }
};

template<typename Expression>
class Operator final : public Symbol<Expression> {
    using Symbol = calculate::Symbol<Expression>;
    using SymbolType = typename Symbol::SymbolType;

public:
    using Type = typename Expression::Type;

    enum class Associativity : int {LEFT=0, RIGHT, BOTH};

private:
    std::size_t _precedence;
    Associativity _associativity;

    bool _equal(const Symbol& other) const noexcept override {
        auto op = static_cast<const Operator&>(other);
        return
            _precedence == op._precedence &&
            _associativity == op._associativity;
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

    SymbolType symbol() const noexcept override { return SymbolType::OPERATOR; }

    std::unique_ptr<Symbol> clone() const noexcept override {
        return std::make_unique<Operator>(*this);
    }

    template<typename... Args>
    Type operator()(Args&&... args) const {
        return Symbol::operator()(std::forward<Args>(args)...);
    }

    std::size_t precedence() const noexcept { return _precedence; }

    Associativity associativity() const noexcept { return _associativity; }
};

}


namespace std {

template<typename Expression>
struct hash<calculate::Symbol<Expression>> {
    size_t operator()(const calculate::Symbol<Expression>& symbol) const {
        return symbol._hash();
    }
};

}

#endif
