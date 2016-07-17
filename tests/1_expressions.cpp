#include "catch.hpp"
#include "calculate.h"

using namespace symbols;
using namespace calculate;


TEST_CASE("Wrong expressions", "[Expressions]") {

    SECTION("Empty expression") {
        CHECK_THROWS_AS(Calculate(""), EmptyExpressionException);
    }

    SECTION("Undefined symbols") {
        CHECK_THROWS_AS(Calculate("~"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("1 + ~"), SyntaxErrorException);
        CHECK_THROWS_AS(Calculate("~ + 1"), SyntaxErrorException);
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
        CHECK_THROWS_AS(Calculate("hypot(1 +, 2)"), SyntaxErrorException);
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

}
