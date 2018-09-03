/*
    Calculate - Version 2.1.1rc10
    Last modified 2018/07/28
    Released under MIT license
    Copyright (c) 2016-2018 Alberto Lorenzo <alorenzo.md@gmail.com>
*/


#ifndef __CALCULATE_NODE_HPP__
#define __CALCULATE_NODE_HPP__

#include <iterator>
#include <ostream>
#include <stack>
#include <regex>
#include <unordered_set>

#include "symbol.hpp"


namespace calculate {

template<typename Parser>
class Node {
    friend struct std::hash<Node>;
    friend calculate::Symbol<Node>;
    friend Parser;

public:
    using Lexer = typename Parser::Lexer;
    using Type = typename Parser::Type;

    using Symbol = calculate::Symbol<Node>;
    using SymbolType = typename Symbol::SymbolType;

    using const_iterator = typename std::vector<Node>::const_iterator;
    using const_reverse_iterator = typename std::vector<Node>::const_reverse_iterator;


    class VariableHandler {
    public:
        const std::vector<std::string> variables;

    private:
        std::size_t _size;
        std::unique_ptr<Type[]> _values;
        std::size_t _refcount;
        std::shared_ptr<VariableHandler> _copy;

        void _update(std::size_t) const {}

        template<typename Last>
        void _update(std::size_t n, Last last) { _values[n] = last; }

        template<typename Head, typename... Args>
        void _update(std::size_t n, Head head, Args... args) {
            _values[n] = head;
            _update(n + 1, args...);
        }

    public:
        VariableHandler(std::vector<std::string> keys, Lexer* lexer) :
                variables{std::move(keys)},
                _size{variables.size()},
                _values{std::make_unique<Type[]>(_size)},
                _refcount{0u},
                _copy{nullptr}
        {
            std::unordered_set<std::string> singles{variables.begin(), variables.end()};

            for (const auto &var : variables) {
                if (!std::regex_match(var, lexer->name_regex))
                    throw UnsuitableName{var};
                else if (singles.erase(var) == 0)
                    throw RepeatedSymbol{var};
            }
        }

        explicit VariableHandler(std::vector<std::string> keys) :
                variables{std::move(keys)},
                _size{variables.size()},
                _values{std::make_unique<Type[]>(_size)},
                _refcount{0u},
                _copy{nullptr}
        {}

        std::shared_ptr<VariableHandler> rebuild(std::vector<std::string> keys) noexcept {
            ++_refcount;
            if (_copy)
                return _copy;
            _copy = std::make_shared<VariableHandler>(std::move(keys));
            return _copy;
        }

        void reset() noexcept {
            if (!--_refcount)
                _copy = nullptr;
        }

        std::size_t index(const std::string& token) const {
            auto found = std::find(variables.begin(), variables.end(), token);
            if (found != variables.end())
                return found - variables.begin();
            throw UndefinedSymbol{token};
        }

        Type& at(const std::string& token) const { return _values[index(token)]; }

        template<typename Args>
        std::enable_if_t<util::is_vectorizable_v<Type, Args>> update(Args&& vals) {
            std::size_t i{};

            for (const auto& val : vals) {
                if (i < _size)
                    _values[i] = val;
                ++i;
            }
            if (_size != i)
                throw ArgumentsMismatch{_size, i};
        }

        template<typename... Args>
        void update(Args&&... vals) {
            if (_size != sizeof...(vals))
                throw ArgumentsMismatch{_size, sizeof...(vals)};
            _update(0, std::forward<Args>(vals)...);
        }
    };


private:
    std::shared_ptr<Lexer> _lexer;
    std::shared_ptr<VariableHandler> _variables;
    std::string _token;
    std::unique_ptr<Symbol> _symbol;
    std::vector<Node> _nodes;
    std::size_t _hash;

    Node(
        std::shared_ptr<Lexer> _lexer,
        std::shared_ptr<VariableHandler> variables,
        std::string token,
        std::unique_ptr<Symbol>&& symbol,
        std::vector<Node>&& nodes,
        std::size_t hash
    ) :
            _lexer{std::move(_lexer)},
            _variables{std::move(variables)},
            _token{std::move(token)},
            _symbol{std::move(symbol)},
            _nodes{std::move(nodes)},
            _hash{hash}
    {
        if (_nodes.size() != _symbol->arguments())
            throw ArgumentsMismatch{_token, _nodes.size(), _symbol->arguments()};
    }

    std::vector<std::string> _pruned() const noexcept {
        std::vector<std::string> pruned{};

        auto tokens = _lexer->tokenize_postfix(postfix());
        for (const auto& var : _variables->variables)
            for (const auto& token : tokens)
                if (var == token.token) {
                    pruned.push_back(var);
                    break;
                }
        return pruned;
    }

    bool _compare(const Node& other) const noexcept {
        std::stack<const_iterator> these{}, those{}, endings{};

        auto equal = [&](auto left, auto right) {
            try {
                return
                    left->_variables->index(left->_token) ==
                    right->_variables->index(right->_token);
            }
            catch (const UndefinedSymbol&) {
                if (left->_symbol == right->_symbol)
                    return true;
                return *(left->_symbol) == *(right->_symbol);
            }
        };

        if (!equal(this, &other))
            return false;

        these.push(this->begin());
        those.push(other.begin());
        endings.push(this->end());
        while(!these.empty()) {
            auto &one = these.top(), &another = those.top();
            if (one != endings.top()) {
                if (!equal(one, another))
                    return false;
                these.push(one->begin());
                those.push(another->begin());
                endings.push(one->end());
                one++, another++;
                continue;
            }
            these.pop();
            those.pop();
            endings.pop();
        }
        return true;
    }

