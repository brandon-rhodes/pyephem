
import de405
from jplephem import Ephemeris
from numpy import sqrt

from ephem import earthlib, timescales
from ephem.coordinates import GeocentricRADec, GeocentricXYZ, ICRS, XYZ

AU_KM = earthlib.AU_KM
DAY_S = 24.0 * 60.0 * 60.0
KM_AU = 1.0 / AU_KM
KMS_AUDAY = KM_AU * DAY_S

T0 = timescales.T0
e = Ephemeris(de405)

class Planet(object):
    def __init__(self, jplname):
        self.jplname = jplname

    def __call__(self, jd):
        pv = e.compute(self.jplname, jd)
        return ICRS(pv[:3] * KM_AU, pv[3:] * KMS_AUDAY, jd)

mercury = Planet('mercury')
mars = Planet('mars')
jupiter = Planet('jupiter')

moon_share = 1.0 / (1.0 + e.EMRAT)
earth_share = e.EMRAT / (1.0 + e.EMRAT)

def earth(jd):
    pv = e.compute('earthmoon', jd) - e.compute('moon', jd) * moon_share
    return ICRS(pv[:3] * KM_AU, pv[3:] * KMS_AUDAY, jd)

def moon(jd):
    pv = e.compute('earthmoon', jd) + e.compute('moon', jd) * earth_share
    return ICRS(pv[:3] * KM_AU, pv[3:] * KMS_AUDAY, jd)
