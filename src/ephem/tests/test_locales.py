#!/usr/bin/env python

import unittest
import ephem
import locale

# Determine whether we can convert values regardless of locale.

class locales_suite(unittest.TestCase):
    def setUp(self):
        self.old_locale = locale.getlocale(locale.LC_NUMERIC)
        locale.setlocale(locale.LC_NUMERIC, 'de_CH.UTF-8')

    def tearDown(self):
        locale.setlocale(locale.LC_NUMERIC, self.old_locale)

    def test_date_creation(self):
        self.assertEqual(ephem.date('2008.5'), 39629.5) # instead of 2008.0

    def test_satellite_creation(self):
        s = ephem.readtle('ISS (ZARYA)',
                          '1 25544U 98067A   08334.54218750  .00025860  '
                          '00000-0  20055-3 0  7556',
                          '2 25544 051.6425 248.8374 0006898 046.3246 '
                          '303.9711 15.71618375574540594')
        self.assertEqual(str(s._raan), '248:50:14.6') # instead of :00:00.0

if __name__ == '__main__':
    unittest.main()
