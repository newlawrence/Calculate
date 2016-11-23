#ifndef __CALCULATE_EXCEPTIONS_HPP__
#define __CALCULATE_EXCEPTIONS_HPP__

#include <stdexcept>

using namespace std;


#define DEFINE_EXCEPTION(NAME, MESSAGE)                                       \
namespace calculate_exceptions {                                              \
    struct NAME : public BaseCalculateException {                             \
        NAME() :                                                              \
               BaseCalculateException(string(MESSAGE)) {}                     \
        NAME(const string &token) :                                           \
               BaseCalculateException(string(MESSAGE) + " '" + token + "'") {}\
    };                                                                        \
}


namespace calculate_exceptions {

    struct BaseCalculateException : public runtime_error {
        BaseCalculateException(const string &what) :
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
