/*
    Calculate - Version 2.1.1rc1
    Last modified 2018/06/15
    Released under MIT license
    Copyright (c) 2016-2018 Alberto Lorenzo <alorenzo.md@gmail.com>
*/


#ifndef __CALCULATE_PARSER_HPP__
#define __CALCULATE_PARSER_HPP__

#include <algorithm>
#include <queue>

#include "container.hpp"
#include "lexer.hpp"
#include "node.hpp"


namespace calculate {

template<typename BaseType>
class BaseParser {
public:
    using Lexer = calculate::BaseLexer<BaseType>;
    using Type = BaseType;

    using Symbol = calculate::Symbol<Node<BaseParser>>;
    using Variable = calculate::Variable<Node<BaseParser>>;
    using Constant = calculate::Constant<Node<BaseParser>>;
    using Function = calculate::Function<Node<BaseParser>>;
    using Operator = calculate::Operator<Node<BaseParser>>;

    using Expression = calculate::Node<BaseParser>;
    private: using VariableHandler = typename Expression::VariableHandler;

    public: using SymbolType = typename Symbol::SymbolType;
    public: using Associativity = typename Operator::Associativity;


private:
    std::shared_ptr<Lexer> _lexer;


public:
    SymbolContainer<Constant, BaseParser> constants;
    SymbolContainer<Function, BaseParser> functions;
    SymbolContainer<Operator, BaseParser> operators;
    AliasContainer<BaseParser> prefixes;
    AliasContainer<BaseParser> suffixes;
    bool optimize;

    BaseParser(const Lexer& lexer) :
            _lexer{lexer.clone()},
            constants{_lexer.get()},
            functions{_lexer.get()},
            operators{_lexer.get()},
            prefixes{_lexer.get()},
            suffixes{_lexer.get()},
            optimize{false}
    {}

    ~BaseParser() = default;

    const Lexer& lexer() const noexcept { return *_lexer; }


private:
    struct SymbolHandler {
        std::string token;
        SymbolType type;
        std::unique_ptr<Symbol> symbol;
    };

    SymbolHandler _left() const noexcept {
        return {_lexer->left, SymbolType::LEFT, nullptr};
    }

    SymbolHandler _right() const noexcept {
        return {_lexer->right, SymbolType::RIGHT, nullptr};
    }

    SymbolHandler _separator() const noexcept {
        return {_lexer->separator, SymbolType::SEPARATOR, nullptr};
    }


    std::queue<SymbolHandler> _tokenize(
        const std::string& expression,
        const std::shared_ptr<VariableHandler>& variables,
        bool infix
    ) const {
        using Associativity = typename Operator::Associativity;
        using TokenType = typename Lexer::TokenType;

        std::queue<SymbolHandler> symbols{};
        decltype(constants.begin()) c;
        decltype(functions.begin()) f;
        decltype(operators.begin()) o;
        decltype(prefixes.begin()) a;

        auto tokens = infix ?
            _lexer->tokenize_infix(expression) :
            _lexer->tokenize_postfix(expression);

        auto has = [](const auto& cont, const auto& t, auto& it) noexcept {
            return (it = cont.find(t)) != cont.end();
        };

        auto push_operator = [&](const auto& token) {
            auto p = symbols.empty() ? SymbolType::LEFT : symbols.back().type;
            auto leftmost =
                p == SymbolType::LEFT ||
                p == SymbolType::SEPARATOR ||
                p == SymbolType::OPERATOR ||
                p == SymbolType::PREFIX;

            if (
                infix && leftmost &&
                has(prefixes, token, a) &&
                has(functions, a->second, f)
            )
                symbols.push({
                    std::move(a->second),
                    SymbolType::PREFIX,
                    f->second.clone()
                });
            else if (
                infix &&
                has(suffixes, token, a) &&
                has(functions, a->second, f)
            )
                symbols.push({
                    std::move(a->second),
                    SymbolType::SUFFIX,
                    f->second.clone()
                });
            else if (has(operators, token, o))
                symbols.push({
                    std::move(token),
                    SymbolType::OPERATOR,
                    o->second.clone()
                });
            else
                throw UndefinedSymbol{token};
        };

        for (auto t = tokens.begin(); t != tokens.end(); t++) {
            auto token = t->first;
            auto type = t->second;
            auto next = t + 1;

            if (type == TokenType::NUMBER) {
                auto prefix =
                    next != tokens.end() && has(operators, next->first, o) &&
                    o->second.associativity() == Associativity::RIGHT;
                auto suffix =
                    (next != tokens.end() && has(suffixes, next->first, a)) ||
                    (symbols.size() && symbols.back().type == SymbolType::SUFFIX);

                if (infix && (prefix || suffix) && _lexer->prefixed(token)) {
                    auto splitted = _lexer->split(token);
                    push_operator(splitted.first);
                    symbols.push({
                        splitted.second,
                        SymbolType::CONSTANT,
                        Constant{_lexer->to_value(splitted.second)}.clone()
                    });
                }
                else
                    symbols.push({
                        token,
                        SymbolType::CONSTANT,
                        Constant{_lexer->to_value(token)}.clone()
                    });
            }
            else if (type == TokenType::LEFT)
                symbols.push(_left());
            else if (type == TokenType::RIGHT)
                symbols.push(_right());
            else if (type == TokenType::SEPARATOR)
                symbols.push(_separator());
            else if (type == TokenType::SYMBOL)
                push_operator(token);
            else if (type == TokenType::NAME && has(constants, token, c))
                symbols.push({
                    std::move(token),
                    SymbolType::CONSTANT,
                    c->second.clone()
                });
            else if (type == TokenType::NAME && has(functions, token, f))
                symbols.push({
                    std::move(token),
                    SymbolType::FUNCTION,
                    f->second.clone()
                });
            else
                symbols.push({
                    token,
                    SymbolType::CONSTANT,
                    Variable{variables->at(token)}.clone()
                });
        }
        return symbols;
    }

