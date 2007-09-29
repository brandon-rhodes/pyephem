#!/usr/bin/env python
# -*- coding: utf-8 -*-

import glob, math, re, unittest
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

class Trial(object):
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

    def examine_content(self):
        firstline = self.lines[0]
        name = firstline.strip()
        self.body = self.select_body(name)

    def run(self, content):
        self.content = content
        self.lines = content.split('\n')
        self.examine_content()
        for line in self.lines:
            if line.strip() and line[0].isdigit():
                self.check_line(line)

# Check an "Astrometric Positions" file.

class Astrometric_Trial(Trial):
    def check_line(self, line):
        date, ra, dec = parse(line)

        self.body.compute(date)
        is_near(ra, self.body.astrometric_ra)
        is_near(dec, self.body.astrometric_dec)

# Check an "Apparent Geocentric Positions" file.

class Apparent_Geocentric_Trial(Trial):
    def check_line(self, line):
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

# Check an "Apparent Topocentric Positions" file.

class Apparent_Topocentric_Trial(Trial):
    def examine_content(self):
        Trial.examine_content(self)
        for line in self.lines:
            if 'Location:' in line:
                self.observer = interpret_observer(line.strip())

    def check_line(self, line):
        date, ra, dec = parse(line)
        self.observer.date = date
        self.body.compute(self.observer)
        is_near(ra, self.body.ra)
        is_near(dec, self.body.dec)

# Check a "Rise and Set for the * for *" file.

class Rise_Set_Trial(Trial):
    def examine_content(self):
        name, year = re.search(r'Rise and Set for (.*) for (\d+)',
                               self.content).groups()
        if name.startswith('the '):
            name = name[4:]
        self.body = self.select_body(name)
        self.year = int(year)

        longstr, latstr = re.search(r'Location: (.\d\d\d \d\d), (.\d\d \d\d)',
                                    self.content).groups()
        longstr = longstr.replace(' ', ':').replace('W', '-').strip('E')
        latstr = latstr.replace(' ', ':').replace('S', '-').strip('N')

        self.observer = o = ephem.Observer()
        o.lat = latstr
        o.long = longstr
        o.elevation = 0

        self.error = ephem.minute # error we tolerate in predicted time

        if isinstance(self.body, ephem.Sun):
            o.pressure = 0
            o.horizon = '-0:50' # per USNO instructions for Sun
        elif isinstance(self.body, ephem.Moon):
            o.pressure = 0
            # horizon is set per-day based on moon diameter; see below

    def check_line(self, line):
        """Determine whether PyEphem rising times match those of the USNO.
        Lines from this data file look like:

               Jan.       Feb.       Mar.       Apr.       May    ...
        Day Rise  Set  Rise  Set  Rise  Set  Rise  Set  Rise  Set ...
             h m  h m   h m  h m   h m  h m   h m  h m   h m  h m ...
        01  0743 1740  0734 1808  0707 1834  0626 1858  0549 1921 ...
        02  0743 1741  0734 1809  0705 1835  0624 1859  0548 1922 ...

        Note that times in this kind of file are in Eastern Standard
        Time, so we have to variously add and subtract fives hours
        from our UT times to perform a valid comparison.
        """

        def two_digit_compare(event, usno_digits):
            usno_hrmin = usno_digits[:2] + ':' + usno_digits[2:]
            usno_date = ephem.Date('%s %s' % (usno_datestr, usno_hrmin))

            difference = o.date - (usno_date + 5 * ephem.hour)
            if abs(difference) > self.error:
                raise ValueError('On %s, the USNO thinks that %s %s'
                                 ' happens at %r but PyEphem finds %s'
                                 % (usno_datestr, body.name, event,
                                    usno_hrmin, o.date))

        o = self.observer
        year = self.year
        body = self.body
        day = int(line[0:2])
        for month in [ int(m) for m in range(1,13) ]:
            usno_datestr = '%s/%s/%s' % (year, month, day)

            offset = month * 11 - 7 # where month is in line
            usno_rise = line[offset : offset + 4].strip()
            usno_set = line[offset + 5 : offset + 9].strip()

            start_of_day = ephem.Date((year, month, day)) + 5 * ephem.hour

            o.date = start_of_day
            body.compute(o)
            if isinstance(self.body, ephem.Moon):
                o.horizon = '-00:34' # per USNO instructions for Moon
                o.horizon -= body.size / 2. * ephem.arcsecond

            if usno_rise:
                o.date = start_of_day
                o.next_rising(body)
                two_digit_compare('rise', usno_rise)

            if usno_set:
                o.date = start_of_day
                o.next_setting(body)
                two_digit_compare('set', usno_set)

# The actual function that drives the test.

class Mixin(object):
    def test_usno(self):

        titles = {
            'Astrometric Positions': Astrometric_Trial,
            'Apparent Geocentric Positions': Apparent_Geocentric_Trial,
            'Apparent Topocentric Positions': Apparent_Topocentric_Trial,
            'Rise and Set for': Rise_Set_Trial,
            }

        content = open(self.path).read()
        for title in titles:
            if title in content:
                trial_class = titles[title]
                trial = trial_class()
                trial.run(content)
                return
        raise ValueError('Cannot find a test trial that recognizes %r'
                         % self.path)

#
# Auto-detect files
#

i = 1
for path in glob.glob('tests/usno/*.txt'):
    exec 'class T%d(unittest.TestCase, Mixin): path = %r' % (i, path)
    i += 1

if __name__ == '__main__':
    unittest.main()
