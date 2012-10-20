from math import asin, acos, pi, sqrt

from ephem import earthlib, timescales
from ephem.angles import interpret_longitude, interpret_latitude
from ephem.coordinates import ICRS, rotation_to_ICRS
from ephem.nutationlib import earth_tilt, nutation_matrix
from ephem.planets import earth
from ephem.precessionlib import precession_matrix

halfpi = pi / 2.0
ERAD = 6378136.6
AU = 1.4959787069098932e+11
rade = ERAD / AU
RAD2DEG = 57.295779513082321

class Topos(object):

    def __init__(self, longitude, latitude, elevation=0.,
                 temperature=10.0, pressure=1010.0):
        self.longitude = interpret_longitude(longitude)
        self.latitude = interpret_latitude(latitude)
        self.elevation = elevation

    def __call__(self, jd_tt):
        e = earth(jd_tt)
        tpos, tvel = self.geocentric_position_and_velocity(jd_tt)
        return ToposXYZ(e.position + tpos, e.velocity + tvel, jd_tt)

    def geocentric_position_and_velocity(self, jd_tt):
        delta_t = 0
        jd_tdb = jd_tt + timescales.tdb_minus_tt(jd_tt)
        jd_ut1 = jd_tt - (delta_t / 86400.)

        gmst = timescales.sidereal_time(jd_ut1, delta_t)
        x1, x2, eqeq, x3, x4 = earth_tilt(jd_tdb)
        gast = gmst + eqeq / 3600.0

        pos, vel = earthlib.terra(self, gast)

        n = nutation_matrix(jd_tdb).T
        p = precession_matrix(jd_tdb).T
        npr = n.dot(p).dot(rotation_to_ICRS)

        return pos.dot(npr), vel.dot(npr)

class ToposXYZ(ICRS):
    """In ICRS, right?"""

    geocentric = False

#

def limb(position, observer_position):
    """Determine the angle of an object above or below the Earth's limb.

    Returns (limb_angle, nadir_angle).

    """
    # Compute the distance to the object and the distance to the observer.

    disobj = sqrt(position.dot(position))
    disobs = sqrt(observer_position.dot(observer_position))

    # Compute apparent angular radius of Earth's limb.

    if disobs >= rade:
        aprad = asin(rade / disobs)
    else:
        aprad = halfpi

    # Compute zenith distance of Earth's limb.

    zdlim = pi - aprad

    # Compute zenith distance of observed object.

    coszd = position.dot(observer_position) / (disobj * disobs)
    if coszd <= -1.0:
        zdobj = pi
    elif coszd >= 1.0:
        zdobj = 0.0
    else:
        zdobj = acos(coszd)

    # Angle of object wrt limb is difference in zenith distances.

    limb_angle = (zdlim - zdobj) * RAD2DEG

    # Nadir angle of object as a fraction of angular radius of limb.

    nadir_angle = (pi - zdobj) / aprad

    return limb_angle, nadir_angle
