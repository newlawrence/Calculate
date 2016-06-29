#ifndef __CALCULATE_H__
#define __CALCULATE_H__

#include <regex>
#include <queue>
#include <stack>

#include "symbols.h"

namespace calculate {
    typedef std::string String;
    typedef std::regex Regex;
    typedef std::smatch Match;
    typedef std::sregex_iterator iRegex;

    typedef symbols::pSymbol pSymbol;
    typedef symbols::vSymbol vSymbol;
    typedef symbols::Operator Operator;
    typedef symbols::Type Type;
    typedef std::queue<pSymbol> qSymbol;
    typedef std::stack<pSymbol> sSymbol;


    qSymbol tokenize(const String &expression);
    qSymbol shuntingYard(qSymbol &&infix);
    double evaluate(qSymbol &&postfix);
    double calculate(const String &expression);
}

#endif
