# -*- coding: utf-8 -*-
from __future__ import absolute_import

from calculate.calculate import Expression

__all__ = ['Expression']

__version__ = '1.2.0.dev0'
__license__ = 'MIT'
__author__ = 'Alberto Lorenzo'
__email__ = 'alorenzo.md@gmail.com'
__site__ = 'https://github.com/newlawrence/Calculate'
__description__ = 'Little math expression parser'
__date__ = '2017/01/16'


def load():
    import sys
    from calculate.calculate import Query

    for query in Query().queries.keys():
        setattr(sys.modules[__name__], query, getattr(Query(), query))
        __all__.append(query)

load()
del load
del absolute_import
