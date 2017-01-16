#include <cmath>
#include <cstring>
#include <string>

#include "catch.hpp"
#include "calculate/binding.h"


TEST_CASE("C interface", "[c_interface]") {

    SECTION("Well constructed expression - No error checking") {
        Expression expr1 = Calculate.build("1", "");
        Expression expr2 = Calculate.build("1 + x", "x");
        Expression expr3 = Calculate.build("x + y", "x, y");
        double x = 2., *xp = &x;
        char output[256];

        CHECK(get_calculate_reference() == &Calculate);

        Calculate.expression(expr1, output);
        CHECK(strcmp(output, "1") == 0);
        Calculate.variables(expr1, output);
        CHECK(strcmp(output, "") == 0);
        CHECK(static_cast<int>(Calculate.value(expr1)) == 1);

        Calculate.expression(expr2, output);
        CHECK(strcmp(output, "1 + x") == 0);
        Calculate.variables(expr2, output);
        CHECK(strcmp(output, "x") == 0);
        CHECK(static_cast<int>(Calculate.value(expr2)) == 1);
        CHECK(static_cast<int>(Calculate.value(expr2, x)) == 3);
        CHECK(static_cast<int>(Calculate.eval(expr2, xp, 1)) == 3);

        Calculate.expression(expr3, output);
        CHECK(strcmp(output, "x + y") == 0);
        Calculate.variables(expr3, output);
        CHECK(strcmp(output, "x,y") == 0);
        Calculate.infix(expr3, output);
        CHECK(strcmp(output, "x + y") == 0);
        Calculate.postfix(expr3, output);
        CHECK(strcmp(output, "x y +") == 0);
        Calculate.tree(expr3, output);
        CHECK(strcmp(output, "[+]\n \\_[x]\n \\_[y]") == 0);

        Calculate.free(expr3);
        Calculate.free(expr2);
        Calculate.free(expr1);
    }

    SECTION("Bad constructed expression - No error checking") {
        Expression expr1 = Calculate.build("1 + x", "");
        Expression expr2 = Calculate.build("1 + x", "");
        double *xp = nullptr;
        char output[256];

        Calculate.expression(expr1, output);
        CHECK(strcmp(output, "") == 0);
        Calculate.variables(expr1, output);
        CHECK(strcmp(output, "") == 0);
        Calculate.infix(expr1, output);
        CHECK(strcmp(output, "") == 0);
        Calculate.postfix(expr1, output);
        CHECK(strcmp(output, "") == 0);
        Calculate.tree(expr1, output);
        CHECK(strcmp(output, "") == 0);
        CHECK(std::isnan(Calculate.value(expr1, 2.)));
        CHECK(std::isnan(Calculate.eval(expr1, xp, 0)));

        Calculate.free(expr2);
        Calculate.free(expr1);
    }

    SECTION("Error checking") {
        Expression expr;
        double values[] = {1., 2.};
        char error[64];

        Calculate.create("x", "", error);
        CHECK(std::string(error) == std::string("Undefined symbol 'x'"));

        expr = Calculate.create("x", "x", error);
        CHECK(std::string(error) == std::string(""));

        Calculate.evaluate(expr, values, 2, error);
        CHECK(std::string(error) == std::string("Variables mismatch"));
    }

    SECTION("Query functions") {
        char output[4096];

        Calculate.constants(output);
        CHECK(strlen(output) == 14);
        Calculate.operators(output);
        CHECK(strlen(output) == 16);
        Calculate.functions(output);
        CHECK(strlen(output) == 249);
    }

}
