#include "catch.hpp"
#include "calculate.h"

using namespace calculate;


TEST_CASE("Parser test", "[Parser]") {

    SECTION("Parser accuracy tests") {
        CHECK(Expression("1")() == Approx(1.).epsilon(1e-3));
        CHECK(Expression("+1")() == Approx(1.).epsilon(1e-3));
        CHECK(Expression("-1")() == Approx(-1.).epsilon(1e-3));
        CHECK(Expression("1.")() == Approx(1.).epsilon(1e-3));
        CHECK(Expression("+1.")() == Approx(1.).epsilon(1e-3));
        CHECK(Expression("-1.")() == Approx(-1.).epsilon(1e-3));
        CHECK(Expression(".1")() == Approx(0.1).epsilon(1e-3));
        CHECK(Expression("+.1")() == Approx(0.1).epsilon(1e-3));
        CHECK(Expression("-.1")() == Approx(-0.1).epsilon(1e-3));
        CHECK(Expression("1e2")() == Approx(100.).epsilon(1e-3));
        CHECK(Expression("1e+2")() == Approx(100.).epsilon(1e-3));
        CHECK(Expression("1e-2")() == Approx(0.01).epsilon(1e-3));
        CHECK(Expression("+1e-2")() == Approx(0.01).epsilon(1e-3));
        CHECK(Expression("-1e-2")() == Approx(-0.01).epsilon(1e-3));
        CHECK(Expression("1.e2")() == Approx(100.).epsilon(1e-3));
        CHECK(Expression(".1e2")() == Approx(10.).epsilon(1e-3));
        CHECK(Expression("1.1e2")() == Approx(110.).epsilon(1e-3));
        CHECK(Expression("pi")() == Approx(3.142).epsilon(1e-3));
        CHECK(Expression("+pi")() == Approx(3.142).epsilon(1e-3));
        CHECK(Expression("-pi")() == Approx(-3.142).epsilon(1e-3));
        CHECK(Expression("1+2")() == Approx(3.).epsilon(1e-3));
        CHECK(Expression("1-2")() == Approx(-1.).epsilon(1e-3));
        CHECK(Expression("pi+e")() == Approx(5.86).epsilon(1e-3));
        CHECK(Expression("pi-e")() == Approx(0.423).epsilon(1e-3));
        CHECK(Expression("(1 + 2)")() == Approx(3.).epsilon(1e-3));
        CHECK(Expression("(+1 + +2)")() == Approx(3.).epsilon(1e-3));
        CHECK(Expression("(-1 + -2)")() == Approx(-3.).epsilon(1e-3));
        CHECK(Expression("-(1 + 2)")() == Approx(-3.).epsilon(1e-3));
        CHECK(Expression("(pi + e)")() == Approx(5.86).epsilon(1e-3));
        CHECK(Expression("(+pi + +e)")() == Approx(5.86).epsilon(1e-3));
        CHECK(Expression("(-pi + -e)")() == Approx(-5.86).epsilon(1e-3));
        CHECK(Expression("-(-pi + -e)")() == Approx(5.86).epsilon(1e-3));
        CHECK(Expression("sin(1)")() == Approx(0.841).epsilon(1e-3));
        CHECK(Expression("sin(+1)")() == Approx(0.841).epsilon(1e-3));
        CHECK(Expression("sin(-1)")() == Approx(-0.841).epsilon(1e-3));
        CHECK(Expression("-sin(1)")() == Approx(-0.841).epsilon(1e-3));
        CHECK(Expression("sin(pi)")() == Approx(0.).epsilon(1e-3));
        CHECK(Expression("sin(+pi)")() == Approx(0.).epsilon(1e-3));
        CHECK(Expression("sin(-pi)")() == Approx(0.).epsilon(1e-3));
        CHECK(Expression("-sin(pi)")() == Approx(0.).epsilon(1e-3));
        CHECK(Expression("hypot(4, 3)")() == Approx(5.).epsilon(1e-3));
        CHECK(Expression("hypot(4, +3)")() == Approx(5.).epsilon(1e-3));
        CHECK(Expression("hypot(4, -3)")() == Approx(5.).epsilon(1e-3));
        CHECK(Expression("hypot(4, pi)")() == Approx(5.086).epsilon(1e-3));
        CHECK(Expression("hypot(4, +pi)")() == Approx(5.086).epsilon(1e-3));
        CHECK(Expression("hypot(4, -pi)")() == Approx(5.086).epsilon(1e-3));
        CHECK(Expression("hypot(4, (3))")() == Approx(5.).epsilon(1e-3));
        CHECK(Expression("hypot(4, +(3))")() == Approx(5.).epsilon(1e-3));
        CHECK(Expression("hypot(4, -(3))")() == Approx(5.).epsilon(1e-3));
        CHECK(Expression("(1)")() == Approx(1.).epsilon(1e-3));
        CHECK(Expression("((1))")() == Approx(1.).epsilon(1e-3));
        CHECK(Expression("((1)+(2))")() == Approx(3.).epsilon(1e-3));
        CHECK(Expression("(((1))-((2)))")() == Approx(-1.).epsilon(1e-3));
        CHECK(Expression("1 + 2")() == Approx(3.).epsilon(1e-3));
        CHECK(Expression("1 + 2 * 3")() == Approx(7.).epsilon(1e-3));
        CHECK(Expression("1 + 2 * 3 ^ 4")() == Approx(163.).epsilon(1e-3));
        CHECK(Expression("1 + 2 * 3 ** 4")() == Approx(163.).epsilon(1e-3));
    }

}
