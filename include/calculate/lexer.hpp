/*
    Calculate - Version 2.1.0dev0
    Last modified 2018/03/11
    Released under MIT license
    Copyright (c) 2016-2018 Alberto Lorenzo <alorenzo.md@gmail.com>
*/


#ifndef __CALCULATE_LEXER_HPP__
#define __CALCULATE_LEXER_HPP__

#include <complex>
#include <iomanip>
#include <locale>
#include <sstream>
#include <type_traits>

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


const char scape[] = {'\\', '.', '^', '$', '*', '+', '?', '(', ')', '[', '{'};

const char isp[] =
    R"(^(?:(?:([+\-]?\d+)([+\-]?\d+)i)|([+\-]?\d+)|(?:([+\-]?\d+)i?))$)";

const char rsp[] =
    R"(^(?:)"
    R"((?:([+\-]?(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?))"
    R"(([+\-](?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)i)|)"
    R"(([+\-]?(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)|)"
    R"((?:([+\-]?(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)i?))"
    R"()$)";

}

const char default_integer[] = R"(^[+\-]?\d+$)";

const char default_real[] =
    R"(^[+\-]?(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?$)";

const char default_integer_complex[]=
    R"(^(?:(?:(?:[+\-]?\d+)(?:[+\-]?\d+)i)|(?:(?:[+\-]?\d+)i?))$)";

const char default_real_complex[] =
    R"(^(?:)"
    R"((?:(?:[+\-]?(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?))"
    R"((?:[+\-](?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)i)|)"
    R"((?:(?:[+\-]?(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)i?))"
    R"()$)";

const char default_name[] = R"(^[A-Za-z_]+[A-Za-z_\d]*$)";

const char default_symbol[] = R"(^[^A-Za-z\d.(),_\s]+$)";


const char default_left[] = "(";

const char default_right[] = ")";

const char default_separator[] = ",";

const char default_decimal[] = ".";


template<typename Type>
class BaseLexer {
public:
    enum class TokenType : int {
        NUMBER=1,
        NAME,
        SYMBOL,
        LEFT,
        RIGHT,
        SEPARATOR,
        DECIMAL
    };
    using TokenHandler = std::pair<std::string, TokenType>;
    using PrefixHandler = std::pair<std::string, std::string>;

    const std::string left;
    const std::string right;
    const std::string separator;
    const std::string decimal;

    const std::string number;
    const std::string name;
    const std::string symbol;

    const std::regex number_regex;
    const std::regex name_regex;
    const std::regex symbol_regex;

private:
    const std::regex _splitter_regex;
    const std::regex _tokenizer_regex;

