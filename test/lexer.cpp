#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "calculate.hpp"


SCENARIO( "Testing default lexer", "[default_lexer]" ) {

    GIVEN( "a default lexer" ) {
        REQUIRE_NOTHROW( calculate::Lexer<double>{} );
        auto lexer = calculate::Lexer<double>{};
    }

}
