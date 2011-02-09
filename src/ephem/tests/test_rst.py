#!/usr/bin/env python

import doctest, unittest
from glob import glob
import os.path

def load_tests(loader, tests, pattern):
    return unittest.TestSuite([
            doctest.DocFileSuite('../doc/%s' % os.path.basename(path))
            for path in glob(os.path.dirname(__file__) + '/../doc/*.rst')
            if os.path.split(path)[-1] != 'index.rst'
            # skips time-dependent doctest in index.rst
            ])
