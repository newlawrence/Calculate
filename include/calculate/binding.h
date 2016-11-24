#ifndef __CALCULATE_C_INTERFACE_H__
#define __CALCULATE_C_INTERFACE_H__

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
    void (*queryConstants)(char*);
    void (*queryOperators)(char*);
    void (*queryFunctions)(char*);

    Expression (*createExpression)(const char*, const char*, char*);
    Expression (*newExpression)(const char*, const char*);
    void (*freeExpression)(Expression);

    void (*getExpression)(Expression, char*);
    void (*getVariables)(Expression, char*);
    void (*getInfix)(Expression, char*);
    void (*getPostfix)(Expression, char*);
    void (*getTree)(Expression, char*);

    double (*evaluateArray)(Expression, double*, int, char*);
    double (*evalArray)(Expression, double*, int);
    double (*eval)(Expression, ...);
};

EXPORT_GLOBAL extern const STRUCT calculate_c_library_template Calculate;
const STRUCT calculate_c_library_template* get_calculate_reference();

#ifdef __cplusplus
}
#endif

#endif
