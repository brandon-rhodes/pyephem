"""Coordinate systems."""

from numpy import ndarray
from math import asin, atan2, cos, sin, pi, sqrt

ecliptic_obliquity = (23 + (26/60.) + (21.406/3600.)) * pi / 180.

class Hours(float):

    def hms(self):
        n = self if self > 0. else -self
        d, fraction = divmod(n * 12. / pi, 1.)
        m, s = divmod(fraction * 3600., 60.)
        return d if self > 0. else -d, m, s

    def __str__(self):
        return '%d:%02d:%02f' % self.hms()

class Degrees(float):

    def dms(self):
        n = self if self > 0. else -self
        d, fraction = divmod(n * 180. / pi, 1.)
        m, s = divmod(fraction * 3600., 60.)
        return d if self > 0. else -d, m, s

    def __str__(self):
        return '%d:%02d:%02f' % self.dms()

class XYZ(ndarray):

    def __new__(cls, x, y, z):
        self = ndarray.__new__(cls, (3,))
        self[0] = x
        self[1] = y
        self[2] = z
        return self

    @property
    def x(self): return self[0]

    @property
    def y(self): return self[1]

    @property
    def z(self): return self[2]

class HeliocentricXYZ(XYZ):
    pass

class GeocentricXYZ(XYZ):
    pass

class GeocentricRADec(ndarray):

    def __new__(cls, other):
        self = ndarray.__new__(cls, (3,))
        if isinstance(other, GeocentricXYZ):
            x, y, z = other
            self[2] = r = sqrt(x*x + y*y + z*z)
            self[0] = atan2(-y, -x) + pi
            self[1] = asin(z / r)
        else:
            raise ValueError('how do I use that?')
        return self

    @property
    def ra(self): return Hours(self[0])

    @property
    def dec(self): return Degrees(self[1])

    @property
    def r(self): return self[2]

class HeliocentricLonLat(ndarray):

    def __new__(cls, other):
        self = ndarray.__new__(cls, (3,))
        if isinstance(other, HeliocentricXYZ):
            x, y, z = other
            y, z = (
                y * cos(ecliptic_obliquity) + z * sin(ecliptic_obliquity),
                z * cos(ecliptic_obliquity) - y * sin(ecliptic_obliquity),
                )
            self[2] = r = sqrt(x*x + y*y + z*z)
            self[0] = atan2(-y, -x) + pi
            self[1] = asin(z / r)
        else:
            raise ValueError('how do I use that?')
        return self

    @property
    def lon(self): return Degrees(self[0])

    @property
    def lat(self): return Degrees(self[1])

    @property
    def r(self): return self[2]
