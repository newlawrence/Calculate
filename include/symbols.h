#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__

#include <memory>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>
#include <limits>
#include <cmath>

namespace symbols {
    using String = std::string;
    using vName = std::vector<double>;
    using mValue = std::unordered_map<String, double>;

    class Symbol;
    using pSymbol = std::shared_ptr<Symbol>;
    using vSymbol = std::vector<pSymbol>;
    using fSymbolGen = pSymbol (*)();
    using mSymbolGen = std::unordered_map<String, fSymbolGen>;

    enum Type {
        CONSTANT, LEFTPARENS, RIGHTPARENS, SEPARATOR, OPERATOR, FUNCTION
    };

    template<typename T>
    T* castChild(pSymbol o) {
        return dynamic_cast<T *>(o.get());
    }
    pSymbol newSymbol(const String &t);


    class Symbol {
        Symbol() = delete;
        Symbol(const Symbol&) = delete;
        Symbol(Symbol&&) = delete;
        Symbol& operator=(const Symbol&) = delete;

    protected:
        Symbol(const String &t, Type y) : token(t), type(y) {}

    public:
        const String token;
        const Type type;
        bool is(Type y) {return type == y;}
        virtual double evaluate() const = 0;
    };


    class Constant : public Symbol {
    protected:
        struct Recorder {
            Recorder(const String &t, double v);
        };
        static mValue _symbols;

        Constant(const String &s) :
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
            s == '(' ? Type::LEFTPARENS : Type::RIGHTPARENS;
        constexpr static const char _symbol[2] = {s, '\0'};

        Parenthesis() : Symbol(_symbol, _type) {}

    public:
        virtual double evaluate() const {
            return std::numeric_limits<double>::quiet_NaN();
        };

        friend pSymbol newSymbol(const String &t);
    };
    template<char s> constexpr const char Parenthesis<s>::_symbol[2];

    template class Parenthesis<'('>;
    template class Parenthesis<')'>;


    class Separator final : public Symbol {
        Separator() : Symbol(",", Type::SEPARATOR) {}

    public:
        virtual double evaluate() const {
            return std::numeric_limits<double>::quiet_NaN();
        };

        friend pSymbol newSymbol(const String &t);
    };


    class Operator : public Symbol {
    protected:
        struct Recorder {
            Recorder(const String &t, fSymbolGen g);
        };
        static mSymbolGen _symbols;
        static String _regex_simple;
        static String _regex_composite;

        pSymbol _left_operand;
        pSymbol _right_operand;

        Operator(const String &t, unsigned p, bool l) :
            Symbol(t, Type::OPERATOR), precedence(p), left_assoc(l) {}

    public:
        const unsigned precedence;
        const bool left_assoc;
        void addBranches(pSymbol l, pSymbol r);
        virtual double evaluate() const = 0;

        static String symbolsRegex();
        friend pSymbol newSymbol(const String &t);
    };


    class Function : public Symbol {
    protected:
        struct Recorder {
            Recorder(const String &t, fSymbolGen g);
        };
        static mSymbolGen _symbols;

        vSymbol _operands;

        Function(const String &t, unsigned s) :
            Symbol(t, Type::FUNCTION), args(s), _operands(s) {}

    public:
        const unsigned args;
        void addBranches(vSymbol &&x);
        virtual double evaluate() const = 0;

        friend pSymbol newSymbol(const String &t);
    };
}


#define RECORD_CONSTANT(TOKEN,VALUE)                                          \
class Constant##TOKEN final : public Constant {                               \
    static const Constant::Recorder _recorder;                                \
};                                                                            \
const Constant::Recorder Constant##TOKEN::_recorder =                         \
    Constant::Recorder(#TOKEN, VALUE);

#define RECORD_OPERATOR(NAME,TOKEN,PREC,LASSOC,FUNC)                          \
class Operator##NAME final : public Operator {                                \
    static const Operator::Recorder _recorder;                                \
    static pSymbol newOperator() {return pSymbol(new Operator##NAME);}        \
                                                                              \
    Operator##NAME() : Operator(#TOKEN, PREC, LASSOC) {}                      \
                                                                              \
public:                                                                       \
    virtual double evaluate() const {                                         \
        double a = _left_operand->evaluate();                                 \
        double b = _right_operand->evaluate();                                \
        return FUNC;                                                          \
    }                                                                         \
};                                                                            \
const Operator::Recorder Operator##NAME::_recorder =                          \
    Operator::Recorder(#TOKEN, &Operator##NAME::newOperator);

#define RECORD_FUNCTION(TOKEN,ARGS,FUNC)                                      \
class Function_##TOKEN final : public Function {                              \
    static const Function::Recorder _recorder;                                \
    static pSymbol newFunction() {return pSymbol(new Function_##TOKEN);}      \
                                                                              \
    Function_##TOKEN() : Function(#TOKEN, ARGS) {}                            \
                                                                              \
public:                                                                       \
    virtual double evaluate() const {                                         \
        vName x(args);                                                        \
        for (unsigned i = 0; i < args; i++)                                   \
            x[i] = _operands[i]->evaluate();                                  \
        return FUNC;                                                          \
    }                                                                         \
};                                                                            \
const Function::Recorder Function_##TOKEN::_recorder =                        \
    Function::Recorder(#TOKEN, &Function_##TOKEN::newFunction);

#endif
