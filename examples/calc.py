#!/usr/bin/env python3
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
            print(expression(values))

        except BaseCalculateException as e:
            print(e.message)
