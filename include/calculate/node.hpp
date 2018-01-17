#ifndef __CALCULATE_NODE_HPP__
#define __CALCULATE_NODE_HPP__

#include <iterator>
#include <ostream>
#include <stack>
#include <sstream>

#include "symbol.hpp"
#include "util.hpp"


namespace calculate {

template<typename Parser>
class Node {
    friend struct std::hash<Node>;
    friend Parser;

public:
    using Lexer = typename Parser::Lexer;
    using Type = typename Parser::Type;

    using Symbol = Symbol<Node>;
    using SymbolType = typename Symbol::SymbolType;


public:
    using const_iterator = typename std::vector<Node>::const_iterator;

    class VariableHandler {
    public:
        const std::vector<std::string> variables;

    private:
        std::size_t _size;
        std::unique_ptr<Type[]> _values;

        void _update(std::size_t) const {}

        template<typename Last>
        void _update(std::size_t n, Last last) { _values[n] = last; }

        template<typename Head, typename... Args>
        void _update(std::size_t n, Head head, Args... args) {
            _values[n] = head;
            _update(n + 1, args...);
        }

    public:
        explicit VariableHandler(
            const std::vector<std::string>& keys,
            Lexer& lexer
        ) :
                variables{keys},
                _size{keys.size()},
                _values{std::make_unique<Type[]>(_size)}
        {
            std::unordered_set<std::string> singles{keys.begin(), keys.end()};
            for (const auto &variable : keys) {
                if (!std::regex_match(variable, lexer.name_regex))
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

        Type& at(const std::string& token) const {
            return _values[index(token)];
        }

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
    std::shared_ptr<Lexer> _lexer;
    std::shared_ptr<VariableHandler> _variables;
    std::string _token;
    std::unique_ptr<Symbol> _symbol;
    std::vector<Node> _nodes;
    std::size_t _hash;

    Node() = delete;

    explicit Node(
        const std::shared_ptr<Lexer>& _lexer,
        const std::shared_ptr<VariableHandler>& variables,
        const std::string& token,
        std::unique_ptr<Symbol>&& symbol,
        std::vector<Node>&& nodes,
        std::size_t hash
    ) :
            _lexer{_lexer},
            _variables{variables},
            _token{token},
            _symbol{std::move(symbol)},
            _nodes{std::move(nodes)},
            _hash{hash}
    {
        if (_nodes.size() != _symbol->arguments())
            throw ArgumentsMismatch{
                _token,
                _nodes.size(),
                _symbol->arguments()
            };
    }


    std::vector<std::string> _pruned() const noexcept {
        std::istringstream extractor{postfix()};
        std::vector<std::string> tokens{
            std::istream_iterator<std::string>{extractor},
            std::istream_iterator<std::string>{}
        };
        std::vector<std::string> pruned{};

        for (const auto& variable : variables())
            if (
                std::find(tokens.begin(), tokens.end(), variable) !=
                tokens.end()
            )
                pruned.push_back(variable);

        pruned.erase(std::unique(pruned.begin(), pruned.end()), pruned.end());
        return pruned;
    }

    void _rebind(const std::shared_ptr<VariableHandler>& context) noexcept {
        _variables = context;
        for (auto& node : _nodes)
            node._rebind(context);
    }


public:
    Node(const Node& other) noexcept :
            _lexer{other._lexer},
            _variables{},
            _token{other._token},
            _symbol{other._symbol->clone()},
            _nodes{other._nodes},
            _hash{other._hash}
    { _rebind(std::make_shared<VariableHandler>(other._pruned(), *_lexer)); }

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

    explicit operator Type() const {
        if (variables().size() > 0)
            throw ArgumentsMismatch{variables().size(), 0};
        return _symbol->evaluate(_nodes);
    }

    Type operator()(const std::vector<Type>& values) const {
        _variables->update(values);
        return _symbol->evaluate(_nodes);
    }

    template<typename... Args>
    Type operator()(Args&&... args) const {
        _variables->update(std::forward<Args>(args)...);
        return _symbol->evaluate(_nodes);
    }

    bool operator==(const Node& other) const noexcept {
        std::stack<std::pair<const_iterator, const_iterator>> these{};
        std::stack<const_iterator> those{};

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

        these.push({this->begin(), this->end()});
        those.push(other.begin());
        while(!these.empty()) {
            auto one = these.top();
            auto another = those.top();
            these.pop();
            those.pop();

            if (one.first != one.second) {
                if (!equal(one.first, another))
                    return false;
                these.push({one.first->begin(), one.first->end()});
                these.push({one.first + 1, one.second});
                those.push(another->begin());
                those.push(another + 1);
            }
        }
        return true;
    }

    bool operator!=(const Node& other) const noexcept {
        return !operator==(other);
    }

    const Node& operator[](std::size_t index) const { return _nodes[index]; }

    const Node& at(std::size_t index) const { return _nodes.at(index); }

    const_iterator begin() const noexcept { return _nodes.begin(); }

    const_iterator end() const noexcept { return _nodes.end(); }

    friend std::ostream& operator<<(
        std::ostream& ostream,
        const Node& node
    ) noexcept {
        ostream << node.infix();
        return ostream;
    }

    static Type evaluate(const Node& node) {
        return node._symbol->evaluate(node._nodes);
    }

    const Lexer& lexer() const noexcept { return *_lexer; }

    const std::string& token() const noexcept { return _token; }

    SymbolType symbol() const noexcept { return _symbol->symbol(); }

    std::size_t branches() const noexcept { return _nodes.size(); }

    std::string infix() const noexcept {
        using Operator = Operator<Node>;
        using Associativity = typename Operator::Associativity;

        std::string infix{};

        auto brace = [&](std::size_t i) {
            const auto& node = _nodes[i];
            if (node._symbol->symbol() == SymbolType::OPERATOR) {
                auto po = static_cast<Operator*>(_symbol.get());
                auto co = static_cast<Operator*>(node._symbol.get());
                auto pp = po->precedence();
                auto cp = co->precedence();
                auto pa = !i ?
                    po->associativity() != Associativity::RIGHT :
                    po->associativity() != Associativity::LEFT;
                if ((pa && cp < pp) || (!pa && cp <= pp))
                    return _lexer->left + node.infix() + _lexer->right;
            }
            return node.infix();
        };

        switch (_symbol->symbol()) {
        case (SymbolType::FUNCTION):
            infix += _token + _lexer->left;
            for (const auto& node : _nodes)
                infix += node.infix() + _lexer->separator;
            infix.back() = _lexer->right.front();
            return infix;
        case (SymbolType::OPERATOR):
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

    std::vector<std::string> variables() const noexcept {
        return _variables->variables;
    }
};

}


namespace std {

template<typename Parser>
struct hash<calculate::Node<Parser>> {
    size_t operator()(const calculate::Node<Parser>& node) const {
        return node._hash;
    }
};

}

#endif
