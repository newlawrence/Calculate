#ifndef __CALCULATE_SYMBOLS_HPP__
#define __CALCULATE_SYMBOLS_HPP__

#include "calculate/definitions.hpp"


#define RECORD_CONSTANT(TOKEN, VALUE)                                         \
namespace calculate_symbols {                                                 \
    template <>                                                               \
    BuiltinConstant<djb2(TOKEN)>::BuiltinConstant() noexcept :                \
            Constant(TOKEN, VALUE) {}                                         \
    template <>                                                               \
    const Recorder<Constant> BuiltinConstant<djb2(TOKEN)>::_recorder(         \
        TOKEN, static_cast<pSymbol(*)()>(make<BuiltinConstant<djb2(TOKEN)>>)  \
    );                                                                        \
}


#define RECORD_OPERATOR(TOKEN, PRECEDENCE, LEFT_ASSOCIATION, FUNCTION)        \
namespace calculate_symbols {                                                 \
    template <>                                                               \
    BuiltinOperator<djb2(TOKEN)>::BuiltinOperator() noexcept :                \
            Operator(TOKEN, PRECEDENCE, LEFT_ASSOCIATION) {}                  \
    template <>                                                               \
    Value BuiltinOperator<djb2(TOKEN)>::evaluate() const noexcept {           \
        Value a = _left_operand->evaluate();                                  \
        Value b = _right_operand->evaluate();                                 \
        return FUNCTION;                                                      \
    }                                                                         \
    template <>                                                               \
    const Recorder<Operator> BuiltinOperator<djb2(TOKEN)>::_recorder(         \
        TOKEN, static_cast<pSymbol(*)()>(make<BuiltinOperator<djb2(TOKEN)>>)  \
    );                                                                        \
}


