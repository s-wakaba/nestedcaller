"""nestedcaller.py - A high-order function to call nested multiple functions.
Fast & Picklable.
"""

__all__ = ['nestedcaller']

class nestedcaller:
    __slots__ = '_funcs',
    def __new__(cls, *args):
        assert type(args) is tuple
        self = super(nestedcaller, cls).__new__(cls)
        self._funcs = args
        return self
    @property
    def funcs(self):
        return self._funcs
    def __reduce__(self):
        return type(self), self._funcs
    def __call__(self, x):
        from functools import reduce
        return reduce(lambda x, f: f(x), self._funcs, x)
    def __repr__(self):
        return '%s(%s)' % (type(self).__qualname__,
                ', '.join(map(repr, self._funcs)))

try:
    from _nestedcaller import nestedcaller
except ImportError:
    pass
