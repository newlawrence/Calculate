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
    class Symbol;
    class Operator;
    class Function;

    typedef std::string String;
    typedef std::vector<double> vName;
    typedef std::unordered_map<String, double> mValue;

    typedef std::shared_ptr<Symbol> pSymbol;
    typedef std::vector<pSymbol> vSymbol;
    typedef pSymbol (*fSymbolGen)();
    typedef std::unordered_map<String, fSymbolGen> mSymbolGen;

    enum Type {
        CONSTANT, LEFTPARENS, RIGHTPARENS, SEPARATOR, OPERATOR, FUNCTION
    };

    void recordConstant(const String &t, double v);
    String symbolsRegex();
    Operator* castOperator(pSymbol o);
    Function* castFunction(pSymbol f);
    pSymbol newSymbol(const String &t);


    class Symbol {
    protected:
        Symbol(const String &t, Type y) : token(t), type(y) {}
        Symbol() = delete;
        Symbol(const Symbol&) = delete;
        Symbol(Symbol&&) = delete;
        Symbol& operator=(const Symbol&) = delete;

    public:
        const String token;
        const Type type;
        bool is(Type y) {return type == y;}
        virtual double evaluate() const = 0;
    };


    class Constant final : public Symbol {
    private:
        static mValue _symbols;
        Constant(const String &s) :
            Symbol(s, Type::CONSTANT), value(std::stod(s)) {}
        Constant() = delete;
        Constant(const Constant&) = delete;
        Constant(Constant&&) = delete;
        Constant& operator=(const Constant&) = delete;

    public:
        const double value;
        virtual double evaluate() const {return value;}

        friend void recordConstant(const String &t, double v);
        friend pSymbol newSymbol(const String &t);
    };


    class Parenthesis : public Symbol {
    protected:
        Parenthesis(const String &t, Type y) : Symbol(t, y) {}
        Parenthesis() = delete;
        Parenthesis(const Parenthesis&) = delete;
        Parenthesis(Parenthesis&&) = delete;
        Parenthesis& operator=(const Parenthesis&) = delete;

    public:
        virtual double evaluate() const = 0;
    };

    class ParenthesisLeft final : public Parenthesis {
    private:
        ParenthesisLeft() : Parenthesis("(", Type::LEFTPARENS) {}
        ParenthesisLeft(const ParenthesisLeft&) = delete;
        ParenthesisLeft(ParenthesisLeft&&) = delete;
        ParenthesisLeft& operator=(const ParenthesisLeft&) = delete;

    public:
        virtual double evaluate() const {
            return std::numeric_limits<double>::quiet_NaN();
        };

        friend pSymbol newSymbol(const String &t);
    };

    class ParenthesisRight final : public Parenthesis {
    private:
        ParenthesisRight() : Parenthesis(")", Type::RIGHTPARENS) {}
        ParenthesisRight(const ParenthesisRight&) = delete;
        ParenthesisRight(ParenthesisRight&&) = delete;
        ParenthesisRight& operator=(const ParenthesisRight&) = delete;

    public:
        virtual double evaluate() const {
            return std::numeric_limits<double>::quiet_NaN();
        };

        friend pSymbol newSymbol(const String &t);
    };


    class Separator final : public Symbol {
    private:
        Separator() : Symbol(",", Type::SEPARATOR) {}
        Separator(const Separator&) = delete;
        Separator(Separator&&) = delete;
        Separator& operator=(const Separator&) = delete;

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

        Operator(const String &t, size_t p, bool l) :
            Symbol(t, Type::OPERATOR), precedence(p), left_assoc(l) {}
        Operator() = delete;\
        Operator(const Operator&) = delete;\
        Operator(Operator&&) = delete;\
        Operator& operator=(const Operator&) = delete;

    public:
        const size_t precedence;
        const bool left_assoc;
        void addBranches(pSymbol l, pSymbol r);
        virtual double evaluate() const = 0;

        friend String symbolsRegex();
        friend pSymbol newSymbol(const String &t);
    };


    class Function : public Symbol {
    protected:
        struct Recorder {
            Recorder(const String &t, fSymbolGen g);
        };
        static mSymbolGen _symbols;

        vSymbol _operands;

        Function(const String &t, size_t s) :
            Symbol(t, Type::FUNCTION), args(s), _operands(s) {}
        Function() = delete;\
        Function(const Function&) = delete;\
        Function(Function&&) = delete;\
        Function& operator=(const Function&) = delete;

    public:
        const size_t args;
        void addBranches(vSymbol &&x);
        virtual double evaluate() const = 0;

        friend pSymbol newSymbol(const String &t);
    };
}


#define RECORD_OPERATOR(NAME,TOKEN,PREC,LASSOC,FUNC)\
class Operator##NAME final : public Operator {\
private:\
    static const Operator::Recorder _recorder;\
    static pSymbol newOperator() {return pSymbol(new Operator##NAME);}\
    Operator##NAME() : Operator(#TOKEN, PREC, LASSOC) {}\
    Operator##NAME(const Operator##NAME&) = delete;\
    Operator##NAME(Operator##NAME&&) = delete;\
    Operator##NAME& operator=(const Operator##NAME&) = delete;\
public:\
    virtual double evaluate() const {\
        double a = _left_operand->evaluate();\
        double b = _right_operand->evaluate();\
        return FUNC;\
    }\
};\
const Operator::Recorder Operator##NAME::_recorder =\
    Operator::Recorder(#TOKEN, &Operator##NAME::newOperator);

#define RECORD_FUNCTION(TOKEN,ARGS,FUNC)\
class Function_##TOKEN final : public Function {\
private:\
    static const Function::Recorder _recorder;\
    static pSymbol newFunction() {return pSymbol(new Function_##TOKEN);}\
    Function_##TOKEN() : Function(#TOKEN, ARGS) {}\
    Function_##TOKEN(const Function_##TOKEN&) = delete;\
    Function_##TOKEN(Function_##TOKEN&&) = delete;\
    Function_##TOKEN& operator=(const Function_##TOKEN&) = delete;\
public:\
    virtual double evaluate() const {\
        vName x(args);\
        for (size_t i = 0; i < args; i++)\
            x[i] = _operands[i]->evaluate();\
        return FUNC;\
    }\
};\
const Function::Recorder Function_##TOKEN::_recorder =\
    Function::Recorder(#TOKEN, &Function_##TOKEN::newFunction);

#endif
