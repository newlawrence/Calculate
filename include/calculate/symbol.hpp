#ifndef __CALCULATE_SYMBOL_HPP__
#define __CALCULATE_SYMBOL_HPP__

#include "wrapper.hpp"


namespace calculate {

template<typename Expression>
class Symbol {
    friend struct std::hash<Symbol>;

public:
    using Type = typename Expression::Type;

    enum class SymbolType : int {
        LEFT=0,
        RIGHT,
        SEPARATOR,
        CONSTANT,
        FUNCTION,
        OPERATOR
    };

private:
    using WrapperConcept = WrapperConcept<Type, Expression>;
    using Wrapper = Wrapper<Type, Expression>;

    template<typename Callable>
    struct Inspect {
        static constexpr bool not_me =
            detail::NotSame<Callable, Symbol>::value;
        static constexpr bool is_model =
            std::is_base_of<WrapperConcept, Callable>::value;
    };

    Wrapper _wrapper;

    std::size_t _hash() const noexcept {
        if (symbol() == SymbolType::CONSTANT)
            return std::hash<Type>()(_wrapper());
        return std::hash<Wrapper>()(_wrapper);
    }

    virtual bool _equal(const Symbol&) const noexcept = 0;

public:
    template<
        typename Callable,
        std::enable_if_t<Inspect<Callable>::not_me>* = nullptr,
        std::enable_if_t<!Inspect<Callable>::is_model>* = nullptr
    >
    Symbol(Callable&& callable) :
            _wrapper{std::forward<Callable>(callable), &Expression::evaluate}
    {}

    template<
        typename Callable,
        std::enable_if_t<Inspect<Callable>::is_model>* = nullptr
    >
    Symbol(Callable&& callable) :
            _wrapper{std::forward<Callable>(callable)}
    {}

    virtual ~Symbol() = default;

    template<typename Class>
    bool operator==(const Class& other) const noexcept {
        if (symbol() != other.symbol())
            return false;
        if (symbol() != SymbolType::CONSTANT && _wrapper != other._wrapper)
            return false;
        return this->_equal(other);
    }

    template<typename Class>
    bool operator!=(const Class& other) const noexcept {
        return !operator==(other);
    }

    template<typename... Args>
    Type operator()(Args&&... args) const {
        return _wrapper(std::forward<Args>(args)...);
    }

    Type evaluate(const std::vector<Expression>& nodes) const {
        return _wrapper.eval(nodes);
    }

    std::size_t arguments() const noexcept { return _wrapper.argc(); }

    virtual SymbolType symbol() const noexcept = 0;
};

template<typename Expression>
class Variable final : public Symbol<Expression> {
    using Symbol = Symbol<Expression>;
    using SymbolType = typename Symbol::SymbolType;

    bool _equal(const Symbol&) const noexcept override { return false; }

public:
    using Type = typename Expression::Type;

    Variable(Type& variable) :
            Symbol{[&variable]() noexcept { return variable; }}
    {}

    SymbolType symbol() const noexcept override { return SymbolType::CONSTANT; }
};

template<typename Expression>
class Constant final : public Symbol<Expression> {
    using Symbol = Symbol<Expression>;
    using SymbolType = typename Symbol::SymbolType;

    bool _equal(const Symbol& other) const noexcept override {
        return (*this)() == other();
    }

public:
    using Type = typename Expression::Type;

    Constant(Type value) :
            Symbol{[value]() noexcept { return value; }}
    {}

    Constant() : Symbol{[]() noexcept { return Type{}; }} {}

    SymbolType symbol() const noexcept override { return SymbolType::CONSTANT; }

    operator Type() const { return Symbol::operator()(); }

    operator Type() { return Symbol::operator()(); }
};

template<typename Expression>
class Function final : public Symbol<Expression> {
    using Symbol = Symbol<Expression>;
    using SymbolType = typename Symbol::SymbolType;

    bool _equal(const Symbol&) const noexcept override { return true; }

public:
    using Type = typename Expression::Type;

    template<typename Callable>
    Function(Callable&& callable) :
            Symbol{std::forward<Callable>(callable)}
    {}

    Function() : Symbol{[](const Type& x) noexcept { return x; }} {}

    SymbolType symbol() const noexcept override { return SymbolType::FUNCTION; }

    template<typename... Args>
    Type operator()(Args&&... args) const {
        return Symbol::operator()(std::forward<Args>(args)...);
    }
};

template<typename Expression>
class Operator final : public Symbol<Expression> {
    using Symbol = Symbol<Expression>;
    using SymbolType = typename Symbol::SymbolType;

public:
    using Type = typename Expression::Type;

    enum class Associativity : int {LEFT=0, RIGHT, BOTH};

private:
    std::string _alias;
    std::size_t _precedence;
    Associativity _associativity;

    bool _equal(const Symbol& other) const noexcept override {
        auto op = static_cast<const Operator&>(other);
        return
            _alias == op._alias &&
            _precedence == op._precedence &&
            _associativity == op._associativity;
    }

public:
    template<typename Callable>
    Operator(
        Callable&& callable,
        const std::string& alias,
        std::size_t precedence,
        Associativity associativity
    ) :
            Symbol{std::forward<Callable>(callable)},
            _alias{alias},
            _precedence{precedence},
            _associativity{associativity}
    {}

    Operator() :
        Symbol{[](const Type& x, const Type&) noexcept { return x; }},
        _alias{""},
        _precedence{0u},
        _associativity{Associativity::BOTH}
    {}

    SymbolType symbol() const noexcept override { return SymbolType::OPERATOR; }

    template<typename... Args>
    Type operator()(Args&&... args) const {
        return Symbol::operator()(std::forward<Args>(args)...);
    }

    const std::string& alias() const noexcept { return _alias; }

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
