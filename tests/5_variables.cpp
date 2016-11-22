#include <string>
#include <vector>

#include "catch.hpp"
#include "calculate.h"

using namespace calculate;
using namespace calculate_exceptions;


TEST_CASE("Variable arguments", "[variables]") {

    SECTION("Well constructed expressions") {
        CHECK_NOTHROW(Expression("0")());
        CHECK_NOTHROW(Expression("x0", "x0")(0));
        CHECK_NOTHROW(Expression("x0 + x1", "x0, x1")(0, 1));
        CHECK_NOTHROW(Expression("x0 * x1 * x2", "x0, x1, x2")(0, 1, 2));
        CHECK_NOTHROW(Expression("x0", "x0")(std::vector<double>({0})));
        CHECK_NOTHROW(Expression("x0 + x1", "x0, x1")({0, 1}));
        CHECK_NOTHROW(Expression("x0 * x1 * x2", "x0, x1, x2")({0, 1, 2}));
    }

    SECTION("Arguments mismatch") {
        CHECK_THROWS_AS(
            Expression("0")(0),
            WrongArgumentsException
        );
        CHECK_THROWS_AS(
            Expression("x0", "x0")(0, 1),
            WrongArgumentsException
        );
        CHECK_THROWS_AS(
            Expression("x0 + x1", "x0, x1")(0, 1, 2),
            WrongArgumentsException
        );
        CHECK_THROWS_AS(
            Expression("x0 * x1 * x2", "x0, x1, x2")(0, 1, 2, 3),
            WrongArgumentsException
        );
        CHECK_THROWS_AS(
            Expression("x0 + x1", "x0, x1")(std::vector<double>({0})),
            WrongArgumentsException
        );
        CHECK_THROWS_AS(
            Expression("x0 + x1", "x0, x1")({0, 1, 2}),
            WrongArgumentsException
        );
    }

    SECTION("Suitable variable names") {
        CHECK_NOTHROW(Expression("var", "var"));
        CHECK_NOTHROW(Expression("_", "_"));
        CHECK_NOTHROW(Expression("_var1", "_var1"));
        CHECK_NOTHROW(Expression("var_1_and_2", "var_1_and_2"));
    }

    SECTION("Unsuitable variable names") {
        CHECK_THROWS_AS(Expression("var%", "var%"), BadNameException);
        CHECK_THROWS_AS(Expression("var$", "var$"), BadNameException);
        CHECK_THROWS_AS(Expression("var#", "var#"), BadNameException);
        CHECK_THROWS_AS(Expression("var&", "var&"), BadNameException);
    }

    SECTION("Duplicate variable names") {
        CHECK_THROWS_AS(Expression("x + x", "x, x"), DuplicatedNameException);
    }

    SECTION("Variables excess") {
        CHECK_THROWS_AS(
            Expression("1")(1),
            WrongArgumentsException
        );
        CHECK_THROWS_AS(
            Expression("x", "x")(),
            WrongArgumentsException
        );
        CHECK_THROWS_AS(
            Expression("x", "x")(1, 2),
            WrongArgumentsException
        );
    }

    SECTION("Bad variable list") {
        CHECK_THROWS_AS(Expression("x+y", "x,,y"), BadNameException);
    }

}
