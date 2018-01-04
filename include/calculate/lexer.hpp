#ifndef __CALCULATE_LEXER_HPP__
#define __CALCULATE_LEXER_HPP__

#include <complex>
#include <iomanip>
#include <sstream>

#include "exception.hpp"
#include "util.hpp"


namespace calculate {

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
    static const std::string& left() {
        static std::string string{"("};
        return string;
    };
    static const std::string& right() {
        static std::string string{")"};
        return string;
    };
    static const std::string& decimal() {
        static std::string string{"."};
        return string;
    };
    static const std::string& separator() {
        static std::string string{","};
        return string;
    };

    static const util::Regex& number() {
        static util::Regex regex{
R"_(^(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?$)_"
        };
        return regex;
    };
    static const util::Regex& name() {
        static util::Regex regex{R"_(^[A-Za-z_]+[A-Za-z_\d]*$)_"};
        return regex;
    };
    static const util::Regex& symbol() {
        static util::Regex regex{R"_(^[^A-Za-z\d.(),_\s]+$)_"};
        return regex;
    };
    static const util::Regex& tokenizer() {
        static util::Regex regex{
            R"_(((?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)|)_"
            R"_(([A-Za-z_]+[A-Za-z_\d]*)|)_"
            R"_(([^A-Za-z\d\.(),_\s]+)|)_"
            R"_((\()|(\))|(,)|(\.))_"
        };
        return regex;
    };

    static Type to_value(const std::string& token) {
        if (!std::regex_search(token, number().regex))
            throw BadCast{token};
        return _cast<Type>(token);
    }

    static std::string to_string(Type value) {
        std::ostringstream string{};

        string << std::setprecision(std::numeric_limits<Type>::digits10);
        string << value;
        return string.str();
    }
};


template<typename Type>
class Lexer<std::complex<Type>> {
    static constexpr Type _zero = static_cast<Type>(0);

public:
    static const std::string& left() {
        static std::string string{"("};
        return string;
    };
    static const std::string& right() {
        static std::string string{")"};
        return string;
    };
    static const std::string& decimal() {
        static std::string string{"."};
        return string;
    };
    static const std::string& separator() {
        static std::string string{","};
        return string;
    };

    static const util::Regex& number() {
        static util::Regex regex{
            R"_(^(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?i?$)_"
        };
        return regex;
    };
    static const util::Regex& name() {
        static util::Regex regex{R"_(^[A-Za-z_]+[A-Za-z_\d]*$)_"};
        return regex;
    };
    static const util::Regex& symbol() {
        static util::Regex regex{R"_(^[^A-Za-z\d.(),_\s]+$)_"};
        return regex;
    };
    static const util::Regex& tokenizer() {
        static util::Regex regex{
            R"_(((?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?i?)|)_"
            R"_(([A-Za-z_]+[A-Za-z_\d]*)|)_"
            R"_(([^A-Za-z\d\.(),_\s]+)|)_"
            R"_((\()|(\))|(,)|(\.))_"
        };
        return regex;
    };

    static std::complex<Type> to_value(const std::string& token) {
        using namespace std::complex_literals;

        if (token.back() != 'i')
            return std::complex<Type>{Lexer<Type>::to_value(token)};
        else
            return 1i * Lexer<Type>::to_value(
                token.substr(0, token.size() - 1)
            );
    }

    static std::string to_string(std::complex<Type> value) {
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
