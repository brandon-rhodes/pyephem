#!/usr/bin/env python

import doctest, unittest
from glob import glob

tests = [ doctest.DocFileSuite("../" + path) for path in glob('doc/*.rst') ]

if __name__ == '__main__':
    names = globals().keys()
    names.sort()
    suite = unittest.TestSuite()
    for test in tests:
        suite.addTest(test)
    unittest.TextTestRunner().run(suite)