    std::queue<SymbolHandler> _parse_infix(
        std::queue<SymbolHandler>&& symbols
    ) const {
        using Associativity = typename Operator::Associativity;

        std::string parsed{};
        std::queue<SymbolHandler> collected{};
        SymbolHandler previous{_left()};
        SymbolHandler current{};
        std::stack<bool> automatic{};

        auto fill_parenthesis = [&]() noexcept {
            while (!automatic.empty()) {
                if (automatic.top()) {
                    collected.push(_right());
                    previous = _right();
                    automatic.pop();
                }
                else
                    break;
            }
        };

        auto get_symbol = [&](const auto& handler) noexcept {
            if (handler.type == SymbolType::PREFIX) {
                for (const auto& prefix : prefixes)
                    if (prefix.second == handler.token)
                        return prefix.first;
            }
            else if (handler.type == SymbolType::SUFFIX) {
                for (const auto& suffix : suffixes)
                    if (suffix.second == handler.token)
                        return suffix.first;
            }
            return handler.token;
        };

        auto collect_symbol = [&](bool log=true) {
            switch (previous.type) {
            case (SymbolType::RIGHT):
            case (SymbolType::CONSTANT):
            case (SymbolType::SUFFIX):
                if (
                    current.type == SymbolType::RIGHT ||
                    current.type == SymbolType::SEPARATOR ||
                    current.type == SymbolType::OPERATOR ||
                    current.type == SymbolType::SUFFIX
                )
                    break;
                else
                    throw SyntaxError{};
            case (SymbolType::LEFT):
            case (SymbolType::SEPARATOR):
            case (SymbolType::OPERATOR):
                if (
                    current.type == SymbolType::CONSTANT ||
                    current.type == SymbolType::LEFT ||
                    current.type == SymbolType::FUNCTION ||
                    current.type == SymbolType::PREFIX
                )
                    break;
                else
                    throw SyntaxError{};
            case (SymbolType::FUNCTION):
            case (SymbolType::PREFIX):
                if (current.type == SymbolType::LEFT)
                    break;
                else
                    throw SyntaxError{};
            }

            if (
                previous.type == SymbolType::RIGHT ||
                previous.type == SymbolType::CONSTANT ||
                previous.type == SymbolType::SUFFIX
            ) {
                auto co = static_cast<Operator*>(current.symbol.get());
                auto not_right =
                    current.type != SymbolType::OPERATOR ||
                    co->associativity() != Associativity::RIGHT;
                not_right = not_right && current.type != SymbolType::SUFFIX;
                if (not_right)
                    fill_parenthesis();
            }

            if (log && current.type == SymbolType::LEFT)
                automatic.push(false);
            else if (current.type == SymbolType::RIGHT)
                if (!automatic.empty() && !automatic.top())
                    automatic.pop();

            if (log)
                parsed += get_symbol(current) + " ";
            previous = {current.token, current.type, nullptr};
            collected.push(std::move(current));
        };

        if (symbols.size() == 0)
            throw EmptyExpression{};

        try {
            while (!symbols.empty()) {
                current = std::move(symbols.front());
                symbols.pop();

                if (current.type != SymbolType::PREFIX)
                    collect_symbol();
                else {
                    if (symbols.empty())
                        throw SyntaxError{};
                    collect_symbol(true);
                    current = _left();
                    automatic.push(true);
                    collect_symbol(false);
                }
            }

            if (
                previous.type == SymbolType::RIGHT ||
                previous.type == SymbolType::CONSTANT ||
                previous.type == SymbolType::SUFFIX
            )
                fill_parenthesis();
            else
                throw SyntaxError{};
        }
        catch (const SyntaxError&) {
            parsed +="'" + get_symbol(current) + "' ";
            while (!symbols.empty()) {
                current = std::move(symbols.front());
                symbols.pop();
                parsed += get_symbol(current) + " ";
            }

            if (current.token.empty()) {
                auto n = parsed.rfind(' ', parsed.size() - 5);
                parsed =
                    (n == std::string::npos ? "" : parsed.substr(0, n) + " ") +
                    "'" + get_symbol(previous) + "'";
            }
            else
                parsed = parsed.substr(0, parsed.size() - 1);
            throw SyntaxError{parsed};
        }
        return collected;
    }

