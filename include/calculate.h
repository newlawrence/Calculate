#ifndef __CALCULATE_H__
#define __CALCULATE_H__

#ifdef __cplusplus

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <regex>

#include "calculate/symbols.hpp"


#define CALCULATE_EXCEPTION(NAME, MESSAGE)                                    \
struct NAME : public BaseCalculateException {                                 \
    const char* what() const noexcept {                                       \
        return MESSAGE;                                                       \
    }                                                                         \
};


namespace calculate {

    using pValue = std::unique_ptr<double[]>;
    using vValue = std::vector<double>;
    using String = std::string;
    using vString = std::vector<String>;
    using Regex = std::regex;

    using symbols::Type;
    using symbols::pSymbol;
    using symbols::vSymbol;
    using qSymbol = std::queue<pSymbol>;
    using sSymbol = std::stack<pSymbol>;


    struct BaseCalculateException : public std::exception {};
    CALCULATE_EXCEPTION(EmptyExpressionException, "Empty expression")
    CALCULATE_EXCEPTION(UndefinedSymbolException, "Undefined symbol")
    CALCULATE_EXCEPTION(BadNameException, "Unsuitable variable name")
    CALCULATE_EXCEPTION(DuplicateNameException, "Duplicated names")
    CALCULATE_EXCEPTION(ParenthesisMismatchException, "Parenthesis mismatch")
    CALCULATE_EXCEPTION(MissingArgumentsException, "Missing arguments")
    CALCULATE_EXCEPTION(ArgumentsExcessException, "Too many arguments")
    CALCULATE_EXCEPTION(SyntaxErrorException, "Syntax error")
    CALCULATE_EXCEPTION(WrongArgumentsException, "Arguments mismatch")


    class Calculate final {
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
            if (sizeof...(tail) + 1 > variables.size())
                throw WrongArgumentsException();
            _values[variables.size() - sizeof...(tail) - 1] = head;
            return this->operator() (tail...);
        };
    };

}

#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef void* calculate_Expression;

struct _calculate_c_library {
    calculate_Expression (*createExpression)(const char*, const char*, char*);
    calculate_Expression (*newExpression)(const char*, const char*);
    void (*freeExpression)(calculate_Expression);

    const char* (*getExpression)(calculate_Expression);
    int (*getVariables)(calculate_Expression);

    double (*evaluateArray)(calculate_Expression, double*, int, char*);
    double (*evalArray)(calculate_Expression, double*, int);
    double (*eval)(calculate_Expression, ...);
};

#ifdef __cplusplus
}
#else
#define calculate_init() \
const struct _calculate_c_library* const calculate = _get_calculate_c_library()
const struct _calculate_c_library* _get_calculate_c_library();
#endif

#endif
