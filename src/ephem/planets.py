
import de421
from jplephem import Ephemeris
from ephem.coordinates import (
    GeocentricRADec, GeocentricXYZ, ICRS, XYZ,
    )
from numpy import sqrt
from ephem.angles import interpret_longitude, interpret_latitude
from ephem import earthlib, timescales

e = Ephemeris(de421)

class Planet(object):
    def __init__(self, jplname):
        self.jplname = jplname

    def __call__(self, date):
        x, y, z, dx, dy, dz = e.compute(self.jplname, date)
        xyz = ICRS(x, y, z)
        xyz.date = date
        return xyz

    def seen_from(self, observer):
        """Where will we appear to be, from an observer at xyz?"""
        date = observer.date
        vector = self.at(date) - observer
        light_travel_time = sqrt(vector.dot(vector)) * days_for_light_to_go_1m
        date -= light_travel_time
        vector = self.at(date) - observer
        return XYZ(*vector[:3])

    def astrometric(self, body):
        diff = body.seen_from(self)
        gxyz = GeocentricXYZ(diff[0], diff[1], diff[2])
        radec = GeocentricRADec(gxyz)
        return radec

mercury = Planet('mercury')
mars = Planet('mars')
jupiter = Planet('jupiter')

def earth(jd):
    earthmoon_xyz = e.compute('earthmoon', jd)
    moon_xyz = e.compute('moon', jd)
    x, y, z, dx, dy, dz = earthmoon_xyz - moon_xyz / e.EMRAT
    xyz = ICRS(x, y, z)
    xyz.jd = jd
    return xyz

def moon(jd):
    earthmoon_xyz = e.compute('earthmoon', jd)
    moon_xyz = e.compute('moon', jd)
    x, y, z, dx, dy, dz = earthmoon_xyz + moon_xyz
    xyz = ICRS(x, y, z)
    xyz.jd = jd
    return xyz

class EarthLocation(object):

    def __init__(self, longitude, latitude, elevation=0.,
                 temperature=10.0, pressure=1010.0):
        self.longitude = interpret_longitude(longitude)
        self.latitude = interpret_latitude(latitude)
        self.elevation = elevation

    def __call__(self, jd_tt):
        delta_t = 0
        jd_tdb = jd_tt + timescales.tdb_minus_tt(jd_tt)
        jd_ut1 = jd_tt - (delta_t / 86400.)

        gmst = timescales.sidereal_time(jd_ut1, delta_t)
        x1, x2, eqeq, x3, x4 = earthlib.earth_tilt(jd_tdb)
        gast = gmst + eqeq / 3600.0;

        terra(&obs->on_surf, gast, pos1, vel1);

        # ...

        return earth(jd_tt)
