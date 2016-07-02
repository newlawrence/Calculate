#ifndef __CALCULATE_H__
#define __CALCULATE_H__

#include <memory>
#include <algorithm>
#include <regex>
#include <vector>
#include <queue>
#include <stack>

#include "symbols.h"

namespace calculate {
    using pValue = std::unique_ptr<double[]>;
    using vValue = std::vector<double>;
    using String = std::string;
    using vString = std::vector<String>;
    using Regex = std::regex;

    using pSymbol = symbols::pSymbol;
    using vSymbol = symbols::vSymbol;
    using qSymbol = std::queue<pSymbol>;
    using sSymbol = std::stack<pSymbol>;

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
            _values[variables.size() - 1] = value;
            return _tree->evaluate();
        }

        double operator() (vValue values) const {
            for (auto i = 0; i < values.size(); i++)
                _values[i] = values[i];
            return _tree->evaluate();
        }

        template <typename Head, typename... Tail>
        double operator() (Head head, Tail... tail) const {
            _values[variables.size() - sizeof...(tail) - 1] = head;
            return this->operator() (tail...);
        };
    };
}

#endif
