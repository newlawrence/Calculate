/*
    Calculate - Version 2.1.1dev0
    Last modified 2018/06/03
    Released under MIT license
    Copyright (c) 2016-2018 Alberto Lorenzo <alorenzo.md@gmail.com>
*/


#ifndef __CALCULATE_LEXER_HPP__
#define __CALCULATE_LEXER_HPP__

#include <cmath>
#include <complex>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <type_traits>

#include "util.hpp"


namespace calculate {

namespace detail {

constexpr const char scape[] =
    {'\\', '.', '^', '$', '*', '+', '?', '(', ')', '[', '{'};

constexpr const char split[] =
    R"(^(?:(?:(.*[^ij])([+\-].+)[ij])|(.*[^ij])|(.+)[ij])$)";


template<typename Type>
Type read(std::istringstream& istream, const std::string& token) {
    if (token == "NaN" || token == "+NaN" || token == "-NaN")
        return std::numeric_limits<Type>::quiet_NaN();
    if (token == "Inf" || token == "+Inf")
        return std::numeric_limits<Type>::infinity();
    if (token == "-Inf")
        return -std::numeric_limits<Type>::infinity();

    Type value;
    istream.str(token);
    istream.clear();
    istream >> value;
    return value;
}

template<typename Type>
std::string write(std::ostringstream& ostream, Type value) {
    if (std::isnan(value))
        return "NaN";
    if (std::isinf(value))
        return std::signbit(value) ? "-Inf" : "+Inf";

    ostream.clear();
    ostream.str("");
    ostream << value;
    return ostream.str();
}

}


constexpr const char default_left[] = "(";
constexpr const char default_right[] = ")";
constexpr const char default_separator[] = ",";
constexpr const char default_decimal[] = ".";

template<bool>
constexpr const char* default_real =
    R"(^[+\-]?\d+$)";

template<>
constexpr const char* default_real<false> =
    R"(^[+\-]?(?:(?:NaN|Inf)|(?:(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?))$)";

template<bool>
constexpr const char* default_complex =
    R"(^(?:(?:(?:[+\-]?\d+)(?:[+\-]?\d+)[ij])|(?:(?:[+\-]?\d+)[ij]?))$)";

template<>
constexpr const char* default_complex<false> =
    R"(^(?:)"
    R"((?:(?:[+\-]?(?:(?:NaN|Inf)|(?:(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?))))"
    R"((?:[+\-](?:(?:NaN|Inf)|(?:(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)))[ij])|)"
    R"((?:(?:[+\-]?(?:(?:NaN|Inf)|(?:(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)))[ij]?))"
    R"()$)";

template<typename Type>
constexpr const char* default_number =
    default_real<util::is_integral<Type>>;

template<typename Type>
constexpr const char* default_number<std::complex<Type>> = 
    default_complex<util::is_integral<Type>>;

constexpr const char default_name[] = R"(^[A-Za-z_]+[A-Za-z_\d]*$)";

constexpr const char default_symbol[] = R"(^[^A-Za-z\d.(),_\s]+$)";


template<typename Type>
class BaseLexer {
public:
    enum class TokenType {
        NUMBER,
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
        return !match[static_cast<int>(type) + 1].str().empty();
    }

