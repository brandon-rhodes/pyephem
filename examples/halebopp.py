#!/usr/bin/env python
# Print an ephemeris for comet Hale-Bopp around the height of its
# recent apparition.
#
# The ephemeris entry is taken from the IAU web page at
# http://cfa-www.harvard.edu/iau/Ephemerides/Soft03.html
# which also has elements for other comets and minor planets.

from ephem import *
fS = formatSexagesimal
ss = scanSexagesimal

print 'Reading the Hale-Bopp elements from an Ephem-style definition'

o = Obj()
entry = ("C/1995 O1 (Hale-Bopp),e,89.4245,282.4515,130.5641,183.6816," +
         "0.0003959,0.995026,0.1825,07/06.0/1998,2000,g -2.0,4.0")
scanDB(entry, o)

print 'Creating a circumstance for Atlanta, Georgia'

c = Circumstance()
c.timezone = 0
c.epoch = J2000
c.latitude =	degrad(ss("33:45:10"))
c.longitude =	degrad(ss("-84:23:37"))
c.elevation =	320.0/EarthRadius       # (earth radii)

print "Here are Hale-Bopp's right ascension and declination for:"

jstart = fromGregorian(4,15,1997) - 1/24.
for jo in range(40):
        c.mjd = jstart + jo/240.
	computeLocation(c, o)
        print formatDay(c.mjd), formatTime(c.mjd, 60), \
              formatHours(o.any.ra, 36000), formatDegrees(o.any.dec, 3600)
