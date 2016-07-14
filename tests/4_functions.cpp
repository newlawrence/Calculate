#include <cmath>

#include "catch.hpp"
#include "calculate.h"

#define eval(e) calculate::Calculate(e)()
#define approx(x) Approx(x).epsilon(1e-12)


TEST_CASE("Builtin functions", "[functions]") {

    SECTION("Other functions") {
        CHECK(eval("fabs(3.1416)") == approx(3.1416));
        CHECK(eval("abs(-1)") == approx(1));
        CHECK(eval("fma(10, 20, 30)") == approx(230));
        CHECK(eval("copysign(10, -1)") == approx(-10));
        CHECK(eval("nextafter(0.0, 1.0)") == approx(4.94065645841e-324));
    }

    SECTION("Minimum, maximum and difference functions") {
        CHECK(eval("fdim(2, 1)") == approx(1));
        CHECK(eval("fmax(-100, 1)") == approx(1));
        CHECK(eval("fmin(-100, -1)") == approx(-100));
    }

    SECTION("Rounding and remainder functions") {
        CHECK(eval("ceil(2.3)") == approx(3));
        CHECK(eval("floor(3.8)") == approx(3));
        CHECK(eval("fmod(5.3, 2)") == approx(1.3));
        CHECK(eval("trunc(5.5)") == approx(5));
        CHECK(eval("round(5.5)") == approx(6));
        CHECK(eval("rint(-3.8)") == approx(-4));
        CHECK(eval("nearbyint(-2.3)") == approx(-2));
        CHECK(eval("remainder(18.5, 4.2)") == approx(1.7));
    }

    SECTION("Power functions") {
        CHECK(eval("pow(32.01, 1.54)") == approx(208.036691405387));
        CHECK(eval("sqrt(1024)") == approx(32));
        CHECK(eval("cbrt(27)") == approx(3));
        CHECK(eval("hypot(3, 4)") == approx(5));
    }

    SECTION("Exponential and logarithmic functions") {
        CHECK(eval("exp(5)") == approx(148.413159102577));
        CHECK(eval("expm1(1)") == approx(1.718281828459));
        CHECK(eval("log(5.5)") == approx(1.704748092238));
        CHECK(eval("log10(1000)") == approx(3));
        CHECK(eval("log1p(1)") == approx(0.693147180560));
        CHECK(eval("exp2(8)") == approx(256));
        CHECK(eval("log2(1024)") == approx(10));
        CHECK(eval("logb(1024)") == approx(10));
    }

    SECTION("Trigonometric functions") {
        CHECK(eval("sin(pi / 6)") == approx(0.5));
        CHECK(eval("cos(pi / 3)") == approx(0.5));
        CHECK(eval("tan(pi / 4)") == approx(1));
        CHECK(eval("asin(0.5)") == approx(0.523598775598));
        CHECK(eval("acos(0.5)") == approx(1.047197551197));
        CHECK(eval("atan(1)") == approx(0.785398163397));
        CHECK(eval("atan2(10, -10)") == approx(2.356194490192));
    }

    SECTION("Hyperbolic functions") {
        CHECK(eval("sinh(0.69314718056)") == approx(0.75));
        CHECK(eval("cosh(0.69314718056)") == approx(1.25));
        CHECK(eval("tanh(0.69314718056)") == approx(0.6));
        CHECK(eval("asinh(3.62686040785)") == approx(2));
        CHECK(eval("acosh(3.76219569108)") == approx(2));
        CHECK(eval("atanh(0.761594155956)") == approx(1));
    }

    SECTION("Error and gamma functions") {
        CHECK(eval("erf(1.)") == approx(0.842700792950));
        CHECK(eval("erfc(1.)") == approx(0.157299207050));
        CHECK(eval("tgamma(0.5)") == approx(1.772453850906));
        CHECK(eval("lgamma(0.5)") == approx(0.572364942925));
    }

}
