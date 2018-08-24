#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "calculate/wrapper.hpp"


class CopyMove {
public:
    const std::size_t copied = 0;
    const std::size_t moved = 0;

    CopyMove() = default;
    ~CopyMove() = default;

    CopyMove(std::size_t cp, std::size_t mv) : copied(cp), moved(mv) {}

    CopyMove(const CopyMove& other) noexcept :
            copied{other.copied + 1},
            moved{other.moved}
    {}

    CopyMove(CopyMove&& other) noexcept :
            copied(other.copied),
            moved(other.moved + 1)
    {}

    auto operator()() const noexcept { return CopyMove{copied, moved}; }
};


SCENARIO( "construction of a Wrapper object", "[construction]" ) {
    using Wrapper = calculate::Wrapper<CopyMove>;

    GIVEN( "a non temporary callable" ) {
        auto copy_move = CopyMove{};

        WHEN( "a wrapper is created" ) {
            auto wrapper = Wrapper{copy_move};

            THEN( "the given callable is copied and not moved" ) {
                CHECK( wrapper().copied == 1 );
                CHECK( wrapper().moved == 0 );
            }
        }
    }

    GIVEN( "a temporary callable" ) {
        WHEN( "a wrapper is created" ) {
            auto wrapper = Wrapper{CopyMove{}};

            THEN( "the given callable is moved and not copied" ) {
                CHECK( wrapper().copied == 0 );
                CHECK( wrapper().moved == 1 );
            }
        }
    }
}

SCENARIO( "copy and clone of a Wrapper object", "[copy]" ) {
    using Wrapper = calculate::Wrapper<int>;
    const auto hash = std::hash<Wrapper>{};

    GIVEN( "a wrapper object" ) {
        auto wrapper = Wrapper{[](int n) noexcept { return n; }};

        WHEN( "a copy is created" ) {
            auto copy = wrapper;

            THEN( "they share the wrapped callable" ) {
                CHECK( wrapper == copy );
                CHECK( !(wrapper != copy) );
                CHECK( hash(wrapper) == hash(copy) );
            }
        }

        WHEN( "a clone is created" ) {
            auto clone = wrapper.clone();

            THEN( "they don't share the wrapped callable" ) {
                CHECK( !(wrapper == clone) );
                CHECK( wrapper != clone );
                CHECK( hash(wrapper) != hash(clone) );
            }
        }
    }
}
