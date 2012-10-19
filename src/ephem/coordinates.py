"""Coordinate systems."""

from numpy import array, dot, ndarray
from math import asin, atan2, cos, sin, pi, sqrt
from ephem.angles import ASEC2RAD
from ephem.timescales import T0

J2000 = 2451545.0
C_AUDAY = 173.1446326846693

ecliptic_obliquity = (23 + (26/60.) + (21.406/3600.)) * pi / 180.

import de421
from jplephem import Ephemeris
e = Ephemeris(de421)

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

class XYZ(object):

    def __init__(self, position, velocity=None, jd=None):
        self.position = position
        self.velocity = velocity
        self.jd = jd

    def radec(self, epoch=T0):
        if epoch != T0:
            raise NotImplementedError()

        # Geocentric astrometric.

        return GeocentricRADec(self)

class ICRS(XYZ):

    def observe(self, body):
        # TODO: should also accept another ICRS?

        jd = self.jd
        lighttime0 = 0.0
        target = body(jd)
        vector = target.position - self.position
        euclidian_distance = distance = sqrt(vector.dot(vector))

        for i in range(10):
            lighttime = distance / C_AUDAY;
            if -1e-12 < lighttime - lighttime0 < 1e-12:
                break
            lighttime0 = lighttime
            target = body(jd - lighttime)
            vector = target.position - self.position
            distance = sqrt(vector.dot(vector))
        else:
            raise ValueError('observe() light-travel time failed to converge')

        g = GCRS(vector, target.velocity - self.velocity, jd)
        g.distance = euclidian_distance
        g.lighttime = lighttime
        return g

class GCRS(XYZ):
    pass

class GeocentricRADec(ndarray):

    def __new__(cls, other):
        self = ndarray.__new__(cls, (3,))
        if isinstance(other, GCRS):
            x, y, z = other.position
            r = sqrt(x*x + y*y + z*z)
            self[0] = atan2(-y, -x) + pi
            self[1] = asin(z / r)
            self[2] = other.distance
        else:
            raise ValueError('how do I use this? %r' % (other,))
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
        if isinstance(other, ICRS):
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

# 'xi0', 'eta0', and 'da0' are ICRS frame biases in arcseconds taken
# from IERS (2003) Conventions, Chapter 5.

xi0  = -0.0166170
eta0 = -0.0068192
da0  = -0.01460

# Compute elements of rotation matrix to first order the first time this
# function is called.  Elements will be saved for future use and not
# recomputed.

xx =  1.0
yx = -da0  * ASEC2RAD
zx =  xi0  * ASEC2RAD
xy =  da0  * ASEC2RAD
yy =  1.0
zy =  eta0 * ASEC2RAD
xz = -xi0  * ASEC2RAD
yz = -eta0 * ASEC2RAD
zz =  1.0

# Include second-order corrections to diagonal elements.

xx = 1.0 - 0.5 * (yx * yx + zx * zx)
yy = 1.0 - 0.5 * (yx * yx + zy * zy)
zz = 1.0 - 0.5 * (zy * zy + zx * zx)

#

rotation_to_ICRS = array((
    (xx, xy, xz),
    (yx, yy, yz),
    (zx, zy, zz),
    ))

rotation_from_ICRS = array((
    (xx, yx, zx),
    (xy, yy, zy),
    (xz, yz, zz),
    ))
