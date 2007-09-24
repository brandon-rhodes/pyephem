# The core functionalty of PyEphem lives in the C-language _libastro
# module, which packages the astronomy routines from XEphem as
# convenient Python types.

import ephem._libastro as _libastro
from math import pi
twopi = pi * 2.0
halfpi = pi / 2.0

# We make available several basic types from _libastro.

Angle = _libastro.Angle
degrees = _libastro.degrees
hours = _libastro.hours

Date = _libastro.Date
hour = _libastro.hour
minute = _libastro.minute
second = _libastro.second

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

def newton(f, x0, x1, side=None):
    f0, f1 = f(x0), f(x1)
    while f1 and x1 != x0 and f1 != f0:
        x0, x1 = x1, x1 + (x1 - x0) / (f0/f1 - 1)
        f0, f1 = f1, f(x1)
        #print ".", f1
    if side is not None:
        pass
    return x1

# We provide a Python extension to our C "Observer" class that can
# find many circumstances.

class Observer(_libastro.Observer):
    pass

def next_spring_equinox(current_date):
    pass

def next_autumn_equinox(current_date):
    pass

def next_equinox(current_date):
    def f(d):
        sun.compute(d, epoch=d)
        return sun.ra.znorm
    sun = Sun()
    f(current_date)
    ra = sun.ra
    if ra > pi:
        angle_to_cover = 2*pi - ra
    else:
        angle_to_cover = pi - ra
    d0 = current_date + 365.25 * angle_to_cover / twopi
    d1 = d0 + hour
    n = newton(f, d0, d1)
    return date(n)

# THINGS TO DO:
# Better docs
# Newton solver
# Use newton to provide risings and settings
# Use newton to find equinoxes/solstices

# For backwards compatibility.

date = Date
angle = Angle
