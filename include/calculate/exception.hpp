/*
    Calculate - Version 2.1.1rc10
    Last modified 2018/02/27
    Released under MIT license
    Copyright (c) 2016-2018 Alberto Lorenzo <alorenzo.md@gmail.com>
*/


#ifndef __CALCULATE_EXCEPTION_HPP__
#define __CALCULATE_EXCEPTION_HPP__

#include <stdexcept>
#include <string>


namespace calculate {

struct BaseError : public std::runtime_error {
    explicit BaseError(std::string what) :
            runtime_error{std::move(what)}
    {}

    BaseError() :
            runtime_error{"Base error: unexpected error"}
    {}
};


struct LexerError : BaseError {
    explicit LexerError(const std::string& what) :
            BaseError{"Lexer error: " + what}
    {}
};

struct BadCast : BaseError {
    explicit BadCast(const std::string& token) :
            BaseError{"Bad cast: cannot perform numeric conversion: '" + token + "'"}
    {}
};

struct ArgumentsMismatch : BaseError {
    const std::size_t needed;
    const std::size_t provided;

    ArgumentsMismatch(std::size_t n, std::size_t p) :
            BaseError{
                "Arguments mismatch: " + std::to_string(n) +
                " needed argument" + (n == 1 ? "" : "s") +
                " vs " + std::to_string(p) + " provided"
            },
            needed{n},
            provided{p}
    {}

    ArgumentsMismatch(const std::string& token, std::size_t n, std::size_t p) :
            BaseError{
                "Arguments mismatch: '" + token + "' " + std::to_string(n) +
                " needed argument" + (n == 1 ? "" : "s") +
                " vs " + std::to_string(p) + " provided"
            },
            needed{n},
            provided{p}
    {}
};

struct EmptyExpression : BaseError {
    EmptyExpression() :
            BaseError{"Empty expression"}
    {}
};

struct ParenthesisMismatch : BaseError {
    ParenthesisMismatch() :
            BaseError{"Parenthesis mismatch"}
    {}
};

struct RepeatedSymbol : BaseError {
    const std::string token;

    explicit RepeatedSymbol(const std::string& t) :
            BaseError{"Repeated symbol: '" + t + "'"},
            token{t}
    {}
};

struct SyntaxError : BaseError {
    explicit SyntaxError(const std::string& what) :
            BaseError{"Syntax error: " + what}
    {}

    SyntaxError() :
            BaseError{"Syntax error"}
    {}
};

struct UndefinedSymbol : BaseError {
    const std::string token;

    explicit UndefinedSymbol(const std::string& t) :
            BaseError{"Undefined symbol: '" + t + "'"},
            token{t}
    {}
};

struct UnsuitableName : BaseError {
    const std::string token;

    explicit UnsuitableName(const std::string& t) :
            BaseError{"Unsuitable symbol name: '" + t + "'"},
            token{t}
    {}
};

struct UnusedSymbol : BaseError {
    const std::string token;

    explicit UnusedSymbol(const std::string& t) :
            BaseError{"Unused symbol: '" + t + "'"},
            token{t}
    {}
};

}

#endif
