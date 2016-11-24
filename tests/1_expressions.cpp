#include <algorithm>
#include <sstream>

#include "catch.hpp"
#include "calculate.h"

using namespace calculate;
using namespace calculate_exceptions;


TEST_CASE("Constructors test", "[Constructors]") {

    SECTION("Constructors and assignments") {
        auto expr1 = Expression("1 + x", "x");
        auto expr2 = expr1;
        CHECK(expr1.expression() == expr2.expression());
        CHECK(expr1.variables() == expr2.variables());
        CHECK(expr1.infix() == expr2.infix());
        CHECK(expr1.postfix() == expr2.postfix());
        CHECK(expr1.tree() == expr2.tree());
        CHECK(static_cast<int>(expr1(2)) == static_cast<int>(expr2(2)));

        expr1 = Expression("1 + x", "x");
        expr2 = expr1;
        CHECK(expr1.expression() == expr2.expression());
        CHECK(expr1.variables() == expr2.variables());
        CHECK(expr1.infix() == expr2.infix());
        CHECK(expr1.postfix() == expr2.postfix());
        CHECK(expr1.tree() == expr2.tree());
        CHECK(static_cast<int>(expr1(2)) == static_cast<int>(expr2(2)));
    }

}


TEST_CASE("Wrong expressions", "[Expressions]") {

    SECTION("Empty expression") {
        CHECK_THROWS_AS(Expression(""), EmptyExpressionException);
    }

    SECTION("Undefined symbols") {
        CHECK_THROWS_AS(Expression("x"), UndefinedSymbolException);
        CHECK_THROWS_AS(Expression("1 + x"), UndefinedSymbolException);
        CHECK_THROWS_AS(Expression("x + 1"), UndefinedSymbolException);
        CHECK_THROWS_AS(Expression("1 + x 2"), UndefinedSymbolException);
        CHECK_THROWS_AS(Expression("pi", "x"), WrongVariablesException);
        CHECK_THROWS_AS(Expression("x + x", "x, y"), WrongVariablesException);
    }

    SECTION("Parenthesis mismatches") {
        CHECK_THROWS_AS(Expression("(1"), ParenthesisMismatchException);
        CHECK_THROWS_AS(Expression("1)"), ParenthesisMismatchException);
        CHECK_THROWS_AS(Expression("((1"), ParenthesisMismatchException);
        CHECK_THROWS_AS(Expression("1))"), ParenthesisMismatchException);
    }

    SECTION("First members") {
        CHECK_NOTHROW(Expression("1"));
        CHECK_NOTHROW(Expression("(1)"));
        CHECK_THROWS_AS(Expression(") + 1"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression(", 1"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("+ 1"), SyntaxErrorException);
        CHECK_NOTHROW(Expression("log(1)"));
    }

    SECTION("Members following constant") {
        CHECK_THROWS_AS(Expression("1 2"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("1 (2)"), SyntaxErrorException);
        CHECK_NOTHROW(Expression("(1)"));
        CHECK_NOTHROW(Expression("hypot(1, 2)"));
        CHECK_NOTHROW(Expression("1 + 2"));
        CHECK_THROWS_AS(Expression("1 log(2)"), SyntaxErrorException);
    }

    SECTION("Members following left parenthesis") {
        CHECK_NOTHROW(Expression("(1)"));
        CHECK_NOTHROW(Expression("((1))"));
        CHECK_THROWS_AS(Expression("()"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("hypot(, 1)"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("( + 1)"), SyntaxErrorException);
        CHECK_NOTHROW(Expression("(log(1))"));
    }

    SECTION("Members following right parenthesis") {
        CHECK_THROWS_AS(Expression("(1) 2"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("(1) (2)"), SyntaxErrorException);
        CHECK_NOTHROW(Expression("((1))"));
        CHECK_NOTHROW(Expression("hypot((1), 2)"));
        CHECK_NOTHROW(Expression("(1) + 2"));
        CHECK_THROWS_AS(Expression("(1) log(2)"), SyntaxErrorException);
    }

    SECTION("Members following separator") {
        CHECK_NOTHROW(Expression("hypot(1, 2)"));
        CHECK_NOTHROW(Expression("hypot(1, (2))"));
        CHECK_THROWS_AS(Expression("log )"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("hypot , 1)"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("log + 1)"), SyntaxErrorException);
        CHECK_NOTHROW(Expression("hypot(1, log(2))"));
    }

    SECTION("Members following operator") {
        CHECK_NOTHROW(Expression("1 + 2"));
        CHECK_NOTHROW(Expression("1 + (2)"));
        CHECK_THROWS_AS(Expression("1 + )"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("hypot(1 + , 2)"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("1 + + 2"), SyntaxErrorException);
        CHECK_NOTHROW(Expression("1 + log(2)"));
    }

    SECTION("Members following function") {
        CHECK_THROWS_AS(Expression("log 1"), SyntaxErrorException);
        CHECK_NOTHROW(Expression("log(1)"));
        CHECK_THROWS_AS(Expression("log )"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("hypot(, 1)"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("log(+ 1)"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("log log(1)"), SyntaxErrorException);
    }

    SECTION("Last members") {
        CHECK_NOTHROW(Expression("1"));
        CHECK_THROWS_AS(Expression("1 ("), SyntaxErrorException);
        CHECK_NOTHROW(Expression("(1)"));
        CHECK_THROWS_AS(Expression("1,"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("1 +"), SyntaxErrorException);
        CHECK_THROWS_AS(Expression("log"), SyntaxErrorException);
    }

    SECTION("Miscellany") {
        CHECK_NOTHROW(Expression("hypot(4 + cos(pi), 4)"));
        CHECK_THROWS_AS(Expression("1, 2)"), ParenthesisMismatchException);
        CHECK_NOTHROW(Expression("sin(pi / 2) + 1"));
        CHECK_NOTHROW(Expression("3 + 4 * 2 / (1 - 5) ^ 2 ^ 3"));
    }
}


TEST_CASE("Queries test", "[Queries]") {

    SECTION("Global queries") {
        auto expected_constants = vString({"pi", "e", "phi", "gamma"});
        auto expected_operators = vString({
            "+", "-", "*", "/", "%", "^", "**"
        });
        auto expected_functions = vString({
            "fabs", "abs", "fma", "copysign", "nextafter",
            "fdim", "fmax", "fmin",
            "ceil", "floor", "fmod", "trunc",
            "round", "rint", "nearbyint", "remainder",
            "pow", "sqrt", "cbrt", "hypot",
            "exp", "expm1", "exp2", "log", "log10", "log1p", "log2", "logb",
            "sin", "cos", "tan", "asin", "acos", "atan", "atan2",
            "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
            "erf", "erfc", "tgamma", "lgamma"
        });

        auto queried_constants = queryConstants();
        auto queried_operators = queryOperators();
        auto queried_functions = queryFunctions();

        std::sort(expected_constants.begin(), expected_constants.end());
        std::sort(queried_constants.begin(), queried_constants.end());
        std::sort(expected_operators.begin(), expected_operators.end());
        std::sort(queried_operators.begin(), queried_operators.end());
        std::sort(expected_functions.begin(), expected_functions.end());
        std::sort(queried_functions.begin(), queried_functions.end());

        CHECK(expected_constants == queried_constants);
        CHECK(expected_operators == queried_operators);
        CHECK(expected_functions == queried_functions);
    }

    SECTION("Instance queries") {
        auto tree = String(
            "[+]\n"
            " \\_[abs]\n"
            " |  \\_[3]\n"
            " \\_[/]\n"
            "    \\_[*]\n"
            "    |  \\_[4]\n"
            "    |  \\_[2]\n"
            "    \\_[^]\n"
            "       \\_[-]\n"
            "       |  \\_[1]\n"
            "       |  \\_[5]\n"
            "       \\_[^]\n"
            "          \\_[2]\n"
            "          \\_[3]"
        );
        auto expr1 = Expression("abs(3)+4*2/(1-5)^2^3");
        CHECK(expr1.expression() == "abs(3)+4*2/(1-5)^2^3");
        CHECK(expr1.infix() == "abs ( 3 ) + 4 * 2 / ( 1 - 5 ) ^ 2 ^ 3");
        CHECK(expr1.postfix() == "3 abs 4 2 * 1 5 - 2 3 ^ ^ / +");
        CHECK(expr1.tree() == tree);
        CHECK(expr1() == Approx(3.0001220703125).epsilon(1e-12));

        std::ostringstream stream;
        auto expr2 = Expression("x+y+z","x,y,z");
        std::copy(
            expr2.variables().begin(),
            expr2.variables().end(),
            std::ostream_iterator<std::string>(stream, ",")
        );
        auto vars = stream.str().erase(stream.str().size() - 1, 1);
        CHECK(vars == "x,y,z");
    }

}
