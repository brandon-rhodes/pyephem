
import de421
from jplephem import Ephemeris
from ephem.coordinates import (
    GeocentricRADec, GeocentricXYZ, ICRS, XYZ,
    )
from numpy import sqrt

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
