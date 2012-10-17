"""Compare the output of PyEphem routines with the same routines from NOVAS."""

from unittest import TestCase
from ephem import earthlib, nutationlib, precessionlib, timescales
try:
    import novas
    import novas.compat as c
    c_nutation = c.nutation
    import novas.compat.nutation  # overwrites nutation() function!
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

    def test_earth_rotation_angle(self):
        self.delta = 1e-12
        self.eq(c.era(T0), timescales.earth_rotation_angle(T0))
        self.eq(c.era(TA), timescales.earth_rotation_angle(TA))
        self.eq(c.era(TB), timescales.earth_rotation_angle(TB))

    def test_earth_tilt(self):
        self.delta = 1e-14
        for a, b in zip(c.e_tilt(T0), earthlib.earth_tilt(T0)):
            self.eq(a, b)
        for a, b in zip(c.e_tilt(TA), earthlib.earth_tilt(TA)):
            self.eq(a, b)
        for a, b in zip(c.e_tilt(TB), earthlib.earth_tilt(TB)):
            self.eq(a, b)

    def test_equation_of_the_equinoxes_complimentary_terms(self):
        self.delta = 1e-23

        self.eq(earthlib.equation_of_the_equinoxes_complimentary_terms(T0),
                c.ee_ct(T0, 0.0, 0))
        self.eq(earthlib.equation_of_the_equinoxes_complimentary_terms(TA),
                c.ee_ct(TA, 0.0, 0))
        self.eq(earthlib.equation_of_the_equinoxes_complimentary_terms(TB),
                c.ee_ct(TB, 0.0, 0))

    def test_fundamental_arguments(self):
        self.delta = 1e-12

        a = earthlib.fundamental_arguments(jcentury(T0))
        b = c.fund_args(jcentury(T0))
        for i in range(5):
            self.eq(a[i], b[i])

        a = earthlib.fundamental_arguments(jcentury(TA))
        b = c.fund_args(jcentury(TA))
        for i in range(5):
            self.eq(a[i], b[i])

        a = earthlib.fundamental_arguments(jcentury(TB))
        b = c.fund_args(jcentury(TB))
        for i in range(5):
            self.eq(a[i], b[i])

    def test_iau2000a(self):
        self.delta = 1e-19

        self.eq(nutationlib.iau2000a(T0)[0], c.nutation.iau2000a(T0, 0.0)[0])
        self.eq(nutationlib.iau2000a(T0)[1], c.nutation.iau2000a(T0, 0.0)[1])

        self.eq(nutationlib.iau2000a(TA)[0], c.nutation.iau2000a(TA, 0.0)[0])
        self.eq(nutationlib.iau2000a(TA)[1], c.nutation.iau2000a(TA, 0.0)[1])

        self.eq(nutationlib.iau2000a(TB)[0], c.nutation.iau2000a(TB, 0.0)[0])
        self.eq(nutationlib.iau2000a(TB)[1], c.nutation.iau2000a(TB, 0.0)[1])

    def test_mean_obliquity(self):
        self.delta = 0

        self.eq(c.mean_obliq(T0), earthlib.mean_obliquity(T0))
        self.eq(c.mean_obliq(TA), earthlib.mean_obliquity(TA))
        self.eq(c.mean_obliq(TB), earthlib.mean_obliquity(TB))

    def test_nutation(self):
        self.delta = 1e-15
        v = [1, 2, 3]

        for a, b in zip(c_nutation(T0, v, direction=0),
                        nutationlib.nutation(T0, v, invert=False)):
            self.eq(a, b)

        for a, b in zip(c_nutation(TA, v, direction=0),
                        nutationlib.nutation(TA, v, invert=False)):
            self.eq(a, b)

        for a, b in zip(c_nutation(TB, v, direction=1),
                        nutationlib.nutation(TB, v, invert=True)):
            self.eq(a, b)

    def test_precess(self):
        self.delta = 1e-15
        v = [1, 2, 3]

        c.precession(T0, v, TA)

        for a, b in zip(c.precession(T0, v, TA),
                        precessionlib.precess(T0, TA, v)):
            self.eq(a, b)

        for a, b in zip(c.precession(TB, v, T0),
                        precessionlib.precess(TB, T0, v)):
            self.eq(a, b)

    def test_sidereal_time(self):
        delta_t = 0.0
        self.delta = 1e-13
        self.eq(c.sidereal_time(T0, 0.0, delta_t, False),
                timescales.sidereal_time(T0, delta_t))
        self.eq(c.sidereal_time(TA, 0.0, delta_t, False),
                timescales.sidereal_time(TA, delta_t))
        self.eq(c.sidereal_time(TB, 0.0, delta_t, False),
                timescales.sidereal_time(TB, delta_t))

    def test_terra(self):
        self.delta = 1e-18

        obs1 = c.make_on_surface(45.0, -75.0, 0.0, 10.0, 1010.0)

        class Obs(object):
            latitude = 45.0
            longitude = -75.0
            elevation = 0.0
        obs2 = Obs()

        for v1, v2 in zip(c.terra(obs1, 11.0), earthlib.terra(obs2, 11.0)):
            for a, b in zip(v1, v2):
                self.eq(a, b)

        for v1, v2 in zip(c.terra(obs1, 23.9), earthlib.terra(obs2, 23.9)):
            for a, b in zip(v1, v2):
                self.eq(a, b)

    def test_tdb_minus_tt(self):
        self.delta = 1e-16
        self.eq(c.tdb2tt(T0)[1], timescales.tdb_minus_tt(T0))
        self.eq(c.tdb2tt(TA)[1], timescales.tdb_minus_tt(TA))
        self.eq(c.tdb2tt(TB)[1], timescales.tdb_minus_tt(TB))

def jcentury(t):
    return (t - T0) / 36525.0
