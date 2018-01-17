#ifndef __CALCULATE_PARSER_HPP__
#define __CALCULATE_PARSER_HPP__

#include <algorithm>
#include <limits>
#include <ostream>
#include <queue>
#include <string>
#include <unordered_set>

#include "lexer.hpp"
#include "node.hpp"


namespace calculate {

template<typename BaseType>
class BaseParser {
public:
    using DefaultLexer = Lexer<BaseType>;
    using Lexer = BaseLexer<BaseType>;
    using Type = BaseType;

    using Symbol = Symbol<Node<BaseParser>>;
    using Variable = Variable<Node<BaseParser>>;
    using Constant = Constant<Node<BaseParser>>;
    using Function = Function<Node<BaseParser>>;
    using Operator = Operator<Node<BaseParser>>;

    using Expression = Node<BaseParser>;
    private: using VariableHandler = typename Expression::VariableHandler;

    public: using SymbolType = typename Symbol::SymbolType;
    public: using Associativity = typename Operator::Associativity;


private:
    std::shared_ptr<Lexer> _lexer;


public:
    util::SymbolContainer<Constant, BaseParser> constants;
    util::SymbolContainer<Function, BaseParser> functions;
    util::SymbolContainer<Operator, BaseParser> operators;

    template<typename LexerType>
    BaseParser(const LexerType& lexer) :
        _lexer{std::make_shared<LexerType>(lexer)},
        constants{_lexer.get()},
        functions{_lexer.get()},
        operators{_lexer.get()}
    {}

    BaseParser() : BaseParser{calculate::Lexer<Type>{}} {}

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
        std::stack<std::size_t> parenthesis_counter{};

        auto fill_parenthesis = [&]() {
            while (!parenthesis_counter.empty()) {
                if (parenthesis_counter.top() == 0) {
                    collected.push(Right());
                    previous = Right();
                    parenthesis_counter.pop();
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

            collected.push(std::move(current));
            if (original)
                parsed += current.token;
            previous = std::move(current);
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
                            parsed += current.token;
                            current = {
                                cop->alias(),
                                SymbolType::FUNCTION,
                                functions.at(cop->alias()).clone()
                            };
                            collect_symbol(false);
                            current = Left();
                            collect_symbol(false);
                            parenthesis_counter.push(0);
                            break;
                        default:
                            collect_symbol();
                        }
                    }
                }
            }

            if (!parenthesis_counter.empty()) {
                if (current.type == SymbolType::LEFT)
                    parenthesis_counter.top()++;
                else if (current.type == SymbolType::RIGHT)
                    parenthesis_counter.top()--;
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
            parsed += " '" + current.token + "' ";
            while (!symbols.empty()) {
                current = std::move(symbols.front());
                symbols.pop();
                parsed += current.token;
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

        std::stack<std::size_t> expected_counter{};
        std::stack<std::size_t> provided_counter{};
        std::stack<bool> apply_function{};
        bool was_function{false};

        if (symbols.size() == 0)
            throw EmptyExpression{};

        while (!symbols.empty()) {
            element = std::move(symbols.front());
            symbols.pop();

            auto& another = operations.top();
            switch (element.type) {
            case (SymbolType::LEFT):
                operations.push(std::move(element));
                apply_function.push(was_function);
                was_function = false;
                break;

            case (SymbolType::RIGHT):
                while (!operations.empty()) {
                    if (another.type != SymbolType::LEFT) {
                        operations.pop();
                        collected.push(std::move(another));
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
                    expected_counter.pop();
                    provided_counter.pop();
                }
                apply_function.pop();
                break;

            case (SymbolType::SEPARATOR):
                while (!operations.empty()) {
                    if (another.type != SymbolType::LEFT) {
                        collected.push(std::move(another));
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
                            operations.pop();
                            collected.push(std::move(another));
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


    Expression _create_node(
        const std::shared_ptr<VariableHandler>& variables,
        SymbolHandler&& symbol,
        std::vector<Expression>&& nodes,
        std::size_t hash
    ) const {
        return Expression{
            _lexer,
            variables,
            symbol.token,
            std::move(symbol.symbol),
            std::move(nodes),
            hash
        };
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
                auto& symbol = *(element.symbol);
                util::hash_combine(hash, symbol);
                operands.emplace(
                    _create_node(variables, std::move(element), {}, hash)
                );
            }

            else {
                std::vector<Expression> nodes{};
                auto& symbol = *(element.symbol);

                util::hash_combine(hash, symbol);
                nodes.reserve(symbol.arguments());

                for (std::size_t i = 0; i <  symbol.arguments(); i++) {
                    if (operands.empty())
                        throw ArgumentsMismatch{
                            element.token,
                            symbol.arguments(),
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
                    _create_node(
                        variables,
                        std::move(element),
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
    Type to_value(const std::string& expression) const {
        return Type{from_infix(expression)};
    }

    std::string to_string(Type value) const {
        return _lexer->to_string(value);
    }

    Expression create_node(Type value) const {
        return from_infix(to_string(value));
    }

    Expression create_node(
        const std::string& token,
        const std::vector<Expression>& nodes={},
        const std::vector<std::string>& variables={}
    ) const {
        std::string postfix{};

        auto context = std::make_shared<VariableHandler>(variables, *_lexer);
        for (const auto& node : nodes)
            postfix += node.postfix() + " ";
        return _build_tree(_tokenize(postfix + token, context), context);
    }

    Expression from_infix(
        const std::string& expression,
        const std::vector<std::string>& variables={}
    ) const {
        auto context = std::make_shared<VariableHandler>(variables, *_lexer);
        return _build_tree(
            _shunting_yard(_parse_infix(_tokenize(expression, context))),
            context
        );
    }

    Expression from_postfix(
        const std::string& expression,
        const std::vector<std::string>& variables={}
    ) const {
        auto context = std::make_shared<VariableHandler>(variables, *_lexer);
        return _build_tree(_tokenize(expression, context), context);
    }

    Expression parse(const std::string& expression) const {
        std::vector<std::string> variables{};

        while (true) {
            try {
                return from_infix(expression, variables);
            }
            catch (const UndefinedSymbol& exception) {
                variables.emplace_back(exception.token);
            }
            catch (const UnsuitableName& exception) {
                throw UndefinedSymbol{exception.token};
            }
        }
    }
/*
    Expression optimize(const Expression& node) const noexcept {
        if (node.variables().empty())
            return create_node(node._symbol->evaluate(node._nodes));
        auto nodes = std::vector<Expression>{};
        nodes.reserve(node._symbol->arguments());
        for (auto another : node._nodes)
            nodes.emplace_back(optimize(another));
        return create_node(node._token, std::move(nodes), node.variables());
    }
*/
};

}

#endif
