#include "catch.hpp"
#include "calculate.h"

using namespace calculate;


TEST_CASE("Builtin constants", "[constants]") {

    SECTION("pi") {
        auto pi = Calculate("pi");
        CHECK(pi() == Approx(3.141592653589).epsilon(1e-12));
    }

    SECTION("e") {
        auto e = Calculate("e");
        CHECK(e() == Approx(2.718281828459).epsilon(1e-12));
    }

    SECTION("phi") {
        auto phi = Calculate("phi");
        CHECK(phi() == Approx(1.618033988749).epsilon(1e-12));
    }

    SECTION("gamma") {
        auto gamma = Calculate("gamma");
        CHECK(gamma() == Approx(0.577215664901).epsilon(1e-12));
    }

}
