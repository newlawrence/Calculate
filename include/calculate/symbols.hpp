#ifndef __CALCULATE_SYMBOLS_HPP__
#define __CALCULATE_SYMBOLS_HPP__

#include "calculate/definitions.hpp"
#include "calculate/meta.hpp"


#define RECORD_CONSTANT(TOKEN, VALUE)                                         \
namespace calculate_symbols {                                                 \
    template <>                                                               \
    BuiltinConstant<TypeString(TOKEN)>::BuiltinConstant() noexcept :          \
        Constant(TOKEN, VALUE) {}                                             \
    template class BuiltinConstant<TypeString(TOKEN)>;                        \
}


#define RECORD_OPERATOR(TOKEN, PRECEDENCE, LEFT_ASSOCIATION, FUNCTION)        \
namespace calculate_symbols {                                                 \
    template <>                                                               \
    BuiltinOperator<TypeString(TOKEN)>::BuiltinOperator() noexcept :          \
        Operator(TOKEN, PRECEDENCE, LEFT_ASSOCIATION, FUNCTION) {}            \
    template class BuiltinOperator<TypeString(TOKEN)>;                        \
}


#define RECORD_FUNCTION(TOKEN, FUNCTION)                                      \
namespace calculate_symbols {                                                 \
    template <>                                                               \
    BuiltinFunction<TypeString(TOKEN)>::BuiltinFunction() noexcept :          \
        Function(TOKEN, FUNCTION) {}                                          \
    template class BuiltinFunction<TypeString(TOKEN)>;                        \
}


namespace calculate_symbols {

    using namespace calculate_definitions;
    using namespace calculate_meta;

    class Symbol;
    using pSymbol = std::shared_ptr<Symbol>;
    using vSymbol = std::vector<pSymbol>;
    using fSymbolGen = pSymbol(*)();
    using mSymbolGen = std::unordered_map<String, fSymbolGen>;
    using qSymbol = std::queue<pSymbol>;
    using sSymbol = std::stack<pSymbol>;

    class Evaluable;
    using pEvaluable = std::shared_ptr<Evaluable>;
    using vEvaluable = std::vector<pEvaluable>;
    using qEvaluable = std::queue<pEvaluable>;
    using sEvaluable = std::stack<pEvaluable>;

    enum Type {LEFT, RIGHT, SEPARATOR, CONSTANT, OPERATOR, FUNCTION};


    template <typename Type>
    std::shared_ptr<Type> cast(pSymbol o) noexcept {
         return std::dynamic_pointer_cast<Type>(o);
    }


    template <typename Type>
    pSymbol make() {
        return std::make_shared<Type>();
    }

    template <typename Type>
    pSymbol make(const String &t) {
        return Type::symbols().at(t)();
    }

    template <typename Type, typename VType>
    pSymbol make(const String &t, VType v) {
        return std::make_shared<Type>(t, v);
    }


    template <typename Type>
    Bool defined(String t) {
        return Type::symbols().find(t) != Type::symbols().end();
    }

    template <typename Type>
    vString query() {
        vString tokens;
        for (const auto& pair : Type::symbols())
            tokens.emplace_back(pair.first);
        return tokens;
    }


    template <typename Type>
    struct Recorder {
        Recorder(const String &t, fSymbolGen g) noexcept {
            Type::symbols()[t] = g;
        }
    };


    class Symbol {
        Symbol() = delete;
        Symbol(const Symbol&) = delete;
        Symbol(Symbol&&) = delete;
        Symbol& operator=(const Symbol&) = delete;
        Symbol& operator=(Symbol&&) = delete;

    protected:
        Symbol(const String &t, Type y) noexcept : token(t), type(y) {}

    public:
        const String token;
        const Type type;

        virtual ~Symbol() noexcept = 0;
        Bool is(Type y) const noexcept { return type == y; }
    };
    inline Symbol::~Symbol() noexcept {}


    template <Byte s>
    class Parenthesis final : public Symbol {
        constexpr static const Type _type =
            s == '(' ? Type::LEFT : Type::RIGHT;
        constexpr static const Byte _symbol[2] = {s, '\0'};

    public:
        Parenthesis() noexcept : Symbol(_symbol, _type) {}
        virtual ~Parenthesis() noexcept {}
    };
    template <Byte s> constexpr const Byte Parenthesis<s>::_symbol[2];


