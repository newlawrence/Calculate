#include <cmath>
#include <string>

#include "catch.hpp"
#include "calculate/c-interface.h"

extern "C" const calculate_c_library_template Calculate;


TEST_CASE("C interface", "[c_interface]") {

    SECTION("Well constructed expression - No error checking") {
        Expression expr1 = Calculate.newExpression("1 + x", "x");
        Expression expr2 = Calculate.newExpression("1 + x", "x");
        Expression expr3 = Calculate.newExpression("x + y", "x, y");
        double x = 2., *xp = &x;

        CHECK(Calculate.compare(expr1, expr2) == 1);
        CHECK(Calculate.compare(expr1, expr3) == 0);
        CHECK(std::string(Calculate.getExpression(expr1)) == "1 + x");
        CHECK(std::string(Calculate.getVariables(expr1)) == "x");
        CHECK(std::string(Calculate.getVariables(expr3)) == "x,y");
        CHECK(static_cast<int>(Calculate.eval(expr1)) == 1);
        CHECK(static_cast<int>(Calculate.eval(expr1, x)) == 3);
        CHECK(static_cast<int>(Calculate.evalArray(expr1, xp, 1)) == 3);

        Calculate.freeExpression(expr3);
        Calculate.freeExpression(expr2);
        Calculate.freeExpression(expr1);
    }

    SECTION("Bad constructed expression - No error checking") {
        Expression expr1 = Calculate.newExpression("1 + x", "");
        Expression expr2 = Calculate.newExpression("1 + x", "");
        double *xp = nullptr;

        CHECK(Calculate.compare(expr1, expr2) == -1);
        CHECK(std::string(Calculate.getExpression(expr1)) == "");
        CHECK(std::string(Calculate.getVariables(expr1)) == "");
        CHECK(std::isnan(Calculate.eval(expr1, 2.)));
        CHECK(std::isnan(Calculate.evalArray(expr1, xp, 0)));

        Calculate.freeExpression(expr2);
        Calculate.freeExpression(expr1);
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
