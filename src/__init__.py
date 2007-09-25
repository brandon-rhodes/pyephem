# The core functionalty of PyEphem lives in the C-language _libastro
# module, which packages the astronomy routines from XEphem as
# convenient Python types.

import ephem._libastro as _libastro
from math import pi

twopi = pi * 2.
halfpi = pi / 2.
quarterpi = pi / 4.
eighthpi = pi / 8.
arcminute = twopi / 360. / 60.
arcsecond = arcminute / 60.
half_arcsecond = arcsecond / 2.
twentieth_arcsecond = arcsecond / 20.

# We make available several basic types from _libastro.

Angle = _libastro.Angle
degrees = _libastro.degrees
hours = _libastro.hours

Date = _libastro.Date
hour = _libastro.hour
minute = _libastro.minute
second = _libastro.second

delta_t = _libastro.delta_t

Body = _libastro.Body
Planet = _libastro.Planet
PlanetMoon = _libastro.PlanetMoon
FixedBody = _libastro.FixedBody
EllipticalBody = _libastro.EllipticalBody
ParabolicBody = _libastro.ParabolicBody
HyperbolicBody = _libastro.HyperbolicBody
EarthSatellite = _libastro.EarthSatellite

readdb = _libastro.readdb
readtle = _libastro.readtle
constellation = _libastro.constellation
separation = _libastro.separation
now = _libastro.now

# We also create a Python class ("Mercury", "Venus", etcetera) for
# each planet and moon for which _libastro offers specific algorithms.

for index, classname, name in _libastro.builtin_planets():
    exec '''
class %s(_libastro.%s):
    __planet__ = %r
''' % (name, classname, index)

del index, classname, name

# We now replace two of the classes we have just created, because
# _libastro actually provides separate types for two of the bodies.

Saturn = _libastro.Saturn
Moon = _libastro.Moon

# The "newton" function supports finding the zero of a function.

def newton(f, x0, x1):
    f0, f1 = f(x0), f(x1)
    while f1 and x1 != x0 and f1 != f0:
        x0, x1 = x1, x1 + (x1 - x0) / (f0/f1 - 1)
        f0, f1 = f1, f(x1)
    return x1

# Find equinoxes and solstices using the ecliptic longitude, not right
# ascension, since computing the cross-quarter days accurately means
# following the sun on its course along the ecliptic, not the right
# ascension numbers it trails along the equator above and below it.

_sun = Sun()                    # used for computing equinoxes

def holiday(d0, motion, offset):
    def f(d):
        _sun.compute(d, epoch=d)
        return (_sun.ra + eighthpi) % quarterpi - eighthpi
    _sun.compute(d0, epoch=d0)
    angle_to_cover = motion - (_sun.ra + offset) % motion
    if abs(angle_to_cover) < twentieth_arcsecond:
        angle_to_cover = motion
    d = d0 + 365.25 * angle_to_cover / twopi
    return date(newton(f, d, d + hour))

def previous_vernal_equinox(date): return holiday(date, -twopi, 0)
def next_vernal_equinox(date): return holiday(date, twopi, 0)

def previous_equinox(date): return holiday(date, -pi, 0)
def next_equinox(date): return holiday(date, pi, 0)

def previous_solstice(date): return holiday(date, -pi, halfpi)
def next_solstice(date): return holiday(date, pi, halfpi)

def previous_quarter_day(date): return holiday(date, -halfpi, 0)
def next_quarter_day(date): return holiday(date, halfpi, 0)

def previous_cross_quarter_day(date): return holiday(date, -halfpi, quarterpi)
def next_cross_quarter_day(date): return holiday(date, halfpi, quarterpi)

# We provide a Python extension to our C "Observer" class that can
# find many circumstances.

class Observer(_libastro.Observer):
    elev = _libastro.Observer.elevation

    def __str__(self):
        return ('<ephem.Observer date=%r epoch=%r'
                ' long=%s lat=%s elevation=%sm'
                ' horizon=%s temp=%sC pressure=%smBar>'
                % (str(self.date), str(self.epoch),
                   self.long, self.lat, self.elevation,
                   self.horizon, self.temp, self.pressure))

# THINGS TO DO:
# Better docs
# Use newton to provide risings and settings

# For backwards compatibility, provide lower-case names for our Date
# and Angle classes.

date = Date
angle = Angle
