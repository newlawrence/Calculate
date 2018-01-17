#ifndef __CALCULATE_LEXER_HPP__
#define __CALCULATE_LEXER_HPP__

#include <complex>
#include <iomanip>
#include <sstream>

#include "util.hpp"


namespace calculate {

namespace detail {

struct StringInitializer { std::string s1, s2, s3, s4; };

inline StringInitializer DefaultPunctuation() { return {"(", ")", ",", "."}; }

inline StringInitializer DefaultRegexes() {
    return {
        R"_(^(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?$)_",
        R"_(^[A-Za-z_]+[A-Za-z_\d]*$)_",
        R"_(^[^A-Za-z\d.(),_\s]+$)_",
        R"_(((?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)|)_"
        R"_(([A-Za-z_]+[A-Za-z_\d]*)|)_"
        R"_(([^A-Za-z\d\.(),_\s]+)|)_"
        R"_((\()|(\))|(,)|(\.))_"
    };
}

inline StringInitializer DefaultComplexRegexes() {
    return {
        R"_(^(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?i?$)_",
        R"_(^[A-Za-z_]+[A-Za-z_\d]*$)_",
        R"_(^[^A-Za-z\d.(),_\s]+$)_",
        R"_(((?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?i?)|)_"
        R"_(([A-Za-z_]+[A-Za-z_\d]*)|)_"
        R"_(([^A-Za-z\d\.(),_\s]+)|)_"
        R"_((\()|(\))|(,)|(\.))_"
    };
}

}


template<typename Type>
struct BaseLexer {
    const std::string left;
    const std::string right;
    const std::string separator;
    const std::string decimal;
    const std::string number;
    const std::string name;
    const std::string symbol;
    const std::string tokenizer;

    const std::regex number_regex;
    const std::regex name_regex;
    const std::regex symbol_regex;
    const std::regex tokenizer_regex;

    BaseLexer(
        const detail::StringInitializer& strings,
        const detail::StringInitializer& regexes
    ) :
            left{strings.s1},
            right{strings.s2},
            separator{strings.s3},
            decimal{strings.s4},
            number{regexes.s1},
            name{regexes.s2},
            symbol{regexes.s3},
            tokenizer{regexes.s4},
            number_regex{regexes.s1},
            name_regex{regexes.s2},
            symbol_regex{regexes.s3},
            tokenizer_regex{regexes.s4}
    {}

    virtual ~BaseLexer() = default;

    virtual std::shared_ptr<BaseLexer> clone() const noexcept = 0;
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
        const detail::StringInitializer& strings=detail::DefaultPunctuation(),
        const detail::StringInitializer& regexes=detail::DefaultRegexes()
    ) : BaseLexer{strings, regexes} {
        if (this->decimal != ".")
            throw UnsuitableName{this->decimal};
    }

    std::shared_ptr<BaseLexer> clone() const noexcept override {
        return std::make_shared<Lexer>(*this);
    }

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
        const detail::StringInitializer& strings=detail::DefaultPunctuation(),
        const detail::StringInitializer& regexes=detail::DefaultComplexRegexes()
    ) : BaseLexer{strings, regexes} {
        if (this->decimal != ".")
            throw UnsuitableName{this->decimal};
    }

    std::shared_ptr<BaseLexer> clone() const noexcept override {
        return std::make_shared<Lexer>(*this);
    }

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
