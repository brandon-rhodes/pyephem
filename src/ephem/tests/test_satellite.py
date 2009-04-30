# -*- coding: utf-8 -*-

import unittest, ephem

tle_lines = (
    'ISS (ZARYA)             ',
    '1 25544U 98067A   09119.77864163  .00009789  00000-0  76089-4 0  7650',
    '2 25544  51.6397 195.1243 0008906 304.8273 151.9344 15.72498628598335',
    )

class Case(unittest.TestCase):
    def setUp(self):
        self.iss = ephem.readtle(*tle_lines)
        self.atlanta = ephem.city('Atlanta')

    def test_normal_methods(self):
        for which in ['previous', 'next']:
            for event in ['transit', 'antitransit', 'rising', 'setting']:
                method_name = which + '_' + event
                method = getattr(self.atlanta, method_name)
                self.assertRaises(TypeError, method, self.iss)
