#include "calculate.h"

namespace calculate {
    qSymbol Calculate::tokenize(const String &expr) {
        qSymbol infix;

        auto next =
            std::sregex_iterator(
                expr.begin(), expr.end(), _regex
            ),
            end = std::sregex_iterator();

        while (next != end) {
            auto match = next->str();
            auto it = std::find(_names.begin(), _names.end(), match);
            if (it != _names.end()) {
                auto position = it - _names.begin();
                infix.push(symbols::newSymbol(_values.get() + position));
            }
            else {
                infix.push(symbols::newSymbol(match));
            }
            next++;
        }
        return infix;
    }

    qSymbol Calculate::shuntingYard(qSymbol &&infix) {
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
                        operations.pop();
                        break;
                    }
                }
            }
        }

        while(!operations.empty()) {
            auto element = operations.top();
            operations.pop();
            postfix.push(element);
        }
        return postfix;
    }

    pSymbol Calculate::buildTree(qSymbol &&postfix) {
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
                    ops[i - 1] = operands.top();
                    operands.pop();
                }
                function->addBranches(std::move(ops));
                operands.push(element);
            }

            else {
                auto binary = symbols::castChild<symbols::Operator>(element);
                pSymbol a, b;
                b = operands.top();
                operands.pop();
                a = operands.top();
                operands.pop();
                binary->addBranches(a, b);
                operands.push(element);
            }        
        }
        return operands.top();
    }

    Calculate::Calculate(const String &expr, const vString &vars) :
        _names(vars), _values(new double[vars.size()]) {
        for (auto i = 0; i < vars.size(); i++)
            _values[i] = 0.;
        _arg = 0;

        auto regex_string = "-?[0-9.]+|[A-Za-z]+|" +
            symbols::Operator::symbolsRegex();

        for (const String &var : vars)
            regex_string += "|" + var;
        _regex = std::regex(regex_string);

        auto infix = tokenize(expr);
        auto postfix = shuntingYard(std::move(infix));
        _tree = buildTree(std::move(postfix));
    }
}