#define RECORD_FUNCTION(TOKEN, FUNCTION)                                      \
namespace calculate_symbols {                                                 \
    template <>                                                               \
    BuiltinFunction<djb2(TOKEN)>::BuiltinFunction() noexcept :                \
            Function(TOKEN, count_args(#FUNCTION)) {}                         \
    template <>                                                               \
    Value BuiltinFunction<djb2(TOKEN)>::evaluate() const noexcept {           \
        vValue x(args);                                                       \
        for (auto i = 0u; i < args; i++)                                      \
            x[i] = _operands[i]->evaluate();                                  \
        return FUNCTION;                                                      \
    }                                                                         \
    template <>                                                               \
    const Recorder<Function> BuiltinFunction<djb2(TOKEN)>::_recorder(         \
        TOKEN, static_cast<pSymbol(*)()>(make<BuiltinFunction<djb2(TOKEN)>>)  \
    );                                                                        \
}


namespace {

    using namespace calculate_definitions;

    constexpr Hash djb2(const Byte *s, Hash h=5381) {
	    return !*s ? h : djb2(s + 1, 33 * h ^ static_cast<Unsigned>(*s));
    }

    constexpr Unsigned count_args(Byte const * const s) {
        return *s == '\0' ? 1 : (*s == ',') + count_args(s + 1);
    }

}


namespace calculate_symbols {

    using namespace calculate_definitions;

    class Symbol;
    using pSymbol = std::shared_ptr<Symbol>;
    using vSymbol = std::vector<pSymbol>;
    using fSymbolGen = std::function<pSymbol()>;
    using mSymbolGen = std::unordered_map<String, fSymbolGen>;
    using qSymbol = std::queue<pSymbol>;
    using sSymbol = std::stack<pSymbol>;

    class Evaluable;
    using pEvaluable = std::shared_ptr<Evaluable>;
    using vEvaluable = std::vector<pEvaluable>;
    using qEvaluable = std::queue<pEvaluable>;
    using sEvaluable = std::stack<pEvaluable>;

    enum Type {LEFT, RIGHT, SEPARATOR, CONSTANT, OPERATOR, FUNCTION};


    template <typename T>
    std::shared_ptr<T> cast(pSymbol o) noexcept {
         return std::dynamic_pointer_cast<T>(o);
    }


    template <typename T>
    pSymbol make() {
        return std::make_shared<T>();
    }

    template <typename T>
    pSymbol make(const String &t) {
        return T::_symbols.at(t)();
    }

    template <typename T, typename D>
    pSymbol make(const String &t, D v) {
        return std::make_shared<T>(t, v);
    }


    template <typename T>
    bool defined(String t) {
        return T::_symbols.find(t) != T::_symbols.end();
    }

    template <typename T>
    vString query() {
        vString tokens;

        for (const auto& pair : T::_symbols)
            tokens.emplace_back(pair.first);
        return tokens;
    }


    template <typename T>
    struct Recorder {
        Recorder(const String &t, fSymbolGen g) noexcept {
            T::_symbols[t] = g;
        }
    };


    class Symbol {
        Symbol() = delete;
        Symbol(const Symbol&) = delete;
        Symbol(Symbol&&) = delete;
        Symbol& operator=(const Symbol&) = delete;
        Symbol& operator=(Symbol&&) = delete;

    protected:
        Symbol(const String &t, Type y) noexcept :
                token(t),
                type(y) {}

    public:
        const String token;
        const Type type;

        virtual ~Symbol() noexcept = 0;
        bool is(Type y) const noexcept { return type == y; }
    };
    inline Symbol::~Symbol() noexcept {}


    template <Byte s>
    class Parenthesis final : public Symbol {
        constexpr static const Type _type =
            s == '(' ? Type::LEFT : Type::RIGHT;
        constexpr static const Byte _symbol[2] = {s, '\0'};

    public:
        Parenthesis() noexcept :
                Symbol(_symbol, _type) {}
        virtual ~Parenthesis() noexcept {}
    };
    template <Byte s> constexpr const Byte Parenthesis<s>::_symbol[2];


    class Separator final : public Symbol {
    public:
        Separator() noexcept :
                Symbol(",", Type::SEPARATOR) {}
        virtual ~Separator() noexcept {}
    };


    class Evaluable : public Symbol {
    protected:
        Evaluable(const String &t, Type y) noexcept :
                Symbol(t, y) {}

    public:
        virtual ~Evaluable() noexcept = 0;
        virtual Value evaluate() const noexcept = 0;
        virtual void print(Stream &stream, String ind="") const noexcept;
    };
    inline Evaluable::~Evaluable() noexcept {}


    class EmptyEvaluable final : public Evaluable {
    public:
        EmptyEvaluable() noexcept : Evaluable("{empty}", Type::CONSTANT) {}
        virtual ~EmptyEvaluable() noexcept {}
        virtual Value evaluate() const noexcept { return nan; }
    };


    class Variable final : public Evaluable {
    public:
        const Value *_value;

        Variable(const String &t, Value *v) noexcept :
                Evaluable(t, Type::CONSTANT),
                _value(v) {}
        virtual ~Variable() noexcept {}
        virtual Value evaluate() const noexcept { return *_value; }
    };


    class Constant : public Evaluable {
    protected:
        static mSymbolGen _symbols;

    public:
        const Value value;

        Constant(const String &t, Value v) noexcept :
                Evaluable(t, Type::CONSTANT),
                value(v) {}
        virtual ~Constant() noexcept {};
        virtual Value evaluate() const noexcept { return value; }
        virtual void print(Stream &stream, String ind="") const noexcept;

        friend struct Recorder<Constant>;
        friend pSymbol make<Constant>(const String &t);
        friend bool defined<Constant>(String t);
        friend vString query<Constant>();
    };

    template <Hash hash>
    class BuiltinConstant final : public Constant {
        static const Recorder<Constant> _recorder;

    public:
        BuiltinConstant() noexcept :
                Constant("{constant}", nan) {}
        virtual ~BuiltinConstant() noexcept {}
    };


    class Operator : public Evaluable {
    protected:
        static mSymbolGen _symbols;

        pEvaluable _left_operand;
        pEvaluable _right_operand;

        Operator(const String &t, Unsigned p, bool l) noexcept :
                Evaluable(t, Type::OPERATOR),
                _left_operand(cast<Evaluable>(make<EmptyEvaluable>())),
                _right_operand(cast<Evaluable>(make<EmptyEvaluable>())),
                precedence(p),
                left_assoc(l) {}

    public:
        const Unsigned precedence;
        const bool left_assoc;

        virtual ~Operator() noexcept {}
        void addBranches(pEvaluable l, pEvaluable r) noexcept;
        virtual Value evaluate() const noexcept = 0;
        virtual void print(Stream &stream, String ind="") const noexcept;

        friend struct Recorder<Operator>;
        friend pSymbol make<Operator>(const String &t);
        friend bool defined<Operator>(String t);
        friend vString query<Operator>();
    };


    template <Hash hash>
    class BuiltinOperator final : public Operator {
        static const Recorder<Operator> _recorder;

    public:
        BuiltinOperator() noexcept :
                Operator("{operator}", 0, true) {}
        virtual ~BuiltinOperator() noexcept {}
        virtual Value evaluate() const noexcept { return nan; }
    };


    class Function : public Evaluable {
    protected:
        static mSymbolGen _symbols;

        vEvaluable _operands;

        Function(const String &t, Unsigned s) noexcept :
                Evaluable(t, Type::FUNCTION),
                _operands(s, cast<Evaluable>(make<EmptyEvaluable>())),
                args(s) {}

    public:
        const Unsigned args;

        virtual ~Function() noexcept {}
        void addBranches(const vEvaluable &x) noexcept;
        virtual Value evaluate() const noexcept = 0;
        virtual void print(Stream &stream, String ind="") const noexcept;

        friend struct Recorder<Function>;
        friend pSymbol make<Function>(const String &t);
        friend bool defined<Function>(String t);
        friend vString query<Function>();
    };


    template <Hash hash>
    class BuiltinFunction final : public Function {
        static const Recorder<Function> _recorder;

    public:
        BuiltinFunction() noexcept :
                 Function("{function}", 0) {}
        virtual ~BuiltinFunction() {}
        virtual Value evaluate() const noexcept { return nan; }
    };

}

#endif
