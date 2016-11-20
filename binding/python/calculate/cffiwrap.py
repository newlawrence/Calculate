# -*- coding: utf-8 -*-
from __future__ import absolute_import

import os.path as path

from cffi import FFI

extensions = ['so', 'dylib', 'dll']
library_name = 'libcalculate'
for ext in extensions:
    basedir = path.realpath(__file__).replace(path.basename(__file__), '')
    library_path = path.join(basedir, library_name + '.' + ext)
    if path.lexists(library_path):
        break

ffi = FFI()
ffi.cdef('''
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

const struct calculate_c_library_template* get_calculate_reference();
''')
clib = ffi.dlopen(library_path)
Calculate = clib.get_calculate_reference()


def decode(cstring):
    string = ffi.string(cstring)
    return string.decode() if not isinstance(string, str) else string
