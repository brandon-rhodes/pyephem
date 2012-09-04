
import de421
from jplephem import Ephemeris
from ephem.coordinates import HeliocentricXYZ

e = Ephemeris(de421)

class Planet(object):
    def __init__(self, jplname):
        self.jplname = jplname

    def at(self, date):
        x, y, z, dx, dy, dz = e.compute(self.jplname, date)
        return HeliocentricXYZ(x, y, z)

mars = Planet('mars')

class Earth(Planet):
    def __init__(self):
        pass

    def at(self, date):
        earthmoon = e.compute('earthmoon', date)
        moon = e.compute('moon', date)
        x, y, z, dx, dy, dz = earthmoon - moon / e.EMRAT
        return HeliocentricXYZ(x, y, z)

earth = Earth()
