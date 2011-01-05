
Example Scripts
===============

Here are a few samples scripts
that show how to use PyEphem to perform simple tasks.

hale_bopp_ephemeris.py
----------------------

The following script prints out an ephemeris.

::

 from ephem import *

 hb = readdb("C/1995 O1 (Hale-Bopp),e,89.4245,282.4515,130.5641,183.6816," +
             "0.0003959,0.995026,0.1825,07/06.0/1998,2000,g -2.0,4.0")

 here = Observer()
 here.lat, here.lon, here.elev = '33:45:10', '-84:23:37', 320.0

 print "Hale-Bopp: date, right ascension, declination, and magnitude:"

 here.date = date('1997/2/15')
 end = date('1997/5/15')
 while here.date < end:
     hb.compute(me)
     print here.date, hb.ra, hb.dec, hb.mag
     here.date += 5

jovian_moon_chart.py
----------------------

This script prints out where the Jovian moons are around Jupiter
for the next few days.

::

 import ephem

 moons = ((ephem.Io(), 'i'),
          (ephem.Europa(), 'e'),
          (ephem.Ganymede(), 'g'),
          (ephem.Callisto(), 'c'))

 # How to place discrete characters on a line that actually represents
 # the real numbers -maxradii to +maxradii.

 linelen = 65
 maxradii = 30.

 def put(line, character, radii):
     if abs(radii) > maxradii:
         return
     offset = radii / maxradii * (linelen - 1) / 2
     i = int(linelen / 2 + offset)
     line[i] = character

 interval = ephem.hour * 3
 now = ephem.now()
 now -= now % interval

 t = now
 while t < now + 2:
     line = [' '] * linelen
     put(line, 'J', 0)
     for moon, character in moons:
         moon.compute(t)
         put(line, character, moon.x)
     print str(ephem.date(t))[5:], ''.join(line)
     t += interval

 print 'East is to the right;',
 print ', '.join([ '%s = %s' % (c, m.name) for m, c in moons ])
