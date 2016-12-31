#ifndef __CALCULATE_EXCEPTIONS_HPP__
#define __CALCULATE_EXCEPTIONS_HPP__

#include "calculate/definitions.hpp"


#define DEFINE_EXCEPTION(NAME, MESSAGE)                                       \
namespace calculate_exceptions {                                              \
    struct NAME : public BaseCalculateException {                             \
        NAME() :                                                              \
            BaseCalculateException(String(MESSAGE)) {}                        \
        NAME(const String &token) :                                           \
            BaseCalculateException(String(MESSAGE) + " '" + token + "'") {}   \
    };                                                                        \
}


namespace calculate_exceptions {

    using namespace calculate_definitions;

    struct BaseCalculateException : public std::runtime_error {
        BaseCalculateException(const String &what) :
            runtime_error(what) {}
    };

}


DEFINE_EXCEPTION(EmptyExpressionException, "Empty expression")
DEFINE_EXCEPTION(SyntaxErrorException, "Syntax error")
DEFINE_EXCEPTION(ParenthesisMismatchException, "Parenthesis mismatch")
DEFINE_EXCEPTION(UndefinedSymbolException, "Undefined symbol")
DEFINE_EXCEPTION(BadNameException, "Unsuitable variable name")
DEFINE_EXCEPTION(DuplicatedNameException, "Duplicated names")
DEFINE_EXCEPTION(MissingArgumentsException, "Missing arguments")
DEFINE_EXCEPTION(ArgumentsExcessException, "Too many arguments")
DEFINE_EXCEPTION(WrongVariablesException, "Variables mismatch")

#endif
