# -*- coding: utf-8 -*-
from __future__ import absolute_import

import os.path as path
from platform import system

from cffi import FFI

ERROR_CHARS = 64
MAX_CHARS = 4096

library = 'calculate'
extensions = {'Linux': 'so', 'Darwin': 'dylib', 'Windows': 'dll'}
extension = extensions.get(system(), '')
library_name = 'lib' + library + '_python'
basedir = path.abspath(path.dirname(__file__))
library_path = path.join(basedir, library_name + '.' + extension)
if not path.lexists(library_path):
    raise EnvironmentError('Missing shared library')

ffi = FFI()
ffi.cdef('''
struct ExpressionClassHandler;
typedef struct ExpressionClassHandler* Expression;

struct calculate_c_library_template {
    void (*constants)(char*);
    void (*operators)(char*);
    void (*functions)(char*);

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
calculate = clib.get_calculate_reference()


def decode(cstring):
    string = ffi.string(cstring)
    return string.decode() if not isinstance(string, str) else string
