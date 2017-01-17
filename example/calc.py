#!/usr/bin/env python
from __future__ import absolute_import
from __future__ import print_function

import sys

from calculate import Expression
from calculate.exceptions import BaseCalculateException


if __name__ == '__main__':

    argc = len(sys.argv)
    if argc > 0 and argc % 2 == 0:

        try:
            variables = []
            values = []
            for i, arg in enumerate(sys.argv):
                if i < 2:
                    continue
                elif i % 2 == 0:
                    variables.append(arg)
                else:
                    values.append(float(arg))

            expression = Expression(sys.argv[1], variables)
            result = expression(values)

            print('Expression:')
            print(expression.expression())
            print('Variables:')
            print(','.join(expression.variables()))
            print('Infix notation:')
            print(expression.infix())
            print('Postfix notation:')
            print(expression.postfix())
            print('Expression tree:')
            print(expression.tree())
            print('Result:')
            print(result)

        except BaseCalculateException as e:
            print(e.message)
