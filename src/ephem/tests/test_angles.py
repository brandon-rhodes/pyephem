#!/usr/bin/env python

import unittest
import math
from ephem import Angle, degrees, hours

# Determine whether angles work reasonably.

arcsecond = 2. * math.pi / 360. / 60. / 60.

class AngleTests(unittest.TestCase):
    def setUp(self):
        self.d = degrees(1.5)
        self.h = hours(1.6)

    def test_Angle_constructor(self):
        self.assertRaises(TypeError, Angle, 1.1)

    def test_degrees_constructor(self):
        self.assertAlmostEqual(self.d, degrees('85:56:37'), delta=arcsecond)
    def test_degrees_float_value(self):
        self.assertAlmostEqual(self.d, 1.5)
    def test_degrees_string_value(self):
        self.assertEqual(str(self.d), '85:56:37.2')

    def test_hours_constructor(self):
        self.assertAlmostEqual(self.h, hours('6:06:41.6'), delta=arcsecond)
    def test_hours_float_value(self):
        self.assertAlmostEqual(self.h, 1.6)
    def test_hours_string_value(self):
        self.assertEqual(str(self.h), '6:06:41.58')

    def test_angle_addition(self):
        self.assertAlmostEqual(degrees('30') + degrees('90'), degrees('120'))
    def test_angle_subtraction(self):
        self.assertAlmostEqual(degrees('180') - hours('9'), degrees('45'))
