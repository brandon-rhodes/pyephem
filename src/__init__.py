# The core functionalty of PyEphem lives in the C-language _libastro
# module, which packages the astronomy routines from XEphem as
# convenient Python types.

import ephem._libastro as _libastro

# We make available several basic types from _libastro.

Observer = _libastro.Observer

angle = _libastro.angle
degrees = _libastro.degrees
hours = _libastro.hours

date = _libastro.date
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

# THINGS TO DO:
# Better docs
# Easy date conversions (to and from datetime)
# Newton solver
# Use newton to provide risings and settings
# Use newton to find equinoxes/solstices
