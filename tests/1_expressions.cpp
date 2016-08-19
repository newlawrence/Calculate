#include "catch.hpp"
#include "calculate.h"

using namespace calculate;
using namespace calculate_exceptions;


TEST_CASE("Constructors test", "[Constructors]") {

    SECTION("Constructors and assignments") {
        auto expr1 = Expression("1 + x", "x");
        auto expr2 = expr1;
        CHECK(static_cast<int>(expr1(2)) == static_cast<int>(expr2(2)));

        expr1 = Expression("1 + x", "x");
        expr2 = expr1;
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