    bool _match(const std::smatch& match, TokenType type) const noexcept {
        return !match[static_cast<int>(type)].str().empty();
    }

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
            for (const auto& character : detail::scape) {
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

    std::vector<TokenHandler> _tokenize(std::string string, bool infix) const {
        std::vector<TokenHandler> tokens{};
        std::smatch match{};
        TokenType last{TokenType::LEFT};

        while (std::regex_search(string, match, _tokenizer_regex)) {
            auto token = match.str();

            if (_match(match, TokenType::NUMBER)) {
                std::sregex_token_iterator
                    nums{token.begin(), token.end(), _splitter_regex, -1},
                    syms{token.begin(), token.end(), _splitter_regex},
                    end{};

                if (nums->str().empty()) {
                    auto sym = (syms++)->str(), num = ((++nums)++)->str();
                    if (
                        infix &&
                        last != TokenType::SYMBOL &&
                        last != TokenType::LEFT &&
                        last != TokenType::SEPARATOR
                    ) {
                        tokens.emplace_back(std::move(sym), TokenType::SYMBOL);
                        tokens.emplace_back(std::move(num), TokenType::NUMBER);
                    }
                    else
                        tokens.emplace_back(sym + num, TokenType::NUMBER);
                }
                else
                    tokens.emplace_back((nums++)->str(), TokenType::NUMBER);

                while (nums != end && syms != end) {
                    tokens.emplace_back((syms++)->str(), TokenType::SYMBOL);
                    tokens.emplace_back((nums++)->str(), TokenType::NUMBER);
                }
            }
            else if (_match(match, TokenType::NAME))
                tokens.emplace_back(std::move(token), TokenType::NAME);
            else if (_match(match, TokenType::SYMBOL))
                tokens.emplace_back(std::move(token), TokenType::SYMBOL);
            else if (_match(match, TokenType::LEFT))
                tokens.emplace_back(std::move(token), TokenType::LEFT);
            else if (_match(match, TokenType::RIGHT))
                tokens.emplace_back(std::move(token), TokenType::RIGHT);
            else if (_match(match, TokenType::SEPARATOR))
                tokens.emplace_back(std::move(token), TokenType::SEPARATOR);
            else
                throw SyntaxError{"orphan decimal mark '" + token + "'"};

            string = match.suffix().str();
            last = tokens.back().second;
        }
        return tokens;
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
            number_regex{number},
            name_regex{name},
            symbol_regex{symbol},
            _splitter_regex{symbol.substr(1, symbol.size() - 2)},
            _tokenizer_regex{_generate_tokenizer()}
    {
        std::smatch match{};

        if (
            left == right ||
            left == separator ||
            left == decimal ||
            right == separator ||
            right == decimal ||
            separator == decimal
        )
            throw LexerError{"tokens must be different"};

        if (std::regex_match(" ", _tokenizer_regex))
            throw LexerError{"tokenizer matching space"};

        std::regex_search(left, match, _tokenizer_regex);
        if (!_match(match, TokenType::LEFT))
            throw LexerError{"tokenizer doesn't match left symbol"};
        std::regex_search(right, match, _tokenizer_regex);
        if (!_match(match, TokenType::RIGHT))
            throw LexerError{"tokenizer doesn't match right symbol"};
        std::regex_search(separator, match, _tokenizer_regex);
        if (!_match(match, TokenType::SEPARATOR))
            throw LexerError{"tokenizer doesn't match separator symbol"};
        std::regex_search(decimal, match, _tokenizer_regex);
        if (!_match(match, TokenType::DECIMAL))
            throw LexerError{"tokenizer doesn't match decimal symbol"};
    }
    BaseLexer(const BaseLexer&) = default;
    virtual ~BaseLexer() = default;

    BaseLexer& operator=(const BaseLexer&) = delete;
    BaseLexer& operator=(BaseLexer&&) = delete;

    std::vector<TokenHandler> tokenize_infix(std::string string) const {
        return _tokenize(std::move(string), true);
    }

    std::vector<TokenHandler> tokenize_postfix(std::string string) const {
        return _tokenize(std::move(string), false);
    }

    bool prefixed(const std::string& token) const noexcept {
        std::sregex_token_iterator
            num{token.begin(), token.end(), _splitter_regex, -1};

        return num->str().empty();
    };

    PrefixHandler split(const std::string& token) const noexcept {
        std::sregex_token_iterator
            num{token.begin(), token.end(), _splitter_regex, -1},
            sym{token.begin(), token.end(), _splitter_regex},
            end{};

        if (sym == end || num == end || !num->str().empty() || ++num == end)
            return {"", ""};
        return {*sym, *num};
    }

    virtual std::shared_ptr<BaseLexer> clone() const noexcept = 0;
    virtual Type to_value(const std::string&) const = 0;
    virtual std::string to_string(Type) const noexcept = 0;
};


template<typename Type>
class Lexer final : public BaseLexer<Type> {
    using BaseLexer = calculate::BaseLexer<Type>;

    mutable std::istringstream _istream;
    mutable std::ostringstream _ostream;

public:
    Lexer(
        const detail::RegexesInitializer& regexes={
            std::is_integral<Type>::value ? default_integer : default_real,
            default_name,
            default_symbol
        },
        const detail::StringsInitializer& strings={
            default_left,
            default_right,
            default_separator,
            default_decimal
        }
    ) : BaseLexer{regexes, strings}, _istream{}, _ostream{} {
        if (this->decimal != default_decimal)
            throw LexerError{
                "default lexer must use '" +
                std::string{default_decimal} +
                "' as decimal mark"
            };

        _istream.imbue(std::locale("C"));
        _ostream.imbue(std::locale("C"));
        _ostream << std::setprecision(std::numeric_limits<Type>::max_digits10);
    }

    Lexer(const Lexer& other) : BaseLexer(other), _istream{}, _ostream{} {
        _istream.imbue(std::locale("C"));
        _ostream.imbue(std::locale("C"));
        _ostream << std::setprecision(std::numeric_limits<Type>::max_digits10);
    }

    std::shared_ptr<BaseLexer> clone() const noexcept override {
        return std::make_shared<Lexer>(*this);
    }

    Type to_value(const std::string& token) const override {
        Type value;

        if (!std::regex_search(token, this->number_regex))
            throw BadCast{token};

        _istream.str(token);
        _istream.clear();

        _istream >> value;
        return value;
    }

    std::string to_string(Type value) const noexcept override {
        _ostream.clear();
        _ostream.str("");

        _ostream << value;
        return _ostream.str();
    }
};

template<typename Type>
class Lexer<std::complex<Type>> final : public BaseLexer<std::complex<Type>> {
    using BaseLexer = calculate::BaseLexer<std::complex<Type>>;

    static constexpr Type _zero = static_cast<Type>(0);

    mutable std::istringstream _istream;
    mutable std::ostringstream _ostream;

    const std::regex _splitter;

public:
    Lexer(
        const detail::RegexesInitializer& regexes={
            std::is_integral<Type>::value ?
                default_integer_complex :
                default_real_complex,
            default_name,
            default_symbol
        },
        const detail::StringsInitializer& strings={
            default_left,
            default_right,
            default_separator,
            default_decimal
        }
    ) :
            BaseLexer{regexes, strings},
            _istream{},
            _ostream{},
            _splitter{std::is_integral<Type>::value ? detail::isp : detail::rsp}
    {
        if (this->decimal != default_decimal)
            throw LexerError{
                "default lexer must use '" +
                std::string{default_decimal} +
                "' as decimal mark"
            };

        _istream.imbue(std::locale("C"));
        _ostream.imbue(std::locale("C"));
        _ostream << std::setprecision(std::numeric_limits<Type>::max_digits10);
    }

    Lexer(const Lexer& other) :
            BaseLexer(other),
            _istream{},
            _ostream{},
            _splitter{std::is_integral<Type>::value ? detail::isp : detail::rsp}
    {
        _istream.imbue(std::locale("C"));
        _ostream.imbue(std::locale("C"));
        _ostream << std::setprecision(std::numeric_limits<Type>::max_digits10);
    }

    std::shared_ptr<BaseLexer> clone() const noexcept override {
        return std::make_shared<Lexer>(*this);
    }

    std::complex<Type> to_value(const std::string& token) const override {
        std::smatch match{};
        Type real{}, imag{};

        if (!std::regex_search(token, this->number_regex))
            throw BadCast{token};

        auto& istream = _istream;
        auto write = [&real, &imag, &match, &istream](std::size_t i) noexcept {
            istream.str(match[i].str());
            istream.clear();
            if (i % 2)
                istream >> real;
            else
                istream >> imag;
        };

        std::regex_search(token, match, _splitter);
        if (!match[3].str().empty())
            write(3);
        else if (!match[4].str().empty())
            write(4);
        else {
            write(1);
            write(2);
        }
        return {real, imag};
    }

    std::string to_string(std::complex<Type> value) const noexcept override {
        Type real{std::real(value)}, imag{std::imag(value)};

        _ostream.clear();
        _ostream.str("");

        if (real != _zero)
            _ostream << real << (imag > _zero ? "+" : "");
        if (real == _zero || imag != _zero)
            _ostream << imag << (real != _zero || imag != _zero ? "i" : "");
        return _ostream.str();
    }
};


template<typename Type>
inline std::shared_ptr<BaseLexer<Type>> make_lexer() noexcept {
    return std::make_shared<Lexer<Type>>();
}

template<typename Type>
inline std::shared_ptr<BaseLexer<Type>> make_lexer(
    const detail::RegexesInitializer& regexes
) noexcept {
    return std::make_shared<Lexer<Type>>(regexes);
}

template<typename Type>
inline std::shared_ptr<BaseLexer<Type>> make_lexer(
    const detail::RegexesInitializer& regexes,
    const detail::StringsInitializer& strings
) noexcept {
    return std::make_shared<Lexer<Type>>(regexes, strings);
}

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
