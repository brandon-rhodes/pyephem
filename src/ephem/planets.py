
import de421
from jplephem import Ephemeris
from ephem.coordinates import HeliocentricEquatorialXYZ

e = Ephemeris(de421)

def mars(date):
    x, y, z, dx, dy, dz = e.compute('mars', date)
    return HeliocentricEquatorialXYZ(x, y, z)
