#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__

#ifdef __cplusplus

#include <memory>
#include <algorithm>
#include <exception>
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>


namespace symbols {

    using String = std::string;
    using vName = std::vector<double>;
    using mValue = std::unordered_map<String, double>;

    class Symbol;
    using pSymbol = std::shared_ptr<Symbol>;
    using vSymbol = std::vector<pSymbol>;
    using fSymbolGen = pSymbol (*)();
    using mSymbolGen = std::unordered_map<String, fSymbolGen>;


    struct BaseSymbolException : public std::exception {};

    struct BadCastException : public BaseSymbolException {
        const char* what() const noexcept {
            return "Bad casting of Operator or Function";
        }
    };

    struct NotEvaluableException : public BaseSymbolException {
        const char* what() const noexcept {
            return "Call to a non evaluable symbol";
        }
    };

    struct UndefinedSymbolException : public BaseSymbolException {
        const char* what() const noexcept {
            return "Undefined symbol";
        }
    };


    enum Type {CONSTANT, LEFT, RIGHT, SEPARATOR, OPERATOR, FUNCTION};


    template<typename T>
    T* castChild(pSymbol o) {
        if (auto child = dynamic_cast<T *>(o.get()))
            return child;
        else
            throw BadCastException();
    }
    pSymbol newSymbol(double *v);
    pSymbol newSymbol(const String &t);


    class Symbol {
        Symbol() = delete;
        Symbol(const Symbol&) = delete;
        Symbol(Symbol&&) = delete;
        Symbol& operator=(const Symbol&) = delete;

    protected:
        Symbol(const String &t, Type y) noexcept :
            token(t), type(y) {}

    public:
        const String token;
        const Type type;
        bool is(Type y) noexcept {return type == y;}
        virtual double evaluate() const = 0;
    };


    class Variable final : public Symbol {
        Variable(double *v) noexcept :
            Symbol("var", Type::CONSTANT), _value(v) {}

    public:
        const double *_value;
        virtual double evaluate() const {return *_value;}

        friend pSymbol newSymbol(double *v);
    };


    class Constant : public Symbol {
    protected:
        struct Recorder {
            Recorder(const String &t, double v) noexcept;
        };
        static mValue _symbols;

        Constant(const String &s) noexcept :
            Symbol(s, Type::CONSTANT), value(std::stod(s)) {}

    public:
        const double value;
        virtual double evaluate() const {return value;}

        friend void recordConstant(const String &t, double v);
        friend pSymbol newSymbol(const String &t);
    };


    template<char s>
    class Parenthesis final : public Symbol {
        constexpr static const Type _type =
            s == '(' ? Type::LEFT : Type::RIGHT;
        constexpr static const char _symbol[2] = {s, '\0'};

        Parenthesis() noexcept :
            Symbol(_symbol, _type) {}

    public:
        virtual double evaluate() const {
            throw NotEvaluableException();
        };

        friend pSymbol newSymbol(const String &t);
    };
    template<char s> constexpr const char Parenthesis<s>::_symbol[2];


    class Separator final : public Symbol {
        Separator() noexcept
            : Symbol(",", Type::SEPARATOR) {}

    public:
        virtual double evaluate() const {
            throw NotEvaluableException();
        };

        friend pSymbol newSymbol(const String &t);
    };


    class Operator : public Symbol {
    protected:
        struct Recorder {
            Recorder(const String &t, fSymbolGen g) noexcept;
        };
        static mSymbolGen _symbols;
        static String _regex_simple;
        static String _regex_composite;

        pSymbol _left_operand;
        pSymbol _right_operand;

        Operator(const String &t, unsigned p, bool l) noexcept :
            Symbol(t, Type::OPERATOR), precedence(p), left_assoc(l) {}

    public:
        const unsigned precedence;
        const bool left_assoc;
        void addBranches(pSymbol l, pSymbol r) noexcept;
        virtual double evaluate() const = 0;

        static String getSymbolsRegex() noexcept;
        friend pSymbol newSymbol(const String &t);
    };


    class Function : public Symbol {
    protected:
        struct Recorder {
            Recorder(const String &t, fSymbolGen g) noexcept;
        };
        static mSymbolGen _symbols;

        vSymbol _operands;

        Function(const String &t, unsigned s) noexcept :
            Symbol(t, Type::FUNCTION), _operands(s), args(s) {}

    public:
        const unsigned args;
        void addBranches(vSymbol &&x) noexcept;
        virtual double evaluate() const = 0;

        friend pSymbol newSymbol(const String &t);
    };

}


#define RECORD_CONSTANT(TOKEN,VALUE)                                          \
class Constant_##TOKEN final : public Constant {                              \
    static const Constant::Recorder _recorder;                                \
};                                                                            \
const Constant::Recorder Constant_##TOKEN::_recorder =                        \
    Constant::Recorder(#TOKEN, VALUE);

#define RECORD_OPERATOR(NAME,TOKEN,PREC,LASSOC,FUNC)                          \
class Operator_##NAME final : public Operator {                               \
    static const Operator::Recorder _recorder;                                \
    static pSymbol newOperator() noexcept {                                   \
        return pSymbol(new Operator_##NAME);                                  \
    }                                                                         \
                                                                              \
    Operator_##NAME() noexcept :                                              \
        Operator(#TOKEN, PREC, LASSOC) {}                                     \
                                                                              \
public:                                                                       \
    virtual double evaluate() const {                                         \
        double a = _left_operand->evaluate();                                 \
        double b = _right_operand->evaluate();                                \
        return FUNC;                                                          \
    }                                                                         \
};                                                                            \
const Operator::Recorder Operator_##NAME::_recorder =                         \
    Operator::Recorder(#TOKEN, &Operator_##NAME::newOperator);

#define RECORD_FUNCTION(TOKEN,ARGS,FUNC)                                      \
class Function_##TOKEN final : public Function {                              \
    static const Function::Recorder _recorder;                                \
    static pSymbol newFunction() noexcept {                                   \
        return pSymbol(new Function_##TOKEN);                                 \
    }                                                                         \
                                                                              \
    Function_##TOKEN() noexcept :                                             \
        Function(#TOKEN, ARGS) {}                                             \
                                                                              \
public:                                                                       \
    virtual double evaluate() const {                                         \
        vName x(args);                                                        \
        for (auto i = 0u; i < args; i++)                                       \
            x[i] = _operands[i]->evaluate();                                  \
        return FUNC;                                                          \
    }                                                                         \
};                                                                            \
const Function::Recorder Function_##TOKEN::_recorder =                        \
    Function::Recorder(#TOKEN, &Function_##TOKEN::newFunction);

#endif

#endif
