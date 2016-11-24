#include <cstdarg>
#include <cstring>
#include <numeric>
#include <limits>

#include "calculate.h"
#include "calculate/binding.h"

#define cast(expression) reinterpret_cast<Expression>(expression)
#define uncast(expression) reinterpret_cast<calculate::Expression*>(expression)


namespace {

    std::string extract(const std::vector<std::string> &vector) {
        if (vector.size())
            return std::accumulate(
                vector.begin() + 1,
                vector.end(),
                vector[0],
                [](const std::string &accumulator, const std::string &next) {
                     return accumulator + "," + next;
                }
            );
        else
            return std::string();
    }


    void queryConstants(char *query) {
        strcpy(query, extract(calculate::queryConstants()).c_str());
    }

    void queryOperators(char *query) {
        strcpy(query, extract(calculate::queryOperators()).c_str());
    }

    void queryFunctions(char *query) {
        strcpy(query, extract(calculate::queryFunctions()).c_str());
    }


    Expression createExpression(const char *expr, const char *vars,
                                char *error) {
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

    Expression newExpression(const char *expr, const char *vars) {
        char error[64];

        return createExpression(expr, vars, error);
    }

    void freeExpression(Expression expr) {
        if (expr)
            delete uncast(expr);
    }

    void getExpression(Expression expr, char *expression) {
        strcpy(expression, expr ? uncast(expr)->expression().c_str() : "");
    }

    void getVariables(Expression expr, char *variables) {
        const auto &vars = uncast(expr)->variables();
        if (expr)
            strcpy(variables, extract(vars).c_str());
        else
            strcpy(variables, "");
    }

    void getInfix(Expression expr, char *infix) {
        strcpy(infix, expr ? uncast(expr)->infix().c_str() : "");
    }

    void getPostfix(Expression expr, char *postfix) {
        strcpy(postfix, expr ? uncast(expr)->postfix().c_str() : "");
    }

    void getTree(Expression expr, char *tree) {
        strcpy(tree, expr ? uncast(expr)->tree().c_str() : "");
    }


    double evaluateArray(Expression expr, double *args, int size,
                         char *error) {
        if (!expr) {
            strcpy(error, "Not initialized");
            return std::numeric_limits<double>::quiet_NaN();
        }

        calculate::vValue values(args, args + size);
        try {
            strcpy(error, "");
            return uncast(expr)->operator()(values);
        }
        catch (const calculate::BaseCalculateException &e) {
            strcpy(error, e.what());
        }

        return std::numeric_limits<double>::quiet_NaN();
    }

    double evalArray(Expression expr, double *args, int size) {
        char error[64];

        return evaluateArray(expr, args, size, error);
    }

    double eval(Expression expr, ...) {
        if (!expr)
            return std::numeric_limits<double>::quiet_NaN();

        auto vars = uncast(expr)->variables().size();
        calculate::vValue values;
        va_list list;
        va_start(list, expr);
        for (auto i = 0u; i < vars; i++)
            values.push_back(va_arg(list, double));
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
