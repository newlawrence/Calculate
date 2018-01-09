#ifndef __CALCULATE_PARSER_HPP__
#define __CALCULATE_PARSER_HPP__

#include <algorithm>
#include <limits>
#include <ostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "lexer.hpp"
#include "node.hpp"


namespace calculate {

template<typename BaseType, template<typename> class DefaultLexer = Lexer>
class BaseParser {
public:
    using Base = BaseParser;
    using Type = BaseType;
    using Lexer = BaseLexer<Type>;

    using Symbol = Symbol<Node<BaseParser>>;
    using Constant = Constant<Node<BaseParser>>;
    using Variable = Variable<Node<BaseParser>>;
    using Function = Function<Node<BaseParser>>;
    using Operator = Operator<Node<BaseParser>>;

    using Expression = Node<BaseParser>;
    using Variables = typename Expression::Variables;


/*protected:
    std::unordered_map<std::string, Constant> _constants;
    std::unordered_map<std::string, Function> _functions;
    std::unordered_map<std::string, Operator> _operators;

    static void _validate(Constant*, const std::string& token) {
        if (!std::regex_match(token, Lexer::name().regex))
            throw UnsuitableName{token};
    }

    static void _validate(Function*, const std::string& token) {
        if (!std::regex_match(token, Lexer::name().regex))
            throw UnsuitableName{token};
    }

    static void _validate(Operator*, const std::string& token) {
        if (!std::regex_match(token, Lexer::symbol().regex))
            throw UnsuitableName{token};
    }

    std::unordered_map<std::string, Constant>& _get(Constant*) noexcept {
        return _constants;
    }

    std::unordered_map<std::string, Function>& _get(Function*) noexcept {
        return _functions;
    }

    std::unordered_map<std::string, Operator>& _get(Operator*) noexcept {
        return _operators;
    }

    template<typename Kind>
    std::unordered_map<std::string, Kind>& _factory() noexcept {
        return _get(static_cast<Kind*>(nullptr));
    }

    template<typename Kind>
    std::unordered_map<std::string, Kind>& _factory() const noexcept {
        return const_cast<BaseParser*>(this)->_get(static_cast<Kind*>(nullptr));
    }


    std::queue<std::pair<std::string, SymbolType>> _tokenize(
        std::string expression,
        const std::shared_ptr<Variables>& variables
    ) const {
        enum Group {NUMBER=1, NAME, SYMBOL, LEFT, RIGHT, SEPARATOR, DECIMAL};
        std::queue<std::pair<std::string, SymbolType>> tokens{};
        std::smatch match{};

        auto is = [&match](auto group) { return !match[group].str().empty(); };
        while (std::regex_search(expression, match, Lexer::tokenizer().regex)) {
            auto token = match.str();
            if (is(Group::DECIMAL))
                throw SyntaxError{"orphan decimal mark '" + token + "'"};
            else if (is(Group::LEFT))
                tokens.push({token, SymbolType::LEFT});
            else if (is(Group::RIGHT))
                tokens.push({token, SymbolType::RIGHT});
            else if (is(Group::SEPARATOR))
                tokens.push({token, SymbolType::SEPARATOR});
            else if (is(Group::NAME) && has<Constant>(match.str()))
                tokens.push({token, SymbolType::CONSTANT});
            else if (is(Group::NAME) && has<Function>(match.str()))
                tokens.push({token, SymbolType::FUNCTION});
            else if (is(Group::SYMBOL) && has<Operator>(match.str()))
                tokens.push({token, SymbolType::OPERATOR});
            else if (is(Group::NUMBER))
                tokens.push({token, SymbolType::CONSTANT});
            else {
                variables->at(token);
                tokens.push({token, SymbolType::CONSTANT});
            }
            expression = match.suffix().str();
        }
        return tokens;
    }

    std::queue<std::pair<std::string, SymbolType>> _parse_infix(
        std::queue<std::pair<std::string, SymbolType>>&& tokens
    ) const {
        using Associativity = typename Operator::Associativity;

        const std::pair<std::string, SymbolType> Left{
            Lexer::left(),
            SymbolType::LEFT
        };
        const std::pair<std::string, SymbolType> Right{
            Lexer::right(),
            SymbolType::RIGHT
        };

        std::string parsed{};
        std::queue<std::pair<std::string, SymbolType>> collected{};
        std::pair<std::string, SymbolType> previous{Left};
        std::pair<std::string, SymbolType> current{};
        std::stack<std::size_t> parenthesis_counter{};

        auto fill_parenthesis = [&]() {
            while (!parenthesis_counter.empty()) {
                if (parenthesis_counter.top() == 0) {
                    collected.push(Right);
                    previous = Right;
                    parenthesis_counter.pop();
                }
                else
                    break;
            }
        };

        auto collect_symbol = [&](bool original=true) {
            switch (previous.second) {
            case (SymbolType::RIGHT):
            case (SymbolType::CONSTANT):
                if (
                    current.second == SymbolType::RIGHT ||
                    current.second == SymbolType::SEPARATOR ||
                    current.second == SymbolType::OPERATOR
                )
                    break;
                else
                    throw SyntaxError{};
            case (SymbolType::LEFT):
            case (SymbolType::SEPARATOR):
            case (SymbolType::OPERATOR):
                if (
                    current.second == SymbolType::CONSTANT ||
                    current.second == SymbolType::LEFT ||
                    current.second == SymbolType::FUNCTION
                )
                    break;
                else
                    throw SyntaxError{};
            case (SymbolType::FUNCTION):
                if (current.second == SymbolType::LEFT)
                    break;
                else
                    throw SyntaxError{};
            }

            if (
                previous.second == SymbolType::CONSTANT ||
                previous.second == SymbolType::RIGHT
            )
                if (
                    current.second != SymbolType::OPERATOR ||
                    get<Operator>(current.first).associativity() !=
                        Associativity::RIGHT
                )
                    fill_parenthesis();

            collected.push(current);
            if (original)
                parsed += current.first;
            previous = current;
        };

        if (tokens.size() == 0)
            throw EmptyExpression{};

        try {
            while (!tokens.empty()) {
                current = tokens.front();
                tokens.pop();

                if (current.second != SymbolType::OPERATOR)
                    collect_symbol();
                else {
                    auto alias = get<Operator>(current.first).alias();
                    if (alias.empty())
                        collect_symbol();
                    else if (tokens.empty())
                        throw SyntaxError{};
                    else {
                        switch (previous.second) {
                        case (SymbolType::LEFT):
                        case (SymbolType::SEPARATOR):
                        case (SymbolType::OPERATOR):
                            parsed += current.first;
                            current = {alias, SymbolType::FUNCTION};
                            collect_symbol(false);
                            current = Left;
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
                if (current.second == SymbolType::LEFT)
                    parenthesis_counter.top()++;
                else if (current.second == SymbolType::RIGHT)
                    parenthesis_counter.top()--;
            }

            if (
                previous.second == SymbolType::CONSTANT ||
                previous.second == SymbolType::RIGHT
            )
                fill_parenthesis();
            else
                throw SyntaxError{};
        }
        catch (const SyntaxError&) {
            parsed += " '" + current.first + "' ";
            while (!tokens.empty()) {
                current = tokens.front();
                tokens.pop();
                parsed += current.first;
            }
            throw SyntaxError{parsed};
        }
        return collected;
    }

    std::queue<std::pair<std::string, SymbolType>> _shunting_yard(
        std::queue<std::pair<std::string, SymbolType>>&& tokens
    ) const {
        using Associativity = typename Operator::Associativity;

        std::queue<std::pair<std::string, SymbolType>> collected{};
        std::stack<std::pair<std::string, SymbolType>> operations{};
        std::pair<std::string, SymbolType> element{};
        std::pair<std::string, SymbolType> another{};

        std::stack<std::size_t> expected_counter{};
        std::stack<std::size_t> provided_counter{};
        std::stack<bool> apply_function{};
        bool was_function{false};

        if (tokens.size() == 0)
            throw EmptyExpression{};

        while (!tokens.empty()) {
            element = tokens.front();
            tokens.pop();

            switch (element.second) {

            case (SymbolType::LEFT):
                operations.push(element);
                apply_function.push(was_function);
                was_function = false;
                break;

            case (SymbolType::RIGHT):
                while (!operations.empty()) {
                    another = operations.top();
                    if (another.second != SymbolType::LEFT) {
                        operations.pop();
                        collected.push(another);
                    }
                    else
                        break;
                }
                if (
                    !operations.empty() &&
                    operations.top().second == SymbolType::LEFT
                )
                    operations.pop();
                else
                    throw ParenthesisMismatch{};
                if (apply_function.top()) {
                    if (expected_counter.top() != provided_counter.top())
                        throw ArgumentsMismatch{
                            operations.top().first,
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
                    another = operations.top();
                    if (another.second != SymbolType::LEFT) {
                        collected.push(another);
                        operations.pop();
                    }
                    else
                        break;
                }
                if (apply_function.empty() || !apply_function.top())
                    throw SyntaxError{
                        "separator '" + element.first + "' outside function"
                    };
                provided_counter.top()++;
                if (operations.empty())
                    throw ParenthesisMismatch{};
                break;

            case (SymbolType::CONSTANT):
                collected.push(element);
                break;

            case (SymbolType::FUNCTION):
                operations.push(element);
                expected_counter.push(get<Function>(element.first).arguments());
                provided_counter.push(1);
                was_function = true;
                break;

            case (SymbolType::OPERATOR):
                while (!operations.empty()) {
                    another = operations.top();
                    if (another.second == SymbolType::LEFT)
                        break;
                    else if (another.second == SymbolType::FUNCTION) {
                        collected.push(another);
                        operations.pop();
                        break;
                    }
                    else {
                        auto l1 =
                            get<Operator>(element.first).associativity() !=
                            Associativity::RIGHT;
                        auto p1 = get<Operator>(element.first).precedence();
                        auto p2 = get<Operator>(another.first).precedence();
                        if ((l1 && (p1 <= p2)) || (!l1 && (p1 < p2))) {
                            operations.pop();
                            collected.push(another);
                        }
                        else
                            break;
                    }
                }
                operations.push(element);
                break;
            }
        }

        while (!operations.empty()) {
            element = operations.top();
            if (element.second == SymbolType::LEFT)
                throw ParenthesisMismatch{};
            operations.pop();
            collected.push(element);
        }

        return collected;
    }

    Expression _create_node(
        const std::pair<std::string, SymbolType>& token,
        std::vector<Expression>&& nodes,
        const std::shared_ptr<Variables>& variables,
        std::size_t hash
    ) const {
        auto found_constant = _factory<Constant>().find(token.first);
        if (found_constant != _factory<Constant>().end()) {
            return Expression{
                token.first,
                variables,
                std::make_shared<Symbol>(*found_constant),
                std::move(nodes),
                hash
            };
        }

        auto found_function = _factory<Function>().find(token.first);
        if (found_function != _factory<Function>().end())
            return Expression{
                token.first,
                variables,
                std::make_shared<Symbol>(*found_function),
                std::move(nodes),
                hash
            };

        auto found_operator = _factory<Operator>().find(token.first);
        if (found_operator != _factory<Operator>().end())
            return Expression{
                token.first,
                variables,
                std::make_shared<Symbol>(*found_operator),
                std::move(nodes),
                hash
            };

        try {
            auto value = Lexer::to_value(token.first);
            return Expression{
                token.first,
                variables,
                std::make_shared<Symbol>(Constant{value}),
                std::move(nodes),
                hash
            };
        }
        catch (const BadCast&) {
            auto& variable = variables->at(token.first);
            return Expression{
                token.first,
                variables,
                std::make_shared<Symbol>(Variable{variable}),
                std::move(nodes),
                hash
            };
        }
    }

    Expression _build_tree(
        std::queue<std::pair<std::string, SymbolType>>&& tokens,
        const std::shared_ptr<Variables>& variables
    ) const {
        std::stack<Expression> operands{};
        std::stack<Expression> extract{};
        std::pair<std::string, SymbolType> element{};
        std::size_t n{};

        auto hash = std::hash<decltype(this)>{}(this);
        while (!tokens.empty()) {
            element = tokens.front();
            tokens.pop();

            if (
                element.second == SymbolType::LEFT ||
                element.second == SymbolType::RIGHT ||
                element.second == SymbolType::SEPARATOR
            )
                throw SyntaxError{
                    "symbol '" + element.first + "' not allowed in "
                    "postfix notation"
                };

            else if (element.second == SymbolType::CONSTANT) {
                if (has<Constant>(element.first))
                    util::hash_combine(hash, get<Constant>(element.first));
                else if (variables->has(element.first))
                    util::hash_combine(hash, variables->index(element.first));
                else
                    util::hash_combine(hash, Lexer::to_value(element.first));
                operands.emplace(
                    _create_node(element, {}, variables, hash)
                );
            }

            else {
                std::vector<Expression> nodes{};
                if (element.second == SymbolType::FUNCTION) {
                    auto function = get<Function>(element.first);
                    n = function.arguments();
                    util::hash_combine(hash, static_cast<Wrapper>(function));
                }
                else {
                    auto function = get<Operator>(element.first).function();
                    n = function.arguments();
                    util::hash_combine(hash, static_cast<Wrapper>(function));
                }
                nodes.reserve(n);

                for (std::size_t i = 0; i < n; i++) {
                    if (operands.empty())
                        throw ArgumentsMismatch{element.first, n, i};
                    extract.emplace(std::move(operands.top()));
                    operands.pop();
                }
                while (!extract.empty()) {
                    nodes.emplace_back(std::move(extract.top()));
                    extract.pop();
                }
                operands.emplace(
                    _create_node(element, std::move(nodes), variables, hash)
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
    Type cast(const std::string& expression) const {
        return Type{from_infix(expression)};
    }

    std::string to_string(Type value) const {
        return Lexer::to_string(value);
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

        auto context = std::make_shared<Variables>(variables);
        for (const auto& node : nodes)
            postfix += node.postfix() + " ";
        return _build_tree(_tokenize(postfix + token, context), context);
    }

    Expression from_infix(
        const std::string& expression,
        const std::vector<std::string>& variables={}
    ) const {
        auto context = std::make_shared<Variables>(variables);
        return _build_tree(
            _shunting_yard(_parse_infix(_tokenize(expression, context))),
            context
        );
    }

    Expression from_postfix(
        const std::string& expression,
        const std::vector<std::string>& variables={}
    ) const {
        auto context = std::make_shared<Variables>(variables);
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


    Expression variables(
        const Expression& node,
        const std::vector<std::string>& variables
    ) const { return from_postfix(node.postfix(), variables); }

    Expression optimize(const Expression& node) const noexcept {
        if (node.variables().empty())
            return create_node(node._symbol->evaluate(node._nodes));
        auto nodes = std::vector<Expression>{};
        nodes.reserve(node._symbol->arguments());
        for (auto another : node._nodes)
            nodes.emplace_back(optimize(another));
        return create_node(node._token, std::move(nodes), node.variables());
    }

    Expression replace(
        const Expression& node,
        std::size_t branch,
        const Expression& another
    ) const { return replace(branch, another, node.variables()); }

    Expression replace(
        const Expression& node,
        std::size_t branch,
        const Expression& another,
        const std::vector<std::string>& variables
    ) const {
        std::string expression{};

        node._nodes.at(branch);
        for (std::size_t i = 0; i < node._nodes.size(); i++)
            if (i != branch)
                expression += node._nodes[i].postfix() + " ";
            else
                expression += another.postfix() + " ";
        return from_postfix(expression + node._token, variables);
    }

    Expression substitute(
        const Expression& node,
        const std::string& variable,
        Type value
    ) const {
        std::vector<std::string> pruned{node.variables()};

        pruned.erase(
            std::remove(pruned.begin(), pruned.end(), variable),
            pruned.end()
        );
        return from_postfix(
            util::replace(
                node.postfix(),
                variable,
                create_node(value).postfix()
            ),
            pruned
        );
    }


    template<typename Kind>
    bool has(const std::string& token) const noexcept {
        return _factory<Kind>().find(token) != _factory<Kind>().end();
    }

    template<typename Kind>
    Kind get(const std::string& token) const {
        auto found = _factory<Kind>().find(token);
        if (found == _factory<Kind>().end())
            throw UndefinedSymbol{token};
        return found->second;
    }

    template<typename Kind, typename... Args>
    void set(const std::string& token, Args&&... args) {
        auto found = _factory<Kind>().find(token);
        _validate(static_cast<Kind*>(nullptr), token);
        if (found != _factory<Kind>().end())
            _factory<Kind>().erase(found);
        _factory<Kind>().emplace(
            std::make_pair(token, Kind{std::forward<Args>(args)...})
        );
    }

    template<typename Kind>
    void remove(const std::string& token) {
        auto found = _factory<Kind>().find(token);
        if (found == _factory<Kind>().end())
            throw UndefinedSymbol{token};
        _factory<Kind>().erase(found);
    }

    template<typename Kind>
    std::vector<std::string> list() const noexcept {
        std::vector<std::string> keys(_factory<Kind>().size());

        std::transform(
            _factory<Kind>().begin(),
            _factory<Kind>().end(),
            keys.begin(),
            [](const auto& pair) { return pair.first; }
        );
        std::sort(keys.begin(), keys.end());
        return keys;
    }
*/};

}

#endif
