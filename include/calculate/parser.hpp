/*
    Calculate - Version 2.0.0rc1
    Date 2018/02/11
    Released under MIT license
    Copyright (c) 2016-2018 Alberto Lorenzo <alorenzo.md@gmail.com>
*/


#ifndef __CALCULATE_PARSER_HPP__
#define __CALCULATE_PARSER_HPP__

#include <algorithm>
#include <limits>
#include <ostream>
#include <queue>
#include <string>

#include "lexer.hpp"
#include "node.hpp"


namespace calculate {

template<typename BaseType>
class BaseParser {
public:
    using DefaultLexer = calculate::Lexer<BaseType>;
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

    template<typename LexerType>
    BaseParser(const LexerType& lexer) :
        _lexer{lexer.clone()},
        constants{_lexer.get()},
        functions{_lexer.get()},
        operators{_lexer.get()}
    {}

    BaseParser() : BaseParser{DefaultLexer{}} {}

    virtual ~BaseParser() = default;

    const Lexer& lexer() const noexcept { return *_lexer; }


private:
    struct SymbolHandler {
        std::string token;
        SymbolType type;
        std::unique_ptr<Symbol> symbol;
    };

    SymbolHandler Left() const noexcept {
        return {_lexer->left, SymbolType::LEFT, nullptr};
    }

    SymbolHandler Right() const noexcept {
        return {_lexer->right, SymbolType::RIGHT, nullptr};
    }

    SymbolHandler Separator() const noexcept {
        return {_lexer->separator, SymbolType::SEPARATOR, nullptr};
    }


    std::queue<SymbolHandler> _tokenize(
        std::string expression,
        const std::shared_ptr<VariableHandler>& variables
    ) const {
        enum Group {NUMBER=1, NAME, SYMBOL, LEFT, RIGHT, SEPARATOR, DECIMAL};
        std::queue<SymbolHandler> symbols{};
        std::smatch match{};

        auto is = [&match](auto group) { return !match[group].str().empty(); };
        auto has = [&match](const auto& container, auto& founded) {
            return (founded = container.find(match.str())) != container.end();
        };

        decltype(constants.begin()) found_constant;
        decltype(functions.begin()) found_function;
        decltype(operators.begin()) found_operator;

        while (std::regex_search(expression, match, _lexer->tokenizer_regex)) {
            auto token = match.str();
            if (is(Group::DECIMAL))
                throw SyntaxError{"orphan decimal mark '" + token + "'"};
            else if (is(Group::LEFT))
                symbols.push(Left());
            else if (is(Group::RIGHT))
                symbols.push(Right());
            else if (is(Group::SEPARATOR))
                symbols.push(Separator());
            else if (is(Group::NAME) && has(constants, found_constant))
                symbols.push({
                    token,
                    SymbolType::CONSTANT,
                    found_constant->second.clone()
                });
            else if (is(Group::NAME) && has(functions, found_function))
                symbols.push({
                    token,
                    SymbolType::FUNCTION,
                    found_function->second.clone()
                });
            else if (is(Group::SYMBOL) && has(operators, found_operator))
                symbols.push({
                    token,
                    SymbolType::OPERATOR,
                    found_operator->second.clone()
                });
            else if (is(Group::NUMBER))
                symbols.push({
                    token,
                    SymbolType::CONSTANT,
                    Constant{_lexer->to_value(token)}.clone()
                });
            else {
                symbols.push({
                    token,
                    SymbolType::CONSTANT,
                    Variable{variables->at(token)}.clone()
                });
            }
            expression = match.suffix().str();
        }
        return symbols;
    }

