#include <unordered_set>

#include "calculate.h"
#include "calculate/info.h"


namespace {

    using namespace calculate_definitions;

    Regex ext_regex(R"_(([^\s,]+)|(,))_");
    Regex var_regex(R"_([A-Za-z_]+[A-Za-z_\d]*)_");

    Regex regex(
        R"_(((?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)|)_"
        R"_(([A-Za-z_]+[A-Za-z_\d]*)|)_"
        R"_(([^A-Za-z\d.(),_\s]+)|)_"
        R"_((\()|(\))|(,))_"
    );

}


namespace calculate {

    String version() { return __CALCULATE_VERSION__; }
    String author() { return __CALCULATE_AUTHOR__; }
    String date() { return __CALCULATE_DATE__; }


    vString constants() { return query<Constant>(); }
    vString operators() { return query<Operator>(); }
    vString functions() { return query<Function>(); }


    vString Expression::_extract(const String &vars) {
        vString variables;
        Match match;

        auto suffix = vars;
        auto counter = 0u;
        while (std::regex_search(suffix, match, ext_regex)) {
            if (!match[2].str().empty()) {
                if (counter % 2 == 0)
                    throw BadNameException(match[2].str());
            }
            else
                variables.emplace_back(match[1].str());
            counter++;
            suffix = match.suffix().str();
        }

        return variables;
    }

    vString Expression::_validate(const vString &vars) {
        for (auto &var : vars)
            if (!std::regex_match(var, var_regex))
                throw BadNameException(var);

        auto no_dups = std::unordered_set<String>(vars.begin(), vars.end());
        for (auto &var : vars)
            if (no_dups.erase(var) == 0)
                throw DuplicatedNameException(var);

        return vars;
    }

    qSymbol Expression::_tokenize(String expr) {
        qSymbol infix;
        Match match;
        Stream stream;

        enum Group {NUMBER=1, NAME, SYMBOL, LEFT, RIGHT, SEPARATOR};
        auto encountered = std::unordered_set<String>();
        auto counter = std::stack<Unsigned>();
        auto previous = make<Parenthesis<'('>>();

        auto fill_parens = [&]() {
            while (!counter.empty()) {
                if (counter.top() == 0) {
                    infix.push(make<Parenthesis<')'>>());
                    counter.pop();
                }
                else
                    break;
            }
        };

        auto infix_push = [&](const pSymbol &symbol) {
            auto current = symbol;
            auto curr_op = cast<Operator>(current);

            switch (previous->type) {
            case (Type::RIGHT):
            case (Type::CONSTANT):
                if (current->is(Type::RIGHT)) break;
                else if (current->is(Type::SEPARATOR)) break;
                else if (current->is(Type::OPERATOR)) break;
                else throw SyntaxErrorException();
            case (Type::LEFT):
            case (Type::SEPARATOR):
            case (Type::OPERATOR):
                if (current->is(Type::CONSTANT)) break;
                else if (current->is(Type::LEFT)) break;
                else if (current->is(Type::FUNCTION)) break;
                else throw SyntaxErrorException();
            case (Type::FUNCTION):
                if (current->is(Type::LEFT)) break;
                else throw SyntaxErrorException();
            }

            if (
                previous->type == Type::CONSTANT ||
                previous->type == Type::RIGHT
            )
                if (
                    curr_op == nullptr ||
                    curr_op->left_assoc ||
                    curr_op->precedence < cast<Operator>(current)->precedence
                )
                    fill_parens();

            infix.push(current);
            stream << " " << symbol->token;
            previous = current;
        };

        auto is = [&match](Integer group) {
            return !match[group].str().empty();
        };

        while (std::regex_search(expr, match, regex)) {
            auto token = match.str();
            auto it = std::find(_variables.begin(), _variables.end(), token);

            if (is(Group::NUMBER))
                infix_push(make<Constant>(token, std::stod(token)));
            else if (is(Group::NAME) && it != _variables.end()) {
                auto position = it - _variables.begin();
                infix_push(make<Variable>(token, _values.get() + position));
                encountered.emplace(token);
            }
            else if (is(Group::NAME) && defined<Constant>(token))
                infix_push(make<Constant>(token));
            else if (is(Group::NAME) && defined<Function>(token))
                infix_push(make<Function>(token));
            else if (is(Group::SYMBOL) && defined<Operator>(token)) {
                auto op = cast<Operator>(make<Operator>(token));
                if (op->unary.size() == 0)
                    infix_push(make<Operator>(token));
                else {
                    switch (previous->type) {
                    case (Type::RIGHT):
                    case (Type::CONSTANT):
                        infix_push(make<Operator>(token));
                        break;
                    case (Type::LEFT):
                    case (Type::SEPARATOR):
                    case (Type::OPERATOR):
                        infix_push(make<Function>(op->unary));
                        infix_push(make<Parenthesis<'('>>());
                        counter.push(0);
                        break;
                    case (Type::FUNCTION):
                        throw SyntaxErrorException();
                    }
                }
            }
            else if (is(Group::LEFT)) {
                infix_push(make<Parenthesis<'('>>());
                if (!counter.empty())
                    counter.top()++;
            }
            else if (is(Group::RIGHT)) {
                infix_push(make<Parenthesis<')'>>());
                if (!counter.empty())
                    counter.top()--;
            }
            else if (is(Group::SEPARATOR))
                infix_push(make<Separator>());
            else
                throw UndefinedSymbolException(token);

            expr = match.suffix().str();
        }

        if (previous->type == Type::CONSTANT || previous->type == Type::RIGHT)
            fill_parens();

        if (encountered.size() < _variables.size()) {
            for (auto &var : _variables)
                if (encountered.find(var) == encountered.end())
                    throw WrongVariablesException(var);
        }

        switch (infix.back()->type) {
        case (Type::CONSTANT):
        case (Type::RIGHT):
            break;
        default:
            throw SyntaxErrorException();
        }

        _infix = stream.str().erase(0, 1);
        return infix;
    }

