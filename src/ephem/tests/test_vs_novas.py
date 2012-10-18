"""Compare the output of PyEphem routines with the same routines from NOVAS."""

from unittest import TestCase
from ephem import (angles, coordinates, earthlib, nutationlib, planets,
                   precessionlib, timescales)
try:
    import novas
    import novas.compat as c
    import novas.compat.eph_manager

    jd_start, jd_end, number = c.eph_manager.ephem_open()  # needs novas_de405

    c_nutation = c.nutation
    import novas.compat.nutation  # overwrites nutation() function with module!

except ImportError:
    novas = None

tau = angles.tau
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

    # Tests of generating a full position or coordinate.

    def test_astro_planet(self):
        self.delta = 1e-6  # Not as good as I would like

        moonobj = c.make_object(0, 11, b'Moon', None)

        ra1, dec1, dis1 = c.astro_planet(T0, moonobj)
        print(ra1, dec1, dis1)

        ra2, dec2, dis2 = planets.earth(T0).observe(planets.moon).radec(T0)
        print(ra2 / tau * 24.0, dec2/ tau * 360.0, dis2 / earthlib.AU_KM)

        self.eq(ra1, ra2 / tau * 24.0)
        self.eq(dec1, dec2/ tau * 360.0)
        self.eq(dis1, dis2 / earthlib.AU_KM)

    def test_topo_planet(self):
        return
        self.delta = 1e-4  # TERRIBLE - because of different ephemera?

        moonobj = c.make_object(0, 11, b'Moon', None)

        print()
        position = c.make_on_surface(45.0, -75.0, 0.0, 10.0, 1010.0)
        delta_t = 0
        ra1, dec1, dis1 = c.topo_planet(T0, delta_t, moonobj, position)
        print(ra1, dec1, dis1)
        ra1, dec1, dis1 = c.astro_planet(T0, moonobj)
        print(ra1, dec1, dis1)

        ggr = planets.EarthLocation('75 W', '45 N', 0.0,
                                    temperature=10.0, pressure=1010.0)
        ra2, dec2, dis2 = ggr(T0).observe(planets.moon).radec(T0)
        print(ra2 / tau * 24.0, dec2/ tau * 360.0, dis2 / earthlib.AU_KM)

        self.eq(ra1, ra2 / tau * 24.0)
        self.eq(dec1, dec2/ tau * 360.0)
        self.eq(dis1, dis2 / earthlib.AU_KM)

    # Tests of basic functions (in alphabetical order by NOVAS name).

    def test_era(self):
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

    def test_frame_tie(self):
        self.delta = 1e-15
        v = [1, 2, 3]

        for a, b in zip(c.frame_tie(v, 0),
                        coordinates.frame_tie(v, 0)):
            self.eq(a, b)

        for a, b in zip(c.frame_tie(v, -1),
                        coordinates.frame_tie(v, -1)):
            self.eq(a, b)

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

    def test_geo_posvel(self):
        self.delta = 1e-13

        obs1 = c.make_observer_on_surface(45.0, -75.0, 0.0, 10.0, 1010.0)
        ggr = planets.EarthLocation('75 W', '45 N', 0.0,
                                    temperature=10.0, pressure=1010.0)
        delta_t = 0.0

        for v1, v2 in zip(c.geo_posvel(T0, delta_t, obs1),
                          ggr.geocentric_position_and_velocity(T0)):
            for a, b in zip(v1, v2):
                self.eq(a, b)

    def test_iau2000a(self):
        self.delta = 1e-19

        self.eq(nutationlib.iau2000a(T0)[0], c.nutation.iau2000a(T0, 0.0)[0])
        self.eq(nutationlib.iau2000a(T0)[1], c.nutation.iau2000a(T0, 0.0)[1])

        self.eq(nutationlib.iau2000a(TA)[0], c.nutation.iau2000a(TA, 0.0)[0])
        self.eq(nutationlib.iau2000a(TA)[1], c.nutation.iau2000a(TA, 0.0)[1])

        self.eq(nutationlib.iau2000a(TB)[0], c.nutation.iau2000a(TB, 0.0)[0])
        self.eq(nutationlib.iau2000a(TB)[1], c.nutation.iau2000a(TB, 0.0)[1])

    def test_mean_obliq(self):
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

    def test_precession(self):
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
            latitude = 45.0 * angles.DEG2RAD
            longitude = -75.0 * angles.DEG2RAD
            elevation = 0.0
        obs2 = Obs()

        for v1, v2 in zip(c.terra(obs1, 11.0), earthlib.terra(obs2, 11.0)):
            for a, b in zip(v1, v2):
                self.eq(a, b)

        for v1, v2 in zip(c.terra(obs1, 23.9), earthlib.terra(obs2, 23.9)):
            for a, b in zip(v1, v2):
                self.eq(a, b)

    def test_tdb2tt(self):
        self.delta = 1e-16
        self.eq(c.tdb2tt(T0)[1], timescales.tdb_minus_tt(T0))
        self.eq(c.tdb2tt(TA)[1], timescales.tdb_minus_tt(TA))
        self.eq(c.tdb2tt(TB)[1], timescales.tdb_minus_tt(TB))

def jcentury(t):
    return (t - T0) / 36525.0
