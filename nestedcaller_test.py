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

def suite():
    suite = unittest.TestSuite()
    suite.addTests(unittest.makeSuite(Test1))
    return suite
