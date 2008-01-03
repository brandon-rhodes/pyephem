#!/usr/bin/env python

from ephem_test import unittest, MyTestCase, Angle, degrees, hours
import math

# Determine whether angles work reasonably.

arcsecond = 2. * math.pi / 360. / 60. / 60.

class angle_suite(MyTestCase):
    def setUp(self):
        self.d = degrees(1.5)
        self.h = hours(1.6)

    def test_Angle_constructor(self):
        self.assertRaises(TypeError, Angle, 1.1)

    def test_degrees_constructor(self):
        self.assertApprox(self.d, degrees('85:56:37'), arcsecond)
    def test_degrees_float_value(self):
        self.assertApprox(self.d, 1.5)
    def test_degrees_string_value(self):
        self.assertEqual(str(self.d), '85:56:37.2')

    def test_hours_constructor(self):
        self.assertApprox(self.h, hours('6:06:41.6'), arcsecond)
    def test_hours_float_value(self):
        self.assertApprox(self.h, 1.6)
    def test_hours_string_value(self):
        self.assertEqual(str(self.h), '6:06:41.58')

    def test_angle_addition(self):
        self.assertApprox(degrees('30') + degrees('90'), degrees('120'))
    def test_angle_subtraction(self):
        self.assertApprox(degrees('180') - hours('9'), degrees('45'))

if __name__ == '__main__':
    unittest.main()
