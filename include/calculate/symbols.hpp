#ifndef __CALCULATE_SYMBOLS_HPP__
#define __CALCULATE_SYMBOLS_HPP__

#include <memory>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdlib>


#define nan std::numeric_limits<double>::quiet_NaN

#define RECORD_CONSTANT(TOKEN, VALUE)                                         \
namespace calculate_symbols {                                                 \
    template <>                                                               \
    BuiltinConstant<hash_djb2(#TOKEN)>::BuiltinConstant() noexcept :          \
            Constant(#TOKEN, VALUE) {}                                        \
    template <>                                                               \
    const Constant::Recorder BuiltinConstant<hash_djb2(#TOKEN)>::_recorder =  \
        Constant::Recorder(#TOKEN, &BuiltinConstant::make);                   \
}


#define RECORD_OPERATOR(TOKEN, PRECEDENCE, LEFT_ASSOCIATION, FUNCTION)        \
namespace calculate_symbols {                                                 \
    template <>                                                               \
    BuiltinOperator<hash_djb2(TOKEN)>::BuiltinOperator() noexcept :           \
            Operator(TOKEN, PRECEDENCE, LEFT_ASSOCIATION) {}                  \
    template <>                                                               \
    double BuiltinOperator<hash_djb2(TOKEN)>::evaluate() const noexcept {     \
        double a = _left_operand->evaluate();                                 \
        double b = _right_operand->evaluate();                                \
        return FUNCTION;                                                      \
    }                                                                         \
    template <>                                                               \
    const Operator::Recorder BuiltinOperator<hash_djb2(TOKEN)>::_recorder =   \
        Operator::Recorder(TOKEN, &BuiltinOperator::make);                    \
}


#define RECORD_FUNCTION(TOKEN, FUNCTION)                                      \
namespace calculate_symbols {                                                 \
    template <>                                                               \
    BuiltinFunction<hash_djb2(#TOKEN)>::BuiltinFunction() noexcept :          \
            Function(#TOKEN, count_args(#FUNCTION)) {}                        \
    template <>                                                               \
    double BuiltinFunction<hash_djb2(#TOKEN)>::evaluate() const noexcept {    \
        vValue x(args);                                                       \
        for (auto i = 0u; i < args; i++)                                      \
            x[i] = _operands[i]->evaluate();                                  \
        return FUNCTION;                                                      \
    }                                                                         \
    template <>                                                               \
    const Function::Recorder BuiltinFunction<hash_djb2(#TOKEN)>::_recorder =  \
        Function::Recorder(#TOKEN, &BuiltinFunction::make);                   \
}


namespace {

    constexpr unsigned long hash_djb2(const char *s, unsigned long h=5381) {
	    return !*s ? h : hash_djb2(s + 1, 33 * h ^ (unsigned)*s);
    }

    constexpr unsigned count_args(char const * const s) {
        return *s == '\0' ? 1 : (*s == ',') + count_args(s + 1);
    }

}


namespace calculate_symbols {

    using Stream = std::ostringstream;
    using String = std::string;
    using vString = std::vector<String>;

    using pValue = std::unique_ptr<double[]>;
    using vValue = std::vector<double>;
    using mValue = std::unordered_map<String, double>;

    class Symbol;
    using pSymbol = std::shared_ptr<Symbol>;
    using vSymbol = std::vector<pSymbol>;
    using fSymbolGen = pSymbol (*)();
    using mSymbolGen = std::unordered_map<String, fSymbolGen>;

    class Evaluable;
    using pEvaluable = std::shared_ptr<Evaluable>;
    using vEvaluable = std::vector<pEvaluable>;

    enum Type {LEFT, RIGHT, SEPARATOR, CONSTANT, OPERATOR, FUNCTION};


    template<typename T>
    std::shared_ptr<T> castChild(pSymbol o) noexcept {
         return std::dynamic_pointer_cast<T>(o);
    }


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


    template<char s>
    class Parenthesis final : public Symbol {
        constexpr static const Type _type =
            s == '(' ? Type::LEFT : Type::RIGHT;
        constexpr static const char _symbol[2] = {s, '\0'};

    public:
        Parenthesis() noexcept :
                Symbol(_symbol, _type) {}
        virtual ~Parenthesis() noexcept {}

        static pSymbol make() {
            return std::make_shared<Parenthesis<s>>();
        }
    };
    template<char s> constexpr const char Parenthesis<s>::_symbol[2];


    class Separator final : public Symbol {
    public:
        Separator() noexcept :
                Symbol(",", Type::SEPARATOR) {}
        virtual ~Separator() noexcept {}

        static pSymbol make() {
            return std::make_shared<Separator>();
        }
    };


    class Evaluable : public Symbol {
    protected:
        Evaluable(const String &t, Type y) noexcept :
                Symbol(t, y) {}

    public:
        virtual ~Evaluable() noexcept = 0;
        virtual double evaluate() const noexcept = 0;
        virtual void print(Stream &stream, String ind="") const noexcept;
    };
    inline Evaluable::~Evaluable() noexcept {}


    class EmptyEvaluable final : public Evaluable {
    public:
        EmptyEvaluable() noexcept : Evaluable("{empty}", Type::CONSTANT) {}
        virtual ~EmptyEvaluable() noexcept {}
        virtual double evaluate() const noexcept { return nan(); }

        static pSymbol make() {
            return std::make_shared<EmptyEvaluable>();
        }
    };


    class Variable final : public Evaluable {
    public:
        const double *_value;

        Variable(const String &t, double *v) noexcept :
                Evaluable(t, Type::CONSTANT),
                _value(v) {}
        virtual ~Variable() noexcept {}
        virtual double evaluate() const noexcept { return *_value; }

        static pSymbol make(const String &t, double *v) {
            return std::make_shared<Variable>(t, v);
        }
    };


    class Constant : public Evaluable {
    protected:
        struct Recorder {
            Recorder(const String &t, fSymbolGen g) noexcept;
        };
        static mSymbolGen _symbols;

    public:
        const double value;

        Constant(const String &t) noexcept :
                Evaluable(t, Type::CONSTANT),
                value(std::strtod(t.c_str(), nullptr)) {}
        Constant(const String &t, double v) noexcept :
                Evaluable(t, Type::CONSTANT),
                value(v) {}
        virtual ~Constant() noexcept {};
        virtual double evaluate() const noexcept { return value; }
        virtual void print(Stream &stream, String ind="") const noexcept;

        static pSymbol makeNumbered(String t) {
            return std::make_shared<Constant>(t);
        }
        static pSymbol makeNamed(String t) {
            return _symbols[t]();
        }
        static bool hasToken(String t);
        static vString queryTokens();
    };


    template <unsigned long hash>
    class BuiltinConstant final : public Constant {
        static const Constant::Recorder _recorder;
        static pSymbol make() noexcept {
            return std::make_shared<BuiltinConstant>();
        }
    public:
        BuiltinConstant() noexcept :
                Constant("{constant}", nan()) {}
        virtual ~BuiltinConstant() noexcept {}
        virtual double evaluate() const noexcept {
            return value;
        }
    };


    class Operator : public Evaluable {
    protected:
        struct Recorder {
            Recorder(const String &t, fSymbolGen g) noexcept;
        };
        static mSymbolGen _symbols;

        pEvaluable _left_operand;
        pEvaluable _right_operand;

        Operator(const String &t, unsigned p, bool l) noexcept :
                Evaluable(t, Type::OPERATOR),
                _left_operand(castChild<Evaluable>(EmptyEvaluable::make())),
                _right_operand(castChild<Evaluable>(EmptyEvaluable::make())),
                precedence(p),
                left_assoc(l) {}

    public:
        const unsigned precedence;
        const bool left_assoc;

        virtual ~Operator() noexcept {}
        void addBranches(pEvaluable l, pEvaluable r) noexcept;
        virtual double evaluate() const noexcept = 0;
        virtual void print(Stream &stream, String ind="") const noexcept;

        static pSymbol make(String t) {
            return _symbols[t]();
        }
        static bool hasToken(String t);
        static vString queryTokens();
    };


    template <unsigned long hash>
    class BuiltinOperator final : public Operator {
        static const Operator::Recorder _recorder;
        static pSymbol make() noexcept {
            return std::make_shared<BuiltinOperator>();
        }
    public:
        BuiltinOperator() noexcept :
                Operator("{operator}", 0, true) {}
        virtual ~BuiltinOperator() noexcept {}
        virtual double evaluate() const noexcept { return nan(); }
    };


    class Function : public Evaluable {
    protected:
        struct Recorder {
            Recorder(const String &t, fSymbolGen g) noexcept;
        };
        static mSymbolGen _symbols;

        vEvaluable _operands;

        Function(const String &t, unsigned s) noexcept :
                Evaluable(t, Type::FUNCTION),
                _operands(s, castChild<Evaluable>(EmptyEvaluable::make())),
                args(s) {}

    public:
        const unsigned args;

        virtual ~Function() noexcept {}
        void addBranches(const vEvaluable &x) noexcept;
        virtual double evaluate() const noexcept = 0;
        virtual void print(Stream &stream, String ind="") const noexcept;

        static pSymbol make(String t) {
            return _symbols[t]();
        }
        static bool hasToken(String t);
        static vString queryTokens();
    };


    template <unsigned long hash>
    class BuiltinFunction final : public Function {
        static const Function::Recorder _recorder;
        static pSymbol make() noexcept {
            return std::make_shared<BuiltinFunction>();
        }
    public:
        BuiltinFunction() noexcept :
                 Function("{function}", 0) {}
        virtual ~BuiltinFunction() {}
        virtual double evaluate() const noexcept { return nan(); }
    };

}

#endif
