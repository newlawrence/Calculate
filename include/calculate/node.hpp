#ifndef __CALCULATE_NODE_HPP__
#define __CALCULATE_NODE_HPP__

#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <utility>
#include <vector>

#include "exception.hpp"


namespace calculate {

namespace detail {

inline std::string replace(std::string where, const std::string& what, const std::string& by) {
    std::size_t index = 0;
    while (true) {
        index = where.find(what, index);
        if (index == std::string::npos)
            break;
        where.replace(index, what.size(), by);
        index += what.size();
    }
    return where;
}

}


template<typename Parser>
class Node {
    using Type = typename Parser::Type;
    using Lexer = typename Parser::Lexer;
    using Symbol = typename Parser::Symbol;
    using Associativity = typename Parser::Associativity;
    using Constant = typename Parser::Constant;
    using Function = typename Parser::Function;
    using Operator = typename Parser::Operator;

    friend typename Parser::Base;
    friend struct std::hash<Node>;


public:
    class Variables {
    public:
        const std::vector<std::string> variables;

    private:
        std::size_t _size;
        std::unique_ptr<Type[]> _values;

        template<typename Last>
        void _update(std::size_t n, Last last) { _values[n] = last; }

        template<typename Head, typename... Args>
        void _update(std::size_t n, Head head, Args... args) {
            _values[n] = head;
            _update(n + 1, args...);
        }

    public:
        Variables(const std::vector<std::string>& keys={}) :
                variables{keys},
                _size{keys.size()},
                _values{std::make_unique<Type[]>(_size)}
        {
            std::unordered_set<std::string> singles{keys.begin(), keys.end()};
            for (const auto &variable : keys) {
                if (!std::regex_match(variable, Lexer::name_regex))
                    throw UnsuitableName{variable};
                else if (singles.erase(variable) == 0)
                    throw RepeatedSymbol{variable};
            }
        }

        std::size_t index(const std::string& token) const {
            auto found = std::find(variables.begin(), variables.end(), token);
            if (found != variables.end())
                return found - variables.begin();
            throw UndefinedSymbol{token};
        }

        Type& at(const std::string& token) const { return _values[index(token)]; }

        void update(const std::vector<Type>& values) {
            if (_size != values.size())
                throw ArgumentsMismatch{_size, values.size()};
            for (std::size_t i = 0; i < _size; i++)
                _values[i] = values[i];
        }

        template<typename... Args>
        void update(Args... args) {
            if (_size != sizeof...(args))
                throw ArgumentsMismatch{_size, sizeof...(args)};
            _update(0, args...);
        }
    };


private:
    std::string _token;
    Symbol _symbol;
    std::shared_ptr<Variables> _variables;
    Function _function;
    std::vector<Node> _nodes;
    std::size_t _footprint;
    mutable std::mutex _mutex;
    std::size_t _precedence;
    Associativity _associativity;

    Node() = delete;

    Node(
        const std::pair<std::string, Symbol>& token,
        const std::shared_ptr<Variables>& variables,
        const Function& function,
        std::vector<Node>&& nodes,
        std::size_t footprint,
        std::size_t precedence=std::numeric_limits<std::size_t>::max(),
        Associativity associativity=Associativity::BOTH
    ) :
            _token{token.first},
            _symbol{token.second},
            _variables{variables},
            _function{function},
            _nodes{std::move(nodes)},
            _footprint{footprint},
            _mutex{},
            _precedence{precedence},
            _associativity{associativity}
    {
        if (_nodes.size() != _function.arguments())
            throw ArgumentsMismatch{_token, _nodes.size(), _function.arguments()};
    }


    std::vector<std::string> _pruned() const noexcept {
        std::istringstream extractor{postfix()};
        std::vector<std::string> tokens{
            std::istream_iterator<std::string>{extractor},
            std::istream_iterator<std::string>{}
        };
        std::vector<std::string> pruned{};

        for (const auto& variable : variables())
            if (std::find(tokens.begin(), tokens.end(), variable) != tokens.end())
                pruned.push_back(variable);

        pruned.erase(std::unique(pruned.begin(), pruned.end()), pruned.end());
        return pruned;
    }

    void _rebind(const std::shared_ptr<Variables>& context) noexcept {
        _variables = context;
        for (auto& node : _nodes)
            node._rebind(context);
    }

    Type _eval(
        std::integral_constant<bool, true>::type,
        const std::vector<Type>& values
    ) const {
        std::lock_guard<std::mutex> guard{_mutex};

        _variables->update(values);
        return _function(_nodes);
    }

    Type _eval(
        std::integral_constant<bool, false>::type,
        const std::vector<Type>& values
    ) const {
        _variables->update(values);
        return _function(_nodes);
    }

    template<typename... Args>
    Type _eval(std::integral_constant<bool, true>::type, Args&&... args) const {
        std::lock_guard<std::mutex> guard{_mutex};

        _variables->update(std::forward<Args>(args)...);
        return _function(_nodes);
    }

