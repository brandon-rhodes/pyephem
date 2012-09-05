# -*- coding: utf-8 -*-

from sgp4.ext import jday
from ephem.planets import mercury, earth
import ephem.coordinates as coordinates

#jed = 2444391.5  # 1980.06.01
#jday(year, mon, day, hr, minute, sec)
jed = jday(2006, 6, 19, 0, 0, 0)
print jed
epos = earth.at(jed)
print('earth xyz and date:')
print(epos)
print(epos.date)

# epos = earth.at(jed)
# print('earth xyz:')
# print(epos)

diff = mercury.seen_from(epos)
print('diff:')
print(diff)

gxyz = coordinates.GeocentricXYZ(diff[0], diff[1], diff[2])

print()
print('mars ra dec:')
radec = coordinates.GeocentricRADec(gxyz)
print(radec.ra)
print(radec.dec)

exit(0)

print()
print('mars ra dec:')
radec = coordinates.HeliocentricRADec(pos)
print(radec)
print(radec.ra, radec.dec)

exit(0)



print()
print('Helio lat lon:')
hll = coordinates.HeliocentricLonLat(pos)
print(hll)
print(hll.lon, hll.lat)
