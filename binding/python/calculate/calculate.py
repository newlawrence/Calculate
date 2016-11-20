# -*- coding: utf-8 -*-
from __future__ import absolute_import

from collections import Iterable

from calculate.cffiwrap import ffi, Calculate, decode
from calculate.exceptions import raise_if


def queryConstants():
    constants = ffi.new('char[4096]')
    Calculate.queryConstants(constants)
    constants = decode(constants)
    return constants.split(',') if constants else []


def queryOperators():
    operators = ffi.new('char[4096]')
    Calculate.queryOperators(operators)
    operators = decode(operators)
    return operators.split(',') if operators else []


def queryFunctions():
    functions = ffi.new('char[4096]')
    Calculate.queryFunctions(functions)
    functions = decode(functions)
    return functions.split(',') if functions else []


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
        raise_if(decode(error))

    @property
    def expression(self):
        _expression = ffi.new('char[256]')
        Calculate.getExpression(self.__handler, _expression)
        return decode(_expression)

    @property
    def variables(self):
        _variables = ffi.new('char[256]')
        Calculate.getVariables(self.__handler, _variables)
        _variables = decode(_variables)
        return _variables.split(',') if _variables else []

    @property
    def infix(self):
        _infix = ffi.new('char[256]')
        Calculate.getInfix(self.__handler, _infix)
        return decode(_infix)

    @property
    def postfix(self):
        _postfix = ffi.new('char[256]')
        Calculate.getPostfix(self.__handler, _postfix)
        return decode(_postfix)

    @property
    def tree(self):
        _tree = ffi.new('char[4096]')
        Calculate.getTree(self.__handler, _tree)
        return decode(_tree)

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
        raise_if(decode(error))

        return result

    def __del__(self):
        try:
            Calculate.freeExpression(self.__handler)
        except Exception:
            pass
        finally:
            self.handler = ffi.NULL
