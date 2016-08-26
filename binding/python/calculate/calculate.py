import numpy as np

from calculate.cffiwrap import ffi, Calculate
from calculate.exceptions import raise_if


class Expression:

    def __init__(self, expression, variables=''):
        error = ffi.new('char[64]')
        self.__handler = Calculate.createExpression(
            expression.encode(),
            variables.encode(),
            error
        )
        error = ffi.string(error).decode()
        raise_if(self.__handler == ffi.NULL, error)

    @property
    def expression(self):
        return ffi.string(Calculate.getExpression(self.__handler)).decode()

    @property
    def variables(self):
        return ffi.string(Calculate.getVariables(self.__handler)).decode()

    def __call__(self, *args):
        values = np.array(args, dtype=np.double)
        size = len(values)
        if size > 0:
            values = ffi.cast('double *', values.ctypes.data)
        else:
            values = ffi.new('double *')

        error = ffi.new('char[64]')
        result = Calculate.evaluateArray(self.__handler, values, size, error)
        error = ffi.string(error).decode()
        raise_if(error, error)

        return result

    def __del__(self):
        try:
            Calculate.freeExpression(self.__handler)
        except TypeError:
            self.handler = ffi.NULL
