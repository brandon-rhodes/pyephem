from ephem import *
import time

# Display the available preferences.

print 'The preferences and their possible values are:'
print
for prefname, valuenames in [['PREF_EQUATORIAL',
                      ['PREF_TOPO', 'PREF_GEO']],
                     ['PREF_DATE_FORMAT',
                      ['PREF_MDY', 'PREF_YMD', 'PREF_DMY']]]:
    print '%s:' % prefname
    pref = eval(prefname)
    for valuename in valuenames:
        value = eval(valuename)
        if pref_get(pref) == value:
            print '    %s (default)' % valuename
        else:
            print '    %s' % valuename
print

# Determine the modified Julian date.

now = time.time()
year, month, day, hour, minute, second = time.gmtime(now)[:6]
fraction = time.time() % 1.0
day_fraction = (((second + fraction) / 60. + minute) / 60. + hour) / 24.
jnow = fromGregorian(month, day + day_fraction, year)

# Build a circumstance describing an observer in Atlanta, Georgia, at
# the current time and date.

c = Circumstance()

c.mjd =		jnow
c.latitude =	degrad(scanSexagesimal("33:45:10"))
c.longitude =	degrad(scanSexagesimal("-84:23:37"))
c.elevation =	320.0/EarthRadius       # (320 meters, in earth radii)
c.timezone =	5.0
c.temperature = 10.0                    # (degrees Celsius)
c.pressure =	1010.0                  # (miliBars)
c.epoch =	J2000

# Build an object that represents Jupiter.

o = Obj()
o.any.type = PLANET
o.pl.code = JUPITER

# Compute the right ascension of Jupiter from the two centrics.

pref_set(PREF_EQUATORIAL, PREF_GEO)
computeLocation(c, o)

print 'Right now:'
print 'the RA of Jupiter viewed from the center of the earth: %s' \
      % formatHours(o.any.ra, 360000)

pref_set(PREF_EQUATORIAL, PREF_TOPO)
computeLocation(c, o)

print 'the RA of Jupiter viewed from Atlanta is:              %s' \
      % formatHours(o.any.ra, 360000)

# Print the date in the three formats.

print
pref_set(PREF_DATE_FORMAT, PREF_MDY)
print 'An American might write today as:   %s' % formatDay(jnow)
pref_set(PREF_DATE_FORMAT, PREF_YMD)
print 'An astronomer might write today as: %s' % formatDay(jnow)
pref_set(PREF_DATE_FORMAT, PREF_DMY)
print 'A European might write today as:    %s' % formatDay(jnow)
print '(The astronomer and European are sensible but the American is foolish)'
