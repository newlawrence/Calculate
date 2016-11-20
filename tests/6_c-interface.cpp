#include <cmath>
#include <cstring>
#include <string>

#include "catch.hpp"
#include "calculate/c-interface.h"


TEST_CASE("C interface", "[c_interface]") {

    SECTION("Well constructed expression - No error checking") {
        Expression expr1 = Calculate.newExpression("1 + x", "x");
        Expression expr2 = Calculate.newExpression("1 + x", "x");
        Expression expr3 = Calculate.newExpression("x + y", "x, y");
        double x = 2., *xp = &x;
        char expr[64], vars[64];

        CHECK(get_calculate_reference() == &Calculate);

        Calculate.getExpression(expr1, expr);
        CHECK(strcmp(expr, "1 + x") == 0);
        Calculate.getVariables(expr1, vars);
        CHECK(strcmp(vars, "x") == 0);
        Calculate.getVariables(expr3, vars);
        CHECK(strcmp(vars, "x,y") == 0);
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
        char expr[64], vars[64];


        Calculate.getExpression(expr1, expr);
        CHECK(strcmp(expr, "") == 0);
        Calculate.getVariables(expr1, vars);
        CHECK(strcmp(vars, "") == 0);
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
