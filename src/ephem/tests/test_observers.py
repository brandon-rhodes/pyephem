#!/usr/bin/env python

from ephem import Observer
from unittest import TestCase

class observer_suite(TestCase):
    def test_pressure_at_sea_level(self):
        o = Observer()
        o.elevation = 0
        o.compute_pressure()
        self.assertEqual(o.pressure, 1013.25)

    def test_pressure_at_11km(self):
        o = Observer()
        o.elevation = 11e3
        o.compute_pressure()
        assert 226.31 < o.pressure < 226.33
