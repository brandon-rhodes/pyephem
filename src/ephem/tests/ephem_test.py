#!/usr/bin/env python

import unittest

class TestError(Exception):
    pass

# Import the module.

from ephem import *

# Improve the standard TestCase class by providing a routine to check
# whether a function call returns a desired exception, and a routine
# to determine whether two floating-point numbers are very close to
# the same value.

class MyTestCase(unittest.TestCase):
    def assertRaises(self, exception, callable, *args):
        try:
            unittest.TestCase.assertRaises(self, exception, callable, *args)
        except AssertionError:
            raise AssertionError, ('%r failed to raise %s with arguments %r'
                                   % (callable, exception, args))

    def assertApprox(self, n, m, tolerance=None):
        if tolerance is None:
            if abs(1. - n / m) > 1e-10:
                raise AssertionError, ('%r is not close to %r' % (n, m))
        else:
            if abs(n - m) > tolerance:
                raise AssertionError, ('%r and %r differ by %r > %r'
                                       % (n, m, abs(n - m), tolerance))
