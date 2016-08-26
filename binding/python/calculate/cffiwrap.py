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
    Expression (*createExpression)(const char*, const char*, char*);
    Expression (*newExpression)(const char*, const char*);
    void (*freeExpression)(Expression);

    const char* (*getExpression)(Expression);
    const char* (*getVariables)(Expression);

    double (*evaluateArray)(Expression, double*, int, char*);
    double (*evalArray)(Expression, double*, int);
    double (*eval)(Expression, ...);
};

const struct calculate_c_library_template* get_calculate_reference();
''')
clib = ffi.dlopen(library_path)
Calculate = clib.get_calculate_reference()
