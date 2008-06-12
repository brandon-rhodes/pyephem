
Asteroid, Comet, and Satellite Catalogs
=======================================

After learning to program with PyEphem,
you will probably want to download the orbital elements
of comets or asteroids that interest you,
or perhaps coordinates for stars and other fixed objects.
Here are a few sources from which such data may be obtained:

http://cfa-www.harvard.edu/iau/Ephemerides/Soft03.html

 The International Astronomical Union's
 *Central Bureau for Astronomical Telegrams and Minor Planet Center*
 is where comet and asteroid discoveries are traditionally reported,
 and is a source of ephemerides and orbital elements for these objects.
 They provide updated orbital elements, in ephem format,
 for many currently visible comets and asteroids.

http://celestrak.com/NORAD/elements/

 Orbital elements for man-made earth satellites are available from NORAD,
 whose Two-Line Element format can be understood
 by PyEphem's ``readtle()`` function.

http://www.maa.mhn.de/Tools/Xephem/

 This directory has not been updated for several years,
 but has quite extensive collections of objects in ephem format.
 Though many of its comet and asteroid elements are doubtless out of date,
 its files of fixed objects like galaxies to pulsars
 should remain valuable.

http://toyvax.glendale.ca.us/

 This page has elements in ephem format
 for several of our longer-lived space probes.
