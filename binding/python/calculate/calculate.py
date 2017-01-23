# -*- coding: utf-8 -*-
from __future__ import absolute_import

from collections import Iterable

from calculate.utility import ffi, calculate, decode, ERROR_CHARS, MAX_CHARS
from calculate.exceptions import raise_if

__all__ = ['Query', 'Expression']


class Query(object):

    _metadata = [
        'version',
        'author',
        'date'
    ]

    _queries = [
        'constants',
        'operators',
        'functions'
    ]

    def __getattr__(self, item):
        try:
            output = ffi.new('char[{}]'.format(MAX_CHARS))
            if item not in self._metadata and item not in self._queries:
                raise KeyError
            getattr(calculate, item)(output)
            output = decode(output)
            if item in self._queries:
                output = output.split(',') if output else []
            return lambda: output
        except KeyError:
            raise AttributeError(
                '{} object has no attribute {}'
                .format(repr(self.__class__.__name__), repr(item))
            )

    @property
    def methods(self):
        return self._metadata + self._queries


class Expression(object):

    _properties = [
        'expression',
        'variables',
        'infix',
        'postfix',
        'tree'
    ]

    __slots__ = ['_handler']

    def __init__(self, expression, variables=''):
        if not isinstance(variables, str) and isinstance(variables, Iterable):
            variables = ','.join(variables) if len(variables) > 0 else ''
        error = ffi.new('char[{}]'.format(ERROR_CHARS))
        self._handler = calculate.create(
            expression.encode(),
            variables.encode(),
            error
        )
        raise_if(decode(error))

    def __getattr__(self, item):
        try:
            output = ffi.new('char[{}]'.format(MAX_CHARS))
            if item not in self._properties:
                raise KeyError
            getattr(calculate, item)(self._handler, output)
            output = decode(output)
            if item == 'variables':
                output = output.split(',') if output else []
            return lambda: output
        except KeyError:
            raise AttributeError(
                '{} object has no attribute {}'
                .format(repr(self.__class__.__name__), repr(item))
            )

    def evaluate(self, args):
        size = len(args)
        if size > 0:
            args = [float(x) for x in args]
            values = ffi.new('double[]', args)
        else:
            values = ffi.new('double *')
        error = ffi.new('char[{}]'.format(ERROR_CHARS))
        result = calculate.evaluate(self._handler, values, size, error)
        raise_if(decode(error))
        return result

    def __call__(self, *args):
        return self.evaluate(args)

    def __repr__(self):
        return "{}('{}', {})".format(
            self.__class__.__name__,
            self.expression(),
            self.variables()
        )

    def __del__(self):
        try:
            calculate.free(self._handler)
        except Exception:
            pass
        finally:
            self._handler = ffi.NULL


def parse(expression):
    output = ffi.new('char[{}]'.format(MAX_CHARS))
    error = ffi.new('char[{}]'.format(ERROR_CHARS))
    handler = calculate.parse(expression.encode(), error)
    raise_if(decode(error))
    calculate.variables(handler, output)
    return Expression(expression, decode(output))
