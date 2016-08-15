#include <cstdarg>
#include <cstring>
#include <limits>

#include "calculate.h"
#include "calculate/c-interface.h"

#define cast(expression) reinterpret_cast<Expression>(expression)
#define uncast(expression) reinterpret_cast<Calculate*>(expression)


namespace calculate_c_interface {
    using namespace calculate;

    Expression createExpression(const char *expr, const char *vars,
                                char *error) {
        try {
            auto expr_obj = cast(new Calculate(expr, vars));
            strcpy(error, "");
            return expr_obj;
        }
        catch (const BaseCalculateException &e) {
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


    int compare(Expression one, Expression another) {
        if (one && another)
            return (uncast(one)->operator==(*uncast(another))) ? 1 : 0;

        return -1;
    }

    const char* getExpression(Expression expr) {
        return expr ? uncast(expr)->expression().c_str() : "";
    }

    int getVariables(Expression expr) {
        return expr ? static_cast<int>(uncast(expr)->variables().size()) : -1;
    }


    double evaluateArray(Expression expr, double *args, int size,
                         char *error) {
        if (!expr)
            return std::numeric_limits<double>::quiet_NaN();

        vValue values(args, args + size);
        try {
            strcpy(error, "");
            return uncast(expr)->operator()(values);
        }
        catch (const BaseCalculateException &e) {
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
        vValue values;
        va_list list;
        va_start(list, expr);
        for (auto i = 0u; i < vars; i++)
            values.push_back(va_arg(list, double));
        va_end(list);

        return uncast(expr)->operator()(values);
    }

}


extern "C" const calculate_c_library_template Calculate = {
    calculate_c_interface::createExpression,
    calculate_c_interface::newExpression,
    calculate_c_interface::freeExpression,
    calculate_c_interface::compare,
    calculate_c_interface::getExpression,
    calculate_c_interface::getVariables,
    calculate_c_interface::evaluateArray,
    calculate_c_interface::evalArray,
    calculate_c_interface::eval
};
