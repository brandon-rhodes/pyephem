"""Coordinate systems."""

from numpy import ndarray
from math import asin, atan2, cos, sin, pi, sqrt
from ephem.angles import ASEC2RAD
from ephem.timescales import T0

J2000 = 2451545.0

ecliptic_obliquity = (23 + (26/60.) + (21.406/3600.)) * pi / 180.

import de421
from jplephem import Ephemeris
e = Ephemeris(de421)

days_for_light_to_go_1m = 1. / (e.CLIGHT * 60. * 60. * 24.)

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

    def __new__(cls, x, y, z, dx, dy, dz):
        self = ndarray.__new__(cls, (6,))
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
        vector = body(jd) - self

        for i in range(10):
            lighttime = sqrt(vector.dot(vector)) * days_for_light_to_go_1m
            if -1e-12 < lighttime - lighttime0 < 1e-12:
                break
            lighttime0 = lighttime
            vector = body(jd - lighttime) - self
        else:
            raise ValueError('observe() light-travel time failed to converge')

        g = GeocentricXYZ(*vector)
        g.lighttime = lighttime
        return g

class GeocentricXYZ(XYZ):
    pass

class GeocentricRADec(ndarray):

    def __new__(cls, other):
        self = ndarray.__new__(cls, (3,))
        if isinstance(other, GeocentricXYZ):
            x, y, z, dx, dy, dz = other
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

def frame_tie(pos1, direction):
    """Transform a vector from the dynamical reference system to the ICRS.

    Or vice versa.  The dynamical reference system is based on the
    dynamical mean equator and equinox of J2000.0.  The ICRS is based on
    the space-fixed ICRS axes defined by the radio catalog positions of
    several hundred extragalactic objects.

    """
    # Perform the rotation in the sense specified by 'direction'.

    if direction < 0:
        # Perform rotation from dynamical system to ICRS.

        return [xx * pos1[0] + yx * pos1[1] + zx * pos1[2],
                xy * pos1[0] + yy * pos1[1] + zy * pos1[2],
                xz * pos1[0] + yz * pos1[1] + zz * pos1[2]]

    else:
        # Perform rotation from ICRS to dynamical system.

        return [xx * pos1[0] + xy * pos1[1] + xz * pos1[2],
                yx * pos1[0] + yy * pos1[1] + yz * pos1[2],
                zx * pos1[0] + zy * pos1[1] + zz * pos1[2]]

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
