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

    String extract(const vString &vector) {
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


    void version(Byte *query) {
        strcpy(query, calculate::version().c_str());
    }

    void author(Byte *query) {
        strcpy(query, calculate::author().c_str());
    }

    void date(Byte *query) {
        strcpy(query, calculate::date().c_str());
    }


    void constants(Byte *query) {
        strcpy(query, extract(calculate::constants()).c_str());
    }

    void operators(Byte *query) {
        strcpy(query, extract(calculate::operators()).c_str());
    }

    void functions(Byte *query) {
        strcpy(query, extract(calculate::functions()).c_str());
    }


    Expression create(const Byte *expr, const Byte *vars, Byte *error) {
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

    Expression build(const Byte *expr, const Byte *vars) {
        Byte error[64];

        return create(expr, vars, error);
    }

    void free(Expression expr) {
        if (expr)
            delete uncast(expr);
    }

    void expression(Expression expr, Byte *expression) {
        strcpy(expression, expr ? uncast(expr)->expression().c_str() : "");
    }

    void variables(Expression expr, Byte *variables) {
        const auto &vars = uncast(expr)->variables();
        if (expr)
            strcpy(variables, extract(vars).c_str());
        else
            strcpy(variables, "");
    }

    void infix(Expression expr, Byte *infix) {
        strcpy(infix, expr ? uncast(expr)->infix().c_str() : "");
    }

    void postfix(Expression expr, Byte *postfix) {
        strcpy(postfix, expr ? uncast(expr)->postfix().c_str() : "");
    }

    void tree(Expression expr, Byte *tree) {
        strcpy(tree, expr ? uncast(expr)->tree().c_str() : "");
    }


    Value evaluate(Expression expr, Value *args, Integer size, Byte *error) {
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

    Value eval(Expression expr, Value *args, Integer size) {
        Byte error[64];

        return evaluate(expr, args, size, error);
    }

    Value value(Expression expr, ...) {
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
    version,
    author,
    date,
    constants,
    operators,
    functions,
    create,
    build,
    free,
    expression,
    variables,
    infix,
    postfix,
    tree,
    evaluate,
    eval,
    value
};

const calculate_c_library_template* get_calculate_reference() {
    return &Calculate;
}
