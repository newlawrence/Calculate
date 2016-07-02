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
        vString _names;
        pValue _values;
        pSymbol _tree;
        unsigned _arg;

        qSymbol tokenize(const String &expression);
        qSymbol shuntingYard(qSymbol &&infix);
        pSymbol buildTree(qSymbol &&postfix);

    public:
        Calculate(const String &expr, const vString &vars={});

        double operator() () {
            return _tree->evaluate();
        };

        double operator() (double value) {
            _values[_arg] = value;
            _arg = 0;
            return _tree->evaluate();
        }

        double operator() (vValue values) {
            for (auto i = 0; i < values.size(); i++)
                _values[i] = values[i];
            return _tree->evaluate();
        }

        template <typename Head, typename... Tail>
        double operator() (Head head, Tail... tail) {
            _values[_arg] = head;
            _arg++;
            return this->operator() (tail...);
        };
    };
}

#endif
