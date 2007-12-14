#!/usr/bin/env python

import doctest, unittest
from glob import glob
import os.path

def additional_tests():
    return unittest.TestSuite([
            doctest.DocFileSuite('../doc/%s' % os.path.basename(path))
            for path in glob(os.path.dirname(__file__) + '/../doc/*.rst')
            ])

if __name__ == '__main__':
    unittest.run()
