#ifndef __CALCULATE_LEXER_HPP__
#define __CALCULATE_LEXER_HPP__

#include <complex>
#include <iomanip>
#include <regex>
#include <sstream>

#include "exception.hpp"


namespace calculate {

namespace detail {

struct Regex : public std::regex {
    const std::string pattern;
    Regex(const std::string& regex) : std::regex(regex), pattern(regex) {}
};

}


template<typename Type>
class Lexer {
    template<typename Num>
    static std::enable_if_t<std::is_integral<Num>::value, Num> _cast(
        const std::string& token
    ) { return static_cast<Num>(std::stoll(token)); }

    template<typename Num>
    static std::enable_if_t<std::is_floating_point<Num>::value, Num> _cast(
        const std::string& token
    ) { return static_cast<Num>(std::stold(token)); }

public:
    static const std::string left;
    static const std::string right;
    static const std::string decimal;
    static const std::string separator;

    static const detail::Regex number_regex;
    static const detail::Regex name_regex;
    static const detail::Regex symbol_regex;
    static const detail::Regex tokenizer_regex;

    static Type to_value(const std::string& token) {
        if (!std::regex_search(token, number_regex))
            throw BadCast{token};
        return _cast<Type>(token);
    }

    static std::string to_string(Type value) {
        std::ostringstream representation{};

        representation << std::setprecision(std::numeric_limits<Type>::digits10) << value;
        return representation.str();
    }
};

template<typename Type> const std::string Lexer<Type>::left = "(";
template<typename Type> const std::string Lexer<Type>::right = ")";
template<typename Type> const std::string Lexer<Type>::decimal = ".";
template<typename Type> const std::string Lexer<Type>::separator = ",";

template<typename Type>
const auto Lexer<Type>::number_regex = detail::Regex{
    R"_(^(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?$)_"
};
template<typename Type>
const auto Lexer<Type>::name_regex = detail::Regex{
    R"_(^[A-Za-z_]+[A-Za-z_\d]*$)_"
};
template<typename Type>
const auto Lexer<Type>::symbol_regex = detail::Regex{
    R"_(^[^A-Za-z\d.(),_\s]+$)_"
};
template<typename Type>
const auto Lexer<Type>::tokenizer_regex = detail::Regex{
    R"_(((?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)|)_"
    R"_(([A-Za-z_]+[A-Za-z_\d]*)|)_"
    R"_(([^A-Za-z\d\.(),_\s]+)|)_"
    R"_((\()|(\))|(,)|(\.))_"
};


template<typename Type>
class Lexer<std::complex<Type>> {
    static constexpr Type _zero = static_cast<Type>(0);

public:
    static const std::string left;
    static const std::string right;
    static const std::string decimal;
    static const std::string separator;

    static const detail::Regex number_regex;
    static const detail::Regex name_regex;
    static const detail::Regex symbol_regex;
    static const detail::Regex tokenizer_regex;

    static std::complex<Type> to_value(const std::string& token) {
        using namespace std::complex_literals;

        if (token.back() != 'i')
            return std::complex<Type>{Lexer<Type>::to_value(token)};
        else
            return 1i * Lexer<Type>::to_value(token.substr(0, token.size() - 1));
    }

    static std::string to_string(std::complex<Type> value) {
        std::ostringstream representation{};
        Type real{std::real(value)}, imag{std::imag(value)};

        representation << std::setprecision(std::numeric_limits<Type>::digits10);
        if (real != _zero)
            representation << real << (imag > _zero ? "+" : "");
        if (real == _zero || imag != _zero)
            representation << imag << (real != _zero || imag != _zero ? "i" : "");
        return representation.str();
    }
};

template<typename Type> const std::string Lexer<std::complex<Type>>::left = "(";
template<typename Type> const std::string Lexer<std::complex<Type>>::right = ")";
template<typename Type> const std::string Lexer<std::complex<Type>>::decimal = ".";
template<typename Type> const std::string Lexer<std::complex<Type>>::separator = ",";

template<typename Type>
const auto Lexer<std::complex<Type>>::number_regex = detail::Regex{
    R"_(^(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?i?$)_"
};
template<typename Type>
const auto Lexer<std::complex<Type>>::name_regex = detail::Regex{
    R"_(^[A-Za-z_]+[A-Za-z_\d]*$)_"
};
template<typename Type>
const auto Lexer<std::complex<Type>>::symbol_regex = detail::Regex{
    R"_(^[^A-Za-z\d.(),_\s]+$)_"
};
template<typename Type>
const auto Lexer<std::complex<Type>>::tokenizer_regex = detail::Regex{
    R"_(((?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?i?)|)_"
    R"_(([A-Za-z_]+[A-Za-z_\d]*)|)_"
    R"_(([^A-Za-z\d\.(),_\s]+)|)_"
    R"_((\()|(\))|(,)|(\.))_"
};

}


namespace std {

template<typename Type>
struct hash<std::complex<Type>> {
    size_t operator()(const std::complex<Type>& z) const {
        size_t combined{hash<Type>{}(real(z))};
        calculate::detail::hash_combine(combined, imag(z));
        return combined;
    }
};

}

#endif
