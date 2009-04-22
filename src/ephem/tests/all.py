# Test everything.  Until nose gets ported to Python 3.0.

import unittest
from ephem.tests.launchpad_236872 import convergence_suite
from ephem.tests.launchpad_244811 import next_rising_suite
from ephem.tests.test_angles import angle_suite
from ephem.tests.test_bodies import body_suite, planet_suite, function_suite
from ephem.tests.test_constants import constant_suite
from ephem.tests.test_dates import date_suite
from ephem.tests.test_locales import locales_suite
from ephem.tests.test_usno import (T1, T2, T3, T4, T5, T6, T7, T8, T9,
                                   T10, T11, T12, T13, T14, T15)
from ephem.tests.test_usno_equinoxes import usno_equinoxes_suite

from ephem.tests import test_jpl
jpl_suite = test_jpl.additional_tests()

from ephem.tests import test_rst
rst_suite = test_rst.additional_tests()

if __name__ == '__main__':
    unittest.main()
