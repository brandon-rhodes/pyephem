# -*- coding: utf-8 -*-

from sgp4.ext import jday
from ephem.angles import Angle
from ephem.planets import earth, moon, mercury, jupiter, EarthLocation
from ephem import J2000

jd = 2450203.5
ggr = EarthLocation('75 W', '45 N', 0.0, temperature=10.0, pressure=1010.0)
ra, dec, dis = ggr(jd).observe(moon).radec(J2000)
print(Angle(ra).hours())
print(Angle(dec).degrees())
print('')
print(Angle(ra).hours() - 11.739849403)
print(Angle(dec).degrees() - -0.31860323)
print('')
print(dis)

#JD = 2450203.500000  RA = 11.739849403  Dec =  -0.31860323  Dis = 0.0026040596

exit(0)

jd = jday(2006, 6, 19, 0, 0, 0)
pos = earth(jd).observe(mercury).radec(J2000)    # Astrometric

print(pos.ra)
print(pos.dec)

print('----')

jd = jday(2008, 7, 25, 0, 0, 0)
pos = earth(jd).observe(jupiter).radec()    # Apparent

print(pos.ra)
print(pos.dec)

# pos = earth(jd).view(mercury).radec()         Apparent
# pos = chicago(jd).view(mercury).radec()       Apparent
# altaz = chicago(jd).view(mercury).altaz()     Apparent

# pos = astrometric(earth, mercury, jd)
#
# This reveals nothing about how the calculation is taking place, and
# also has too many arguments.  But since earth would almost always be
# the origin, it could perhaps be:
#
# pos = astrometric(mercury, jd=now(), from=earth)
#
# Other approaches could reveal more about how the operation works:
#
# pos = earth.astrometric(mercury.at(jd))
#
# THIS CANNOT WORK - because we need to run mercury() again once we know
# the light time delay, and this syntax has thrown mercury() away and
# only kept a single position.
#
# pos = earth.astrometric(mercury, jd)
#
# This could work fine.  But it raises the awkward question of ordering:
# how will users remember that it's (body, date) and not the other way
# around?  Date will probably be optional, with a default of now(),
# which would naturally force that argument to be the second one.  But
# it might still seem arbitrary for novice Python users.
#
# pos = earth.at(jd).astrometric(mercury)
# pos = mercury.astrometric_from(earth.at(date))
# pos = mercury.astrometric_from(earth, date)
#
# Should earth even be an explicit argument?  Is that too odd?
#
# pos = mercury.astrometric(date)
#
# pos = astrometric(earth.at(jd).view(mercury))
#
# But is that slightly verbose?
#
# pos = astrometric(earth(jd).view(mercury))
# pos = apparent(earth(jd).view(mercury))
# pos = apparent(chicago(jd).view(mercury))
# altaz = horizontal(chicago(jd).view(mercury))
#
# This works if the velocity is kept in the vector returned by
# earth(jd) and chicago(jd) because velocity is needed for
# aberration.
#
# pos = astrometric(earth.at(jd) - mercury)
#
# Too mathy and bad.
#
# What about the fact that I personally have realized that I do not like
# expressions where you have to jump around to figure out what is going
# on?  Look at this again:
#
# pos = astrometric(earth(jd).view(mercury))
#
# You have to read the inside left-to-right, then jump out of the parens
# and return to the enclosing function to understand the process.  What
# if instead we took a chaining approach?
#
# pos = earth(jd).view(mercury).radec()
#
# The problem is, how do we distinguish the three kinds of radec?
# Let's try:

# pos = earth(jd).view(mercury).radec()
# pos = earth(jd).view(mercury).radec()  <--- how to do geocentric apparent?
# pos = chicago(jd).view(mercury).radec()
# altaz = chicago(jd).view(mercury).altaz()
#
# Okay: what if providing an epoch was the way to do astrometric?
#
# pos = earth(jd).view(mercury).radec(J2000)    Astrometric
# pos = earth(jd).view(mercury).radec()         Apparent
# pos = chicago(jd).view(mercury).radec()       Apparent
# altaz = chicago(jd).view(mercury).altaz()     Apparent
#
# That works well!  By making the epoch explicit, it even makes clear to
# people that astrometric numbers have the choice of epoch built-in to
# them.  But why is this a function call followed by method calls?
# Should all three steps be method calls, for symmetry?
#
# pos = earth.at(jd).view(mercury).radec(J2000)    Astrometric
# pos = earth.at(jd).view(mercury).radec()         Apparent
# pos = chicago.at(jd).view(mercury).radec()       Apparent
# altaz = chicago.at(jd).view(mercury).altaz()     Apparent

# Um.
#
# Uh-oh.
#
# Our positions are also ndarrays, and an ndarray already defines a
# method named view()!  What should we use instead?
#
# earth(jd).look(mercury)
# earth(jd).observe(mercury)
# earth(jd).find(mercury)
# earth(jd).to(mercury)
# earth(jd).toward(mercury)
# earth(jd).vector(mercury)
# earth(jd).see(mercury)
# earth(jd).measure(mercury)
# earth(jd).trace(mercury)
# earth(jd).point(mercury)
# earth(jd).orient(mercury)
# earth(jd).face(mercury)

exit(0)

#jd = 2444391.5  # 1980.06.01
#jday(year, mon, day, hr, minute, sec)
jd = jday(2006, 6, 19, 0, 0, 0)
epos = earth.at(jd)
print('earth xyz and date:')
print(epos)
print(epos.date)

# epos = earth.at(jd)
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
