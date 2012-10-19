
import de405
from jplephem import Ephemeris
from numpy import sqrt

from ephem import earthlib, timescales
from ephem.coordinates import GeocentricRADec, GeocentricXYZ, ICRS, XYZ

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

moon_share = 1.0 / (1.0 + e.EMRAT)
earth_share = e.EMRAT / (1.0 + e.EMRAT)

def earth(jd):
    earthmoon_xyz = e.compute('earthmoon', jd)
    moon_xyz = e.compute('moon', jd)
    x, y, z, dx, dy, dz = earthmoon_xyz - moon_xyz * moon_share
    xyz = ICRS(x, y, z, dx, dy, dz)
    xyz.jd = jd
    return xyz

def moon(jd):
    earthmoon_xyz = e.compute('earthmoon', jd)
    moon_xyz = e.compute('moon', jd)
    x, y, z, dx, dy, dz = earthmoon_xyz + moon_xyz * earth_share
    xyz = ICRS(x, y, z, dx, dy, dz)
    xyz.jd = jd
    return xyz
