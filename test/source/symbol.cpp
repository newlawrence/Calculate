#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "calculate/parser.hpp"


SCENARIO( "some static assertions on the Symbol class", "[assertions][symbol]" ) {
    using Parser = calculate::BaseParser<int>;
    using Symbol = calculate::Symbol<Parser::Expression>;

    static_assert(!std::is_default_constructible<Symbol>::value, "");
    static_assert(std::is_nothrow_destructible<Symbol>::value, "");
    static_assert(std::has_virtual_destructor<Symbol>::value, "");
}


SCENARIO( "some static assertions on the Constant class", "[assertions][constant]" ) {
    using Parser = calculate::BaseParser<int>;
    using Constant = Parser::Constant;

    static_assert(std::is_final<Constant>::value, "");
    static_assert(!std::is_default_constructible<Constant>::value, "");
    static_assert(std::is_nothrow_destructible<Constant>::value, "");
    static_assert(std::is_copy_constructible<Constant>::value, "");
    static_assert(std::is_nothrow_move_constructible<Constant>::value, "");
    static_assert(std::is_copy_assignable<Constant>::value, "");
    static_assert(std::is_move_assignable<Constant>::value, "");
    static_assert(std::has_virtual_destructor<Constant>::value, "");
}

SCENARIO( "constants usage", "[usage][constant]" ) {
    using Parser = calculate::BaseParser<int>;
    using Symbol = calculate::Symbol<Parser::Expression>;
    using Constant = Parser::Constant;

    auto hash = std::hash<Symbol>{};

    GIVEN( "some constants" ) {
        auto c1 = Constant{1};
        Constant c2 = 2;

        REQUIRE( c1.type() == Symbol::SymbolType::CONSTANT );

        THEN( "they can be converted to its wrapped value" ) {
            CHECK( static_cast<int>(c1) == 1 );
            CHECK( int{c2} == 2 );
            CHECK( hash(Constant{3}) == std::hash<int>{}(3) );
        }

        THEN( "they can be reassigned from non-temporary constants" ) {
            c1 = c2;
            CHECK( c1 == c2 );
            CHECK( hash(c1) == hash(c2) );
        }

        THEN( "they can be reassigned from temporary constants" ) {
            c1 = Constant{3};
            CHECK( c1 != c2 );
            CHECK( hash(c1) != hash(c2) );
        }

        THEN( "they can be converted implicitly" ) {
            auto check = [](int i) -> bool { return i == 0; };
            CHECK( check(Constant{0}) );
            CHECK( c1 + 1 == 2 );
            CHECK( c1 - 1 == 0 );
            CHECK( c1 * 2 == 2 );
            CHECK( c1 / 2 == 0 );
            CHECK( c1 % 3 == 1 );
            CHECK( c2 == 2 );
            CHECK( c2 != 0 );
            CHECK( c2 > 1 );
            CHECK( c2 < 3 );
            CHECK( c2 >= 0 );
            CHECK( c2 <= 4 );
        }
    }
}