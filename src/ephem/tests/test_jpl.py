#!/usr/bin/env python
# -*- coding: utf-8 -*-

import glob, os.path, re, traceback, unittest
from datetime import datetime
from time import strptime
import ephem

# Read an ephemeris from the JPL, and confirm that PyEphem returns the
# same measurements to within one arcsecond of accuracy.

class JPLTest(unittest.TestCase):

    def runTest(self):
        if not hasattr(self, 'path'):
            return

        in_data = False

        for line in open(self.path):

            if line.startswith('Target body name:'):
                name = line.split()[3]
                if not hasattr(ephem, name):
                    raise ValueError('ephem lacks a body named %r' % name)
                body_class = getattr(ephem, name)
                body = body_class()

            elif line.startswith('$$SOE'):
                in_data = True

            elif line.startswith('$$EOE'):
                in_data = False

            elif in_data:
                date = datetime.strptime(line[1:18], '%Y-%b-%d %H:%M')
                body.compute(date)
                ra = line[23:34]
                dec = line[35:46]
                j = ephem.Jupiter()
                j.compute(date)
                print
                print body.name
                print date, j.a_ra, '|', body.a_ra, ':', ra
                break

re, traceback, datetime, strptime, ephem

def additional_tests():
    suite = unittest.TestSuite()
    for path in glob.glob(os.path.dirname(__file__) + '/jpl/*.txt'):
        case = JPLTest()
        case.path = path
        suite.addTest(case)
    return suite

if __name__ == '__main__':
    unittest.main()
