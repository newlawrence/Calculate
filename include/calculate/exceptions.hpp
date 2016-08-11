#ifndef __CALCULATE_EXCEPTIONS_HPP__
#define __CALCULATE_EXCEPTIONS_HPP__

#include <exception>


#define DEFINE_EXCEPTION(NAME, MESSAGE)                                       \
namespace calculate_exceptions {                                              \
    struct NAME : public BaseCalculateException {                             \
        const char* what() const noexcept {                                   \
            return MESSAGE;                                                   \
        }                                                                     \
    };                                                                        \
}


namespace calculate_exceptions {

    struct BaseCalculateException : public std::exception {};

}


DEFINE_EXCEPTION(EmptyExpressionException, "Empty expression")
DEFINE_EXCEPTION(UndefinedSymbolException, "Undefined symbol")
DEFINE_EXCEPTION(BadNameException, "Unsuitable variable name")
DEFINE_EXCEPTION(DuplicateNameException, "Duplicated names")
DEFINE_EXCEPTION(ParenthesisMismatchException, "Parenthesis mismatch")
DEFINE_EXCEPTION(MissingArgumentsException, "Missing arguments")
DEFINE_EXCEPTION(ArgumentsExcessException, "Too many arguments")
DEFINE_EXCEPTION(SyntaxErrorException, "Syntax error")
DEFINE_EXCEPTION(WrongArgumentsException, "Arguments mismatch")

#endif
