#ifndef __CALCULATE_HPP__
#define __CALCULATE_HPP__

#include <cmath>

#include "calculate/parser.hpp"


namespace calculate {

using Parser = BaseParser<double>;

class DefaultParser : public Parser {
public:
    template<typename LexerType>
    DefaultParser(const LexerType& lexer) : Parser{lexer} {
        using Associativity = Operator::Associativity;

        auto add = [](Type x1, Type x2) noexcept { return x1 + x2; };
        auto subtract = [](Type x1, Type x2) noexcept { return x1 - x2; };
        auto multiply = [](Type x1, Type x2) noexcept { return x1 * x2; };
        auto divide = [](Type x1, Type x2) noexcept { return x1 / x2; };
        auto truncate = static_cast<Type(*)(Type, Type)>(std::fmod);
        auto raise = static_cast<Type(*)(Type, Type)>(std::pow);

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
            {"fabs", static_cast<Type(*)(Type)>(std::fabs)},
            {"abs", static_cast<Type(*)(Type)>(std::abs)},
            {"fma", static_cast<Type(*)(Type, Type, Type)>(std::fma)},
            {"copysign", static_cast<Type(*)(Type, Type)>(std::copysign)},
            {"nextafter", static_cast<Type(*)(Type, Type)>(std::nextafter)},
            {"fdim", static_cast<Type(*)(Type, Type)>(std::fdim)},
            {"fmax", static_cast<Type(*)(Type, Type)>(std::fmax)},
            {"fmin", static_cast<Type(*)(Type, Type)>(std::fmin)},
            {"fdim", static_cast<Type(*)(Type, Type)>(std::fdim)},
            {"fmax", static_cast<Type(*)(Type, Type)>(std::fmax)},
            {"fmin", static_cast<Type(*)(Type, Type)>(std::fmin)},
            {"ceil", static_cast<Type(*)(Type)>(std::ceil)},
            {"floor", static_cast<Type(*)(Type)>(std::floor)},
            {"fmod", static_cast<Type(*)(Type, Type)>(std::fmod)},
            {"trunc", static_cast<Type(*)(Type)>(std::trunc)},
            {"round", static_cast<Type(*)(Type)>(std::round)},
            {"rint", static_cast<Type(*)(Type)>(std::rint)},
            {"nearbyint", static_cast<Type(*)(Type)>(std::nearbyint)},
            {"remainder", static_cast<Type(*)(Type, Type)>(std::remainder)},
            {"pow", static_cast<Type(*)(Type, Type)>(std::pow)},
            {"sqrt", static_cast<Type(*)(Type)>(std::sqrt)},
            {"cbrt", static_cast<Type(*)(Type)>(std::cbrt)},
            {"hypot", static_cast<Type(*)(Type, Type)>(std::hypot)},
            {"exp", static_cast<Type(*)(Type)>(std::exp)},
            {"expm1", static_cast<Type(*)(Type)>(std::expm1)},
            {"exp2", static_cast<Type(*)(Type)>(std::exp2)},
            {"log", static_cast<Type(*)(Type)>(std::log)},
            {"log10", static_cast<Type(*)(Type)>(std::log10)},
            {"log1p", static_cast<Type(*)(Type)>(std::log1p)},
            {"log2", static_cast<Type(*)(Type)>(std::log2)},
            {"logb", static_cast<Type(*)(Type)>(std::logb)},
            {"sin", static_cast<Type(*)(Type)>(std::sin)},
            {"cos", static_cast<Type(*)(Type)>(std::cos)},
            {"tan", static_cast<Type(*)(Type)>(std::tan)},
            {"asin", static_cast<Type(*)(Type)>(std::asin)},
            {"acos", static_cast<Type(*)(Type)>(std::acos)},
            {"atan", static_cast<Type(*)(Type)>(std::atan)},
            {"atan2", static_cast<Type(*)(Type, Type)>(std::atan2)},
            {"sinh", static_cast<Type(*)(Type)>(std::sinh)},
            {"cosh", static_cast<Type(*)(Type)>(std::cosh)},
            {"tanh", static_cast<Type(*)(Type)>(std::tanh)},
            {"asinh", static_cast<Type(*)(Type)>(std::asinh)},
            {"acosh", static_cast<Type(*)(Type)>(std::acosh)},
            {"atanh", static_cast<Type(*)(Type)>(std::atanh)},
            {"erf", static_cast<Type(*)(Type)>(std::erf)},
            {"erfc", static_cast<Type(*)(Type)>(std::erfc)},
            {"tgamma", static_cast<Type(*)(Type)>(std::tgamma)},
            {"lgamma", static_cast<Type(*)(Type)>(std::lgamma)}
        });

