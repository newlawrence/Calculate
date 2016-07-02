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

        qSymbol tokenize(const String &expression);
        qSymbol shuntingYard(qSymbol &&infix);
        pSymbol buildTree(qSymbol &&postfix);

    public:
        Calculate(const String &expression, const vString &vars={});
    };
}

#endif