    template<typename... Args>
    Type _eval(std::integral_constant<bool, false>::type, Args&&... args) const {
        _variables->update(std::forward<Args>(args)...);
        return _function(_nodes);
    }

public:
    using const_iterator = typename std::vector<Node>::const_iterator;

    Node(const Node& other) noexcept :
            _token{other._token},
            _symbol{other._symbol},
            _variables{},
            _function{other._function},
            _nodes{other._nodes},
            _footprint{other._footprint},
            _mutex{},
            _precedence{other._precedence},
            _associativity{other._associativity}
    { _rebind(std::make_shared<Variables>(other._pruned())); }

    Node(Node&& other) noexcept :
            _token{std::move(other._token)},
            _symbol{std::move(other._symbol)},
            _variables{std::move(other._variables)},
            _function{std::move(other._function)},
            _nodes{std::move(other._nodes)},
            _footprint{std::move(other._footprint)},
            _mutex{},
            _precedence{std::move(other._precedence)},
            _associativity{std::move(other._associativity)}
    {}

    Node& operator=(Node other) noexcept {
        swap(*this, other);
        return *this;
    }

    friend void swap(Node& one, Node& another) noexcept {
        using std::swap;
        swap(one._token, another._token);
        swap(one._symbol, another._symbol);
        swap(one._variables, another._variables);
        swap(one._function, another._function);
        swap(one._nodes, another._nodes);
        swap(one._precedence, another._precedence);
        swap(one._associativity, another._associativity);
    }

    explicit operator Type() const {
        if (variables().size() > 0)
            throw ArgumentsMismatch{variables().size(), 0};
        return _function(_nodes);
    }

    Type operator()(const std::vector<Type>& values) const {
        return _eval(std::integral_constant<bool, Parser::thread_safe>{}, values);
    }

    template<typename... Args>
    Type operator()(Args&&... args) const {
        return _eval(
            std::integral_constant<bool, Parser::thread_safe>{},
            std::forward<Args>(args)...
        );
    }

    bool operator==(const Node& other) const noexcept {
        std::string left{postfix()};
        std::string right{other.postfix()};

        if (_footprint != other._footprint)
            return false;
        else if (variables().size() != other.variables().size())
            return false;
        for (std::size_t i = 0; i < variables().size(); i++) {
            std::string variable{"var#" + std::to_string(i)};
            left = detail::replace(left, variables()[i], variable);
            right = detail::replace(right, other.variables()[i], variable);
        }
        return left == right;
    }

    const Node& operator[](std::size_t index) const { return _nodes[index]; }

    const Node& at(std::size_t index) const { return _nodes.at(index); }

    const const_iterator begin() const noexcept { return _nodes.begin(); }

    const const_iterator end() const noexcept { return _nodes.end(); }

    friend std::ostream& operator<<(std::ostream& ostream, const Node& node) noexcept {
        ostream << node.infix();
        return ostream;
    }

    std::string token() const noexcept { return _token; }

    Symbol symbol() const noexcept { return _symbol; }

    std::size_t branches() const noexcept { return _nodes.size(); }

    std::string infix() const noexcept {
        std::string infix{};

        auto brace = [&](std::size_t i) {
            const auto& node = _nodes[i];
            if (node._symbol == Symbol::OPERATOR) {
                auto pp = _precedence;
                auto cp = node._precedence;
                auto pa = !i ?
                    _associativity != Associativity::RIGHT :
                    _associativity != Associativity::LEFT;
                if ((pa && cp < pp) || (!pa && cp <= pp))
                    return Lexer::left + node.infix() + Lexer::right;
            }
            return node.infix();
        };

        switch (_symbol) {
        case (Symbol::FUNCTION):
            infix += _token + Lexer::left;
            for (const auto& node : _nodes)
                infix += node.infix() + Lexer::separator;
            infix.back() = Lexer::right.front();
            return infix;
        case (Symbol::OPERATOR):
            infix += brace(0) + _token + brace(1);
            return infix;
        default:
            return _token;
        }
    }

    std::string postfix() const noexcept {
        std::string postfix{};

        for (const auto& node : _nodes)
            postfix += node.postfix() + " ";
        return postfix + _token;
    }

    std::vector<std::string> variables() const noexcept { return _variables->variables; }
};

}


namespace std {

template<typename Parser>
struct hash<calculate::Node<Parser>> {
    size_t operator()(const calculate::Node<Parser>& node) const {
        size_t combined{node._footprint};
        string postfix{node.postfix()};
        for (size_t i = 0; i < node.variables().size(); i++) {
            string variable{"var#" + to_string(i)};
            postfix = calculate::detail::replace(postfix, node.variables()[i], variable);
        }
        calculate::detail::hash_combine(combined, hash<string>{}(postfix));
        return combined;
    }
};

}

#endif
