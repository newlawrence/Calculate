#ifndef __CALCULATE_C_INTERFACE_H__
#define __CALCULATE_C_INTERFACE_H__

#include "calculate/definitions.hpp"

#ifdef _MSC_VER
#define EXPORT_GLOBAL __declspec(dllimport)
#else
#define EXPORT_GLOBAL
#endif

#ifdef __cplusplus
extern "C" {
#define STRUCT
#else
#define STRUCT struct
#endif

struct ExpressionClassHandler;
typedef struct ExpressionClassHandler* Expression;

struct calculate_c_library_template {
    void (*queryConstants)(Byte*);
    void (*queryOperators)(Byte*);
    void (*queryFunctions)(Byte*);

    Expression (*createExpression)(const Byte*, const Byte*, Byte*);
    Expression (*newExpression)(const Byte*, const Byte*);
    void (*freeExpression)(Expression);

    void (*getExpression)(Expression, Byte*);
    void (*getVariables)(Expression, Byte*);
    void (*getInfix)(Expression, Byte*);
    void (*getPostfix)(Expression, Byte*);
    void (*getTree)(Expression, Byte*);

    Value (*evaluateArray)(Expression, Value*, int, Byte*);
    Value (*evalArray)(Expression, Value*, int);
    Value (*eval)(Expression, ...);
};

EXPORT_GLOBAL extern const STRUCT calculate_c_library_template Calculate;
const STRUCT calculate_c_library_template* get_calculate_reference();

#ifdef __cplusplus
}
#endif

#endif
