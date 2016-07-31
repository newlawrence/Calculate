#include "catch.hpp"
#include "calculate.h"

using namespace calculate;


TEST_CASE("Builtin constants", "[constants]") {

    SECTION("pi") {
        Calculate pi("pi");
        CHECK(pi() == Approx(3.141592653589).epsilon(1e-12));
    }

    SECTION("e") {
        Calculate e("e");
        CHECK(e() == Approx(2.718281828459).epsilon(1e-12));
    }

    SECTION("phi") {
        Calculate phi("phi");
        CHECK(phi() == Approx(1.618033988749).epsilon(1e-12));
    }

    SECTION("gamma") {
        Calculate gamma("gamma");
        CHECK(gamma() == Approx(0.577215664901).epsilon(1e-12));
    }

}
