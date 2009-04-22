#!/usr/bin/env python
# -*- coding: utf-8 -*-

from .ephem_test import unittest, MyTestCase
import ephem

# See whether asking for the rising-time of Mars hangs indefinitely.

class convergence_suite(MyTestCase):
    def runTest(self):
        mars = ephem.Mars()
        boston = ephem.city('Boston')
        boston.date = ephem.Date('2008/5/29 15:59:16')
        boston.next_rising(mars)

if __name__ == '__main__':
    unittest.main()
