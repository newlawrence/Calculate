#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <array>
#include <list>
#include <vector>

#include "calculate/wrapper.hpp"


class NoThrowObj {
public:
    const std::size_t copied = 0;
    const std::size_t moved = 0;

    NoThrowObj() = default;
    NoThrowObj(std::size_t cp, std::size_t mv) : copied(cp), moved(mv) {}
    NoThrowObj(const NoThrowObj& other) : copied{other.copied + 1}, moved{other.moved} {}
    NoThrowObj(NoThrowObj&& other) noexcept : copied(other.copied), moved(other.moved + 1) {}
    ~NoThrowObj() = default;

    auto operator()() const noexcept { return NoThrowObj{copied, moved}; }
};

class ThrowObj {
public:
    const std::size_t copied = 0;
    const std::size_t moved = 0;

    ThrowObj() = default;
    ThrowObj(std::size_t cp, std::size_t mv) : copied(cp), moved(mv) {}
    ThrowObj(const ThrowObj& other) : copied{other.copied + 1}, moved{other.moved} {}
    ThrowObj(ThrowObj&& other) : copied(other.copied), moved(other.moved + 1) {}
    ~ThrowObj() = default;

    auto operator()() const noexcept { return ThrowObj{copied, moved}; }
};

class Intermediary {
    int _n;

public:
    explicit Intermediary(int n) : _n{n} {}
    Intermediary() : Intermediary{0} {}

    explicit operator int() const noexcept { return _n; }
};


SCENARIO( "Some static assertions on the Wrapper class", "[assertions]" ) {
    using Wrapper = calculate::Wrapper<int>;

    static_assert(
        !std::is_default_constructible<Wrapper>::value,
        "Wrapper objects must not be default constructible"
    );
    static_assert(
        std::is_copy_constructible<Wrapper>::value,
        "Wrapper objects must be copy constructible"
    );
    static_assert(
        std::is_nothrow_move_constructible<Wrapper>::value,
        "Wrapper objects must be no-throw move constructible"
    );
    static_assert(
        std::is_copy_assignable<Wrapper>::value,
        "Wrapper objects must be copy assignable"
    );
    static_assert(
        std::is_move_assignable<Wrapper>::value,
        "Wrapper objects must be move assignable"
    );
    static_assert(
        std::is_nothrow_destructible<Wrapper>::value,
        "Wrapper objects must be no-throw destructible"
    );
    static_assert(
        !std::has_virtual_destructor<Wrapper>::value,
        "Wrapper objects must not be polymorphic"
    );
}

SCENARIO( "construction of a wrapper object", "[construction]" ) {
    GIVEN( "a no-throw move constructible callable" ) {
        using Wrapper = calculate::Wrapper<NoThrowObj>;

        WHEN( "a wrapper of a non temporary object is created" ) {
            auto copy_move = NoThrowObj{};
            auto wrapper = Wrapper{copy_move};

            THEN( "the given object is copied and not moved" ) {
                CHECK( wrapper().copied == 1 );
                CHECK( wrapper().moved == 0 );
            }
        }

        WHEN( "a wrapper of a temporary object is created" ) {
            auto wrapper = Wrapper{NoThrowObj{}};

            THEN( "the given object is moved and not copied" ) {
                CHECK( wrapper().copied == 0 );
                CHECK( wrapper().moved == 1 );
            }
        }
    }

    GIVEN( "a throw move constructible callable" ) {
        using Wrapper = calculate::Wrapper<ThrowObj>;

        WHEN( "a wrapper of a non temporary object is created" ) {
            auto copy_move = ThrowObj{};
            auto wrapper = Wrapper{copy_move};

            THEN( "the given object is copied and not moved" ) {
                CHECK( wrapper().copied == 1 );
                CHECK( wrapper().moved == 0 );
            }
        }

        WHEN( "a wrapper of a temporary object is created" ) {
            auto wrapper = Wrapper{ThrowObj{}};

            THEN( "the given object is copied and not moved" ) {
                CHECK( wrapper().copied == 1 );
                CHECK( wrapper().moved == 0 );
            }
        }
    }
}

SCENARIO( "copy and clone of a wrapper object", "[copy]" ) {
    using Wrapper = calculate::Wrapper<int>;
    const auto hash = std::hash<Wrapper>{};

    GIVEN( "a wrapper object" ) {
        auto wrapper = Wrapper{[]{ return 0; }};

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

SCENARIO( "moved from state wrappers", "[move]" ) {
    using Wrapper = calculate::Wrapper<int>;

    GIVEN( "a wrapper object" ) {
        auto wrapper = Wrapper{[]{ return 0; }};

        WHEN( "it is moved" ) {
            auto other = std::move(wrapper);
            CHECK( !wrapper.valid() );

            THEN( "it cannot be used again until being reassigned" ) {
                wrapper = []{ return 0; };
                CHECK( wrapper.valid() );
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
            [](Intermediary i) noexcept { return static_cast<int>(i) + 1; }
        };

        WHEN( "it is called the adapter function is not applied" ) {
            CHECK( wrapper(Intermediary{1}) != 2 );
        }

        WHEN( "it is evaluated the adapter function is applied" ) {
            CHECK( wrapper.eval(Intermediary{1}) == 2 );
        }
    }
}