    class Separator final : public Symbol {
    public:
        Separator() noexcept : Symbol(",", Type::SEPARATOR) {}
        virtual ~Separator() noexcept {}
    };


    class Evaluable : public Symbol {
    protected:
        FunctionWrapper _function;
        vEvaluable _operands;

        Evaluable() noexcept :
            Symbol("", Type::CONSTANT),
            _function([](){ throw; return nan; }),
            args(0) {}

        template<typename Func>
        Evaluable(const String &t, Type y, Func&& f) noexcept :
            Symbol(t, y),
            _function(std::forward<Func>(f)),
            args(FunctionWrapper(std::forward<Func>(f)).args()) {}

    public:
        const SizeT args;

        virtual ~Evaluable() noexcept = 0;
        void addBranches(const vEvaluable &x) noexcept;
        Value evaluate() const noexcept;
        void print(Stream &stream, String ind="") const noexcept;
    };
    inline Evaluable::~Evaluable() noexcept {}


    class Variable final : public Evaluable {
    public:
        Variable(const String &t, Value *v) noexcept :
            Evaluable(t, Type::CONSTANT, [v](){ return *v; }) {}
        virtual ~Variable() noexcept {}
    };


    class Constant : public Evaluable {
    protected:
        static mSymbolGen& symbols() {
            static mSymbolGen _symbols;
            return _symbols;
        }

    public:
        Constant() : Evaluable() {}
        Constant(const String &t, Value v) noexcept :
            Evaluable(t, Type::CONSTANT, [v](){ return v; }) {}
        virtual ~Constant() noexcept {};

        friend struct Recorder<Constant>;
        friend pSymbol make<Constant>(const String &t);
        friend Bool defined<Constant>(String t);
        friend vString query<Constant>();
    };

    template <typename Token>
    class BuiltinConstant final : public Constant {
        static const Recorder<Constant> _recorder;

    public:
        BuiltinConstant() noexcept : Constant() {}
        virtual ~BuiltinConstant() noexcept {}
    };
    template <typename Token>
    const Recorder<Constant> BuiltinConstant<Token>::_recorder(
        Token::str, make<BuiltinConstant<Token>>
    );


    class Operator : public Evaluable {
    protected:
        static mSymbolGen& symbols() {
            static mSymbolGen _symbols;
            return _symbols;
        }

        Operator() : Evaluable(), precedence(0), left_assoc(true) {}

        template<typename Func>
        Operator(const String &t, SizeT p, Bool l, Func&& f) noexcept :
            Evaluable(t, Type::OPERATOR, std::forward<Func>(f)),
            precedence(p),
            left_assoc(l) {}

    public:
        const SizeT precedence;
        const Bool left_assoc;

        virtual ~Operator() noexcept {}

        friend struct Recorder<Operator>;
        friend pSymbol make<Operator>(const String &t);
        friend Bool defined<Operator>(String t);
        friend vString query<Operator>();
    };


    template <typename Token>
    class BuiltinOperator final : public Operator {
        static const Recorder<Operator> _recorder;

    public:
        BuiltinOperator() noexcept : Operator() {}
        virtual ~BuiltinOperator() noexcept {}
    };
    template <typename Token>
    const Recorder<Operator> BuiltinOperator<Token>::_recorder(
        Token::str, make<BuiltinOperator<Token>>
    );


    class Function : public Evaluable {
    protected:
        static mSymbolGen& symbols() {
            static mSymbolGen _symbols;
            return _symbols;
        }

        Function() : Evaluable() {}

        template<typename Func>
        Function(const String &t, Func&& f) noexcept :
            Evaluable(t, Type::FUNCTION, std::forward<Func>(f)) {}

    public:
        virtual ~Function() noexcept {}

        friend struct Recorder<Function>;
        friend pSymbol make<Function>(const String &t);
        friend Bool defined<Function>(String t);
        friend vString query<Function>();
    };


    template <typename Token>
    class BuiltinFunction final : public Function {
        static const Recorder<Function> _recorder;

    public:
        BuiltinFunction() noexcept : Function() {}
        virtual ~BuiltinFunction() noexcept {}
    };
    template <typename Token>
    const Recorder<Function> BuiltinFunction<Token>::_recorder(
        Token::str, make<BuiltinFunction<Token>>
    );

}

#endif
