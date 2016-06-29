#include "calculate.h"

namespace calculate {
    qSymbol tokenize(const String &expression) {
        qSymbol infix;
        Regex regex(String("-?[0-9.]+|[A-Za-z]+|") + symbols::symbolsRegex());

        iRegex next(expression.begin(), expression.end(), regex), end;
        while (next != end) {
            Match match = *next;
            infix.push(symbols::newSymbol(match.str()));
            next++;
        }
        return infix;
    }


    qSymbol shuntingYard(qSymbol &&infix) {
        qSymbol postfix;
        sSymbol operations;
        pSymbol element, another;

        while(!infix.empty()) {
            element = infix.front();
            infix.pop();

            if (element->is(Type::CONSTANT)) {
                postfix.push(element);
            }

            else if (element->is(Type::FUNCTION)) {
                operations.push(element);
            }

            else if (element->is(Type::SEPARATOR)) {
                while (!operations.empty()) {
                    another = operations.top();
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
                    another = operations.top();
                    if (another->is(Type::LEFTPARENS)) {
                        break;
                    }
                    else if (another->is(Type::FUNCTION)) {
                        postfix.push(another);
                        operations.pop();
                        break;
                    }
                    else {
                        auto op1 = symbols::castOperator(element);
                        auto op2 = symbols::castOperator(another);
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
                    another = operations.top();
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
            element = operations.top();
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
                auto args = symbols::castFunction(element)->args;
                vSymbol ops(args);
                for (size_t i = args; i > 0; i--) {
                    ops[i - 1] = operands.top();
                    operands.pop();
                }
                symbols::castFunction(element)->addBranches(std::move(ops));
                operands.push(element);
            }

            else {
                pSymbol op1, op2;
                op2 = operands.top();
                operands.pop();
                op1 = operands.top();
                operands.pop();
                symbols::castOperator(element)->addBranches(op1, op2);
                operands.push(element);
            }        
        }
        return operands.top()->evaluate();
    }


    double calculate(const String &expression) {
        qSymbol symbols = tokenize(expression);
        symbols = shuntingYard(std::move(symbols));
        return evaluate(std::move(symbols));
    }
}
