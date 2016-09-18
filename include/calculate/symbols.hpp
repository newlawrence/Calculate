#ifndef __CALCULATE_SYMBOLS_HPP__
#define __CALCULATE_SYMBOLS_HPP__

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>


#define RECORD_CONSTANT(TOKEN, VALUE)                                         \
namespace calculate_symbols {                                                 \
    class Constant_##TOKEN final : public Constant {                          \
        static const Constant::Recorder _recorder;                            \
        static pSymbol newConstant() noexcept {                               \
            return std::make_shared<Constant_##TOKEN>();                      \
        }                                                                     \
    public:                                                                   \
        Constant_##TOKEN() noexcept :                                         \
                Constant(std::to_string(VALUE)) {}                            \
        virtual ~Constant_##TOKEN() {}                                        \
        virtual double evaluate() const noexcept { return VALUE; }            \
    };                                                                        \
    const Constant::Recorder Constant_##TOKEN::_recorder =                    \
        Constant::Recorder(#TOKEN, &Constant_##TOKEN::newConstant);           \
}


#define RECORD_OPERATOR(NAME, TOKEN, PRECEDENCE, L_ASSOCIATION, FUNCTION)     \
namespace calculate_symbols {                                                 \
    class Operator_##NAME final : public Operator {                           \
        static const Operator::Recorder _recorder;                            \
        static pSymbol newOperator() noexcept {                               \
            return std::make_shared<Operator_##NAME>();                       \
        }                                                                     \
    public:                                                                   \
        Operator_##NAME() noexcept :                                          \
                Operator(TOKEN, PRECEDENCE, L_ASSOCIATION) {}                 \
        virtual ~Operator_##NAME() {}                                         \
        virtual double evaluate() const noexcept {                            \
            double a = _left_operand->evaluate();                             \
            double b = _right_operand->evaluate();                            \
            return FUNCTION;                                                  \
        }                                                                     \
    };                                                                        \
    const Operator::Recorder Operator_##NAME::_recorder =                     \
        Operator::Recorder(TOKEN, &Operator_##NAME::newOperator);             \
}


#define RECORD_FUNCTION(TOKEN, ARGS, FUNCTION)                                \
namespace calculate_symbols {                                                 \
    class Function_##TOKEN final : public Function {                          \
        static const Function::Recorder _recorder;                            \
        static pSymbol newFunction() noexcept {                               \
            return std::make_shared<Function_##TOKEN>();                      \
        }                                                                     \
    public:                                                                   \
        Function_##TOKEN() noexcept :                                         \
                 Function(#TOKEN, ARGS) {}                                    \
        virtual ~Function_##TOKEN() {}                                        \
        virtual double evaluate() const noexcept {                            \
            vValue x(args);                                                   \
            for (auto i = 0u; i < args; i++)                                  \
                x[i] = _operands[i]->evaluate();                              \
            return FUNCTION;                                                  \
        }                                                                     \
    };                                                                        \
    const Function::Recorder Function_##TOKEN::_recorder =                    \
        Function::Recorder(#TOKEN, &Function_##TOKEN::newFunction);           \
}


namespace calculate_symbols {

    using String = std::string;
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

    pSymbol newSymbol(double *v);
    pSymbol newSymbol(const String &t);


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

        virtual ~Symbol() = 0;
        bool is(Type y) const noexcept { return type == y; }
    };
    inline Symbol::~Symbol() {}


    template<char s>
    class Parenthesis final : public Symbol {
        constexpr static const Type _type =
            s == '(' ? Type::LEFT : Type::RIGHT;
        constexpr static const char _symbol[2] = {s, '\0'};

    public:
        Parenthesis() noexcept :
            Symbol(_symbol, _type) {}
        virtual ~Parenthesis() {}
    };
    template<char s> constexpr const char Parenthesis<s>::_symbol[2];


    class Separator final : public Symbol {
    public:
        Separator() noexcept :
            Symbol(",", Type::SEPARATOR) {}
        virtual ~Separator() {}
    };


    class Evaluable : public Symbol {
    protected:
        Evaluable(const String &t, Type y) noexcept :
                Symbol(t, y) {}

    public:
        virtual ~Evaluable() = 0;
        virtual double evaluate() const noexcept = 0;
    };
    inline Evaluable::~Evaluable() {}


    class Variable final : public Evaluable {
    public:
        const double *_value;

        Variable(double *v) noexcept :
            Evaluable("var", Type::CONSTANT),
            _value(v) {}
        virtual ~Variable() {}
        virtual double evaluate() const noexcept { return *_value; }
    };


    class Constant : public Evaluable {
    protected:
        struct Recorder {
            Recorder(const String &t, fSymbolGen g) noexcept;
        };
        static mSymbolGen _symbols;

    public:
        const double value;

        Constant(const String &s) noexcept :
            Evaluable(s, Type::CONSTANT),
            value(std::stod(s)) {}
        virtual ~Constant() {};
        virtual double evaluate() const noexcept { return value; }

        friend pSymbol newSymbol(const String &t);
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
                precedence(p),
                left_assoc(l) {}

    public:
        const unsigned precedence;
        const bool left_assoc;

        virtual ~Operator() {}
        void addBranches(pEvaluable l, pEvaluable r) noexcept;
        virtual double evaluate() const noexcept = 0;

        friend pSymbol newSymbol(const String &t);
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
                _operands(s),
                args(s) {}

    public:
        const unsigned args;

        virtual ~Function() {}
        void addBranches(vEvaluable &&x) noexcept;
        virtual double evaluate() const noexcept = 0;

        friend pSymbol newSymbol(const String &t);
    };

}

#endif
