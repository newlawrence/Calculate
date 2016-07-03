#ifndef __CALCULATE_H__
#define __CALCULATE_H__

#include <iostream>
#include <memory>
#include <algorithm>
#include <exception>
#include <regex>
#include <vector>
#include <queue>
#include <stack>

#include "symbols.h"


namespace calculate {

    using symbols::BaseSymbolException;

    using pValue = std::unique_ptr<double[]>;
    using vValue = std::vector<double>;
    using String = std::string;
    using vString = std::vector<String>;
    using Regex = std::regex;

    using pSymbol = symbols::pSymbol;
    using vSymbol = symbols::vSymbol;
    using qSymbol = std::queue<pSymbol>;
    using sSymbol = std::stack<pSymbol>;


    struct EmptyExpressionException : public BaseSymbolException {
        const char* what() const noexcept {
            return "Empty expression";
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
        qSymbol shuntingYard(qSymbol &&infix) const;
        pSymbol buildTree(qSymbol &&postfix) const;

    public:
        const String expression;
        const vString variables;

        Calculate() = delete;
        Calculate& operator=(const Calculate &other) = delete;
        Calculate& operator=(Calculate &&other) = delete;

        Calculate(const String &expr, const vString &vars={});
        Calculate(const Calculate &other);
        Calculate(Calculate &&other);
        bool operator==(const Calculate &other) const noexcept;

        double operator() () const {
            return _tree->evaluate();
        };

        double operator() (double value) const {
            if (variables.size() < 1)
                throw EvaluationException();
            _values[variables.size() - 1] = value;
            return _tree->evaluate();
        }

        double operator() (vValue values) const {
            if (values.size() != variables.size())
                throw EvaluationException();
            for (auto i = 0; i < values.size(); i++)
                _values[i] = values[i];
            return _tree->evaluate();
        }

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
