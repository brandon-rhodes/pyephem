"""Compare the output of PyEphem routines with the same routines from NOVAS."""

from unittest import TestCase, skip
from ephem import earthlib, nutationlib, timescales
try:
    import novas
    import novas.compat as c
    import novas.compat.nutation
except ImportError:
    novas = None

T0 = timescales.T0
TA = c.julian_date(1969, 7, 20, 20. + 18./60.)  # arbitrary test date
TB = c.julian_date(2012, 12, 21)                # arbitrary test date

class NOVASTests(TestCase):

    delta = 'the delta needs to be specified at the top of each test'

    @classmethod
    def setUpClass(cls):
        if novas is None:
            cls.__unittest_skip__ = True

    def eq(self, first, second):
        self.assertAlmostEqual(first, second, delta=self.delta)

    @skip('not yet implemented')
    def test_earth_tilt(self):
        self.delta = 1e-12
        self.eq(c.e_tilt(T0), earthlib.earth_tilt(T0))
        self.eq(c.e_tilt(TA), earthlib.earth_tilt(TA))
        self.eq(c.e_tilt(TB), earthlib.earth_tilt(TB))

    def test_earth_rotation_angle(self):
        self.delta = 1e-12
        self.eq(c.era(T0), timescales.earth_rotation_angle(T0))
        self.eq(c.era(TA), timescales.earth_rotation_angle(TA))
        self.eq(c.era(TB), timescales.earth_rotation_angle(TB))

    def test_iau2000a(self):
        self.delta = 1e-19

        self.eq(nutationlib.iau2000a(T0)[0], c.nutation.iau2000a(T0, 0.0)[0])
        self.eq(nutationlib.iau2000a(T0)[1], c.nutation.iau2000a(T0, 0.0)[1])

        self.eq(nutationlib.iau2000a(TA)[0], c.nutation.iau2000a(TA, 0.0)[0])
        self.eq(nutationlib.iau2000a(TA)[1], c.nutation.iau2000a(TA, 0.0)[1])

        self.eq(nutationlib.iau2000a(TB)[0], c.nutation.iau2000a(TB, 0.0)[0])
        self.eq(nutationlib.iau2000a(TB)[1], c.nutation.iau2000a(TB, 0.0)[1])

    def test_sidereal_time(self):
        delta_t = 0.0
        self.delta = 1e-13
        self.eq(c.sidereal_time(T0, 0.0, delta_t, False),
                timescales.sidereal_time(T0, delta_t))
        self.eq(c.sidereal_time(TA, 0.0, delta_t, False),
                timescales.sidereal_time(TA, delta_t))
        self.eq(c.sidereal_time(TB, 0.0, delta_t, False),
                timescales.sidereal_time(TB, delta_t))

    def test_tdb_minus_tt(self):
        self.delta = 1e-16
        self.eq(c.tdb2tt(T0)[1], timescales.tdb_minus_tt(T0))
        self.eq(c.tdb2tt(TA)[1], timescales.tdb_minus_tt(TA))
        self.eq(c.tdb2tt(TB)[1], timescales.tdb_minus_tt(TB))
