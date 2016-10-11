# from distutils.core import setup, Extension
from setuptools import setup, Extension

setup(
    name = 'nestedcaller',
    version = '0.1',
    description = \
        'A high-order function to call nested multiple function',
    ext_modules = [
        Extension('_nestedcaller',
            sources = ['_nestedcallermodule.c'],
            optional = True,
        ),
    ],
    py_modules = ['nestedcaller'],
    test_suite = 'nestedcaller_test.suite',
)
