#!/usr/bin/env python

import sys, unittest
from glob import glob
from sets import Set

class TestError(Exception):
    pass

# Work backwords from the `test' directory in which this script sits
# to find where the distutils have placed the new module; note that
# this attempt to file the `lib.*' directory will fail if the user has
# created several by building the module for several architectures.

(build_lib,) = glob(sys.path[0] + '/../build/lib.*')
sys.path.insert(0, build_lib)

# Now we can import the module.

import ephem
from ephem import *
#import test_ephem

class MyTestCase(unittest.TestCase):
    def assertRaises(self, exception, callable, *args):
        try:
            unittest.TestCase.assertRaises(self, exception, callable, *args)
        except AssertionError:
            raise AssertionError, ('%r failed to raise %s with arguments %r'
                                   % (callable, exception, args))

#print dir(ephem)
#print ephem.Date, ephem.Body
#sys.exit(0)

class angles(unittest.TestCase):
    def setUp(self):
        self.d = degrees(1.5)
        self.h = hours(1.6)

    def test_degrees_constructor(self):
        self.assertEqual(self.d, degrees('85:56:37.20937064454'))
    def test_degrees_float_value(self):
        self.assertEqual(self.d, 1.5)
    def test_degrees_string_value(self):
        self.assertEqual(str(self.d), '85:56:37.21')

    def test_hours_constructor(self):
        self.assertEqual(self.h, hours('6:06:41.579333023612'))
    def test_hours_float_value(self):
        self.assertEqual(self.h, 1.6)
    def test_hours_string_value(self):
        self.assertEqual(str(self.h), '6:06:41.58')

#
#

class dates(unittest.TestCase):
    def setUp(self):
        self.date = Date('2004/09/04 00:17:15.8')

    def test_date_constructor(self):
        std = ('2004/09/04 00:17:15.8',)
        pairs = [std, ('2004.67489614324023472',),
                 std, ('2004/9/4.0119884259259257',),
                 std, ('2004/9/4 0.28772222222222221',),
                 std, ('2004/9/4 0:17.263333333333332',),
                 std, ('2004/9/4 0:17:15.8',),
                 ('2004',), ((2004,),),
                 ('2004/09',), ((2004, 9),),
                 std, ((2004, 9, 4.0119884259259257),),
                 std, ((2004, 9, 4, 0.28772222222222221),),
                 std, ((2004, 9, 4, 0, 17.263333333333332),),
                 std, ((2004, 9, 4, 0, 17, 15.8),),
                 ]
        for i in range(0, len(pairs), 2):
            args1, args2 = pairs[i:i+2]
            d1, d2 = Date(*args1), Date(*args2)
            self.assert_(-1e-15 < (d1 / d2 - 1) < 1e-15,
                         'dates not equal:\n %r = date%r\n %r = date%r'
                         % (d1.tuple(), args1, d2.tuple(), args2))

    def test_date_string_value(self):
        self.assertEqual(str(self.date), '2004/9/4 00:17:15')

    def test_date_triple_value(self):
        self.assertEqual(self.date.triple(), (2004, 9, 4.0119884259256651))

    def test_date_triple_value(self):
        self.assertEqual(self.date.tuple(),
                         (2004, 9, 4, 0, 17, 15.799999977461994))

# fixed

saturn_attributes = ('earth_tilt', 'sun_tilt')

satellite_attributes = ('sublat', 'sublong', 'elevation',
                        'range', 'range_velocity', 'eclipsed')

attribute_list = (
    (Body, False,
     ('ra', 'dec', 'elong', 'mag', 'size')),
    (Body, True,
     ('az', 'alt', 'apparent_ra', 'apparent_dec',
      'rise_time', 'rise_az', 'transit_time', 'transit_alt',
      'set_time', 'set_az')),
    (Planet, False,
     ('hlong', 'hlat', 'sun_distance', 'earth_distance', 'phase')),
    (Moon, False,
     ('colong', 'subsolar_lat', 'libration_lat', 'libration_long')),
    (Saturn, False,
     ('earth_tilt', 'sun_tilt')),
    )

class bodies(MyTestCase):
    def setUp(self):
        self.o = o = Observer()
        o.lat, o.long, o.elev = '33:45:10', '-84:23:37', 320.0
        o.date = '1997/2/15'

    # Return a dictionary whose keys are all known body attributes,
    # and whose values are the exceptions we expect to receive for
    # trying to access each attribute from the given body, or the
    # value True for attributes which should not raise exceptions.

    def predict_attributes(self, body, was_computed, was_given_observer):
        predictions = {}
        for bodytype, needs_observer, attrs in attribute_list:
            for attr in attrs:
                if not isinstance(body, bodytype):
                    predictions[attr] = AttributeError
                elif not was_computed:
                    predictions[attr] = RuntimeError
                elif needs_observer and not was_given_observer:
                    predictions[attr] = RuntimeError
                else:
                    predictions[attr] = None
        return predictions

    # Try accessing each attribute in attribute_list from the given
    # body, recording the exceptions we receive in a dictionary, and
    # storing True for attributes whose access raises no exception.

    def measure_attributes(self, body):
        attributes = {}
        for bodytype, needs_observer, attrs in attribute_list:
            for attr in attrs:
                try:
                    getattr(body, attr)
                except:
                    attributes[attr] = sys.exc_info()[1]
                else:
                    attributes[attr] = None
        return attributes

    # Use the above two functions to predict which attributes of the
    # given body should be accessible, and then test to see whether
    # the reality matches our prediction.

    def compare_attributes(self, body, was_computed, was_given_observer):
        p = self.predict_attributes(body, was_computed, was_given_observer)
        t = self.measure_attributes(body)
        for a in Set(p).union(t):
            if p[a] is None and t[a] is None:
                continue
            if p[a] and isinstance(t[a], p[a]):
                continue
            if was_computed:
                if was_given_observer:
                    adjective = 'topo'
                else:
                    adjective = 'geo'
                adjective += 'centrically computed'
            else:
                adjective = 'uncomputed'
            raise TestError('accessing %s of %s %s '
                            'raised %r "%s" instead of %r'
                            % (a, adjective, body,
                               t[a], t[a].args[0], p[a]))

    def run(self, body):
        self.compare_attributes(body, False, False)
        body.compute()
        self.compare_attributes(body, True, False)
        body.compute(self.o)
        self.compare_attributes(body, True, True)
        body.compute()
        self.compare_attributes(body, True, False)

    def test_Planets(self):
        for init in (Mercury, Venus, Mars, Jupiter, Saturn, Uranus,
                     Neptune, Pluto, Sun, Moon):
            self.run(init())

    def test_Fixed(self):
        fb = FixedBody()
        fb._epoch, fb._ra, fb._dec = '2000', '1:30', '15:00'
        self.run(fb)

if __name__ == '__main__':
    unittest.main()
