# -*- coding: utf-8 -*-
from __future__ import absolute_import

from calculate.calculate import Expression, parse

__all__ = ['Expression', 'parse']

__version__ = '1.2.4'
__license__ = 'MIT'
__author__ = 'Alberto Lorenzo'
__email__ = 'alorenzo.md@gmail.com'
__site__ = 'https://github.com/newlawrence/Calculate'
__description__ = 'Little math expression parser'
__date__ = '2017/05/28'


def load():
    import sys
    from calculate.calculate import Query

    for method in Query().methods:
        setattr(sys.modules[__name__], method, getattr(Query(), method))
        __all__.append(method)

load()
del load
del absolute_import
