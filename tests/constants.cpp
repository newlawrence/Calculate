#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "calculate.h"


TEST_CASE("Testing builtin constants precision", "[constants]") {

    SECTION("Testing pi...") {
        auto c = calculate::Calculate("pi");
        REQUIRE(c() == Approx(3.141592653589).epsilon(1e-12));
    }

    SECTION("Testing e...") {
        auto c = calculate::Calculate("e");
        REQUIRE(c() == Approx(2.718281828459).epsilon(1e-12));
    }

    SECTION("Testing phi...") {
        auto c = calculate::Calculate("phi");
        REQUIRE(c() == Approx(1.618033988749).epsilon(1e-12));
    }

    SECTION("Testing gamma...") {
        auto c = calculate::Calculate("gamma");
        REQUIRE(c() == Approx(0.577215664901).epsilon(1e-12));
    }

}
