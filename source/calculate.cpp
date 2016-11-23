#include <unordered_set>

#include "calculate.h"


namespace calculate {

    const Regex Expression::_ext_regex(R"_(([^\s,]+)|(,))_");

    const Regex Expression::_var_regex(R"_([A-Za-z_]+[A-Za-z_\d]*)_");

    const Regex Expression::_pre_regex(
        R"_(([A-Za-z_\d\.)]+\s*[+\-])(?=\d+\.?\d*|\.\d+))_"
    );

    const Regex Expression::_regex(
        R"_(((?:[+\-])?(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)|)_"
        R"_(([A-Za-z_]+[A-Za-z_\d]*)|)_"
        R"_(([^A-Za-z\d(),\s]+)|)_"
        R"_((\()|(\))|(,))_"
    );


    vString queryConstants() {
        return Constant::queryTokens();
    };

    vString queryOperators() {
        return Operator::queryTokens();
    };

    vString queryFunctions() {
        return Function::queryTokens();
    };


    vString Expression::_extract(const String &vars) {
        vString variables;
        Match match;

        auto suffix = vars;
        auto counter = 0u;
        while (std::regex_search(suffix, match, _ext_regex)) {
            if (!match[2].str().empty()) {
                if (counter % 2 == 0)
                    throw BadNameException(match[2].str());
            }
            else
                variables.push_back(match[1].str());
            counter++;
            suffix = match.suffix().str();
        }

        return variables;
    }

    vString Expression::_validate(const vString &vars) {
        for (auto &var : vars)
            if (!std::regex_match(var, _var_regex))
                throw BadNameException(var);

        auto no_dups = std::unordered_set<String>(vars.begin(), vars.end());
        for (auto &var : vars)
            if (no_dups.erase(var) == 0)
                throw DuplicatedNameException(var);

        return vars;
    }

    qSymbol Expression::_tokenize() {
        qSymbol infix;
        Match match;
        Stream stream;

        enum Group {NUMBER=1, NAME, SYMBOL, LEFT, RIGHT, SEPARATOR};
        auto is = [&match](int group) { return !match[group].str().empty(); };
        auto encountered = std::unordered_set<String>();

        auto expression = std::regex_replace(_expression, _pre_regex, "$1 ");
        while (std::regex_search(expression, match, _regex)) {
            auto token = match.str();
            auto it = std::find(_variables.begin(), _variables.end(), token);

            if (is(Group::NUMBER))
                infix.push(Constant::makeNumbered(token));
            else if (is(Group::NAME) && it != _variables.end()) {
                auto position = it - _variables.begin();
                infix.push(Variable::make(token, _values.get() + position));
                encountered.emplace(token);
            }
            else if (is(Group::NAME) && Constant::hasToken(token))
                infix.push(Constant::makeNamed(token));
            else if (is(Group::NAME) && Function::hasToken(token))
                infix.push(Function::make(token));
            else if (is(Group::SYMBOL) && Operator::hasToken(token))
                infix.push(Operator::make(token));
            else if (is(Group::LEFT))
                infix.push(Parenthesis<'('>::make());
            else if (is(Group::RIGHT))
                infix.push(Parenthesis<')'>::make());
            else if (is(Group::SEPARATOR))
                infix.push(Separator::make());
            else
                throw UndefinedSymbolException(token);

            expression = match.suffix().str();
            stream << " " << token;
        }

        if (encountered.size() < _variables.size()) {
            for (auto &var : _variables)
                if (encountered.find(var) == encountered.end())
                    throw WrongVariablesException(var);
        }

        _infix = stream.str().erase(0, 1);
        return infix;
    }

    qSymbol Expression::_check(qSymbol &&input) {
        qSymbol output;
        pSymbol current, next;

        current = input.front();
        input.pop();
        output.push(current);

        switch (current->type) {
        case (Type::RIGHT): case (Type::SEPARATOR): case (Type::OPERATOR):
            throw SyntaxErrorException();
        default:
            break;
        }

        while (!input.empty()) {
            next = input.front();
            input.pop();
            output.push(next);

            switch (current->type) {
            case (Type::CONSTANT): case (Type::RIGHT):
                if (next->is(Type::RIGHT)) break;
                else if (next->is(Type::SEPARATOR)) break;
                else if (next->is(Type::OPERATOR)) break;
                else throw SyntaxErrorException();
            case (Type::LEFT): case (Type::SEPARATOR): case (Type::OPERATOR):
                if (next->is(Type::CONSTANT)) break;
                else if (next->is(Type::LEFT)) break;
                else if (next->is(Type::FUNCTION)) break;
                else throw SyntaxErrorException();
            case (Type::FUNCTION):
                if (next->is(Type::LEFT)) break;
                else throw SyntaxErrorException();
            }
            current = next;
        }

        switch (current->type) {
        case (Type::CONSTANT): case (Type::RIGHT):
            break;
        default:
            throw SyntaxErrorException();
        }

        return output;
    }

