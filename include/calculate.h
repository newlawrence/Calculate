#ifndef __CALCULATE_H__
#define __CALCULATE_H__

#ifdef __cplusplus

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <regex>

#include "symbols.hpp"


#define CAST(expression) static_cast<calculate::Calculate*>(expression)
#define REV_CAST(expression) static_cast<CALC_Expression>(expression)


namespace calculate {

    using symbols::BaseSymbolException;
    using symbols::Type;

    using pValue = std::unique_ptr<double[]>;
    using vValue = std::vector<double>;
    using String = std::string;
    using vString = std::vector<String>;
    using Regex = std::regex;

    using symbols::pSymbol;
    using symbols::vSymbol;
    using qSymbol = std::queue<pSymbol>;
    using sSymbol = std::stack<pSymbol>;


    RECORD_EXCEPTION(EmptyExpressionException, "Empty expression")
    RECORD_EXCEPTION(BadNameException, "Unsuitable variable name")
    RECORD_EXCEPTION(DuplicateNameException, "Duplicated names")
    RECORD_EXCEPTION(ParenthesisMismatchException, "Parenthesis mismatch")
    RECORD_EXCEPTION(MissingArgumentsException, "Missing arguments")
    RECORD_EXCEPTION(ConstantsExcessException, "Too many arguments")
    RECORD_EXCEPTION(SyntaxErrorException, "Syntax error")
    RECORD_EXCEPTION(EvaluationException, "Arguments mismatch")


    class Calculate final {
        Regex _regex;
        pValue _values;
        pSymbol _tree;

        qSymbol tokenize(const String &expression) const;
        qSymbol check(qSymbol &&input) const;
        qSymbol shuntingYard(qSymbol &&infix) const;
        pSymbol buildTree(qSymbol &&postfix) const;

        static vString extract(const String &vars);
        static vString validate(const vString &vars);

        Calculate() = delete;
        Calculate& operator=(const Calculate &other) = delete;
        Calculate& operator=(Calculate &&other) = delete;

    public:
        const String expression;
        const vString variables;

        Calculate(const Calculate &other);
        Calculate(Calculate &&other);
        Calculate(const String &expr, const String &vars);
        Calculate(const String &expr, const vString &vars={});

        bool operator==(const Calculate &other) const noexcept;
        double operator() () const;
        double operator() (double value) const;
        double operator() (vValue values) const;

        template <typename Head, typename... Tail>
        double operator() (Head head, Tail... tail) const {
            if (sizeof...(tail) + 1 != variables.size())
                throw EvaluationException();
            _values[variables.size() - sizeof...(tail) - 1] = head;
            return this->operator() (tail...);
        };
    };

}

#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef void* CALC_Expression;
typedef const char* c_str;

CALC_Expression CALC_createExpression(c_str expr, c_str vars, char *errors);
CALC_Expression CALC_newExpression(c_str expr, c_str vars);
const char* CALC_getExpression(CALC_Expression c);
int CALC_getVariables(CALC_Expression c);
double CALC_evaluateArray(CALC_Expression c, double *v, int s, char *errors);
double CALC_evalArray(CALC_Expression c, double *v, int s);
double CALC_eval(CALC_Expression c, ...);
void CALC_freeExpression(CALC_Expression c);

#ifdef __cplusplus
}
#endif

#endif
