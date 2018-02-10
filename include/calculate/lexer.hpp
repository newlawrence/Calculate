#ifndef __CALCULATE_LEXER_HPP__
#define __CALCULATE_LEXER_HPP__

#include <complex>
#include <iomanip>
#include <locale>
#include <sstream>
#include <unordered_set>

#include "util.hpp"


namespace calculate {

namespace detail {

struct RegexesInitializer {
    std::string number;
    std::string name;
    std::string symbol;
};

struct StringsInitializer {
    std::string left;
    std::string right;
    std::string separator;
    std::string decimal;
};


inline std::unordered_set<char> regex_scaped() {
    static std::unordered_set<char> chars =
        {'\\', '.', '^', '$', '*', '+', '?', '(', ')', '[', '{'};
    return chars;
}

}

inline std::string default_number() {
    return R"_(^(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?$)_";
}

inline std::string default_complex() {
    return R"_(^(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?i?$)_";
}

inline std::string default_name() { return R"_(^[A-Za-z_]+[A-Za-z_\d]*$)_"; }

inline std::string default_symbol() { return R"_(^[^A-Za-z\d.(),_\s]+$)_"; }


inline std::string default_left() { return "("; }

inline std::string default_right() { return ")"; }

inline std::string default_separator() { return ","; }

inline std::string default_decimal() { return "."; }


template<typename Type>
class BaseLexer {
public:
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

private:
    std::string _adapt_regex(std::string regex) const {
        if (regex.front() != '^')
            regex.insert(0, 1, '^');
        if (regex.back() != '$')
            regex.append(1, '$');

        try{
            std::regex{regex};
        }
        catch(const std::regex_error&) {
            throw LexerError{"bad regex '" + regex + "'"};
        }
        return regex;
    }

    std::string _generate_tokenizer() const {
        std::string tokenizer{};

        auto escape = [](std::string token) {
            for (const auto& character : detail::regex_scaped()) {
                size_t index = 0;
                while (true) {
                    index = token.find(character, index);
                    if (index == std::string::npos)
                        break;
                    token.insert(index, 1, '\\');
                    index += 2;
                }
            }
            return token;
        };

        tokenizer += "(" + number.substr(1, number.size() - 2) + ")|";
        tokenizer += "(" + name.substr(1, name.size() - 2) + ")|";
        tokenizer += "(" + symbol.substr(1, symbol.size() - 2) + ")|";
        tokenizer += "(" + escape(left) + ")|";
        tokenizer += "(" + escape(right) + ")|";
        tokenizer += "(" + escape(separator) + ")|";
        tokenizer += "(" + escape(decimal) + ")";
        return tokenizer;
    }

public:
    BaseLexer(
        const detail::RegexesInitializer& regexes,
        const detail::StringsInitializer& strings
    ) :
            left{strings.left},
            right{strings.right},
            separator{strings.separator},
            decimal{strings.decimal},
            number{_adapt_regex(regexes.number)},
            name{_adapt_regex(regexes.name)},
            symbol{_adapt_regex(regexes.symbol)},
            tokenizer{_generate_tokenizer()},
            number_regex{number},
            name_regex{name},
            symbol_regex{symbol},
            tokenizer_regex{tokenizer}
    {
        enum Group {NUMBER=1, NAME, SYMBOL, LEFT, RIGHT, SEPARATOR, DECIMAL};
        std::smatch match{};

        if (left == right ||
            left == separator ||
            left == decimal ||
            right == separator ||
            right == decimal ||
            separator == decimal
        )
            throw LexerError{"tokens must be different"};

        std::regex_search(left, match, tokenizer_regex);
        if (match[Group::LEFT].str().empty())
            throw LexerError{"tokenizer doesn't match left symbol"};
        std::regex_search(right, match, tokenizer_regex);
        if (match[Group::RIGHT].str().empty())
            throw LexerError{"tokenizer doesn't match right symbol"};
        std::regex_search(separator, match, tokenizer_regex);
        if (match[Group::SEPARATOR].str().empty())
            throw LexerError{"tokenizer doesn't match separator symbol"};
        std::regex_search(decimal, match, tokenizer_regex);
        if (match[Group::DECIMAL].str().empty())
            throw LexerError{"tokenizer doesn't match decimal symbol"};
    }
    BaseLexer(const BaseLexer&) = default;
    virtual ~BaseLexer() = default;

    BaseLexer& operator=(const BaseLexer&) = delete;
    BaseLexer& operator=(BaseLexer&&) = delete;

    virtual std::shared_ptr<BaseLexer> clone() const noexcept = 0;
    virtual Type to_value(const std::string&) const = 0;
    virtual std::string to_string(Type) const noexcept = 0;
};


template<typename Type>
class Lexer final : public BaseLexer<Type> {
    using BaseLexer = calculate::BaseLexer<Type>;

public:
    Lexer(
        const detail::RegexesInitializer& regexes={
            default_number(),
            default_name(),
            default_symbol()
        },
        const detail::StringsInitializer& strings={
            default_left(),
            default_right(),
            default_separator(),
            default_decimal()
        }
    ) : BaseLexer{regexes, strings} {}

    std::shared_ptr<BaseLexer> clone() const noexcept override {
        return std::make_shared<Lexer>(*this);
    }

    Type to_value(const std::string& token) const override {
        std::istringstream converter{token};
        Type value;

        if (!std::regex_search(token, this->number_regex))
            throw BadCast{token};
        converter.imbue(std::locale("C"));
        converter >> value;
        return value;
    }

    std::string to_string(Type value) const noexcept override {
        std::ostringstream string{};

        string << std::setprecision(std::numeric_limits<Type>::max_digits10);
        string << value;
        return string.str();
    }
};

template<typename Type>
class Lexer<std::complex<Type>> final : public BaseLexer<std::complex<Type>> {
    using BaseLexer = calculate::BaseLexer<std::complex<Type>>;

    static constexpr Type _zero = static_cast<Type>(0);

public:
    Lexer(
        const detail::RegexesInitializer& regexes={
            default_complex(),
            default_name(),
            default_symbol()
        },
        const detail::StringsInitializer& strings={
            default_left(),
            default_right(),
            default_separator(),
            default_decimal()
        }
    ) : BaseLexer{regexes, strings} {}

    std::shared_ptr<BaseLexer> clone() const noexcept override {
        return std::make_shared<Lexer>(*this);
    }

    std::complex<Type> to_value(const std::string& token) const override {
        using namespace std::complex_literals;

        std::istringstream converter{token};
        Type value;

        if (!std::regex_search(token, this->number_regex))
            throw BadCast{token};
        converter.imbue(std::locale("C"));
        converter >> value;

        if (token.back() != 'i')
            return value;
        return 1i * value;
    }

    std::string to_string(std::complex<Type> value) const noexcept override {
        std::ostringstream string{};
        Type real{std::real(value)}, imag{std::imag(value)};

        string << std::setprecision(std::numeric_limits<Type>::max_digits10);
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
