"""Compare the output of PyEphem routines with the same routines from NOVAS."""

from unittest import TestCase
from ephem import timescales
try:
    import novas
    import novas.compat as c
except ImportError:
    novas = None

T0 = timescales.T0
Y1970 = c.julian_date(1970, 1, 1)
Y2012 = c.julian_date(2012, 12, 21)

class NOVASTests(TestCase):

    @classmethod
    def setUpClass(cls):
        if novas is None:
            cls.__unittest_skip__ = True

    def eq(self, first, second):
        self.assertAlmostEqual(first, second, 16)

    def test_tdb_minus_tt(self):
        self.eq(c.tdb2tt(T0)[1], timescales.tdb_minus_tt(T0))
        self.eq(c.tdb2tt(Y1970)[1], timescales.tdb_minus_tt(Y1970))
        self.eq(c.tdb2tt(Y2012)[1], timescales.tdb_minus_tt(Y2012))
