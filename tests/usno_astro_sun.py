#!/usr/bin/env python
# -*- coding: utf-8 -*-

from ephem_test import *
import math

# Determine whether the astrometric positions of the sun which PyEphem
# predicts fall within one arcsecond of those predicted by the United
# States Naval Observatory.

# Data is from http://aa.usno.navy.mil/data/docs/geocentric.php

usno_data = """
                                    Sun                                   
     
                           Astrometric Positions                          
                    Mean Equator and Equinox of J2000.0                   
     
   Date        Time      Right Ascension     Declination        Distance
        (UT1)  
             h  m   s       h  m   s           °  '   "            AU
2007 Sep 25 00:00:00.0     12 05 19.599     -  0 34 37.58      1.003053293
2007 Sep 26 00:00:00.0     12 08 55.286     -  0 57 59.08      1.002768719
2007 Sep 27 00:00:00.0     12 12 31.140     -  1 21 20.50      1.002484781
2007 Sep 28 00:00:00.0     12 16 07.186     -  1 44 41.53      1.002201616
2007 Sep 29 00:00:00.0     12 19 43.449     -  2 08 01.86      1.001919251
"""

#

import datetime, time
import ephem

sun = ephem.Sun()

def check_against_line(line):
    parts = line.split()
    dt = datetime.datetime(*time.strptime(line[:20], '%Y %b %d %H:%M:%S')[0:6])
    d = ephem.date(dt)
    sun.compute(d)
    pretty_ra = line[27:39].replace(' ', ':')
    pretty_dec = line[44:45] + line[45:57].strip().replace(' ', ':')
    expected_ra = ephem.hours(pretty_ra)
    expected_dec = ephem.degrees(pretty_dec)
    if abs(sun.astrometric_ra - expected_ra) > ephem.arcsecond:
        raise AssertionError('at %s the USNO thinks the sun should be at RA'
                             ' %s but PyEphem thinks %s'
                             % (d, expected_ra, sun.astrometric_ra))
    if abs(sun.astrometric_dec - expected_dec) > ephem.arcsecond:
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
