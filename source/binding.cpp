#include <cstdarg>
#include <cstring>
#include <numeric>
#include <limits>

#include "calculate/definitions.hpp"
#include "calculate/binding.h"
#include "calculate.h"

#define cast(expression) reinterpret_cast<Expression>(expression)
#define uncast(expression) reinterpret_cast<calculate::Expression*>(expression)


namespace {

    using namespace calculate_definitions;

    String extract(const std::vector<String> &vector) {
        if (vector.size())
            return std::accumulate(
                vector.begin() + 1,
                vector.end(),
                vector[0],
                [](const String &accumulator, const String &next) {
                     return accumulator + "," + next;
                }
            );
        else
            return String();
    }


    void queryConstants(Byte *query) {
        strcpy(query, extract(calculate::queryConstants()).c_str());
    }

    void queryOperators(Byte *query) {
        strcpy(query, extract(calculate::queryOperators()).c_str());
    }

    void queryFunctions(Byte *query) {
        strcpy(query, extract(calculate::queryFunctions()).c_str());
    }


    Expression createExpression(const Byte *expr, const Byte *vars,
                                Byte *error) {
        try {
            auto expr_obj = cast(new calculate::Expression(expr, vars));
            strcpy(error, "");
            return expr_obj;
        }
        catch (const calculate::BaseCalculateException &e) {
            strcpy(error, e.what());
        }

        return nullptr;
    }

    Expression newExpression(const Byte *expr, const Byte *vars) {
        Byte error[64];

        return createExpression(expr, vars, error);
    }

    void freeExpression(Expression expr) {
        if (expr)
            delete uncast(expr);
    }

    void getExpression(Expression expr, Byte *expression) {
        strcpy(expression, expr ? uncast(expr)->expression().c_str() : "");
    }

    void getVariables(Expression expr, Byte *variables) {
        const auto &vars = uncast(expr)->variables();
        if (expr)
            strcpy(variables, extract(vars).c_str());
        else
            strcpy(variables, "");
    }

    void getInfix(Expression expr, Byte *infix) {
        strcpy(infix, expr ? uncast(expr)->infix().c_str() : "");
    }

    void getPostfix(Expression expr, Byte *postfix) {
        strcpy(postfix, expr ? uncast(expr)->postfix().c_str() : "");
    }

    void getTree(Expression expr, Byte *tree) {
        strcpy(tree, expr ? uncast(expr)->tree().c_str() : "");
    }


    Value evaluateArray(Expression expr, Value *args, int size,
                         Byte *error) {
        if (!expr) {
            strcpy(error, "Not initialized");
            return std::numeric_limits<Value>::quiet_NaN();
        }

        calculate::vValue values(args, args + size);
        try {
            strcpy(error, "");
            return uncast(expr)->operator()(values);
        }
        catch (const calculate::BaseCalculateException &e) {
            strcpy(error, e.what());
        }

        return std::numeric_limits<Value>::quiet_NaN();
    }

    Value evalArray(Expression expr, Value *args, int size) {
        Byte error[64];

        return evaluateArray(expr, args, size, error);
    }

    Value eval(Expression expr, ...) {
        if (!expr)
            return std::numeric_limits<Value>::quiet_NaN();

        auto vars = uncast(expr)->variables().size();
        calculate::vValue values;
        va_list list;
        va_start(list, expr);
        for (auto i = 0u; i < vars; i++)
            values.emplace_back(va_arg(list, Value));
        va_end(list);

        return uncast(expr)->operator()(values);
    }

}


const calculate_c_library_template Calculate = {
    queryConstants,
    queryOperators,
    queryFunctions,
    createExpression,
    newExpression,
    freeExpression,
    getExpression,
    getVariables,
    getInfix,
    getPostfix,
    getTree,
    evaluateArray,
    evalArray,
    eval
};

const calculate_c_library_template* get_calculate_reference() {
    return &Calculate;
}