    qEvaluable Expression::_shuntingYard(qSymbol &&infix) {
        qEvaluable postfix;
        sSymbol operations;
        pSymbol element, another;
        Stream stream;

        auto postfix_push = [&postfix, &stream](const pEvaluable &symbol) {
            postfix.push(symbol);
            stream << " " << symbol->token;
        };

        while(!infix.empty()) {
            element = infix.front();
            infix.pop();

            switch (element->type) {
            case (Type::CONSTANT):
                postfix_push(cast<Evaluable>(element));
                break;

            case (Type::FUNCTION):
                operations.push(element);
                break;

            case (Type::SEPARATOR):
                while (!operations.empty()) {
                    another = operations.top();
                    if (!another->is(Type::LEFT)) {
                        postfix_push(cast<Evaluable>(another));
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
                        postfix_push(cast<Evaluable>(another));
                        operations.pop();
                        break;
                    }
                    else {
                        auto left = cast<Operator>(element)->left_assoc;
                        auto p1 = cast<Operator>(element)->precedence;
                        auto p2 = cast<Operator>(another)->precedence;
                        if ((left && (p1 <= p2)) || (!left && (p1 < p2))) {
                            operations.pop();
                            postfix_push(cast<Evaluable>(another));
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
                        postfix_push(cast<Evaluable>(another));
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
            postfix_push(cast<Evaluable>(element));
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

            vEvaluable ops(element->args);
            for (auto i = element->args; i > 0; i--) {
                if (operands.empty())
                    throw MissingArgumentsException(element->token);
                ops[i - 1] = operands.top();
                operands.pop();
            }
            element->addBranches(ops);
            operands.push(element);
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
            _values(std::make_unique<Value[]>(vars.size())) {

        if (expr.length() == 0)
            throw EmptyExpressionException();

        for (auto i = 0u; i < vars.size(); i++)
            _values[i] = 0.;

        _tree = _buildTree(_shuntingYard(_tokenize(_expression)));
    }

    Expression& Expression::operator=(const Expression &other) {
        *this = Expression(other);
        
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

    Value Expression::evaluate(const vValue &values) const {
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


    Expression parse(const String &expr) {
        Regex regex(R"_(.*'(.+)'.*)_");
        Match match;
        vString vars;
        String error;

        while (true) {
            try {
                return Expression(expr, vars);
            }
            catch (const UndefinedSymbolException &e) {
                error = e.what();
                std::regex_search(error, match, regex);
                vars.emplace_back(match.str(1));
            }
        }
    }

}
