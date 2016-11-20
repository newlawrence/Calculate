#include <unordered_set>

#include <algorithm>
#include <sstream>

#include "calculate.h"


namespace calculate {

    qSymbol Expression::_tokenize(const String &expr, const vString &vars,
                                  const pValue &values) {
        qSymbol infix;

        auto prepare = std::regex(
            R"_(([A-Za-z_\d\.)]+\s*[+\-])(?=\d+\.?\d*|\.\d+))_"
        );
        auto regex = std::regex(
            R"_(((?:[+\-])?(?:\d+\.?\d*|\.\d+)+(?:[eE][+\-]?\d+)?)|)_"
            R"_(([A-Za-z_]+[A-Za-z_\d]*)|)_"
            R"_(([^A-Za-z\d(),\s]+)|)_"
            R"_((\()|(\))|(,))_"
        );

        auto expression = std::regex_replace(expr, prepare, "$1 ");
        auto next = std::sregex_iterator(
                expression.begin(),
                expression.end(),
                regex
            ),
            end = std::sregex_iterator();

        auto encountered = std::unordered_set<String>();
        while (next != end) {
            auto match = next->str();
            auto it = std::find(vars.begin(), vars.end(), match);
            if (it != vars.end()) {
                auto position = it - vars.begin();
                infix.push(newSymbol(values.get() + position));
                encountered.emplace(match);
            }
            else {
                try {
                    infix.push(newSymbol(match));
                }
                catch (std::exception) {
                    throw UndefinedSymbolException();
                }
            }
            next++;
        }
        if (encountered.size() < vars.size())
            throw ArgumentsExcessException();

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

        while(!infix.empty()) {
            auto element = infix.front();
            infix.pop();

            switch (element->type) {
            case (Type::CONSTANT):
                postfix.push(castChild<Evaluable>(element));
                break;

            case (Type::FUNCTION):
                operations.push(element);
                break;

            case (Type::SEPARATOR):
                while (!operations.empty()) {
                    auto another = operations.top();
                    if (!another->is(Type::LEFT)) {
                        postfix.push(castChild<Evaluable>(another));
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
                    auto another = operations.top();
                    if (another->is(Type::LEFT)) {
                        break;
                    }
                    else if (another->is(Type::FUNCTION)) {
                        postfix.push(castChild<Evaluable>(another));
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
                    auto another = operations.top();
                    if (!another->is(Type::LEFT)) {
                        operations.pop();
                        postfix.push(castChild<Evaluable>(another));
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
            auto element = operations.top();
            if (element->is(Type::LEFT))
                throw ParenthesisMismatchException();
            operations.pop();
            postfix.push(castChild<Evaluable>(element));
        }
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
                        throw MissingArgumentsException();
                    ops[i - 1] = operands.top();
                    operands.pop();
                }
                function->addBranches(std::move(ops));
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
            throw ArgumentsExcessException();

        return operands.top();
    }


    vString Expression::_extract(const String &vars) {
        auto no_spaces = vars;
        no_spaces.erase(
            std::remove_if(
                no_spaces.begin(),
                no_spaces.end(),
                [](char c) { return c == ' '; }
            ),
            no_spaces.end()
        );

        auto stream = std::istringstream(no_spaces);
        vString variables;

        String item;
        while(std::getline(stream, item, ','))
            variables.push_back(item);

        return variables;
    }

    vString Expression::_validate(const vString &vars) {
        auto regex = std::regex(R"_([A-Za-z_]+[A-Za-z_\d]*)_");

        if (
            !std::all_of(
                vars.begin(),
                vars.end(),
                [&regex](String var) { return std::regex_match(var, regex); }
            )
        )
            throw BadNameException();

        auto no_duplicates = vars;
        std::sort(no_duplicates.begin(), no_duplicates.end());
        no_duplicates.erase(
            std::unique(no_duplicates.begin(), no_duplicates.end()),
            no_duplicates.end()
        );

        if (no_duplicates.size() != vars.size())
            throw DuplicateNameException();

        return vars;
    }


    Expression::Expression(const Expression &other) :
            Expression(other._expression, other._variables) {
    }

    Expression::Expression(Expression &&other) :
            _expression(std::move(other._expression)),
            _variables(std::move(other._variables)),
            _values(std::move(other._values)) {
        _tree = std::move(other._tree);
    }

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

        auto infix = _check(_tokenize(expr, _variables, _values));
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
        _tree = std::move(other._tree);

        return *this;
    }


    double Expression::operator() (const vValue &values) const {
        if (values.size() != _variables.size())
            throw WrongArgumentsException();
        for (auto i = 0u; i < values.size(); i++)
            _values[i] = values[i];

        return _tree->evaluate();
    }

}
