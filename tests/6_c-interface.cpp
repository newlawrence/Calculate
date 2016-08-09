#include <cmath>
#include <string>

#include "catch.hpp"
#include "calculate.h"

extern "C" const _calculate_c_library* _get_calculate_c_library();
const auto calculate_lib = _get_calculate_c_library();


TEST_CASE("C interface", "[c_interface]") {

    SECTION("Well constructed expression - No error checking") {
        calculate_Expression expr = calculate_lib->newExpression("1 + x", "x");
        double x = 2., *xp = &x;

        CHECK(
            std::string(calculate_lib->getExpression(expr)) ==
            std::string("1 + x")
        );
        CHECK(calculate_lib->getVariables(expr) == 1);
        CHECK(static_cast<int>(calculate_lib->eval(expr)) == 1);
        CHECK(static_cast<int>(calculate_lib->eval(expr, x)) == 3);
        CHECK(static_cast<int>(calculate_lib->evalArray(expr, xp, 1)) == 3);

        calculate_lib->freeExpression(expr);
    }

    SECTION("Bad constructed expression - No error checking") {
        calculate_Expression expr = calculate_lib->newExpression("1 + x", "");
        double *xp = nullptr;

        CHECK(
            std::string(calculate_lib->getExpression(expr)) ==
            std::string("")
        );
        CHECK(calculate_lib->getVariables(expr) == -1);
        CHECK(std::isnan(calculate_lib->eval(expr, 2.)));
        CHECK(std::isnan(calculate_lib->evalArray(expr, xp, 0)));

        calculate_lib->freeExpression(expr);
    }

    SECTION("Error checking") {
        calculate_Expression expr;
        double values[] = {1., 2.};
        char error[64];

        expr = calculate_lib->createExpression("x", "", error);
        CHECK(std::string(error) == std::string("Undefined symbol"));

        expr = calculate_lib->createExpression("x", "x", error);
        CHECK(std::string(error) == std::string(""));

        calculate_lib->evaluateArray(expr, values, 2, error);
        CHECK(std::string(error) == std::string("Arguments mismatch"));
    }

}
