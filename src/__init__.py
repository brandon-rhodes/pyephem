# The core functionalty of PyEphem lives in the C-language _ephem
# module, which packages the "libastro" astronomy routines as
# convenient Python types.

import ephem._libastro as _

Observer = _.Observer
degrees = _.degrees
date = _.date
hours = _.hours
minute = _.minute
second = _.second

Body = _.Body
Planet = _.Planet
PlanetMoon = _.PlanetMoon
FixedBody = _.FixedBody
EllipticalBody = _.EllipticalBody
ParabolicBody = _.ParabolicBody
HyperbolicBody = _.HyperbolicBody
EarthSatellite = _.EarthSatellite

readdb = _.readdb
readtle = _.readtle
constellation = _.constellation
separation = _.separation
now = _.now

#

for index, classname, name in _.builtin_planets():
    exec '''
class %s(_.%s):
    __planet__ = %r
''' % (name, classname, index)

#

Saturn = _.Saturn
Moon = _.Moon
