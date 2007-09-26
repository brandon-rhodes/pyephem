#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math, re, unittest
from datetime import datetime
from time import strptime
import ephem

# Read an ephemeris from the United States Naval Observatory, and
# confirm that PyEphem returns the same measurements to within one
# arcsecond of accuracy.

def interpret_observer(line):
    """Return the Observer represented by a line from a USNO file.
              Location:  W 84°24'36.0", N33°45'36.0",   322m
    """
    fields = line.split()
    observer = ephem.Observer()
    observer.long = ':'.join(re.findall(r'[0-9.]+', fields[2]))
    observer.lat = ':'.join(re.findall(r'[0-9.]+', fields[3]))
    if fields[1] == 'W':
        observer.long *= -1
    if fields[3].startswith('S'):
        observer.lat *= -1
    observer.elevation = float(fields[-1][:-1])
    observer.pressure = 0
    return observer

def parse(line):
    """Read the date, RA, and dec from a USNO data file line."""

    fields = re.split(r'   +', line)
    dt = datetime(*strptime(fields[0][:-2], "%Y %b %d %H:%M:%S")[0:6])
    date = ephem.Date(dt)
    ra = ephem.hours(fields[1].replace(' ', ':'))
    sign, mag = fields[2].split(None, 1)
    dec = ephem.degrees(sign + mag.replace(' ', ':'))
    return date, ra, dec

def is_near(n, m):
    """Raise an exception if two values are not within an arcsecond."""

    if abs(n - m) > ephem.arcsecond:
        raise AssertionError('the USNO asserts the value %s'
                             ' but PyEphem instead returns %s'
                             % (n, m))

class Mixin(object):

    def select_body(self, name):
        if name == 'Deneb':
            return ephem.readdb('Deneb,f|D|A2,20:41:25.91|2.25,'
                                '45:16:49.22|1.6,1.33,2000')
        elif name == 'Antares':
            return ephem.readdb('Antares,f|D|M1,16:29:24.46|-11.4,'
                                '-26:25:55.21|-23.2,1.07,2000')
        elif hasattr(ephem, name):
            return getattr(ephem, name)()
        else:
            raise ValueError('USNO test: unknown body %r' % name)

    # Check a line of an "Astrometric Positions" file.

    def astrometric_positions(self, line):
        date, ra, dec = parse(line)

        self.body.compute(date)
        is_near(ra, self.body.astrometric_ra)
        is_near(dec, self.body.astrometric_dec)

    # Check a line of an "Apparent Geocentric Positions" file.

    def apparent_geocentric_positions(self, line):
        date, ra, dec = parse(line)

        self.body.compute(date)
        is_near(ra, self.body.apparent_ra)
        is_near(dec, self.body.apparent_dec)

        # The values we get should not depend on our epoch.

        self.body.compute(date, epoch=date)
        is_near(ra, self.body.apparent_ra)
        is_near(dec, self.body.apparent_dec)

        # Since we only provided the body with a date, and not an
        # Observer, the computation should have been geocentric, and
        # thus the "ra" and "dec" fields should have had the apparent
        # positions copied into them as well.

        assert self.body.apparent_ra == self.body.ra
        assert self.body.apparent_dec == self.body.dec

    # Check a line of an "Apparent Topocentric Positions" file.

    def apparent_topocentric_positions(self, line):
        date, ra, dec = parse(line)

        self.observer.date = date
        self.body.compute(self.observer)
        is_near(ra, self.body.ra)
        is_near(dec, self.body.dec)

    # The actual function that drives the test.

    def test_usno(self):
        self.body = func = None
        for line in open('tests/usno/' + self.filename):
            stripped = line.strip()
            if not stripped:
                continue
            elif not self.body:
                self.body = self.select_body(stripped)
            elif not func:
                func = getattr(self, stripped.lower().replace(' ', '_'))
            elif stripped.startswith('Location:'):
                self.observer = interpret_observer(stripped)
            elif line and line[0] != ' ':
                func(line)


class T01(unittest.TestCase, Mixin): filename = 'astrom_antares.txt'
class T02(unittest.TestCase, Mixin): filename = 'astrom_mercury.txt'
class T03(unittest.TestCase, Mixin): filename = 'astrom_neptune.txt'

class T11(unittest.TestCase, Mixin): filename = 'appgeo_deneb.txt'
class T12(unittest.TestCase, Mixin): filename = 'appgeo_jupiter.txt'
class T13(unittest.TestCase, Mixin): filename = 'appgeo_moon.txt'
class T14(unittest.TestCase, Mixin): filename = 'appgeo_sun.txt'

class T21(unittest.TestCase, Mixin): filename = 'apptopo_deneb.txt'
class T22(unittest.TestCase, Mixin): filename = 'apptopo_moon.txt'
class T23(unittest.TestCase, Mixin): filename = 'apptopo_sun.txt'

if __name__ == '__main__':
    unittest.main()