    std::queue<SymbolHandler> _parse_infix(
        std::queue<SymbolHandler>&& symbols
    ) const {
        using Associativity = typename Operator::Associativity;

        std::string parsed{};
        std::queue<SymbolHandler> collected{};
        SymbolHandler previous{Left()};
        SymbolHandler current{};
        std::stack<bool> automatic{};

        auto fill_parenthesis = [&]() {
            while (!automatic.empty()) {
                if (automatic.top()) {
                    collected.push(Right());
                    previous = Right();
                    automatic.pop();
                }
                else
                    break;
            }
        };

        auto collect_symbol = [&](bool original=true) {
            switch (previous.type) {
            case (SymbolType::RIGHT):
            case (SymbolType::CONSTANT):
                if (
                    current.type == SymbolType::RIGHT ||
                    current.type == SymbolType::SEPARATOR ||
                    current.type == SymbolType::OPERATOR
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
                    current.type == SymbolType::FUNCTION
                )
                    break;
                else
                    throw SyntaxError{};
            case (SymbolType::FUNCTION):
                if (current.type == SymbolType::LEFT)
                    break;
                else
                    throw SyntaxError{};
            }

            if (
                previous.type == SymbolType::CONSTANT ||
                previous.type == SymbolType::RIGHT
            ) {
                auto cop = static_cast<Operator*>(current.symbol.get());
                if (
                    current.type != SymbolType::OPERATOR ||
                    cop->associativity() != Associativity::RIGHT
                )
                    fill_parenthesis();
            }

            if (original && current.type == SymbolType::LEFT)
                automatic.push(false);
            else if (current.type == SymbolType::RIGHT)
                if (!automatic.empty() && !automatic.top())
                    automatic.pop();

            if (original)
                parsed += current.token + " ";
            previous = {current.token, current.type, nullptr};
            collected.push(std::move(current));
        };

        if (symbols.size() == 0)
            throw EmptyExpression{};

        try {
            while (!symbols.empty()) {
                current = std::move(symbols.front());
                symbols.pop();

                if (current.type != SymbolType::OPERATOR)
                    collect_symbol();
                else {
                    auto cop = static_cast<Operator*>(current.symbol.get());
                    if (cop->alias().empty())
                        collect_symbol();
                    else if (symbols.empty())
                        throw SyntaxError{};
                    else {
                        switch (previous.type) {
                        case (SymbolType::LEFT):
                        case (SymbolType::SEPARATOR):
                        case (SymbolType::OPERATOR):
                            parsed += current.token + " ";
                            current = {
                                cop->alias(),
                                SymbolType::FUNCTION,
                                functions.at(cop->alias()).clone()
                            };
                            collect_symbol(false);
                            current = Left();
                            automatic.push(true);
                            collect_symbol(false);
                            break;
                        default:
                            collect_symbol();
                        }
                    }
                }
            }

            if (
                previous.type == SymbolType::CONSTANT ||
                previous.type == SymbolType::RIGHT
            )
                fill_parenthesis();
            else
                throw SyntaxError{};
        }
        catch (const SyntaxError&) {
            parsed +=" '" + current.token + "' ";
            while (!symbols.empty()) {
                current = std::move(symbols.front());
                symbols.pop();
                parsed += current.token + " ";
            }
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
                collected.push(std::move(element));
                break;

            case (SymbolType::FUNCTION):
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
                    else if (another.type == SymbolType::FUNCTION) {
                        collected.push(std::move(another));
                        operations.pop();
                        break;
                    }
                    else {
                        auto eop = static_cast<Operator*>(element.symbol.get());
                        auto aop = static_cast<Operator*>(another.symbol.get());
                        auto l1 =
                            eop->associativity() !=
                            Associativity::RIGHT;
                        auto p1 = eop->precedence();
                        auto p2 = aop->precedence();
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
        std::size_t hash{};

        while (!symbols.empty()) {
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

            else if (element.type == SymbolType::CONSTANT) {
                util::hash_combine(hash, *(element.symbol));
                operands.emplace(
                    Expression(
                        _lexer,
                        variables,
                        element.token,
                        std::move(element.symbol),
                        {},
                        hash
                    )
                );
            }

            else {
                std::vector<Expression> nodes{};
                util::hash_combine(hash, *(element.symbol));
                nodes.reserve(element.symbol->arguments());

                for (std::size_t i = 0; i < element.symbol->arguments(); i++) {
                    if (operands.empty())
                        throw ArgumentsMismatch{
                            element.token,
                            element.symbol->arguments(),
                            i
                        };
                    extract.emplace(std::move(operands.top()));
                    operands.pop();
                }
                while (!extract.empty()) {
                    nodes.emplace_back(std::move(extract.top()));
                    extract.pop();
                }
                operands.emplace(
                    Expression(
                        _lexer,
                        variables,
                        element.token,
                        std::move(element.symbol),
                        std::move(nodes),
                        hash
                    )
                );
            }
        }

        if (operands.size() > 1) {
            operands.pop();
            throw UnusedSymbol{operands.top().token()};
        }

        auto pruned = operands.top().variables();
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
    Type to_value(const std::string& expression) const {
        return Type{from_infix(expression)};
    }

    std::string to_string(Type value) const {
        return _lexer->to_string(value);
    }

    template<typename... Args>
    Expression from_infix(const std::string& expr, Args&&... vars) const {
        auto variables = std::make_shared<VariableHandler>(
            util::to_vector<std::string>(std::forward<Args>(vars)...),
            *_lexer
        );
        auto postfix = _shunting_yard(_parse_infix(_tokenize(expr, variables)));

        return _build_tree(std::move(postfix), variables);
    }

    template<typename... Args>
    Expression from_postfix(const std::string& expr, Args&&... vars) const {
        auto variables = std::make_shared<VariableHandler>(
            util::to_vector<std::string>(std::forward<Args>(vars)...),
            *_lexer
        );

        return _build_tree(_tokenize(expr, variables), variables);
    }

    Expression parse(const std::string& expression) const {
        std::vector<std::string> vars{};

        while (true) {
            try {
                return from_infix(expression, vars);
            }
            catch (const UndefinedSymbol& exception) {
                vars.emplace_back(exception.token);
            }
            catch (const UnsuitableName& exception) {
                throw UndefinedSymbol{exception.token};
            }
        }
    }

    Expression optimize(const Expression& node) const noexcept {
        auto vars = node._pruned();
        if (vars.empty())
            return from_infix(to_string(Type{from_postfix(node.postfix())}));

        auto postfix = std::string{};
        auto variables =
            std::make_shared<VariableHandler>(std::move(vars), *_lexer);
        for (const auto& branch : node._nodes)
            postfix += optimize(branch).postfix() + " ";
        postfix += node.token();

        return _build_tree(_tokenize(postfix, variables), variables);
    }
};

}

#endif
