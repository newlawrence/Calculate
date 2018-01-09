#ifndef __CALCULATE_HPP__
#define __CALCULATE_HPP__

#include <cmath>

#include "calculate/parser.hpp"


namespace calculate {

class Parser : public BaseParser<double> {};

class DefaultParser : public Parser {
public:
    DefaultParser() {
        set<Constant>("pi", 3.14159265358979323846);
        set<Constant>("e", 2.71828182845904523536);
        set<Constant>("phi", 1.61803398874989484820);
        set<Constant>("gamma", 0.57721566490153286060);

        set<Function>("id",
            [](Type x) noexcept { return x; }
        );
        set<Function>("neg",
            [](Type x) noexcept { return -x; }
        );
        set<Function>("inv",
            [](Type x) noexcept { return Type{1} / x; }
        );

        set<Function>("fabs",
            static_cast<Type(*)(Type)>(std::fabs)
        );
        set<Function>("abs",
            static_cast<Type(*)(Type)>(std::abs)
        );
        set<Function>("fma",
            static_cast<Type(*)(Type, Type, Type)>(std::fma)
        );
        set<Function>("copysign",
            static_cast<Type(*)(Type, Type)>(std::copysign)
        );
        set<Function>("nextafter",
            static_cast<Type(*)(Type, Type)>(std::nextafter)
        );

        set<Function>("fdim",
            static_cast<Type(*)(Type, Type)>(std::fdim)
        );
        set<Function>("fmax",
            static_cast<Type(*)(Type, Type)>(std::fmax)
        );
        set<Function>("fmin",
            static_cast<Type(*)(Type, Type)>(std::fmin)
        );

        set<Function>("ceil",
            static_cast<Type(*)(Type)>(std::ceil)
        );
        set<Function>("floor",
            static_cast<Type(*)(Type)>(std::floor)
        );
        set<Function>("fmod",
            static_cast<Type(*)(Type, Type)>(std::fmod)
        );
        set<Function>("trunc",
            static_cast<Type(*)(Type)>(std::trunc)
        );
        set<Function>("round",
            static_cast<Type(*)(Type)>(std::round)
        );
        set<Function>("rint",
            static_cast<Type(*)(Type)>(std::rint)
        );
        set<Function>("nearbyint",
            static_cast<Type(*)(Type)>(std::nearbyint)
        );
        set<Function>("remainder",
            static_cast<Type(*)(Type, Type)>(std::remainder)
        );

        set<Function>("pow",
            static_cast<Type(*)(Type, Type)>(std::pow)
        );
        set<Function>("sqrt",
            static_cast<Type(*)(Type)>(std::sqrt)
        );
        set<Function>("cbrt",
            static_cast<Type(*)(Type)>(std::cbrt)
        );
        set<Function>("hypot",
            static_cast<Type(*)(Type, Type)>(std::hypot)
        );
        set<Function>("exp",
            static_cast<Type(*)(Type)>(std::exp)
        );
        set<Function>("expm1",
            static_cast<Type(*)(Type)>(std::expm1)
        );
        set<Function>("exp2",
            static_cast<Type(*)(Type)>(std::exp2)
        );
        set<Function>("log",
            static_cast<Type(*)(Type)>(std::log)
        );
        set<Function>("log10",
            static_cast<Type(*)(Type)>(std::log10)
        );
        set<Function>("log1p",
            static_cast<Type(*)(Type)>(std::log1p)
        );
        set<Function>("log2",
            static_cast<Type(*)(Type)>(std::log2)
        );
        set<Function>("logb",
            static_cast<Type(*)(Type)>(std::logb)
        );

        set<Function>("sin",
            static_cast<Type(*)(Type)>(std::sin)
        );
        set<Function>("cos",
            static_cast<Type(*)(Type)>(std::cos)
        );
        set<Function>("tan",
            static_cast<Type(*)(Type)>(std::tan)
        );
        set<Function>("asin",
            static_cast<Type(*)(Type)>(std::asin)
        );
        set<Function>("acos",
            static_cast<Type(*)(Type)>(std::acos)
        );
        set<Function>("atan",
            static_cast<Type(*)(Type)>(std::atan)
        );
        set<Function>("atan2",
            static_cast<Type(*)(Type, Type)>(std::atan2)
        );

        set<Function>("sinh",
            static_cast<Type(*)(Type)>(std::sinh)
        );
        set<Function>("cosh",
            static_cast<Type(*)(Type)>(std::cosh)
        );
        set<Function>("tanh",
            static_cast<Type(*)(Type)>(std::tanh)
        );
        set<Function>("asinh",
            static_cast<Type(*)(Type)>(std::asinh)
        );
        set<Function>("acosh",
            static_cast<Type(*)(Type)>(std::acosh)
        );
        set<Function>("atanh",
            static_cast<Type(*)(Type)>(std::atanh)
        );

        set<Function>("erf",
            static_cast<Type(*)(Type)>(std::erf)
        );
        set<Function>("erfc",
            static_cast<Type(*)(Type)>(std::erfc)
        );
        set<Function>("tgamma",
            static_cast<Type(*)(Type)>(std::tgamma)
        );
        set<Function>("lgamma",
            static_cast<Type(*)(Type)>(std::lgamma)
        );

        set<Operator>("+",
            [](Type x1, Type x2) noexcept { return x1 + x2; },
            "id", std::size_t{3333}, Operator::Associativity::BOTH
        );
        set<Operator>("-",
            [](Type x1, Type x2) noexcept { return x1 - x2; },
            "neg", std::size_t{3333}, Operator::Associativity::LEFT
        );
        set<Operator>("*",
            [](Type x1, Type x2) noexcept { return x1 * x2; },
            "", std::size_t{6666}, Operator::Associativity::BOTH
        );
        set<Operator>("/",
            [](Type x1, Type x2) noexcept { return x1 / x2; },
            "", std::size_t{6666}, Operator::Associativity::LEFT
        );
        set<Operator>("%",
            static_cast<Type(*)(Type, Type)>(std::fmod),
            "", std::size_t{6666}, Operator::Associativity::LEFT
        );
        set<Operator>("^",
            static_cast<Type(*)(Type, Type)>(std::pow),
            "", std::size_t{9999}, Operator::Associativity::RIGHT
        );
    }
};


class ComplexParser : public BaseParser<std::complex<double>> {};

class DefaultComplexParser : public ComplexParser {
public:
    DefaultComplexParser() {
        using namespace std::complex_literals;

        set<Constant>("i", Type{0., 1.});
        set<Constant>("pi", Type{3.14159265358979323846, 0.});
        set<Constant>("e", Type{2.71828182845904523536, 0.});
        set<Constant>("phi", Type{1.61803398874989484820, 0.});
        set<Constant>("gamma", Type{0.57721566490153286060, 0.});

        set<Function>("id",
            [](const Type& z) noexcept { return z; }
        );
        set<Function>("neg",
            [](const Type& z) noexcept { return -z; }
        );
        set<Function>("inv",
            [](const Type& z) noexcept { return Type{1} / z; }
        );

        set<Function>("real",
            [](const Type& z) noexcept {
                return static_cast<Type>(std::real(z));
            }
        );
        set<Function>("imag",
            [](const Type& z) noexcept {
                return static_cast<Type>(std::imag(z));
            }
        );
        set<Function>("abs",
            [](const Type& z) noexcept {
                return static_cast<Type>(std::abs(z));
            }
        );
        set<Function>("arg",
            [](const Type& z) noexcept {
                return static_cast<Type>(std::arg(z));
            }
        );
        set<Function>("norm",
            [](const Type& z) noexcept {
                return static_cast<Type>(std::norm(z));
            }
        );
        set<Function>("polar",
            [](const Type& z1, const Type& z2) noexcept {
                return z1 * std::exp(1i * z2);
            }
        );
        set<Function>("conj",
            static_cast<Type(*)(const Type&)>(std::conj)
        );
        set<Function>("proj",
            static_cast<Type(*)(const Type&)>(std::proj)
        );

        set<Function>("exp",
            static_cast<Type(*)(const Type&)>(std::exp)
        );
        set<Function>("log",
            static_cast<Type(*)(const Type&)>(std::log)
        );
        set<Function>("log10",
            static_cast<Type(*)(const Type&)>(std::log10)
        );
        set<Function>("pow",
            static_cast<Type(*)(const Type&, const Type&)>(std::pow)
        );
        set<Function>("sqrt",
            static_cast<Type(*)(const Type&)>(std::sqrt)
        );
        set<Function>("sin",
            static_cast<Type(*)(const Type&)>(std::sin)
        );
        set<Function>("sinh",
            static_cast<Type(*)(const Type&)>(std::sinh)
        );
        set<Function>("cos",
            static_cast<Type(*)(const Type&)>(std::cos)
        );
        set<Function>("cosh",
            static_cast<Type(*)(const Type&)>(std::cosh)
        );
        set<Function>("tan",
            static_cast<Type(*)(const Type&)>(std::tan)
        );
        set<Function>("tanh",
            static_cast<Type(*)(const Type&)>(std::tanh)
        );
        set<Function>("asin",
            static_cast<Type(*)(const Type&)>(std::asin)
        );
        set<Function>("asinh",
            static_cast<Type(*)(const Type&)>(std::asinh)
        );
        set<Function>("acos",
            static_cast<Type(*)(const Type&)>(std::acos)
        );
        set<Function>("acosh",
            static_cast<Type(*)(const Type&)>(std::acosh)
        );
        set<Function>("atan",
            static_cast<Type(*)(const Type&)>(std::atan)
        );
        set<Function>("atanh",
            static_cast<Type(*)(const Type&)>(std::atanh)
        );

        set<Operator>("+",
            [](const Type& z1, const Type& z2) noexcept { return z1 + z2; },
            "id", std::size_t{3333}, Operator::Associativity::BOTH
        );
        set<Operator>("-",
            [](const Type& z1, const Type& z2) noexcept { return z1 - z2; },
            "neg", std::size_t{3333}, Operator::Associativity::LEFT
        );
        set<Operator>("*",
            [](const Type& z1, const Type& z2) noexcept { return z1 * z2; },
            "", std::size_t{6666}, Operator::Associativity::BOTH
        );
        set<Operator>("/",
            [](const Type& z1, const Type& z2) noexcept { return z1 / z2; },
            "", std::size_t{6666}, Operator::Associativity::LEFT
        );
        set<Operator>("^",
            [](const Type& z1, const Type& z2) { return std::pow(z1, z2); },
            "", std::size_t{9999}, Operator::Associativity::RIGHT
        );
    }
};

}

#endif
