#include <string>
#include <vector>

#include "catch.hpp"
#include "calculate.h"

using namespace calculate;


TEST_CASE("Variable arguments", "[variables]") {

    SECTION("Well constructed expressions") {
        CHECK_NOTHROW(Calculate("0")());
        CHECK_NOTHROW(Calculate("x0", "x0")(0));
        CHECK_NOTHROW(Calculate("x0 + x1", "x0, x1")(0, 1));
        CHECK_NOTHROW(Calculate("x0 * x1 * x2", "x0, x1, x2")(0, 1, 2));
        CHECK_NOTHROW(Calculate("x0", "x0")(std::vector<double>({0})));
        CHECK_NOTHROW(Calculate("x0 + x1", "x0, x1")({0, 1}));
        CHECK_NOTHROW(Calculate("x0 * x1 * x2", "x0, x1, x2")({0, 1, 2}));
    }

    SECTION("Arguments mismatch") {
        CHECK_THROWS_AS(
            Calculate("0")(0),
            WrongArgumentsException
        );
        CHECK_THROWS_AS(
            Calculate("x0", "x0")(0, 1),
            WrongArgumentsException
        );
        CHECK_THROWS_AS(
            Calculate("x0 + x1", "x0, x1")(0, 1, 2),
            WrongArgumentsException
        );
        CHECK_THROWS_AS(
            Calculate("x0 * x1 * x2", "x0, x1, x2")(0, 1, 2, 3),
            WrongArgumentsException
        );
        CHECK_THROWS_AS(
            Calculate("x0 + x1", "x0, x1")(std::vector<double>({0})),
            WrongArgumentsException
        );
        CHECK_THROWS_AS(
            Calculate("x0 + x1", "x0, x1")({0, 1, 2}),
            WrongArgumentsException
        );
    }

    SECTION("Unsuitable variable names") {
        CHECK_THROWS_AS(Calculate("var%", "var%"), BadNameException);
        CHECK_THROWS_AS(Calculate("var$", "var$"), BadNameException);
        CHECK_THROWS_AS(Calculate("var#", "var#"), BadNameException);
        CHECK_THROWS_AS(Calculate("var&", "var&"), BadNameException);
        CHECK_THROWS_AS(Calculate("var_", "var_"), BadNameException);
    }

    SECTION("Duplicate variable names") {
        CHECK_THROWS_AS(Calculate("x + x", "x, x"), DuplicateNameException);
    }

}
