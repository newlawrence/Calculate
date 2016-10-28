# -*- coding: utf-8 -*-
from __future__ import absolute_import

from collections import Iterable

from calculate.cffiwrap import ffi, Calculate
from calculate.exceptions import raise_if


class Expression(object):

    def __init__(self, expression, variables=''):
        if not isinstance(variables, str) and isinstance(variables, Iterable):
            variables = ','.join(variables) if len(variables) > 0 else ''
        error = ffi.new('char[64]')
        self.__handler = Calculate.createExpression(
            expression.encode(),
            variables.encode(),
            error
        )
        raise_if(Expression.__decode(error))

    @staticmethod
    def __decode(cstring):
        string = ffi.string(cstring)
        return string.decode() if not isinstance(string, str) else string

    @property
    def expression(self):
        return Expression.__decode(Calculate.getExpression(self.__handler))

    @property
    def variables(self):
        vars = Expression.__decode(Calculate.getVariables(self.__handler))
        return vars.split(',') if vars else []

    def __call__(self, *args):
        if args:
            args = args[0] if isinstance(args[0], Iterable) else args
            args = list(map(lambda x: float(x), args))
            values = ffi.new('double[]', args)
        else:
            values = ffi.new('double *')

        error = ffi.new('char[64]')
        size = len(args)
        result = Calculate.evaluateArray(self.__handler, values, size, error)
        raise_if(Expression.__decode(error))

        return result

    def __del__(self):
        try:
            Calculate.freeExpression(self.__handler)
        except Exception:
            pass
        finally:
            self.handler = ffi.NULL