    std::queue<SymbolHandler> _shunting_yard(
        std::queue<SymbolHandler>&& symbols
    ) const {
        using Associativity = typename Operator::Associativity;

        std::queue<SymbolHandler> collected{};
        std::stack<SymbolHandler> operations{};
        SymbolHandler element{};
        SymbolHandler another{};

        std::stack<std::size_t> expected_counter{};
        std::stack<std::size_t> provided_counter{};
        std::stack<bool> apply_function{};
        bool was_function{false};

        if (symbols.size() == 0)
            throw EmptyExpression{};

        while (!symbols.empty()) {
            element = std::move(symbols.front());
            symbols.pop();

            switch (element.type) {
            case (SymbolType::LEFT):
                operations.push(std::move(element));
                apply_function.push(was_function);
                was_function = false;
                break;

            case (SymbolType::RIGHT):
                while (!operations.empty()) {
                    if (operations.top().type != SymbolType::LEFT) {
                        collected.push(std::move(operations.top()));
                        operations.pop();
                    }
                    else
                        break;
                }
                if (
                    !operations.empty() &&
                    operations.top().type == SymbolType::LEFT
                )
                    operations.pop();
                else
                    throw ParenthesisMismatch{};
                if (apply_function.top()) {
                    if (expected_counter.top() != provided_counter.top())
                        throw ArgumentsMismatch{
                            operations.top().token,
                            expected_counter.top(),
                            provided_counter.top()
                        };
                    collected.push(std::move(operations.top()));
                    operations.pop();
                    expected_counter.pop();
                    provided_counter.pop();
                }
                apply_function.pop();
                break;

            case (SymbolType::SEPARATOR):
                while (!operations.empty()) {
                    if (operations.top().type != SymbolType::LEFT) {
                        collected.push(std::move(operations.top()));
                        operations.pop();
                    }
                    else
                        break;
                }
                if (apply_function.empty() || !apply_function.top())
                    throw SyntaxError{
                        "separator '" + element.token + "' outside function"
                    };
                provided_counter.top()++;
                if (operations.empty())
                    throw ParenthesisMismatch{};
                break;

            case (SymbolType::CONSTANT):
            case (SymbolType::SUFFIX):
                collected.push(std::move(element));
                break;

            case (SymbolType::FUNCTION):
            case (SymbolType::PREFIX):
                expected_counter.push(element.symbol->arguments());
                provided_counter.push(1);
                was_function = true;
                operations.push(std::move(element));
                break;

            case (SymbolType::OPERATOR):
                while (!operations.empty()) {
                    auto& another = operations.top();
                    if (another.type == SymbolType::LEFT)
                        break;
                    else if (
                        another.type == SymbolType::FUNCTION ||
                        another.type == SymbolType::PREFIX
                    ) {
                        collected.push(std::move(another));
                        operations.pop();
                        break;
                    }
                    else {
                        auto eo = static_cast<Operator*>(element.symbol.get());
                        auto ao = static_cast<Operator*>(another.symbol.get());
                        auto l1 = eo->associativity() != Associativity::RIGHT;
                        auto p1 = eo->precedence();
                        auto p2 = ao->precedence();
                        if ((l1 && (p1 <= p2)) || (!l1 && (p1 < p2))) {
                            collected.push(std::move(another));
                            operations.pop();
                        }
                        else
                            break;
                    }
                }
                operations.push(std::move(element));
                break;
            }
        }

        while (!operations.empty()) {
            element = std::move(operations.top());
            operations.pop();
            if (element.type == SymbolType::LEFT)
                throw ParenthesisMismatch{};
            collected.push(std::move(element));
        }

        return collected;
    }

