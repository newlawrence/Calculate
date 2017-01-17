#ifndef __CALCULATE_BINDING_H__
#define __CALCULATE_BINDING_H__

#include "calculate/definitions.hpp"

#ifdef _MSC_VER
    #ifdef __COMPILING_LIBRARY__
        #define EXPORT_GLOBAL __declspec(dllexport)
    #else
        #define EXPORT_GLOBAL __declspec(dllimport)
    #endif
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
    void (*version)(Byte*);
    void (*author)(Byte*);
    void (*date)(Byte*);

    void (*constants)(Byte*);
    void (*operators)(Byte*);
    void (*functions)(Byte*);

    Expression (*create)(const Byte*, const Byte*, Byte*);
    Expression (*build)(const Byte*, const Byte*);
    void (*free)(Expression);

    void (*expression)(Expression, Byte*);
    void (*variables)(Expression, Byte*);
    void (*infix)(Expression, Byte*);
    void (*postfix)(Expression, Byte*);
    void (*tree)(Expression, Byte*);

    Value (*evaluate)(Expression, Value*, Integer, Byte*);
    Value (*eval)(Expression, Value*, Integer);
    Value (*value)(Expression, ...);
};

EXPORT_GLOBAL extern const STRUCT calculate_c_library_template calculate_c;
const STRUCT calculate_c_library_template* get_calculate_reference();

#ifdef __cplusplus
}
#else
#define calculate calculate_c
#endif

#endif
