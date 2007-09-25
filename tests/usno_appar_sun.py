#!/usr/bin/env python
# -*- coding: utf-8 -*-

from ephem_test import *
import math

# Determine whether the apparent positions of the sun which PyEphem
# predicts fall within one arcsecond of those predicted by the United
# States Naval Observatory.

# Data is from http://aa.usno.navy.mil/data/docs/geocentric.php

usno_data = """
                                    Sun                                   
     
                       Apparent Geocentric Positions                      
                     True Equator and Equinox of Date                     
     
   Date        Time          Right       Declination     Distance     Equation
        (UT1)              Ascension                                   of Time
             h  m   s     h  m   s         °  '   "         AU          m   s
2007 Sep 25 00:00:00.0   12 05 42.537   -  0 37 07.31   1.003053293   + 8 02.5
2007 Sep 26 00:00:00.0   12 09 18.224   -  1 00 28.87   1.002768719   + 8 23.4
2007 Sep 27 00:00:00.0   12 12 54.079   -  1 23 50.31   1.002484781   + 8 44.1
2007 Sep 28 00:00:00.0   12 16 30.129   -  1 47 11.33   1.002201616   + 9 04.6
2007 Sep 29 00:00:00.0   12 20 06.402   -  2 10 31.63   1.001919251   + 9 24.8
"""

#

import datetime, time
import ephem

sun = ephem.Sun()

def check_against_line(line):
    parts = line.split()
    dt = datetime.datetime(*time.strptime(line[:20], '%Y %b %d %H:%M:%S')[0:6])
    d = ephem.date(dt)
    sun.compute(d, d)
    pretty_ra = line[25:37].replace(' ', ':')
    pretty_dec = line[40:41] + line[41:53].strip().replace(' ', ':')
    expected_ra = ephem.hours(pretty_ra)
    expected_dec = ephem.degrees(pretty_dec)
    if abs(sun.apparent_ra - expected_ra) > ephem.arcsecond:
        raise AssertionError('at %s the USNO thinks the sun should be at RA'
                             ' %s but PyEphem thinks %s'
                             % (d, expected_ra, sun.astrometric_ra))
    if abs(sun.apparent_dec - expected_dec) > ephem.arcsecond:
        raise AssertionError('at %s the USNO thinks the sun should be at dec'
                             ' %s but PyEphem thinks %s'
                             % (d, expected_dec, sun.astrometric_dec))

class suite(MyTestCase):
    def test_usno(self):
        for line in usno_data.split('\n'):
            if line.startswith('2'):
                check_against_line(line)

if __name__ == '__main__':
    unittest.main()