    Expression _build_tree(
        std::queue<SymbolHandler>&& symbols,
        const std::shared_ptr<VariableHandler>& variables
    ) const {
        std::stack<Expression> operands{};
        std::stack<Expression> extract{};
        SymbolHandler element{};

        while (!symbols.empty()) {
            std::vector<Expression> nodes{};
            std::unique_ptr<Symbol> symbol{};
            std::string token{};
            std::size_t hash{};
            bool collapse{optimize};

            element = std::move(symbols.front());
            symbols.pop();

            if (
                element.type == SymbolType::LEFT ||
                element.type == SymbolType::RIGHT ||
                element.type == SymbolType::SEPARATOR
            )
                throw SyntaxError{
                    "symbol '" + element.token + "' not allowed in "
                    "postfix notation"
                };

            if (element.type != SymbolType::CONSTANT) {
                nodes.reserve(element.symbol->arguments());
                for (std::size_t i = 0; i < element.symbol->arguments(); i++) {
                    if (operands.empty())
                        throw ArgumentsMismatch{
                            element.token,
                            element.symbol->arguments(),
                            i
                        };
                    collapse = collapse && operands.top()._pruned().empty();
                    util::hash_combine(hash, operands.top());
                    extract.emplace(std::move(operands.top()));
                    operands.pop();
                }
                while (!extract.empty()) {
                    nodes.emplace_back(std::move(extract.top()));
                    extract.pop();
                }
            }
            else
                collapse = false;

            util::hash_combine(hash, *(element.symbol));
            if (collapse) {
                symbol = Constant{element.symbol->eval(nodes)}.clone();
                token = _lexer->to_string(symbol->eval());
                nodes.clear();
                hash = std::size_t{};
                util::hash_combine(hash, *symbol);
            }
            else {
                symbol = std::move(element.symbol);
                token = std::move(element.token);
            }

            operands.emplace(
                Expression(
                    _lexer,
                    variables,
                    std::move(token),
                    std::move(symbol),
                    std::move(nodes),
                    hash
                )
            );
        }

        if (operands.size() > 1) {
            operands.pop();
            throw UnusedSymbol{operands.top().token()};
        }

        auto pruned = operands.top()._pruned();
        for (const auto& variable : variables->variables)
            if (
                std::find(
                    pruned.begin(),
                    pruned.end(),
                    variable
                ) == pruned.end()
            )
                throw UnusedSymbol(variable);
        return std::move(operands.top());
    }


public:
    template<typename... Args>
    Expression from_infix(const std::string& expr, Args&&... vars) const {
        auto variables = std::make_shared<VariableHandler>(
            util::to_vector<std::string>(std::forward<Args>(vars)...),
            _lexer.get()
        );
        auto symbols = _shunting_yard(_parse_infix(_tokenize(expr, variables, true)));

        return _build_tree(std::move(symbols), variables);
    }

    template<typename... Args>
    Expression from_postfix(const std::string& expr, Args&&... vars) const {
        auto variables = std::make_shared<VariableHandler>(
            util::to_vector<std::string>(std::forward<Args>(vars)...),
            _lexer.get()
        );
        auto symbols = _tokenize(expr, variables, false);

        return _build_tree(std::move(symbols), variables);
    }

    Expression parse(const std::string& expr) const {
        std::vector<std::string> vars{};

        while (true) {
            try {
                return from_infix(expr, vars);
            }
            catch (const UndefinedSymbol& exception) {
                vars.emplace_back(exception.token);
            }
            catch (const UnsuitableName& exception) {
                throw UndefinedSymbol{exception.token};
            }
        }
    }
};

}

#endif