    std::string _adapt_regex(std::string&& regex) const {
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
        return std::move(regex);
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
                    auto sym = (syms++)->str(), num = (nums++)->str();
                    auto back = tokens.back().first;
                    if (std::regex_search(back, number_regex)) {
                        tokens.emplace_back(sym, TokenType::SYMBOL);
                        tokens.emplace_back(num, TokenType::NUMBER);
                    }
                    else
                        tokens.back().first = back + sym + num;
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
            else if (_match(match, TokenType::DECIMAL))
                throw SyntaxError{"orphan decimal mark '" + token + "'"};
            else
                throw SyntaxError{"not matching token '" + token + "'"};

            string = match.suffix().str();
            last = tokens.back().second;
        }
        return tokens;
    }


public:
    BaseLexer(
        std::string lft, std::string rgt, std::string sep, std::string dec,
        std::string num, std::string nam, std::string sym        
    ) :
            left{std::move(lft)},
            right{std::move(rgt)},
            separator{std::move(sep)},
            decimal{std::move(dec)},
            number{_adapt_regex(std::move(num))},
            name{_adapt_regex(std::move(nam))},
            symbol{_adapt_regex(std::move(sym))},
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
            num{token.begin(), token.end(), _splitter_regex, -1},
            sym{token.begin(), token.end(), _splitter_regex};

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
        std::string lft, std::string rgt, std::string sep,
        std::string num, std::string nam, std::string sym
    ) : 
            BaseLexer{
                std::move(lft), std::move(rgt), std::move(sep), ".",
                std::move(num), std::move(nam), std::move(sym)
            },
            _istream{},
            _ostream{}
    {
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
        if (!std::regex_search(token, this->number_regex))
            throw BadCast{token};
        return detail::read<Type>(_istream, token);
    }

    std::string to_string(Type value) const noexcept override {
        return detail::write<Type>(_ostream, value);
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
        std::string lft, std::string rgt, std::string sep,
        std::string num, std::string nam, std::string sym
    ) :
            BaseLexer{
                std::move(lft), std::move(rgt), std::move(sep), ".",
                std::move(num), std::move(nam), std::move(sym)
            },
            _istream{},
            _ostream{},
            _splitter{detail::split}
    {
        _istream.imbue(std::locale("C"));
        _ostream.imbue(std::locale("C"));
        _ostream << std::setprecision(std::numeric_limits<Type>::max_digits10);
    }

    Lexer(const Lexer& other) :
            BaseLexer(other),
            _istream{},
            _ostream{},
            _splitter{detail::split}
    {
        _istream.imbue(std::locale("C"));
        _ostream.imbue(std::locale("C"));
        _ostream << std::setprecision(std::numeric_limits<Type>::max_digits10);
    }

    std::shared_ptr<BaseLexer> clone() const noexcept override {
        return std::make_shared<Lexer>(*this);
    }

    std::complex<Type> to_value(const std::string& token) const override {
        if (!std::regex_search(token, this->number_regex))
            throw BadCast{token};

        std::smatch match{};
        std::regex_search(token, match, _splitter);
        if (!match[3].str().empty())
            return {detail::read<Type>(_istream, match[3].str()), Type{}};
        if (!match[4].str().empty())
            return {Type{}, detail::read<Type>(_istream, match[4].str())};
        return {
            detail::read<Type>(_istream, match[1].str()),
            detail::read<Type>(_istream, match[2].str())
        };
    }

    std::string to_string(std::complex<Type> value) const noexcept override {
        Type real{std::real(value)}, imag{std::imag(value)};
        std::string token;

        if (real != _zero)
            token +=
                detail::write<Type>(_ostream, real) +
                (imag > _zero && std::isfinite(imag) ? "+" : "");
        if (real == _zero || imag != _zero)
            token +=
                detail::write<Type>(_ostream, imag) +
                (real != _zero || imag != _zero ? "j" : "");
        return token;
    }
};


template<typename Type>
Lexer<Type> make_lexer() noexcept {
    return Lexer<Type>{
        default_left,
        default_right,
        default_separator,
        default_number<Type>,
        default_name,
        default_symbol        
    };
}

template<typename Type>
Lexer<Type> make_from_punctuation(std::string lft, std::string rgt, std::string sep) {
    return Lexer<Type>{
        std::move(lft),
        std::move(rgt),
        std::move(sep),
        default_number<Type>,
        default_name,
        default_symbol
    };
}

template<typename Type>
Lexer<Type> make_from_lexicon(std::string num, std::string nam, std::string sym) {
    return Lexer<Type>{
        default_left,
        default_right,
        default_separator,
        std::move(num),
        std::move(nam),
        std::move(sym)
    };
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