    qEvaluable Expression::_shuntingYard(qSymbol &&infix) {
        qEvaluable postfix;
        sSymbol operations;
        pSymbol element, another;
        Stream stream;

        while(!infix.empty()) {
            element = infix.front();
            infix.pop();

            switch (element->type) {
            case (Type::CONSTANT):
                postfix.push(castChild<Evaluable>(element));
                stream << " " << element->token;
                break;

            case (Type::FUNCTION):
                operations.push(element);
                break;

            case (Type::SEPARATOR):
                while (!operations.empty()) {
                    another = operations.top();
                    if (!another->is(Type::LEFT)) {
                        postfix.push(castChild<Evaluable>(another));
                        stream << " " << another->token;
                        operations.pop();
                    }
                    else {
                        break;
                    }
                }
                if (operations.empty())
                    throw ParenthesisMismatchException();
                break;

            case (Type::OPERATOR):
                while (!operations.empty()) {
                    another = operations.top();
                    if (another->is(Type::LEFT)) {
                        break;
                    }
                    else if (another->is(Type::FUNCTION)) {
                        postfix.push(castChild<Evaluable>(another));
                        stream << " " << another->token;
                        operations.pop();
                        break;
                    }
                    else {
                        auto left = castChild<Operator>(element)->left_assoc;
                        auto p1 = castChild<Operator>(element)->precedence;
                        auto p2 = castChild<Operator>(another)->precedence;
                        if ((left && (p1 <= p2)) || (!left && (p1 < p2))) {
                            operations.pop();
                            postfix.push(castChild<Evaluable>(another));
                            stream << " " << another->token;
                        }
                        else {
                            break;
                        }
                    }
                }
                operations.push(element);
                break;

            case (Type::LEFT):
                operations.push(element);
                break;

            case (Type::RIGHT):
                while (!operations.empty()) {
                    another = operations.top();
                    if (!another->is(Type::LEFT)) {
                        operations.pop();
                        postfix.push(castChild<Evaluable>(another));
                        stream << " " << another->token;
                    }
                    else {
                        break;
                    }
                }
                if (!operations.empty() && operations.top()->is(Type::LEFT))
                    operations.pop();
                else
                    throw ParenthesisMismatchException();
                break;
            }
        }

        while(!operations.empty()) {
            element = operations.top();
            if (element->is(Type::LEFT))
                throw ParenthesisMismatchException();
            operations.pop();
            postfix.push(castChild<Evaluable>(element));
            stream << " " << element->token;
        }

        _postfix = stream.str().erase(0, 1);
        return postfix;
    }

    pEvaluable Expression::_buildTree(qEvaluable &&postfix) {
        sEvaluable operands;
        pEvaluable element;

        while (!postfix.empty()) {
            element = postfix.front();
            postfix.pop();

            if (element->is(Type::CONSTANT)) {
                operands.push(element);
            }

            else if (element->is(Type::FUNCTION)) {
                auto function = castChild<Function>(element);
                auto args = function->args;
                vEvaluable ops(args);
                for (auto i = args; i > 0; i--) {
                    if (operands.empty())
                        throw MissingArgumentsException(function->token);
                    ops[i - 1] = operands.top();
                    operands.pop();
                }
                function->addBranches(ops);
                operands.push(element);
            }

            else if (element->is(Type::OPERATOR)) {
                auto binary = castChild<Operator>(element);
                pEvaluable a, b;
                b = operands.top();
                operands.pop();
                a = operands.top();
                operands.pop();
                binary->addBranches(a, b);
                operands.push(element);
            }
        }
        if (operands.size() > 1)
            throw ArgumentsExcessException(operands.top()->token);

        return operands.top();
    }

    Expression::Expression(const Expression &other) :
            Expression(other._expression, other._variables) {
    }

    Expression::Expression(Expression &&other) :
            _expression(std::move(other._expression)),
            _variables(std::move(other._variables)),
            _values(std::move(other._values)),
            _infix(std::move(other._infix)),
            _postfix(std::move(other._postfix)),
            _tree(std::move(other._tree)) {}

    Expression::Expression(const String &expr, const String &vars) :
            Expression(expr, _extract(vars)) {
    }

    Expression::Expression(const String &expr, const vString &vars) :
            _expression(expr),
            _variables(_validate(vars)),
            _values(std::make_unique<double[]>(vars.size())) {

        if (expr.length() == 0)
            throw EmptyExpressionException();

        for (auto i = 0u; i < vars.size(); i++)
            _values[i] = 0.;

        auto infix = _check(_tokenize());
        auto postfix = _shuntingYard(std::move(infix));
        _tree = _buildTree(std::move(postfix));
    }

    Expression& Expression::operator=(const Expression &other) {
        auto copy = Expression(other);
        *this = std::move(copy);

        return *this;
    }

    Expression& Expression::operator=(Expression &&other) {
        _expression = std::move(other._expression);
        _variables = std::move(other._variables);
        _values = std::move(other._values);
        _infix = std::move(other._infix);
        _postfix = std::move(other._postfix);
        _tree = std::move(other._tree);

        return *this;
    }

    double Expression::operator() (const vValue &values) const {
        if (values.size() != _variables.size())
            throw WrongVariablesException();
        for (auto i = 0u; i < values.size(); i++)
            _values[i] = values[i];

        return _tree->evaluate();
    }

    String Expression::tree() const noexcept {
        Stream stream;

        _tree->print(stream);
        return stream.str().erase(stream.str().size() - 1, 1);
    }

}
