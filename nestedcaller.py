"""nestedcaller.py - A high-order function to call nested multiple functions.
Fast & Picklable.
"""

__all__ = ['nestedcaller']

class nestedcaller:
    __slots__ = '_funcs',
    def __new__(cls, *args):
        assert type(args) is tuple
        if not all(map(callable, args)):
            raise TypeError('not callable')
        self = super(nestedcaller, cls).__new__(cls)
        a = []
        for func in args:
            if cls != nestedcaller or type(func) != nestedcaller:
                a.append(func)
            else:
                a.extend(func.funcs)
        self._funcs = tuple(a)
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
        return '%s.%s(%s)' % (
                __name__, type(self).__qualname__,
                ', '.join(map(repr, self._funcs)))

try:
    # raise ImportError() # for debug
    from _nestedcaller import nestedcaller
except ImportError:
    from warnings import warn
    warn('import native version of nestedcaller module has failed. using python version')
