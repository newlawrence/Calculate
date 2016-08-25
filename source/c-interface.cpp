#include <cstdarg>
#include <cstring>
#include <numeric>
#include <limits>


#include "calculate.h"
#include "calculate/c-interface.h"

#define cast(expression) reinterpret_cast<Expression>(expression)
#define uncast(expression) reinterpret_cast<calculate::Expression*>(expression)


namespace {

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


    const char* getExpression(Expression expr) {
        return expr ? uncast(expr)->expression().c_str() : "";
    }

    const char* getVariables(Expression expr) {
        static std::string vars;

        const auto &variables = uncast(expr)->variables();
        if (expr && variables.size()) {
            vars = std::accumulate(
                variables.begin() + 1,
                variables.end(),
                variables[0],
                [](const std::string &accumulator, const std::string &next) {
                    return accumulator + "," + next;
                }
            );
        }
        else {
            vars = std::string();
        }
        return vars.c_str();
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
    createExpression,
    newExpression,
    freeExpression,
    getExpression,
    getVariables,
    evaluateArray,
    evalArray,
    eval
};

const calculate_c_library_template* get_calculate_reference() {
    return &Calculate;
}
