# -*- coding: utf-8 -*-

from ephem.planets import mars, earth
import ephem.coordinates as coordinates

jed = 2444391.5  # 1980.06.01
mpos = mars.at(jed)
print('mars xyz:')
print(mpos)

epos = earth.at(jed)
print('earth xyz:')
print(epos)

diff = mpos - epos
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
