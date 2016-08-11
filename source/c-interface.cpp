#include <cstdarg>
#include <cstring>
#include <limits>

#include "calculate.h"
#include "calculate/c-interface.h"


namespace calculate_c_interface {
    using namespace calculate;

    Expression createExpression(const char *expr, const char *vars,
                                char *error) {
        try {
            auto expr_obj = static_cast<Expression>(
                new Calculate(expr, vars)
            );
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

    void freeExpression(Expression expr_obj) {
        if (expr_obj)
            delete static_cast<Calculate*>(expr_obj);
    }


    const char* getExpression(Expression expr_obj) {
        return expr_obj ?
               static_cast<Calculate*>(expr_obj)->getExpression().c_str() : "";
    }

    int getVariables(Expression expr_obj) {
        return expr_obj ?
               static_cast<int>(
                   static_cast<Calculate*>(expr_obj)->getVariables().size()
               ) : -1;
    }


    double evaluateArray(Expression expr_obj, double *args, int size,
                         char *error) {
        if (!expr_obj)
            return std::numeric_limits<double>::quiet_NaN();

        vValue values(args, args + size);
        try {
            strcpy(error, "");
            return static_cast<Calculate*>(expr_obj)->operator()(values);
        }
        catch (const BaseCalculateException &e) {
            strcpy(error, e.what());
        }

        return std::numeric_limits<double>::quiet_NaN();
    }

    double evalArray(Expression expr_obj, double *args, int size) {
        char error[64];
        return evaluateArray(expr_obj, args, size, error);
    }

    double eval(Expression expr_obj, ...) {
        if (!expr_obj)
            return std::numeric_limits<double>::quiet_NaN();

        auto vars = static_cast<Calculate*>(expr_obj)->getVariables().size();
        vValue values;
        va_list list;
        va_start(list, expr_obj);
        for (auto i = 0u; i < vars; i++)
            values.push_back(va_arg(list, double));
        va_end(list);

        return static_cast<Calculate*>(expr_obj)->operator()(values);
    }

}


extern "C" const calculate_c_library_template Calculate = {
    calculate_c_interface::createExpression,
    calculate_c_interface::newExpression,
    calculate_c_interface::freeExpression,
    calculate_c_interface::getExpression,
    calculate_c_interface::getVariables,
    calculate_c_interface::evaluateArray,
    calculate_c_interface::evalArray,
    calculate_c_interface::eval
};
