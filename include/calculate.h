#ifndef __CALCULATE_H__
#define __CALCULATE_H__

#include <regex>
#include <queue>
#include <stack>

#include "symbols.h"

namespace calculate {
    using String = std::string;
    using pSymbol = symbols::pSymbol;
    using vSymbol = symbols::vSymbol;
    using qSymbol = std::queue<pSymbol>;
    using sSymbol =std::stack<pSymbol>;
    using Type = symbols::Type;


    qSymbol tokenize(const String &expression);
    qSymbol shuntingYard(qSymbol &&infix);
    double evaluate(qSymbol &&postfix);
    double calculate(const String &expression);
}

#endif
