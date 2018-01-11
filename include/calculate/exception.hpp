#ifndef __CALCULATE_EXCEPTION_HPP__
#define __CALCULATE_EXCEPTION_HPP__

#include <stdexcept>
#include <string>


namespace calculate {

struct BaseError : public std::runtime_error {
    BaseError(const std::string& what) : runtime_error{what} {}
    BaseError() :
            runtime_error{std::string{"Base error: unexpected error"}}
    {}
};


struct BadCast : BaseError {
    BadCast(const std::string& token) :
            BaseError{
                "Bad cast: cannot perform numeric conversion: '" + token + "'"
            }
    {}
};

struct AccessViolation : BaseError {
    AccessViolation() :
            BaseError{
                "Access violation: call to a non const method from a const "
                "wrapper"
            }
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
    EmptyExpression() : BaseError{"empty expression"} {}
};

struct ParenthesisMismatch : BaseError {
    ParenthesisMismatch() : BaseError{"parenthesis mismatch"} {}
};

struct RepeatedSymbol : BaseError {
    const std::string token;
    RepeatedSymbol(const std::string& t) :
            BaseError{"Repeated symbol: '" + t + "'"},
            token{t}
    {}
};

struct SyntaxError : BaseError {
    SyntaxError() : BaseError{"Syntax error"} {}
    SyntaxError(const std::string& what) : BaseError{"Syntax error: " + what} {}
};

struct UndefinedSymbol : BaseError {
    const std::string token;
    UndefinedSymbol(const std::string& t) :
            BaseError{"Undefined symbol: '" + t + "'"},
            token{t}
    {}
};

struct UnsuitableName : BaseError {
    const std::string token;
    UnsuitableName(const std::string& t) :
            BaseError{"Unsuitable symbol name: '" + t + "'"},
            token{t}
    {}
};

struct UnusedSymbol : BaseError {
    const std::string token;
    UnusedSymbol(const std::string& t) :
            BaseError{"Unused symbol: '" + t + "'"}, token{t}
    {}
};

}

#endif