#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "calculate.hpp"


SCENARIO( "Testing default parser", "[default_parser]" ) {

    GIVEN( "provisional example to trigger the coverage of all the files" ) {
        auto parser = calculate::Parser{};
        auto sum = parser.parse("x+y");
        CHECK( sum(1., 2.) == Approx(3.) );
    }

}
