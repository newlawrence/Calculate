#include <cmath>
#include <cstring>
#include <string>

#include "catch.hpp"
#include "calculate/c-interface.h"


TEST_CASE("C interface", "[c_interface]") {

    SECTION("Well constructed expression - No error checking") {
        Expression expr1 = Calculate.newExpression("1", "");
        Expression expr2 = Calculate.newExpression("1 + x", "x");
        Expression expr3 = Calculate.newExpression("x + y", "x, y");
        double x = 2., *xp = &x;
        char output[256];

        CHECK(get_calculate_reference() == &Calculate);

        Calculate.getExpression(expr1, output);
        CHECK(strcmp(output, "1") == 0);
        Calculate.getVariables(expr1, output);
        CHECK(strcmp(output, "") == 0);
        CHECK(static_cast<int>(Calculate.eval(expr1)) == 1);

        Calculate.getExpression(expr2, output);
        CHECK(strcmp(output, "1 + x") == 0);
        Calculate.getVariables(expr2, output);
        CHECK(strcmp(output, "x") == 0);
        CHECK(static_cast<int>(Calculate.eval(expr2)) == 1);
        CHECK(static_cast<int>(Calculate.eval(expr2, x)) == 3);
        CHECK(static_cast<int>(Calculate.evalArray(expr2, xp, 1)) == 3);

        Calculate.getExpression(expr3, output);
        CHECK(strcmp(output, "x + y") == 0);
        Calculate.getVariables(expr3, output);
        CHECK(strcmp(output, "x,y") == 0);
        Calculate.getInfix(expr3, output);
        CHECK(strcmp(output, "x + y") == 0);
        Calculate.getPostfix(expr3, output);
        CHECK(strcmp(output, "x y +") == 0);
        Calculate.getTree(expr3, output);
        CHECK(strcmp(output, "[+]\n \\_[x]\n \\_[y]") == 0);

        Calculate.freeExpression(expr3);
        Calculate.freeExpression(expr2);
        Calculate.freeExpression(expr1);
    }

    SECTION("Bad constructed expression - No error checking") {
        Expression expr1 = Calculate.newExpression("1 + x", "");
        Expression expr2 = Calculate.newExpression("1 + x", "");
        double *xp = nullptr;
        char output[256];

        Calculate.getExpression(expr1, output);
        CHECK(strcmp(output, "") == 0);
        Calculate.getVariables(expr1, output);
        CHECK(strcmp(output, "") == 0);
        Calculate.getInfix(expr1, output);
        CHECK(strcmp(output, "") == 0);
        Calculate.getPostfix(expr1, output);
        CHECK(strcmp(output, "") == 0);
        Calculate.getTree(expr1, output);
        CHECK(strcmp(output, "") == 0);
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

    SECTION("Query functions") {
        char output[4096];

        Calculate.queryConstants(output);
        CHECK(strlen(output) == 14);
        Calculate.queryOperators(output);
        CHECK(strlen(output) == 14);
        Calculate.queryFunctions(output);
        CHECK(strlen(output) == 249);
    }

}