    std::string _infix(bool right) const noexcept {
        using Operator = Operator<Node>;
        using Associativity = typename Operator::Associativity;

        std::string infix{};

        auto brace = [&](std::size_t i) {
            const auto& node = _nodes[i];
            auto po = static_cast<Operator*>(_symbol.get());
            if (node._symbol->type() == SymbolType::OPERATOR) {
                auto co = static_cast<Operator*>(node._symbol.get());
                auto pp = po->precedence();
                auto cp = co->precedence();
                auto pa = !i ?
                    po->associativity() != Associativity::RIGHT :
                    po->associativity() != Associativity::LEFT;
                if ((pa && cp < pp) || (!pa && cp <= pp))
                    return _lexer->left + node._infix(false) + _lexer->right;
            }
            auto r = right || i || po->associativity() == Associativity::RIGHT;
            return node._infix(r);
        };

        switch (_symbol->type()) {
        case (SymbolType::FUNCTION):
            infix += _token + _lexer->left;
            for (auto node = _nodes.begin(); node != _nodes.end(); node++) {
                infix += node->_infix(false);
                infix += (node + 1 != _nodes.end() ? _lexer->separator : "");
            }
            return infix + _lexer->right;

        case (SymbolType::OPERATOR):
            return infix + brace(0) + _token + brace(1);

        default:
            if (right && _lexer->prefixed(_token))
                return _lexer->left + _token + _lexer->right;
            return _token;
        }
        return infix;
    }

public:
    Node(const Node& other) noexcept :
            _lexer{other._lexer},
            _variables{other._variables->rebuild(other._pruned())},
            _token{other._token},
            _symbol{nullptr},
            _nodes{other._nodes},
            _hash{other._hash}
    {
        using Variable = Variable<Node>;

        other._variables->reset();
        try {
            _symbol = Variable{_variables->at(_token)}.clone();
        }
        catch (const UndefinedSymbol&) {
            _symbol = other._symbol->clone();
        }
    }

    Node(Node&& other) noexcept :
            _lexer{std::move(other._lexer)},
            _variables{std::move(other._variables)},
            _token{std::move(other._token)},
            _symbol{std::move(other._symbol)},
            _nodes{std::move(other._nodes)},
            _hash{std::move(other._hash)}
    {}

    const Node& operator=(Node other) noexcept {
        swap(*this, other);
        return *this;
    }

    friend void swap(Node& one, Node& another) noexcept {
        using std::swap;
        swap(one._lexer, another._lexer);
        swap(one._variables, another._variables);
        swap(one._token, another._token);
        swap(one._symbol, another._symbol);
        swap(one._nodes, another._nodes);
    }

    operator Type() const {
        if (_variables->variables.size() > 0)
            throw ArgumentsMismatch{_variables->variables.size(), 0};
        return _symbol->eval(_nodes);
    }

    template<typename... Args>
    Type operator()(Args&&... args) const {
        _variables->update(std::forward<Args>(args)...);
        return _symbol->eval(_nodes);
    }

    bool operator==(const Node& other) const noexcept {
        if (_hash != other._hash)
            return false;
        return _compare(other);
    }

    bool operator!=(const Node& other) const noexcept { return !operator==(other); }

    const Node& operator[](std::size_t index) const { return _nodes[index]; }

    const Node& at(std::size_t index) const { return _nodes.at(index); }

    std::size_t size() const noexcept { return _nodes.size(); }

    const_iterator begin() const noexcept { return _nodes.cbegin(); }

    const_iterator end() const noexcept { return _nodes.cend(); }

    const_iterator cbegin() const noexcept { return _nodes.cbegin(); }

    const_iterator cend() const noexcept { return _nodes.cend(); }

    const_reverse_iterator rbegin() const noexcept { return _nodes.crbegin(); }

    const_reverse_iterator rend() const noexcept { return _nodes.crend(); }

    const_reverse_iterator crbegin() const noexcept { return _nodes.crbegin(); }

    const_reverse_iterator crend() const noexcept { return _nodes.crend(); }

    friend std::ostream& operator<<(std::ostream& ostream, const Node& node) noexcept {
        ostream << node.infix();
        return ostream;
    }

    const Lexer& lexer() const noexcept { return *_lexer; }

    const std::string& token() const noexcept { return _token; }

    SymbolType type() const noexcept { return _symbol->type(); }

    std::size_t branches() const noexcept { return _nodes.size(); }

    std::string infix() const noexcept { return _infix(false); }

    std::string postfix() const noexcept {
        std::string postfix{};

        for (const auto& node : _nodes)
            postfix += node.postfix() + " ";
        return postfix + _token;
    }

    const std::vector<std::string>& variables() const noexcept { return _variables->variables; }
};

}


namespace std {

template<typename Parser>
struct hash<calculate::Node<Parser>> {
    size_t operator()(const calculate::Node<Parser>& node) const { return node._hash; }
};

}

#endif
