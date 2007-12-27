
==========================
Coordinate Transformations
==========================

.. _PyEphem Quick Reference: quick
.. _Right Ascension and Declination: radec

This document describes how to use PyEphem
to convert between equatorial, ecliptic, and galactic coordinates,
and how to convert coordinates between different epochs.

To begin with,
Celestial bodies in PyEphem
can express their coordinates in only two ways;
you can ask for their *equatorial* position,
which provides a *right ascension* and a *declination*
relative to the Earth's equator and poles,
or you can ask where they stand in the sky above a particular observer,
which yields an *azimuth* and an *altitude*.
(There are actually several versions of the equatorial position available;
see the `Right Ascension and Declination`_ document for details.)

But PyEphem does support two other coordinate systems,
which differ by which great circle they use as the equator of the sky:

Ecliptic Coordinates
  This involves a pair of coordinates,
  *ecliptic longitude* and *ecliptic latitude*,
  which measure position from the ecliptic —
  the plane in which the planets orbit,
  which can be drawn across the sky as an alternate equator.

Galactic Coordinates
  The *galactic longitude* and *galactic latitude* of an object
  measure its location from the tilted plane that our own galaxy,
  the Milky Way, makes across our sky.

As explained below,
the user can either simply ask for a body's coordinates
to be re-expressed in either of these alternate systems,
or can convert freely between the three main systems of coordinate
without getting bodies involved at all.

Summary of coordinate transforms
================================

Before wading into all of the specific examples below
which outline many different ways of using PyEphem's coordinate engine,
we should summarize the basic rules of coordinate handling.

* There are three coordinate classes,
  instances of which have three attributes:

  | ``Equatorial`` coordinates have ``ra`` right ascension,
    ``dec`` declination, and an ``equinox``.
  | ``Ecliptic`` coordinates have ``long`` longitude,
    ``lat`` latitude, and an ``equinox``.
  | ``Galactic`` coordinates have ``long`` longitude,
    ``lat`` latitude, and an ``equinox``.

* You can instantiate any kind of coordinate
  by passing it a body, or a body together with an alternate equinox,
  and it will use the body's astrometric right ascension and declination::

   Ecliptic(mars)
   Ecliptic(mars, equinox='1950')

* You can instantiate any kind of coordinate
  by passing it any other coordinate,
  and can optionally provide an alternate equinox::

   Galactic(north_pole)
   Galactic(north_pole, equinox='1900')

* Finally, you can instantiate a coordinate
  by simply providing its parts manually
  (in the same order as they are listed in the first point above).
  If you do not specify an equinox,
  then J2000 is assumed::

   Equatorial('23:19:01.27', '-17:14:22.1')
   Equatorial('23:19:01.27', '-17:14:22.1', equinox='1950')

All of the examples below
are constructed using some combination of the possibilities above.

Common Cases
============

**Find the position of a body in galactic or ecliptic coordinates**
  Pass the body to the ``Ecliptic`` or ``Galactic`` class:

  >>> import ephem
  >>> m = ephem.Mars('2003/08/27')
  >>> print m.a_ra, m.a_dec
  22:39:06.87 -15:41:53.2

  >>> ecl = ephem.Ecliptic(m)
  >>> print ecl.long, ecl.lat
  335:26:06.3 -6:39:15.6

  The equinox of the resulting coordinates
  is the same as that used by the body for its astrometric coordinates:

  >>> print ecl.equinox
  2000/1/1 12:00:00

**Asking for other epochs**

  Unless you ask for a different epoch when converting coordinates,
  the new coordinate returned will use the same epoch
  as the old one.
  For example,
  if we repeat the above calculation
  but ask for the position of Mars in old 1950 coordinates,
  all of the values will, of course, be slightly different:

  >>> import ephem
  >>> m = ephem.Mars('2003/08/27', epoch='1950')
  >>> print m.a_ra, m.a_dec
  22:36:26.49 -15:57:31.6

  >>> ecl = ephem.Ecliptic(m)
  >>> print ecl.long, ecl.lat
  334:44:09.4 -6:39:07.8

  However, it is important to realize that these coordinates
  in fact name *exactly* the same position in the sky
  as those in the previous example above.
  Note that if we now ask for the ecliptic coordinates of Mars
  but ask for J2000 coordinates,
  we get exactly the same values as in the earlier example:

  >>> ecl = ephem.Ecliptic(m, equinox='2000')
  >>> print ecl.long, ecl.lat
  335:26:06.3 -6:39:15.6

**Using Non-astrometric Right Ascension and Declination**
  In the above example,
  where we passed a body directly to ``Ecliptic()`` and ``Galactic()``,
  they automatically used the body's
  *astrometric* right ascension and declination.
  If for some particular application
  you want to use the apparent version of the coordinates instead,
  then pass the body's right ascension, declination,
  and the epoch manually:

