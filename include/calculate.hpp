/*
    Calculate - Version 2.1.1rc3
    Last modified 2018/06/27
    Released under MIT license
    Copyright (c) 2016-2018 Alberto Lorenzo <alorenzo.md@gmail.com>
*/


#ifndef __CALCULATE_HPP__
#define __CALCULATE_HPP__

#include "calculate/parser.hpp"


namespace calculate {

namespace defaults {

template<typename T> T add(const T& x, const T& y) noexcept { return x + y; }
template<typename T> T sub(const T& x, const T& y) noexcept { return x - y; }
template<typename T> T mul(const T& x, const T& y) noexcept { return x * y; }
template<typename T> T div(const T& x, const T& y) noexcept { return x / y; }

struct Precedence {
    static constexpr std::size_t very_low = 1111u;
    static constexpr std::size_t low = 2222u;
    static constexpr std::size_t normal = 5555u;
    static constexpr std::size_t high = 8888u;
    static constexpr std::size_t very_high = 9999u;
};

}


class Parser : public BaseParser<double> {
public:
    Parser(const Lexer& lexer=make_lexer<Type>()) : BaseParser<Type>{lexer} {
        using namespace defaults;
        using F1 = Type(*)(Type);
        using F2 = Type(*)(Type, Type);
        using F3 = Type(*)(Type, Type, Type);

        auto pow = [](Type x1, Type x2) noexcept {
            if (x2 <= 0. || x2 > 256 || std::trunc(x2) != x2)
                return std::pow(x1, x2);

            auto exp = static_cast<int>(x2);
            auto prod = 1.;
            while (exp) {
                if (exp & 1)
                    prod *= x1;
                exp >>= 1;
                x1 *= x1;
            }
            return prod;
        };

        auto fact = [](Type x) noexcept {
            if (x > 256)
                return std::numeric_limits<Type>::infinity();

            auto prod = 1.;
            for (auto i = 2.; i <= x; i++)
                prod *= i;
            return prod;
        };

        constants.insert({
            {"pi", 3.14159265358979323846},
            {"e", 2.71828182845904523536},
            {"phi", 1.61803398874989484820},
            {"gamma", 0.57721566490153286060}
        });

        functions.insert({
            {"id", [](Type x) noexcept { return x; }},
            {"neg", [](Type x) noexcept { return -x; }},
            {"inv", [](Type x) noexcept { return Type{1} / x; }},
            {"fabs", static_cast<F1>(std::fabs)},
            {"abs", static_cast<F1>(std::abs)},
            {"fma", static_cast<F3>(std::fma)},
            {"copysign", static_cast<F2>(std::copysign)},
            {"nextafter", static_cast<F2>(std::nextafter)},
            {"fdim", static_cast<F2>(std::fdim)},
            {"fmax", static_cast<F2>(std::fmax)},
            {"fmin", static_cast<F2>(std::fmin)},
            {"fdim", static_cast<F2>(std::fdim)},
            {"fmax", static_cast<F2>(std::fmax)},
            {"fmin", static_cast<F2>(std::fmin)},
            {"ceil", static_cast<F1>(std::ceil)},
            {"floor", static_cast<F1>(std::floor)},
            {"fmod", static_cast<F2>(std::fmod)},
            {"trunc", static_cast<F1>(std::trunc)},
            {"round", static_cast<F1>(std::round)},
            {"rint", static_cast<F1>(std::rint)},
            {"nearbyint", static_cast<F1>(std::nearbyint)},
            {"remainder", static_cast<F2>(std::remainder)},
            {"pow", static_cast<F2>(std::pow)},
            {"sqrt", static_cast<F1>(std::sqrt)},
            {"cbrt", static_cast<F1>(std::cbrt)},
            {"hypot", static_cast<F2>(std::hypot)},
            {"exp", static_cast<F1>(std::exp)},
            {"expm1", static_cast<F1>(std::expm1)},
            {"exp2", static_cast<F1>(std::exp2)},
            {"log", static_cast<F1>(std::log)},
            {"log10", static_cast<F1>(std::log10)},
            {"log1p", static_cast<F1>(std::log1p)},
            {"log2", static_cast<F1>(std::log2)},
            {"logb", static_cast<F1>(std::logb)},
            {"sin", static_cast<F1>(std::sin)},
            {"cos", static_cast<F1>(std::cos)},
            {"tan", static_cast<F1>(std::tan)},
            {"asin", static_cast<F1>(std::asin)},
            {"acos", static_cast<F1>(std::acos)},
            {"atan", static_cast<F1>(std::atan)},
            {"atan2", static_cast<F2>(std::atan2)},
            {"sinh", static_cast<F1>(std::sinh)},
            {"cosh", static_cast<F1>(std::cosh)},
            {"tanh", static_cast<F1>(std::tanh)},
            {"asinh", static_cast<F1>(std::asinh)},
            {"acosh", static_cast<F1>(std::acosh)},
            {"atanh", static_cast<F1>(std::atanh)},
            {"erf", static_cast<F1>(std::erf)},
            {"erfc", static_cast<F1>(std::erfc)},
            {"tgamma", static_cast<F1>(std::tgamma)},
            {"lgamma", static_cast<F1>(std::lgamma)},
            {"fact", fact}
        });

        operators.insert({
            {"+", {add<Type>, Precedence::low, Associativity::FULL}},
            {"-", {sub<Type>, Precedence::low, Associativity::LEFT}},
            {"*", {mul<Type>, Precedence::normal, Associativity::FULL}},
            {"/", {div<Type>, Precedence::normal, Associativity::LEFT}},
            {"%", {static_cast<F2>(std::fmod), Precedence::normal, Associativity::LEFT}},
            {"^", {pow, Precedence::high, Associativity::RIGHT}}
        });

        prefixes.insert({{"+", "id"}, {"-", "neg"}});

        suffixes.insert({{"!", "fact"}});
    }
};


class ComplexParser : public BaseParser<std::complex<double>> {
public:
    ComplexParser(const Lexer& lexer=make_lexer<Type>()) : BaseParser<Type>{lexer} {
        using namespace std::complex_literals;
        using namespace defaults;
        using F1 = Type(*)(const Type&);
        using F2 = Type(*)(const Type&, const Type&);

        auto polar = [](const Type& z1, const Type& z2) noexcept {
            return z1 * std::exp(1.i * z2);
        };

        constants.insert({
            {"i", Type{0., 1.}},
            {"pi", Type{3.14159265358979323846, 0.}},
            {"e", Type{2.71828182845904523536, 0.}},
            {"phi", Type{1.61803398874989484820, 0.}},
            {"gamma", Type{0.57721566490153286060, 0.}}
        });

        functions.insert({
            {"id", [](const Type& z) noexcept { return z; }},
            {"neg", [](const Type& z) noexcept { return -z; }},
            {"inv", [](const Type& z) noexcept { return Type{1} / z; }},
            {"real", [](const Type& z) noexcept { return Type{std::real(z)}; }},
            {"imag", [](const Type& z) noexcept { return Type{std::imag(z)}; }},
            {"abs", [](const Type& z) noexcept { return Type{std::abs(z)}; }},
            {"arg", [](const Type& z) noexcept { return Type{std::arg(z)}; }},
            {"norm", [](const Type& z) noexcept { return Type{std::norm(z)}; }},
            {"polar", polar},
            {"conj", static_cast<F1>(std::conj)},
            {"proj", static_cast<F1>(std::exp)},
            {"exp", static_cast<F1>(std::exp)},
            {"log", static_cast<F1>(std::log)},
            {"log10", static_cast<F1>(std::log10)},
            {"pow", static_cast<F2>(std::pow)},
            {"sqrt", static_cast<F1>(std::sqrt)},
            {"sin", static_cast<F1>(std::sin)},
            {"sinh", static_cast<F1>(std::sinh)},
            {"cos", static_cast<F1>(std::cos)},
            {"cosh", static_cast<F1>(std::cosh)},
            {"tan", static_cast<F1>(std::tan)},
            {"tanh", static_cast<F1>(std::tanh)},
            {"asin", static_cast<F1>(std::asin)},
            {"asinh", static_cast<F1>(std::asinh)},
            {"acos", static_cast<F1>(std::acos)},
            {"acosh", static_cast<F1>(std::acosh)},
            {"atan", static_cast<F1>(std::atan)},
            {"atanh", static_cast<F1>(std::atanh)}
        });

        operators.insert({
            {"+", {add<Type>, Precedence::low, Associativity::FULL}},
            {"-", {sub<Type>, Precedence::low, Associativity::LEFT}},
            {"*", {mul<Type>, Precedence::normal, Associativity::FULL}},
            {"/", {div<Type>, Precedence::normal, Associativity::LEFT}},
            {"^", {static_cast<F2>(std::pow), Precedence::high, Associativity::RIGHT}}
        });

        prefixes.insert({{"+", "id"}, {"-", "neg"}});
    }
};

}

#endif
