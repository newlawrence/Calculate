# -*- coding: utf-8 -*-
from __future__ import absolute_import

import sys


class BaseCalculateException(Exception):
    default_message = 'Unknown error'

    def __init__(self, message=default_message):
        self.message = message
        super(BaseCalculateException, self).__init__(self.message)

__exceptions = {
    'EmptyExpressionException': 'Empty expression',
    'SyntaxErrorException': 'Syntax error',
    'ParenthesisMismatchException': 'Parenthesis mismatch',
    'UndefinedSymbolException': 'Undefined symbol',
    'BadNameException': 'Unsuitable variable name',
    'DuplicatedNameException': 'Duplicated names',
    'MissingArgumentsException': 'Missing arguments',
    'ArgumentsExcessException': 'Too many arguments',
    'WrongVariablesException': 'Variables mismatch'
}


def __constructor(self, message=''):
    self.message = message if message else self.__class__.default_message
    super(self.__class__, self).__init__(self.message)

for exception, message in __exceptions.items():
    setattr(
        sys.modules[__name__],
        exception,
        type(
            exception,
            (BaseCalculateException,),
            {'__init__': __constructor, 'default_message': message}
        )
    )


def raise_if(error):
    if error:
        for symbol in globals().values():
            if hasattr(symbol, 'default_message'):
                if error.startswith(symbol.default_message):
                    raise symbol(error)
        else:
            raise BaseCalculateException
