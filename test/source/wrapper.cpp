#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <array>
#include <list>
#include <vector>

#include "calculate/wrapper.hpp"


class CopyMove {
public:
    const std::size_t copied = 0;
    const std::size_t moved = 0;

    CopyMove() = default;
    CopyMove(std::size_t cp, std::size_t mv) : copied(cp), moved(mv) {}
    CopyMove(const CopyMove& other) noexcept : copied{other.copied + 1}, moved{other.moved} {}
    CopyMove(CopyMove&& other) noexcept : copied(other.copied), moved(other.moved + 1) {}
    ~CopyMove() = default;

    auto operator()() const noexcept { return CopyMove{copied, moved}; }
};

class Intermediary {
    int _n;

public:
    explicit Intermediary(int n) : _n{n} {}
    Intermediary() : Intermediary{0} {}

    explicit operator int() const noexcept { return _n; }
};


SCENARIO( "construction of a wrapper object", "[construction]" ) {
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

SCENARIO( "copy and clone of a wrapper object", "[copy]" ) {
    using Wrapper = calculate::Wrapper<int>;
    const auto hash = std::hash<Wrapper>{};

    GIVEN( "a wrapper object" ) {
        auto wrapper = Wrapper{};

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

SCENARIO( "moved wrapper objects are safe to move", "[move]" ) {
    using Wrapper = calculate::Wrapper<int>;

    GIVEN( "a wrapper object" ) {
        auto wrapper = Wrapper{};
        auto args = std::vector<decltype(wrapper())>(wrapper.argc());

        WHEN( "it is moved" ) {
            std::move(wrapper);

            THEN( "it still remains a valid object" ) {
                REQUIRE_NOTHROW( wrapper(args) );
                CHECK( wrapper(args) == Wrapper{}(args) );
            }
        }
    }
}

SCENARIO( "calling wrapper objects", "[call]" ) {
    using Wrapper = calculate::Wrapper<int>;

    GIVEN( "a wrapped callable" ) {
        auto wrapper = Wrapper{[](int a, int b) noexcept { return a + b; }};

        THEN( "it can be called directly or used on any iterable container" ) {
            int carray[] = {1, 2};
            auto array = std::array<int, 2>{1, 2};
            auto list = std::list<int>{1, 2};
            auto vector = std::vector<int>{1, 2};

            REQUIRE_NOTHROW( wrapper(1, 2) );
            REQUIRE_NOTHROW( wrapper(carray) );
            REQUIRE_NOTHROW( wrapper(array) );
            REQUIRE_NOTHROW( wrapper(list) );
            REQUIRE_NOTHROW( wrapper(vector) );
            CHECK( wrapper(1, 2) == 3 );
            CHECK( wrapper(carray) == 3 );
            CHECK( wrapper(array) == 3 );
            CHECK( wrapper(list) == 3 );
            CHECK( wrapper(vector) == 3 );
        }

        WHEN( "the wrapper object is called with the wrong number of arguments" ) {
            THEN( "it throws an exception" ) {
                CHECK_THROWS_AS( wrapper(1), calculate::ArgumentsMismatch );
                CHECK_NOTHROW( wrapper(1, 2));
                CHECK_THROWS_AS( wrapper(1, 2, 3), calculate::ArgumentsMismatch );
            }
        }
    }
}

SCENARIO( "evaluating wrapper objects", "[evaluation]" ) {
    using Wrapper = calculate::Wrapper<int, Intermediary>;

    GIVEN( "a wrapped callable using the default adapter" ) {
        auto wrapper = Wrapper{[](int n) noexcept { return n; }};

        THEN( "it can be evaluated directly or used on any iterable container" ) {
            Intermediary carray[] = {Intermediary{1}};
            auto array = std::array<Intermediary, 1>{Intermediary{1}};
            auto list = std::list<Intermediary>{Intermediary{1}};
            auto vector = std::vector<Intermediary>{Intermediary{1}};

            REQUIRE_NOTHROW( wrapper.eval(Intermediary{1}) );
            REQUIRE_NOTHROW( wrapper.eval(carray) );
            REQUIRE_NOTHROW( wrapper.eval(array) );
            REQUIRE_NOTHROW( wrapper.eval(list) );
            REQUIRE_NOTHROW( wrapper.eval(vector) );
            CHECK( wrapper.eval(Intermediary{1}) == 1 );
            CHECK( wrapper.eval(carray) == 1 );
            CHECK( wrapper.eval(array) == 1 );
            CHECK( wrapper.eval(list) == 1 );
            CHECK( wrapper.eval(vector) == 1 );
        }

        WHEN( "the wrapper object is evaluated with the wrong number of arguments" ) {
            THEN( "it throws an exception" ) {
                CHECK_THROWS_AS( wrapper.eval(), calculate::ArgumentsMismatch );
                CHECK_NOTHROW( wrapper.eval(Intermediary{1}) );
            }
        }
    }

    GIVEN ( "a wrapped callable using a custom adapter" ) {
        auto wrapper = Wrapper{
            [](int n) noexcept { return n; },
            [](Intermediary i) noexcept { return int{i} + 1; }
        };

        WHEN( "it is called the adapter function is not applied" ) {
            CHECK( wrapper(Intermediary{1}) != 2 );
        }

        WHEN( "it is evaluated the adapter function is applied" ) {
            CHECK( wrapper.eval(Intermediary{1}) == 2 );
        }
    }
}
