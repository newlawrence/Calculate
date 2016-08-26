from calculate.cffiwrap import ffi, Calculate
from calculate.exceptions import raise_if


class Expression:

    def __init__(self, expression, variables=''):
        if variables.__class__.__name__ != 'str':
            variables = ','.join(variables) if len(variables) > 0 else ''
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
        size = len(args)
        if size > 0:
            values = ffi.new('double[{}]'.format(size))
            for i, arg in enumerate(args):
                values[i] = float(arg)
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
        except Exception:
            pass
        finally:
            self.handler = ffi.NULL
