
import de405
from jplephem import Ephemeris
from ephem.coordinates import (
    GeocentricRADec, GeocentricXYZ, ICRS, XYZ, frame_tie,
    )
from numpy import sqrt
from ephem.angles import interpret_longitude, interpret_latitude
from ephem import earthlib, precessionlib, nutationlib, timescales

AU_KM = earthlib.AU_KM
T0 = timescales.T0
e = Ephemeris(de405)

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
        xyz = earth(jd_tt)
        pos, vel = self.geocentric_position_and_velocity(jd_tt)
        xyz[0] += pos[0] * AU_KM
        xyz[1] += pos[1] * AU_KM
        xyz[2] += pos[2] * AU_KM
        return xyz

    def geocentric_position_and_velocity(self, jd_tt):
        delta_t = 0
        jd_tdb = jd_tt + timescales.tdb_minus_tt(jd_tt)
        jd_ut1 = jd_tt - (delta_t / 86400.)

        gmst = timescales.sidereal_time(jd_ut1, delta_t)
        x1, x2, eqeq, x3, x4 = earthlib.earth_tilt(jd_tdb)
        gast = gmst + eqeq / 3600.0

        pos1, vel1 = earthlib.terra(self, gast)

        pos2 = nutationlib.nutation(jd_tdb, pos1, invert=True)
        pos3 = precessionlib.precess(jd_tdb, T0, pos2)
        pos = frame_tie(pos3, -1)

        vel2 = nutationlib.nutation(jd_tdb, vel1, invert=True)
        vel3 = precessionlib.precess(jd_tdb, T0, vel2)
        vel = frame_tie(vel3, -1)

        return pos, vel
