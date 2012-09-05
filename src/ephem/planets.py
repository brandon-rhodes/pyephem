
import de421
from jplephem import Ephemeris
from ephem.coordinates import HeliocentricXYZ, XYZ
from numpy import sqrt

e = Ephemeris(de421)
days_for_light_to_go_1m = 1. / (e.CLIGHT * 60. * 60. * 24.)

class Planet(object):
    def __init__(self, jplname):
        self.jplname = jplname

    def at(self, date):
        x, y, z, dx, dy, dz = e.compute(self.jplname, date)
        xyz = HeliocentricXYZ(x, y, z)
        xyz.date = date
        return xyz

    def seen_from(self, observer):
        """Where will we appear to be, from an observer at xyz?"""
        date = observer.date
        vector = self.at(date) - observer
        light_travel_time = sqrt(vector.dot(vector)) * days_for_light_to_go_1m
        print 'ltt:', light_travel_time
        date -= light_travel_time
        vector = self.at(date) - observer
        return XYZ(*vector[:3])

mercury = Planet('mercury')
mars = Planet('mars')

class Earth(Planet):
    def __init__(self):
        pass

    def at(self, date):
        earthmoon = e.compute('earthmoon', date)
        moon = e.compute('moon', date)
        x, y, z, dx, dy, dz = earthmoon - moon / e.EMRAT
        xyz = HeliocentricXYZ(x, y, z)
        xyz.date = date
        return xyz

    def astrometric(self):
        pass

earth = Earth()
