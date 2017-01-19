#include <cmath>
#include <cstring>
#include <string>

#include "catch.hpp"
#include "calculate/binding.h"


TEST_CASE("Bindings", "[bindings]") {

    SECTION("Well constructed expression - No error checking") {
        Expression expr1 = calculate_c.build("1", "");
        Expression expr2 = calculate_c.build("1 + x", "x");
        Expression expr3 = calculate_c.build("x + y", "x, y");
        double x = 2., *xp = &x;
        char output[256];

        CHECK(get_calculate_reference() == &calculate_c);

        calculate_c.expression(expr1, output);
        CHECK(strcmp(output, "1") == 0);
        calculate_c.variables(expr1, output);
        CHECK(strcmp(output, "") == 0);
        CHECK(static_cast<int>(calculate_c.value(expr1)) == 1);

        calculate_c.expression(expr2, output);
        CHECK(strcmp(output, "1 + x") == 0);
        calculate_c.variables(expr2, output);
        CHECK(strcmp(output, "x") == 0);
        CHECK(static_cast<int>(calculate_c.value(expr2, x)) == 3);
        CHECK(static_cast<int>(calculate_c.eval(expr2, xp, 1)) == 3);

        calculate_c.expression(expr3, output);
        CHECK(strcmp(output, "x + y") == 0);
        calculate_c.variables(expr3, output);
        CHECK(strcmp(output, "x,y") == 0);
        calculate_c.infix(expr3, output);
        CHECK(strcmp(output, "x + y") == 0);
        calculate_c.postfix(expr3, output);
        CHECK(strcmp(output, "x y +") == 0);
        calculate_c.tree(expr3, output);
        CHECK(strcmp(output, "[+]\n \\_[x]\n \\_[y]") == 0);

        calculate_c.free(expr3);
        calculate_c.free(expr2);
        calculate_c.free(expr1);
    }

    SECTION("Bad constructed expression - No error checking") {
        Expression expr1 = calculate_c.build("1 + x", "");
        Expression expr2 = calculate_c.build("1 + x", "");
        double *xp = nullptr;
        char output[256];

        calculate_c.expression(expr1, output);
        CHECK(strcmp(output, "") == 0);
        calculate_c.variables(expr1, output);
        CHECK(strcmp(output, "") == 0);
        calculate_c.infix(expr1, output);
        CHECK(strcmp(output, "") == 0);
        calculate_c.postfix(expr1, output);
        CHECK(strcmp(output, "") == 0);
        calculate_c.tree(expr1, output);
        CHECK(strcmp(output, "") == 0);
        CHECK(std::isnan(calculate_c.value(expr1, 2.)));
        CHECK(std::isnan(calculate_c.eval(expr1, xp, 0)));

        calculate_c.free(expr2);
        calculate_c.free(expr1);
    }

    SECTION("Error checking") {
        Expression expr;
        double values[] = {1., 2.};
        char error[64];

        calculate_c.create("x", "", error);
        CHECK(std::string(error) == std::string("Undefined symbol 'x'"));

        expr = calculate_c.create("x", "x", error);
        CHECK(std::string(error) == std::string(""));

        calculate_c.evaluate(expr, values, 2, error);
        CHECK(std::string(error) == std::string("Variables mismatch"));
    }

    SECTION("Query functions") {
        char output[4096];

        calculate_c.constants(output);
        CHECK(strlen(output) == 14);
        calculate_c.operators(output);
        CHECK(strlen(output) == 14);
        calculate_c.functions(output);
        CHECK(strlen(output) == 256);
    }

}
