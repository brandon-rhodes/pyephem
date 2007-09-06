# The core functionalty of PyEphem lives in the C-language _ephem
# module, which packages the "libastro" astronomy routines as
# convenient Python types.

import _ephem

Observer = _ephem.Observer
degrees = _ephem.degrees
date = _ephem.date
hours = _ephem.hours
minute = _ephem.minute
second = _ephem.second

Body = _ephem.Body
Planet = _ephem.Planet
PlanetMoon = _ephem.PlanetMoon
FixedBody = _ephem.FixedBody
EllipticalBody = _ephem.EllipticalBody
ParabolicBody = _ephem.ParabolicBody
HyperbolicBody = _ephem.HyperbolicBody
EarthSatellite = _ephem.EarthSatellite

readdb = _ephem.readdb
readtle = _ephem.readtle
constellation = _ephem.constellation
separation = _ephem.separation
now = _ephem.now

#

for index, classname, name in _ephem.builtin_planets():
    exec '''
class %s(_ephem.%s):
    __planet__ = %r
''' % (name, classname, index)

#

Saturn = _ephem.Saturn
Moon = _ephem.Moon
