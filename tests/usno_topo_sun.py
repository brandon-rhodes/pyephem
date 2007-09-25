#!/usr/bin/env python
# -*- coding: utf-8 -*-

from ephem_test import *
import math

# Determine whether the topocentric positions of the sun which PyEphem
# predicts fall within one arcsecond of those predicted by the United
# States Naval Observatory.

# Data is from http://aa.usno.navy.mil/data/docs/geocentric.php

usno_data = """
                                    Sun                                   
     
                      Apparent Topocentric Positions                      
                     True Equator and Equinox of Date                     
     
                             ATLANTA, GEORGIA                             
              Location:  W 84°24'36.0", N33°45'36.0",   322m              
                (Longitude referred to Greenwich meridian)                
     
   Date        Time      Right Ascension     Declination        Distance
        (UT1)  
             h  m   s       h  m   s           °  '   "            AU
2007 Sep 25 00:00:00.0     12 05 42.053     -  0 37 12.14      1.003058239
2007 Sep 26 00:00:00.0     12 09 17.739     -  1 00 33.71      1.002773878
2007 Sep 27 00:00:00.0     12 12 53.594     -  1 23 55.14      1.002490152
2007 Sep 28 00:00:00.0     12 16 29.644     -  1 47 16.15      1.002207199
2007 Sep 29 00:00:00.0     12 20 05.917     -  2 10 36.45      1.001925045
"""

#

import datetime, time
import ephem

sun = ephem.Sun()

def check_against_line(line):
    parts = line.split()
    dt = datetime.datetime(*time.strptime(line[:20], '%Y %b %d %H:%M:%S')[0:6])
    d = ephem.date(dt)

    o = ephem.Observer()
    o.date = d
    o.long = '-84:24:36.0'
    o.lat = '33:45:36.0'
    o.elevation = 322.0
    o.pressure = 0

    sun.compute(o)
    pretty_ra = line[27:39].replace(' ', ':')
    pretty_dec = line[44:45] + line[45:57].strip().replace(' ', ':')
    expected_ra = ephem.hours(pretty_ra)
    expected_dec = ephem.degrees(pretty_dec)
    if abs(sun.ra - expected_ra) > 2 * ephem.arcsecond:
        raise AssertionError('at %s the USNO thinks the sun should be at RA'
                             ' %s but PyEphem thinks %s'
                             % (d, expected_ra, sun.astrometric_ra))
    if abs(sun.dec - expected_dec) > ephem.arcsecond:
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
