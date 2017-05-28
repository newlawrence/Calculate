#include <string>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "calculate/binding.h"


TEST_CASE("Metadata", "[metadata]") {

    SECTION("Set metadata") {
        char output[4096];

        calculate_c.version(output);
        CHECK(output == std::string("1.2.4"));
        calculate_c.author(output);
        CHECK(output == std::string("Alberto Lorenzo"));
        calculate_c.date(output);
        CHECK(output == std::string("2017/05/28"));
    }

}
