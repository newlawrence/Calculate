#ifndef __CALCULATE_LEXER_HPP__
#define __CALCULATE_LEXER_HPP__

#include <complex>
#include <iomanip>
#include <sstream>

#include "exception.hpp"
#include "util.hpp"


namespace calculate {

template<typename Type>
struct BaseLexer {
    const std::string left;
    const std::string right;
    const std::string decimal;
    const std::string separator;
    const std::string number;
    const std::string name;
    const std::string symbol;
    const std::string tokenizer;

    const std::regex number_regex;
    const std::regex name_regex;
    const std::regex symbol_regex;
    const std::regex tokenizer_regex;

    BaseLexer(
        const std::string& s1, const std::string& s2,
        const std::string& s3, const std::string& s4,
        const std::string& r1, const std::string& r2,
        const std::string& r3, const std::string& r4
    ) :
            left{s1}, right{s2}, decimal{s3}, separator{s4},
            number{s1}, name{s2}, symbol{s3}, tokenizer{s4},
            number_regex{r1},
            name_regex{r2},
            symbol_regex{r3},
            tokenizer_regex{r4}
    {}

    virtual Type to_value(const std::string&) const = 0;
    virtual std::string to_string(Type) const noexcept = 0;
};


template<typename Type>
class Lexer final : public BaseLexer<Type> {
    using BaseLexer = BaseLexer<Type>;

    template<typename Num>
    static std::enable_if_t<std::is_integral<Num>::value, Num> _cast(
        const std::string& token
    ) { return static_cast<Num>(std::stoll(token)); }

    template<typename Num>
    static std::enable_if_t<std::is_floating_point<Num>::value, Num> _cast(
        const std::string& token
    ) { return static_cast<Num>(std::stold(token)); }

public:
    Lexer(
        const std::string& s1="(",
        const std::string& s2=")",
        const std::string& s3=".",
        const std::string& s4=",",
        const std::string& r1=R"_(^(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?$)_",
        const std::string& r2=R"_(^[A-Za-z_]+[A-Za-z_\d]*$)_",
        const std::string& r3=R"_(^[^A-Za-z\d.(),_\s]+$)_",
        const std::string& r4=
            R"_(((?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)|)_"
            R"_(([A-Za-z_]+[A-Za-z_\d]*)|)_"
            R"_(([^A-Za-z\d\.(),_\s]+)|)_"
            R"_((\()|(\))|(,)|(\.))_"
    ) : BaseLexer{s1, s2, s3, s4, r1, r2, r3, r4} {}

    Type to_value(const std::string& token) const override {
        if (!std::regex_search(token, this->number_regex))
            throw BadCast{token};
        return _cast<Type>(token);
    }

    std::string to_string(Type value) const noexcept override {
        std::ostringstream string{};

        string << std::setprecision(std::numeric_limits<Type>::digits10);
        string << value;
        return string.str();
    }
};


template<typename Type>
class Lexer<std::complex<Type>> final : public BaseLexer<std::complex<Type>> {
    using BaseLexer = BaseLexer<std::complex<Type>>;

    template<typename Num>
    static std::enable_if_t<std::is_integral<Num>::value, Num> _cast(
        const std::string& token
    ) { return static_cast<Num>(std::stoll(token)); }

    template<typename Num>
    static std::enable_if_t<std::is_floating_point<Num>::value, Num> _cast(
        const std::string& token
    ) { return static_cast<Num>(std::stold(token)); }

    static constexpr Type _zero = static_cast<Type>(0);

public:
    Lexer(
        const std::string& s1="(",
        const std::string& s2=")",
        const std::string& s3=".",
        const std::string& s4=",",
        const std::string& r1=R"_(^(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?i?$)_",
        const std::string& r2=R"_(^[A-Za-z_]+[A-Za-z_\d]*$)_",
        const std::string& r3=R"_(^[^A-Za-z\d.(),_\s]+$)_",
        const std::string& r4=
            R"_(((?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?i?)|)_"
            R"_(([A-Za-z_]+[A-Za-z_\d]*)|)_"
            R"_(([^A-Za-z\d\.(),_\s]+)|)_"
            R"_((\()|(\))|(,)|(\.))_"
    ) : BaseLexer{s1, s2, s3, s4, r1, r2, r3, r4} {}

    std::complex<Type> to_value(const std::string& token) const override {
        using namespace std::complex_literals;

        if (!std::regex_search(token, this->number_regex))
            throw BadCast{token};

        if (token.back() != 'i')
            return _cast<Type>(token);
        return 1i * _cast<Type>(token);
    }

    std::string to_string(std::complex<Type> value) const noexcept override {
        std::ostringstream string{};
        Type real{std::real(value)}, imag{std::imag(value)};

        string << std::setprecision(std::numeric_limits<Type>::digits10);
        if (real != _zero)
            string << real << (imag > _zero ? "+" : "");
        if (real == _zero || imag != _zero)
            string << imag << (real != _zero || imag != _zero ? "i" : "");
        return string.str();
    }
};

}


namespace std {

template<typename Type>
struct hash<std::complex<Type>> {
    size_t operator()(const std::complex<Type>& z) const {
        size_t combined{hash<Type>{}(real(z))};
        calculate::util::hash_combine(combined, imag(z));
        return combined;
    }
};

}

#endif
