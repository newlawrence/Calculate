#ifndef __CALCULATE_H__
#define __CALCULATE_H__

#ifdef __cplusplus

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <regex>

#include "symbols.h"


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


    struct EmptyExpressionException : public BaseSymbolException {
        const char* what() const noexcept {
            return "Empty expression";
        }
    };

    struct BadNameException : public BaseSymbolException {
        const char* what() const noexcept {
            return "Unsuitable variable name";
        }
    };

    struct DuplicateNameException : public BaseSymbolException {
        const char* what() const noexcept {
            return "Duplicated names";
        }
    };

    struct ParenthesisMismatchException : public BaseSymbolException {
        const char* what() const noexcept {
            return "Parenthesis mismatch";
        }
    };

    struct MissingArgumentsException : public BaseSymbolException {
        const char* what() const noexcept {
            return "Missing arguments";
        }
    };

    struct ConstantsExcessException : public BaseSymbolException {
        const char* what() const noexcept {
            return "Too many arguments";
        }
    };

    struct SyntaxErrorException : public BaseSymbolException {
        const char* what() const noexcept {
            return "Syntax error";
        }
    };

    struct EvaluationException : public BaseSymbolException {
        const char* what() const noexcept {
            return "Arguments mismatch";
        }
    };


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

CALC_Expression CALC_newExpression(const char *expr, const char *vars);
const char* CALC_getExpression(CALC_Expression cexpr);
int CALC_getVariables(CALC_Expression cexpr);
double CALC_evaluate(CALC_Expression cexpr, ...);
double CALC_evalArray(CALC_Expression cexpr, double *v, unsigned s);
void CALC_freeExpression(CALC_Expression cexpr);

#ifdef __cplusplus
}
#endif

#endif
