#!/usr/bin/env python
# Print an ephemeris for comet Hale-Bopp around the height of its
# recent apparition.
#
# The ephemeris entry is taken from the IAU web page at
# http://cfa-www.harvard.edu/iau/Ephemerides/Soft03.html
# which also has elements for other comets and minor planets.

from ephem import *

hb = readdb("C/1995 O1 (Hale-Bopp),e,89.4245,282.4515,130.5641,183.6816," +
	    "0.0003959,0.995026,0.1825,07/06.0/1998,2000,g -2.0,4.0")

me = Observer()
me.lat, me.long, me.elev = '33:45:10', '-84:23:37', 320.0

print "Hale-Bopp: date, right ascension, declination, and magnitude:"

me.date = date('1997/2/15')
end = date('1997/5/15')
while me.date < end:
        hb.compute(me)
        print me.date, hb.ra, hb.dec, hb.mag
	me.date += 5