        operators.insert({
            {"+", {std::move(add), "id", 3333u, Associativity::BOTH}},
            {"-", {std::move(subtract), "neg", 3333u, Associativity::LEFT}},
            {"*", {std::move(multiply), "", 6666u, Associativity::BOTH}},
            {"/", {std::move(divide), "", 6666u, Associativity::LEFT}},
            {"%", {std::move(truncate), "", 6666u, Associativity::LEFT}},
            {"^", {std::move(raise), "", 9999u, Associativity::RIGHT}}
        });
    }

    DefaultParser() : DefaultParser{DefaultLexer{}} {}
};


using ComplexParser = BaseParser<std::complex<double>>;

class DefaultComplexParser : public ComplexParser {
public:
    template<typename LexerType>
    DefaultComplexParser(const LexerType& lexer) : ComplexParser{lexer} {
        using namespace std::complex_literals;
        using Associativity = Operator::Associativity;

        auto real = [](const Type& z) noexcept {
            return static_cast<Type>(std::real(z));
        };
        auto imag = [](const Type& z) noexcept {
            return static_cast<Type>(std::imag(z));
        };
        auto abs = [](const Type& z) noexcept {
            return static_cast<Type>(std::abs(z));
        };
        auto arg = [](const Type& z) noexcept {
            return static_cast<Type>(std::arg(z));
        };
        auto norm = [](const Type& z) noexcept {
            return static_cast<Type>(std::norm(z));
        };
        auto polar = [](const Type& z1, const Type& z2) noexcept {
            return z1 * std::exp(1i * z2);
        };

        auto add = [](const Type& z1, const Type& z2) noexcept {
            return z1 + z2;
        };
        auto subtract = [](const Type& z1, const Type& z2) noexcept {
            return z1 - z2;
        };
        auto multiply = [](const Type& z1, const Type& z2) noexcept {
            return z1 * z2;
        };
        auto divide = [](const Type& z1, const Type& z2) noexcept {
            return z1 / z2;
        };
        auto raise = [](const Type& z1, const Type& z2) {
            return std::pow(z1, z2);
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
            {"real", std::move(real)},
            {"imag", std::move(imag)},
            {"abs", std::move(abs)},
            {"arg", std::move(arg)},
            {"norm", std::move(norm)},
            {"polar", std::move(polar)},
            {"conj", static_cast<Type(*)(const Type&)>(std::conj)},
            {"proj", static_cast<Type(*)(const Type&)>(std::exp)},
            {"exp", static_cast<Type(*)(const Type&)>(std::exp)},
            {"log", static_cast<Type(*)(const Type&)>(std::log)},
            {"log10", static_cast<Type(*)(const Type&)>(std::log10)},
            {"pow", static_cast<Type(*)(const Type&, const Type&)>(std::pow)},
            {"sqrt", static_cast<Type(*)(const Type&)>(std::sqrt)},
            {"sin", static_cast<Type(*)(const Type&)>(std::sin)},
            {"sinh", static_cast<Type(*)(const Type&)>(std::sinh)},
            {"cos", static_cast<Type(*)(const Type&)>(std::cos)},
            {"cosh", static_cast<Type(*)(const Type&)>(std::cosh)},
            {"tan", static_cast<Type(*)(const Type&)>(std::tan)},
            {"tanh", static_cast<Type(*)(const Type&)>(std::tanh)},
            {"asin", static_cast<Type(*)(const Type&)>(std::asin)},
            {"asinh", static_cast<Type(*)(const Type&)>(std::asinh)},
            {"acos", static_cast<Type(*)(const Type&)>(std::acos)},
            {"acosh", static_cast<Type(*)(const Type&)>(std::acosh)},
            {"atan", static_cast<Type(*)(const Type&)>(std::atan)},
            {"atanh", static_cast<Type(*)(const Type&)>(std::atanh)}
        });

        operators.insert({
            {"+", {std::move(add), "id", 3333u, Associativity::BOTH}},
            {"-", {std::move(subtract), "neg", 3333u, Associativity::LEFT}},
            {"*", {std::move(multiply), "", 6666u, Associativity::BOTH}},
            {"/", {std::move(divide), "", 6666u, Associativity::LEFT}},
            {"^", {std::move(raise), "", 9999u, Associativity::RIGHT}}
        });
    }

    DefaultComplexParser() : DefaultComplexParser{DefaultLexer{}} {}
};

}

#endif
