#include "calculate.h"


namespace calculate {

    qSymbol Calculate::tokenize(const String &expr) const {
        qSymbol infix;

        auto next =
            std::sregex_iterator(
                expr.begin(), expr.end(), _regex
            ),
            end = std::sregex_iterator();

        while (next != end) {
            auto match = next->str();
            auto it = std::find(variables.begin(), variables.end(), match);
            if (it != variables.end()) {
                auto position = it - variables.begin();
                infix.push(symbols::newSymbol(_values.get() + position));
            }
            else {
                infix.push(symbols::newSymbol(match));
            }
            next++;
        }
        if (infix.size() == 0)
            throw symbols::UndefinedSymbolException();

        return infix;
    }

    qSymbol Calculate::shuntingYard(qSymbol &&infix) const {
        qSymbol postfix;
        sSymbol operations;

        while(!infix.empty()) {
            auto element = infix.front();
            infix.pop();

            if (element->is(symbols::Type::CONSTANT)) {
                postfix.push(element);
            }

            else if (element->is(symbols::Type::FUNCTION)) {
                operations.push(element);
            }

            else if (element->is(symbols::Type::SEPARATOR)) {
                while (!operations.empty()) {
                    auto another = operations.top();
                    if (!another->is(symbols::Type::LEFT)) {
                        postfix.push(another);
                        operations.pop();
                    }
                    else {
                        break;
                    }
                }
                if (operations.empty())
                    throw ParenthesisMismatchException();
            }

            else if (element->is(symbols::Type::OPERATOR)) {
                while (!operations.empty()) {
                    auto another = operations.top();
                    if (another->is(symbols::Type::LEFT)) {
                        break;
                    }
                    else if (another->is(symbols::Type::FUNCTION)) {
                        postfix.push(another);
                        operations.pop();
                        break;
                    }
                    else {
                        auto op1 =
                            symbols::castChild<symbols::Operator>(element);
                        auto op2 =
                            symbols::castChild<symbols::Operator>(another);
                        if ((op1->left_assoc &&
                                op1->precedence <= op2->precedence) ||
                            (!op1->left_assoc &&
                                op1->precedence < op2->precedence)
                            ) {
                            operations.pop();
                            postfix.push(another);
                        }
                        else {
                            break;
                        }
                    }
                }
                operations.push(element);
            }

            else if (element->is(symbols::Type::LEFT)) {
                operations.push(element);
            }

            else {
                while (!operations.empty()) {
                    auto another = operations.top();
                    if (!another->is(symbols::Type::LEFT)) {
                        operations.pop();
                        postfix.push(another);
                    }
                    else {
                        break;
                    }
                }
                if (!operations.empty() &&
                    operations.top()->is(symbols::Type::LEFT)
                    )
                    operations.pop();
                else
                    throw ParenthesisMismatchException();
            }
        }

        while(!operations.empty()) {
            auto element = operations.top();
            if (element->is(symbols::Type::LEFT))
                throw ParenthesisMismatchException();
            operations.pop();
            postfix.push(element);
        }
        return postfix;
    }

    pSymbol Calculate::buildTree(qSymbol &&postfix) const {
        sSymbol operands;
        pSymbol element;

        while (!postfix.empty()) {
            element = postfix.front();
            postfix.pop();

            if (element->is(symbols::Type::CONSTANT)) {
                operands.push(element);
            }

            else if (element->is(symbols::Type::FUNCTION)) {
                auto function = symbols::castChild<symbols::Function>(element);
                auto args = function->args;
                vSymbol ops(args);
                for (auto i = args; i > 0; i--) {
                    if (operands.empty())
                        throw MissingArgumentsException();
                    ops[i - 1] = operands.top();
                    operands.pop();
                }
                function->addBranches(std::move(ops));
                operands.push(element);
            }

            else {
                auto binary = symbols::castChild<symbols::Operator>(element);
                pSymbol a, b;
                if (operands.size() < 2)
                    throw MissingArgumentsException();
                b = operands.top();
                operands.pop();
                a = operands.top();
                operands.pop();
                binary->addBranches(a, b);
                operands.push(element);
            }        
        }
        if (operands.size() > 1)
            throw ConstantsExcessException();

        return operands.top();
    }

    Calculate::Calculate(const String &expr, const vString &vars) :
        expression(expr), variables(vars), _values(new double[vars.size()]) {
        if (expr.length() == 0)
            throw EmptyExpressionException();

        for (auto i = 0; i < vars.size(); i++)
            _values[i] = 0.;

        auto regex_string = "-?[0-9.]+|[A-Za-z]+|" +
            symbols::Operator::getSymbolsRegex();

        for (const String &var : vars)
            regex_string += "|" + var;
        _regex = std::regex(regex_string);

        auto infix = tokenize(expr);
        auto postfix = shuntingYard(std::move(infix));
        _tree = buildTree(std::move(postfix));
    }

    Calculate::Calculate(const Calculate &other) :
        Calculate(other.expression, other.variables) {
    }

    Calculate::Calculate(Calculate &&other) :
        expression(other.expression), variables(other.variables),
        _values(new double[other.variables.size()]) {
        this->_regex = other._regex;
        this->_tree = std::move(other._tree);
    }

    bool Calculate::operator==(const Calculate &other) const noexcept {
        return this->expression == other.expression;
    }

}
