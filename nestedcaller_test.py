import unittest

from nestedcaller import nestedcaller

class TestSample(unittest.TestCase):
    def test_call(self):
        self.assertEqual('OK', nestedcaller(str.strip, str.upper)('  ok  '))
    def test_repr(self):
        self.assertEqual('nestedcaller.nestedcaller({str!r} )'.format(str=str), repr(nestedcaller(str)))

def suite():
    suite = unittest.TestSuite()
    suite.addTests(unittest.makeSuite(TestSample))
    return suite
