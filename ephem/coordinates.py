"""Coordinate systems."""

from numpy import ndarray
from math import asin, atan2, cos, sin, pi, sqrt
from ephem.angles import interpret_longitude, interpret_latitude
from ephem.framelib import ICRS_to_J2000
from ephem.nutationlib import nutation_matrix
from ephem.precessionlib import precession_matrix
from ephem.relativity import add_aberration, add_deflection

J2000 = 2451545.0
C_AUDAY = 173.1446326846693

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

class XYZ(object):

    def __init__(self, position, velocity=None, jd=None):
        self.position = position
        self.velocity = velocity
        self.jd = jd

class ICRS(XYZ):

    geocentric = True

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
        g.observer = self
        g.distance = euclidian_distance
        g.lighttime = lighttime
        return g

class Topos(object):

    def __init__(self, longitude, latitude, elevation=0.,
                 temperature=10.0, pressure=1010.0):
        self.longitude = interpret_longitude(longitude)
        self.latitude = interpret_latitude(latitude)
        self.elevation = elevation

    def __call__(self, jd_tt):
        from ephem.earthlib import geocentric_position_and_velocity
        e = self.earth(jd_tt)
        tpos, tvel = geocentric_position_and_velocity(self, jd_tt)
        t = ToposICRS(e.position + tpos, e.velocity + tvel, jd_tt)
        t.ephemeris = self.ephemeris
        return t

class ToposICRS(ICRS):
    """In ICRS, right?"""

    geocentric = False

class GCRS(XYZ):

    def astrometric(self, epoch=None):
        eq = Astrometric()
        eq.ra, eq.dec = to_polar(self.position)
        eq.distance = self.distance
        # TODO: epoch
        return eq

    def apparent(self):
        jd = self.jd
        observer = self.observer
        position = self.position.copy()

        from ephem.earthlib import limb

        if observer.geocentric:
            use_earth = False
        else:
            limb_angle, nadir_angle = limb(position, observer.position)
            use_earth = limb_angle >= 0.8
        add_deflection(position, observer.position, observer.ephemeris,
                       jd, use_earth)
        add_aberration(position, observer.velocity, self.lighttime)

        position = position.dot(ICRS_to_J2000)
        position = position.dot(precession_matrix(jd))
        position = position.dot(nutation_matrix(jd))

        eq = Apparent()
        eq.ra, eq.dec = to_polar(position)
        eq.distance = self.distance
        return eq

class RADec():
    pass

class Astrometric(RADec):
    pass

class Apparent(RADec):
    pass

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

def to_polar(position):
    r = sqrt(position.dot(position))
    phi = atan2(-position[1], -position[0]) + pi
    theta = asin(position[2] / r)
    return phi, theta
