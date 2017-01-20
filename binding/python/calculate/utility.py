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
    void (*version)(char*);
    void (*author)(char*);
    void (*date)(char*);

    void (*constants)(char*);
    void (*operators)(char*);
    void (*functions)(char*);

    Expression (*create)(const char*, const char*, char*);
    Expression (*build)(const char*, const char*);
    Expression (*parse)(const char*, char*);
    void (*free)(Expression);

    void (*expression)(Expression, char*);
    void (*variables)(Expression, char*);
    void (*infix)(Expression, char*);
    void (*postfix)(Expression, char*);
    void (*tree)(Expression, char*);

    double (*evaluate)(Expression, double*, int, char*);
    double (*eval)(Expression, double*, int);
    double (*value)(Expression, ...);
};

const struct calculate_c_library_template* get_calculate_reference();
''')
clib = ffi.dlopen(library_path)
calculate = clib.get_calculate_reference()


def decode(cstring):
    string = ffi.string(cstring)
    return string.decode() if not isinstance(string, str) else string
