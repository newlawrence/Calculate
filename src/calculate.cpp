#include "calculate.h"

namespace calculate {
    qSymbol tokenize(const String &expression) {
        qSymbol infix;

        auto regex = std::regex(
            String("-?[0-9.]+|[A-Za-z]+|") + symbols::Operator::symbolsRegex()
        );
        auto next = std::sregex_iterator(
            expression.begin(), expression.end(), regex
        ),
            end = std::sregex_iterator();

        while (next != end) {
            auto match = *next;
            infix.push(symbols::newSymbol(match.str()));
            next++;
        }
        return infix;
    }


    qSymbol shuntingYard(qSymbol &&infix) {
        qSymbol postfix;
        sSymbol operations;

        while(!infix.empty()) {
            auto element = infix.front();
            infix.pop();

            if (element->is(Type::CONSTANT)) {
                postfix.push(element);
            }

            else if (element->is(Type::FUNCTION)) {
                operations.push(element);
            }

            else if (element->is(Type::SEPARATOR)) {
                while (!operations.empty()) {
                    auto another = operations.top();
                    if (!another->is(Type::LEFTPARENS)) {
                        postfix.push(another);
                        operations.pop();
                    }
                    else {
                        break;
                    }
                }
            }

            else if (element->is(Type::OPERATOR)) {
                while (!operations.empty()) {
                    auto another = operations.top();
                    if (another->is(Type::LEFTPARENS)) {
                        break;
                    }
                    else if (another->is(Type::FUNCTION)) {
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
                             op1->precedence < op2->precedence)) {
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

            else if (element->is(Type::LEFTPARENS)) {
                operations.push(element);
            }

            else {
                while (!operations.empty()) {
                    auto another = operations.top();
                    if (!another->is(Type::LEFTPARENS)) {
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


    double evaluate(qSymbol &&postfix) {
        sSymbol operands;
        pSymbol element;

        while (!postfix.empty()) {
            element = postfix.front();
            postfix.pop();

            if (element->is(Type::CONSTANT)) {
                operands.push(element);
            }

            else if (element->is(Type::FUNCTION)) {
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
        return operands.top()->evaluate();
    }


    double calculate(const String &expression) {
        auto symbols = tokenize(expression);
        symbols = shuntingYard(std::move(symbols));
        return evaluate(std::move(symbols));
    }
}
