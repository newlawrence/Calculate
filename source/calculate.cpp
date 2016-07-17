#include <cstdarg>
#include <cstring>
#include <limits>
#include <algorithm>
#include <sstream>

#include "calculate.h"


namespace calculate {

    qSymbol Calculate::tokenize(const String &expr) const {
        qSymbol infix;

        auto next = std::sregex_iterator(expr.begin(), expr.end(), _regex),
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

        return infix;
    }

    qSymbol Calculate::check(qSymbol &&input) const {
        qSymbol output;
        pSymbol current, next;

        if (input.size() == 0)
            throw SyntaxErrorException();

        current = input.front();
        input.pop();
        output.push(current);

        switch (current->type) {
            case (Type::RIGHT):
            case (Type::SEPARATOR):
            case (Type::OPERATOR):
                throw SyntaxErrorException();
            default:
                break;
        }

        while (!input.empty()) {
            next = input.front();
            input.pop();
            output.push(next);

            switch (current->type) {
                case (Type::CONSTANT):
                case (Type::RIGHT):
                    if (next->is(Type::RIGHT)) break;
                    else if (next->is(Type::SEPARATOR)) break;
                    else if (next->is(Type::OPERATOR)) break;
                    else throw SyntaxErrorException();
                case (Type::LEFT):
                case (Type::SEPARATOR):
                case (Type::OPERATOR):
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
            case (Type::LEFT):
            case (Type::SEPARATOR):
            case (Type::OPERATOR):
            case (Type::FUNCTION):
                throw SyntaxErrorException();
            default:
                break;
        }

        return output;
    }

    qSymbol Calculate::shuntingYard(qSymbol &&infix) const {
        qSymbol postfix;
        sSymbol operations;

        while(!infix.empty()) {
            auto element = infix.front();
            infix.pop();

            switch (element->type) {
                case (Type::CONSTANT):
                    postfix.push(element);
                    break;

                case (Type::FUNCTION):
                    operations.push(element);
                    break;

                case (Type::SEPARATOR):
                    while (!operations.empty()) {
                        auto another = operations.top();
                        if (!another->is(Type::LEFT)) {
                            postfix.push(another);
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
                    break;

                case (Type::LEFT):
                    operations.push(element);
                    break;

                case (Type::RIGHT):
                    while (!operations.empty()) {
                        auto another = operations.top();
                        if (!another->is(Type::LEFT)) {
                            operations.pop();
                            postfix.push(another);
                        }
                        else {
                            break;
                        }
                    }
                    if (!operations.empty() &&
                        operations.top()->is(Type::LEFT)
                        )
                        operations.pop();
                    else
                        throw ParenthesisMismatchException();
                    break;

                default:
                    throw symbols::UndefinedSymbolException();
            }
        }

        while(!operations.empty()) {
            auto element = operations.top();
            if (element->is(Type::LEFT))
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

            if (element->is(Type::CONSTANT)) {
                operands.push(element);
            }

            else if (element->is(Type::FUNCTION)) {
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

            else if (element->is(Type::OPERATOR)) {
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

            else
                throw symbols::UndefinedSymbolException();
        }
        if (operands.size() > 1)
            throw ConstantsExcessException();

        return operands.top();
    }


    vString Calculate::extract(const String &vars) {
        auto no_spaces = vars;
        no_spaces.erase(
            std::remove_if(
                no_spaces.begin(),
                no_spaces.end(),
                [](char c) {return c == ' ';}
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

    vString Calculate::validate(const vString &vars) {
        auto regex = std::regex("[A-Za-z]+\\d*");

        if (!std::all_of(vars.begin(), vars.end(),
            [&regex](String var) {return std::regex_match(var, regex);})
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


    Calculate::Calculate(const Calculate &other) :
        Calculate(other.expression, other.variables) {
    }

    Calculate::Calculate(Calculate &&other) :
        _values(new double[other.variables.size()]),
        expression(other.expression), variables(other.variables) {
        this->_regex = other._regex;
        this->_tree = std::move(other._tree);
    }

    Calculate::Calculate(const String &expr, const String &vars) :
        Calculate(expr, extract(vars)) {
    }

    Calculate::Calculate(const String &expr, const vString &vars) :
        _values(new double[vars.size()]),
        expression(expr), variables(validate(vars)) {

        if (expr.length() == 0)
            throw EmptyExpressionException();

        for (auto i = 0u; i < vars.size(); i++)
            _values[i] = 0.;

        _regex = std::regex(
            "-?[0-9.]+(?!e\\+?-?\\d+)|"
            "-?\\d+e-?\\d+|"
            "[A-Za-z]+\\d*[A-Za-z]*|"
            "[,()]|" +
            symbols::Operator::getSymbolsRegex()
        );

        auto infix = check(tokenize(expr));
        auto postfix = shuntingYard(std::move(infix));
        _tree = buildTree(std::move(postfix));
    }


    bool Calculate::operator==(const Calculate &other) const noexcept {
        return this->expression == other.expression;
    }

    double Calculate::operator() () const {
        return _tree->evaluate();
    };

    double Calculate::operator() (double value) const {
        if (variables.size() < 1)
            throw EvaluationException();
        _values[variables.size() - 1] = value;
        return _tree->evaluate();
    }

    double Calculate::operator() (vValue values) const {
        if (values.size() != variables.size())
            throw EvaluationException();
        for (auto i = 0u; i < values.size(); i++)
            _values[i] = values[i];
        return _tree->evaluate();
    }

}


namespace calculate_c_interface {
    using namespace calculate;

    calculate_Expression createExpression(const char *expr, const char *vars,
                                char *errors) {
        try {
            auto expr_obj = static_cast<calculate_Expression>(
                new Calculate(expr, vars)
            );
            strcpy(errors, "");
            return expr_obj;
        }
        catch (const BaseSymbolException &e) {
            strcpy(errors, e.what());
            return nullptr;
        }
    }

    calculate_Expression newExpression(const char *expr, const char *vars) {
        char errors[64];
        return createExpression(expr, vars, errors);
    }

    void freeExpression(calculate_Expression expr_obj) {
        if (expr_obj)
            delete static_cast<Calculate*>(expr_obj);
    }


    const char* getExpression(calculate_Expression expr_obj) {
        return expr_obj ?
               static_cast<Calculate*>(expr_obj)->expression.c_str() : "";
    }

    int getVariables(calculate_Expression expr_obj) {
        return expr_obj ?
               static_cast<int>(
                   static_cast<Calculate*>(expr_obj)->variables.size()
               ) : -1;
    }


    double evaluateArray(calculate_Expression expr_obj, double *values,
                         int size, char *errors) {
        if (!expr_obj)
            return std::numeric_limits<double>::quiet_NaN();

        vValue values_vector(values, values + size);
        try {
            strcpy(errors, "");
            return static_cast<Calculate*>(expr_obj)->operator()(values_vector);
        }
        catch (const BaseSymbolException &e) {
            strcpy(errors, e.what());
            return std::numeric_limits<double>::quiet_NaN();
        }
    }

    double evalArray(calculate_Expression expr_obj, double *values, int size) {
        char errors[64];
        return evaluateArray(expr_obj, values, size, errors);
    }

    double eval(calculate_Expression expr_obj, ...) {
        if (!expr_obj)
            return std::numeric_limits<double>::quiet_NaN();

        auto vars = static_cast<Calculate*>(expr_obj)->variables.size();
        vValue values;
        va_list list;
        va_start(list, expr_obj);
        for (auto i = 0u; i < vars; i++)
            values.push_back(va_arg(list, double));
        va_end(list);

        try {
            return static_cast<Calculate*>(expr_obj)->operator()(values);
        }
        catch (BaseSymbolException) {
            return std::numeric_limits<double>::quiet_NaN();
        }
    }

}


extern "C" const _calculate_c_library* _get_calculate_c_library() {
    static const _calculate_c_library library = {
        calculate_c_interface::createExpression,
        calculate_c_interface::newExpression,
        calculate_c_interface::freeExpression,
        calculate_c_interface::getExpression,
        calculate_c_interface::getVariables,
        calculate_c_interface::evaluateArray,
        calculate_c_interface:: evalArray,
        calculate_c_interface::eval
    };
    return &library;
}
