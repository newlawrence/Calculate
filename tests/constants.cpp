#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <limits>
#include "calculate.h"


RECORD_FUNCTION(
    select4,
    5,
    [&x]() -> double {
        switch (static_cast<int>(x[4])) {
            case 1: return x[0];
            case 2: return x[1];
            case 3: return x[2];
            case 4: return x[3];
            default: return std::numeric_limits<double>::quiet_NaN();
        }
    }();
)


TEST_CASE("Testing builtin constants precision", "[constants]") {

    auto cs = calculate::Calculate("select4(pi, e, phi, gamma, n)", "n");

    SECTION("pi") {
        CHECK(cs(1) == Approx(3.141592653589).epsilon(1e-12));
    }
    SECTION("e") {
        CHECK(cs(2) == Approx(2.718281828459).epsilon(1e-12));
    }
    SECTION("phi") {
        CHECK(cs(3) == Approx(1.618033988749).epsilon(1e-12));
    }
    SECTION("gamma") {
        CHECK(cs(4) == Approx(0.577215664901).epsilon(1e-12));
    }

}
