#include "catch.hpp"
#include "calculate.h"

using namespace calculate;
using namespace calculate_exceptions;


TEST_CASE("Constructors test", "[Constructors]") {

    SECTION("Constructors and assignments") {
        auto expr1 = Calculate("1 + x", "x");
        auto expr2 = expr1;
        CHECK(expr1 == expr2);
        CHECK(static_cast<int>(expr1(2)) == static_cast<int>(expr2(2)));

        expr1 = Calculate("1 + x", "x");
        expr2 = expr1;
        CHECK(expr1 == expr2);
        CHECK(static_cast<int>(expr1(2)) == static_cast<int>(expr2(2)));
    }

}


TEST_CASE("Wrong expressions", "[Expressions]") {

    SECTION("Empty expression") {
        CHECK_THROWS_AS(Calculate(""), EmptyExpressionException);
    }

    SECTION("Undefined symbols") {
        CHECK_THROWS_AS(Calculate("x"), UndefinedSymbolException);
        CHECK_THROWS_AS(Calculate("1 + x"), UndefinedSymbolException);
        CHECK_THROWS_AS(Calculate("x + 1"), UndefinedSymbolException);
        CHECK_THROWS_AS(Calculate("1 + x 2"), UndefinedSymbolException);
    }

    SECTION("Parenthesis mismatches") {
        CHECK_THROWS_AS(Calculate("(1"), ParenthesisMismatchException);
        CHECK_THROWS_AS(Calculate("1)"), ParenthesisMismatchException);
        CHECK_THROWS_AS(Calculate("((1"), ParenthesisMismatchException);
        CHECK_THROWS_AS(Calculate("1))"), ParenthesisMismatchException);
    }

    SECTION("First members") {
        CHECK_NOTHROW(Calculate("1"));
        CHECK_NOTHROW(Calculate("(1)"));
        CHECK_THROWS_AS(Calculate(") + 1"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate(", 1"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("+ 1"), SyntaxErrorException);
        CHECK_NOTHROW(Calculate("log(1)"));
    }

    SECTION("Members following constant") {
        CHECK_THROWS_AS(Calculate("1 2"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("1 (2)"), SyntaxErrorException);
        CHECK_NOTHROW(Calculate("(1)"));
        CHECK_NOTHROW(Calculate("hypot(1, 2)"));
        CHECK_NOTHROW(Calculate("1 + 2"));
        CHECK_THROWS_AS(Calculate("1 log(2)"), SyntaxErrorException);
    }

    SECTION("Members following left parenthesis") {
        CHECK_NOTHROW(Calculate("(1)"));
        CHECK_NOTHROW(Calculate("((1))"));
        CHECK_THROWS_AS(Calculate("()"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("hypot(, 1)"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("( + 1)"), SyntaxErrorException);
        CHECK_NOTHROW(Calculate("(log(1))"));
    }

    SECTION("Members following right parenthesis") {
        CHECK_THROWS_AS(Calculate("(1) 2"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("(1) (2)"), SyntaxErrorException);
        CHECK_NOTHROW(Calculate("((1))"));
        CHECK_NOTHROW(Calculate("hypot((1), 2)"));
        CHECK_NOTHROW(Calculate("(1) + 2"));
        CHECK_THROWS_AS(Calculate("(1) log(2)"), SyntaxErrorException);
    }

    SECTION("Members following separator") {
        CHECK_NOTHROW(Calculate("hypot(1, 2)"));
        CHECK_NOTHROW(Calculate("hypot(1, (2))"));
        CHECK_THROWS_AS(Calculate("log )"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("hypot , 1)"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("log + 1)"), SyntaxErrorException);
        CHECK_NOTHROW(Calculate("hypot(1, log(2))"));
    }

    SECTION("Members following operator") {
        CHECK_NOTHROW(Calculate("1 + 2"));
        CHECK_NOTHROW(Calculate("1 + (2)"));
        CHECK_THROWS_AS(Calculate("1 + )"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("hypot(1 + , 2)"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("1 + + 2"), SyntaxErrorException);
        CHECK_NOTHROW(Calculate("1 + log(2)"));
    }

    SECTION("Members following function") {
        CHECK_THROWS_AS(Calculate("log 1"), SyntaxErrorException);
        CHECK_NOTHROW(Calculate("log(1)"));
        CHECK_THROWS_AS(Calculate("log )"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("hypot(, 1)"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("log(+ 1)"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("log log(1)"), SyntaxErrorException);
    }

    SECTION("Last members") {
        CHECK_NOTHROW(Calculate("1"));
        CHECK_THROWS_AS(Calculate("1 ("), SyntaxErrorException);
        CHECK_NOTHROW(Calculate("(1)"));
        CHECK_THROWS_AS(Calculate("1,"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("1 +"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("log"), SyntaxErrorException);
    }

    SECTION("Miscellany") {
        CHECK_NOTHROW(Calculate("hypot(4 + cos(pi), 4)"));
        CHECK_THROWS_AS(Calculate("1, 2)"), ParenthesisMismatchException);
        CHECK_NOTHROW(Calculate("sin(pi / 2) + 1"));
        CHECK_NOTHROW(Calculate("3 + 4 * 2 / (1 - 5) ^ 2 ^ 3"));
    }
}
