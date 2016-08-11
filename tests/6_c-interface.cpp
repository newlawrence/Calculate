#include <cmath>
#include <string>

#include "catch.hpp"
#include "calculate.h"

extern "C" const calculate_c_library_template Calculate;


TEST_CASE("C interface", "[c_interface]") {

    SECTION("Well constructed expression - No error checking") {
        Expression expr = Calculate.newExpression("1 + x", "x");
        double x = 2., *xp = &x;

        CHECK(
            std::string(Calculate.getExpression(expr)) ==
            std::string("1 + x")
        );
        CHECK(Calculate.getVariables(expr) == 1);
        CHECK(static_cast<int>(Calculate.eval(expr)) == 1);
        CHECK(static_cast<int>(Calculate.eval(expr, x)) == 3);
        CHECK(static_cast<int>(Calculate.evalArray(expr, xp, 1)) == 3);

        Calculate.freeExpression(expr);
    }

    SECTION("Bad constructed expression - No error checking") {
        Expression expr = Calculate.newExpression("1 + x", "");
        double *xp = nullptr;

        CHECK(
            std::string(Calculate.getExpression(expr)) ==
            std::string("")
        );
        CHECK(Calculate.getVariables(expr) == -1);
        CHECK(std::isnan(Calculate.eval(expr, 2.)));
        CHECK(std::isnan(Calculate.evalArray(expr, xp, 0)));

        Calculate.freeExpression(expr);
    }

    SECTION("Error checking") {
        Expression expr;
        double values[] = {1., 2.};
        char error[64];

        Calculate.createExpression("x", "", error);
        CHECK(std::string(error) == std::string("Undefined symbol"));

        expr = Calculate.createExpression("x", "x", error);
        CHECK(std::string(error) == std::string(""));

        Calculate.evaluateArray(expr, values, 2, error);
        CHECK(std::string(error) == std::string("Arguments mismatch"));
    }

}
