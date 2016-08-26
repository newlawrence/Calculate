import sys


class BaseCalculateException(Exception):
    default_message = 'Unknown error'

    def __init__(self, message=default_message):
        super().__init__(message)

__exceptions = {
    'EmptyExpressionException': 'Empty expression',
    'UndefinedSymbolException': 'Undefined symbol',
    'BadNameException': 'Unsuitable variable name',
    'DuplicateNameException': 'Duplicated names',
    'ParenthesisMismatchException': 'Parenthesis mismatch',
    'MissingArgumentsException': 'Missing arguments',
    'ArgumentsExcessException': 'Too many arguments',
    'SyntaxErrorException': 'Syntax error',
    'WrongArgumentsException': 'Arguments mismatch'
}


def __constructor(self, message=''):
    if not message:
        message = self.__class__.default_message
    super(self.__class__, self).__init__(message)

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


def raise_if(condition, error):
    if condition:
        for name, obj in globals().items():
            if hasattr(globals()[name], 'default_message'):
                if error == globals()[name].default_message:
                    raise globals()[name]
        else:
            raise BaseCalculateException
