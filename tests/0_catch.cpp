#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "calculate.h"

using namespace calculate;
using namespace calculate_exceptions;


TEST_CASE("Random stuff to get 100% coverage", "[Random]") {

    SECTION("Non generated symbols") {
        auto empty = castChild<Evaluable>(EmptyEvaluable::make());
        CHECK(std::isnan(empty->evaluate()));
    }

}
