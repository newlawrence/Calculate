#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "calculate/parser.hpp"


SCENARIO( "some static assertions on symbols", "[assertions][symbols]" ) {
    using Parser = calculate::BaseParser<int>;
    using Symbol = calculate::Symbol<Parser::Expression>;

    static_assert(!std::is_default_constructible<Symbol>::value, "");
    static_assert(std::is_nothrow_destructible<Symbol>::value, "");
    static_assert(std::has_virtual_destructor<Symbol>::value, "");
}


SCENARIO( "some static assertions on variables", "[assertions][variables]" ) {
    using Parser = calculate::BaseParser<int>;
    using Variable = calculate::Variable<Parser::Expression>;

    static_assert(std::is_final<Variable>::value, "");
    static_assert(!std::is_default_constructible<Variable>::value, "");
    static_assert(std::is_nothrow_destructible<Variable>::value, "");
    static_assert(std::is_copy_constructible<Variable>::value, "");
    static_assert(std::is_nothrow_move_constructible<Variable>::value, "");
    static_assert(std::is_copy_assignable<Variable>::value, "");
    static_assert(std::is_move_assignable<Variable>::value, "");
    static_assert(std::has_virtual_destructor<Variable>::value, "");
}

SCENARIO( "variables usage", "[usage][variables]" ) {
    using Parser = calculate::BaseParser<int>;
    using Symbol = calculate::Symbol<Parser::Expression>;
    using Variable = calculate::Variable<Parser::Expression>;

    GIVEN( "some variables" ) {
        int a = 1, b = 2;
        Variable v1 = a;
        auto v2 = Variable{b};

        REQUIRE( v1.type() == Symbol::SymbolType::CONSTANT );
        REQUIRE( v2.type() == Symbol::SymbolType::CONSTANT );

        THEN( "they wrap an external symbol" ) {
            CHECK( static_cast<Symbol&>(v1)() == 1 );
            a = 2;
            CHECK( static_cast<Symbol&>(v1)() == 2 );
        }

        THEN( "their hashes are equal to their stored values" ) {
            CHECK( std::hash<Symbol>{}(v1) == std::hash<int>{}(1) );
            a = 2;
            CHECK( std::hash<Symbol>{}(v1) == std::hash<int>{}(2) );
        }

        THEN( "they are always different between them" ) {
            CHECK( v1 != v2 );
            a = 2;
            CHECK( v1 != v2 );
            auto v3 = v1;
            CHECK( v1 != v3 );
            auto v4 = static_cast<Variable&>(*v1.clone());
            CHECK( v1 != v4 );
        }
    }
}


SCENARIO( "some static assertions on constants", "[assertions][constants]" ) {
    using Parser = calculate::BaseParser<int>;
    using Constant = calculate::Constant<Parser::Expression>;

    static_assert(std::is_final<Constant>::value, "");
    static_assert(!std::is_default_constructible<Constant>::value, "");
    static_assert(std::is_nothrow_destructible<Constant>::value, "");
    static_assert(std::is_copy_constructible<Constant>::value, "");
    static_assert(std::is_nothrow_move_constructible<Constant>::value, "");
    static_assert(std::is_copy_assignable<Constant>::value, "");
    static_assert(std::is_move_assignable<Constant>::value, "");
    static_assert(std::has_virtual_destructor<Constant>::value, "");
}

SCENARIO( "constants usage", "[usage][constants]" ) {
    using Parser = calculate::BaseParser<int>;
    using Symbol = calculate::Symbol<Parser::Expression>;
    using Constant = calculate::Constant<Parser::Expression>;

    GIVEN( "some constants" ) {
        Constant c1 = 1;
        auto c2 = Constant{2};

        REQUIRE( c1.type() == Symbol::SymbolType::CONSTANT );
        REQUIRE( c2.type() == Symbol::SymbolType::CONSTANT );

        THEN( "they can be converted to its wrapped value" ) {
            CHECK( static_cast<int>(c1) == 1 );
            CHECK( int{c2} == 2 );
            CHECK( std::hash<Symbol>{}(Constant{3}) == std::hash<int>{}(3) );
        }

        THEN( "they can be reassigned from non-temporary constants" ) {
            c1 = c2;
            CHECK( c1 == c2 );
            CHECK( static_cast<Symbol&>(c1) == static_cast<Symbol&>(c2) );
            CHECK( std::hash<Symbol>{}(c1) == std::hash<Symbol>{}(c2) );
        }

        THEN( "they can be reassigned from temporary constants" ) {
            c1 = Constant{3};
            CHECK( c1 != c2 );
            CHECK( static_cast<Symbol&>(c1) != static_cast<Symbol&>(c2) );
            CHECK( std::hash<Symbol>{}(c1) != std::hash<Symbol>{}(c2) );
        }

        THEN( "cloning has the same effect as copying" ) {
            c1 = static_cast<Constant&>(*c2.clone());
            CHECK( c1 == c2 );
            CHECK( static_cast<Symbol&>(c1) == static_cast<Symbol&>(c2) );
            CHECK( std::hash<Symbol>{}(c1) == std::hash<Symbol>{}(c2) );
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