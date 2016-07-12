#include "catch.hpp"
#include "calculate.h"


TEST_CASE("Builtin constants", "[constants]") {

    SECTION("pi") {
        auto pi = calculate::Calculate("pi");
        REQUIRE(pi() == Approx(3.141592653589).epsilon(1e-12));
    }

    SECTION("e") {
        auto e = calculate::Calculate("e");
        REQUIRE(e() == Approx(2.718281828459).epsilon(1e-12));
    }

    SECTION("phi") {
        auto phi = calculate::Calculate("phi");
        REQUIRE(phi() == Approx(1.618033988749).epsilon(1e-12));
    }

    SECTION("gamma") {
        auto gamma = calculate::Calculate("gamma");
        REQUIRE(gamma() == Approx(0.577215664901).epsilon(1e-12));
    }

}
