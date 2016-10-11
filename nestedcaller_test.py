import unittest

from nestedcaller import nestedcaller
import pickle

class Test1(unittest.TestCase):
    def test_call(self):
        self.assertEqual('OK', nestedcaller(str.strip, str.upper)('  ok  '))
    def test_repr(self):
        self.assertEqual('nestedcaller.nestedcaller( )'.format(str=str), repr(nestedcaller()))
        self.assertEqual('nestedcaller.nestedcaller({str!r} )'.format(str=str), repr(nestedcaller(str)))
        self.assertEqual('nestedcaller.nestedcaller({0} )'.format(', '.join(map(repr, [str, int]))), repr(nestedcaller(str, int)))
    def test_pickle(self):
        a = nestedcaller(str.strip, str.upper)
        b = pickle.loads(pickle.dumps(a))
        self.assertIs(type(a), type(b))
        self.assertEqual(a.funcs, b.funcs)
    def test_nested(self):
        a = nestedcaller(str, nestedcaller(int, float), tuple,
            nestedcaller(), nestedcaller(dict), nestedcaller(min, max, all, any))
        self.assertEqual(a.funcs, (str, int, float, tuple, dict, min, max, all, any))
    def test_nonfuncerror(self):
        with self.assertRaises(TypeError):
            nestedcaller(str, None, int)
    def test_error_in_funcs(self):
        def f0(n):
            def f1(a):
                if n == 8:
                    raise ValueError('n == 8')
                a.append(n)
                return a
            return f1
        b = []
        nc = nestedcaller(f0(1), f0(2), f0(4), f0(8), f0(16))
        with self.assertRaises(ValueError):
            nc(b)
        self.assertEqual(b, [1, 2, 4])
            

def suite():
    suite = unittest.TestSuite()
    suite.addTests(unittest.makeSuite(Test1))
    return suite
