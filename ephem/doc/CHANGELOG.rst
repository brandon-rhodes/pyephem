=================
PyEphem CHANGELOG
=================

Version 4.1.5 (2023 October 8)
------------------------------

- Add support for Python 3.12.
  `#259 <https://github.com/brandon-rhodes/pyephem/pull/259>`_

Version 4.1.4 (2022 December 21)
--------------------------------

- A memory leak has been resolved, that was failing to free the storage
  for the satellite name (a Python string) and catalog number (a Python
  integer) when the satellite object itself was freed.

- In previous versions, if you asked for the position of a body
  (a) whose elliptical or hyperbolic orbit has an eccentricity very
  close to 1.0 and (b) which is very far from perihelion, then the
  underlying C library would print a warning ``Near-parabolic orbit:
  inaccurate result`` but let your Python script continue on unawares.
  Now, no message is printed directly to the screen, and instead a
  ``RuntimeError`` will tell you why PyEphem can’t compute the body’s
  position.
  `#239 <https://github.com/brandon-rhodes/pyephem/issues/239>`_

- The underlying C library should no longer produce a segmentation fault
  if given the floating point number ``NaN`` as a date.  The Python
  rising and setting logic now also watches out for ``NaN`` dates, and
  raises a ``ValueError`` when one is detected.
  `#237 <https://github.com/brandon-rhodes/pyephem/issues/237>`_

Version 4.1.3 (2021 December 13)
--------------------------------

- Fixed an inadvertent loss of precision in the routine that computes a
  date’s hours, minutes, and seconds.  It was sometimes returning a
  small negative number of seconds, which caused Python’s ``datetime``
  type to complain ``ValueError: second must be in 0..59``.

- Users installing from source on Windows are now protected against a
  possible encoding error as ``setup.py`` reads in two text files.

Version 4.1.2 (2021 December 5)
-------------------------------

- Fixed the new rising and setting routines so they properly detect if a
  body is always below the horizon and raise a ``NeverUpError`` instead
  of a plain ``ValueError``.

- Gave bodies a new ``ha`` Hour Angle attribute, since the quantity was
  computed internally but then discarded.

- Renamed the observer attribute ``temp`` to ``temperature``, leaving an
  alias behind to support the old spelling.

Version 4.1.1 (2021 November 27)
--------------------------------

- When you provide PyEphem with a Python ``datetime`` that has a time
  zone attached, PyEphem now detects the time zone and converts the date
  and time to UTC automatically.

- A new search routine had been written and tested
  to power the :ref:`transit-rising-setting` methods
  ``previous_rising()``,
  ``previous_setting()``,
  ``next_rising()``, and
  ``next_setting()``.
  They should no longer be susceptible to getting hung up in a loop.
  You should also find them substantially faster.

- Fixed the ``constellation()`` routine so that it uses astrometric,
  rather than apparent, right ascension and declination.  This should
  make it more accurate along the borders of each constellation.

- Fixed how the underlying “libastro” library computes whether a body’s
  image is deflected by gravity when its light passes close to the Sun.
  Previously, users would see coordinates jump unexpectedly as the
  deflection formula turned on and off haphazardly.

- Fixed the star positions in the ``ephem.stars`` star catalog by adding
  8.75 years of proper motion to each star.  Previously, each 1991.25
  position from the Hipparcos catalog was being incorrectly treated as a
  2000.0 position.

- A new routine ``unrefract()`` lets you compute the real altitude
  of a body that you observe in the sky at a given apparent altitude.

- The old ``cities.lookup()`` function is now officially deprecated.
  Because of a Google API restriction, it stopped working in 2018.

Version 4.1 (2021 September 29)
-------------------------------

- Planetary moon positions are now available through the year 2040
  (previously, asking for a position past 2020 returned zeroes).

- The ``Date`` object is improved such that the return values of
  ``str(d)``, ``d.datetime()``, and ``d.tuple()`` always agree and are
  always rounded to the nearest microsecond.

- Earth satellites offer new orbit parameter attributes.  The old names
  (which are still present, but no longer documented) started awkwardly
  with underscores, and had inconsistent getters and setters that would
  change the satellite if you attempted a round trip like ``sat._inc =
  sat._inc``.  The new attributes (see the list in the Quick Reference)
  have simple names and use the same units when getting and setting.

- Updated Pluto’s long-term orbital elements
  to match the Astronomical Almanac 2020.

- Expanded the ∆T table so that it now runs through 2018,
  with data from the Astronomical Almanac 2020.

- The ``Observer.copy()`` method is now documented,
  and (after a user requested it)
  the class also now works with Python’s ``copy`` module.

- PyEphem should now be able to compile for pypy3.

Version 4.0.0.2 (2020 June 14)
------------------------------

- Restore PyEphem’s undocumented ability to parse angle strings like
  ``'12 34 56'`` that are only separated with spaces, instead of
  insisting on ``'12:34:56'`` for 12 degrees, 34 arcminutes, and 56
  arcseconds.

- Fix a compile error `‘for’ loop initial declarations are only allowed
  in C99 mode` reported from a user on Oracle Linux.

Version 4.0.0.1 (2020 June 12)
------------------------------

- Fix ``MANIFEST.in`` so the ``.tar.gz`` source distribution includes
  all the header files necessary for compilation.

Version 4.0.0 (2020 June 12)
----------------------------

- Upgraded to the MIT license following Elwood Downey’s generous
  decision to open-source XEphem’s code.

- Fix a bug where supplying a string with a decimal degree measurement
  could send the parser into an infinite loop.

- The ``FixedBody`` constructor, which accepts no arguments, now
  correctly raises a ``TypeError`` if any are supplied.

Version 3.7.7.1 (2020 February 22)
----------------------------------

- Fixed the body ``copy()`` method to correctly copy the extra
  attributes that some bodies have beyond those of a normal body, like
  the catalog number of an Earth satellite.  This bug had in some cases
  caused segmentation faults.

- GitHub issue #166: Fixed a memory leak in ``readdb()``.

- GitHub issue #119: Fixed the ``Body.copy()`` method to correctly copy
  object-specific fields across to the new object, like Saturn ring tilt
  and Earth satellite catalog number.

Version 3.7.7.0 (2019 August 18)
--------------------------------

- Upgraded libastro to 3.7.7.

- The internal star catalog now includes all 57 navigational stars.

- GitHub issue #63: The rise, culminate, and set returned by
  ``next_pass()`` are now consecutive values for a single pass.  Pass
  ``singlepass=False`` to return the original next_rise, next_culminate,
  next_set even if next_set < next_rise (the satellite is already up).

- GitHub issue #141: ``ephem.delta_t('0')`` now returns the correct
  value, instead of misbehaving for that particular input.

Version 3.7.6.0 (2015 August 19)
--------------------------------

- The new ``ephem.cities.lookup()`` function runs a Google geocoding
  search and returns an ``Observer`` object for the top result.

- When an Earth satellite position cannot be computed, PyEphem now
  raises an exception instead of freezing and locking up Python.

- Upgraded to the ``libastro`` from XEphem 3.7.6, bringing improvements
  to Earth satellite transit calculations.

- GitHub issue #76: Earth satellite velocity is now calculated with
  greater accuracy.

- GitHub issue #64: rising and setting routines are now careful to
  restore your ``Observer.date`` even if they die with an exception.

- GitHub issue #56: Earth satellites now raise an exception for dates a
  year or more from their TLE epoch, because ``libastro`` refuses to
  process old elements and would return nonsense coordinates.

- GitHub issue #44: a segmentation fault would eventually kill Python 3
  if a script called ``Body.copy()`` either directly or via the Standard
  Library ``copy.copy()`` function.

Version 3.7.5.3 (2014 May 29)
-----------------------------

- Gave all bodies a ``.parallactic_angle()`` method that computes the
  same angle as the ``PA`` column in XEphem itself (GitHub #24).

- Added a ``.long`` alias for the ``.lon`` longitude attribute on
  ecliptic and galactic coordinates (GitHub #41).

- Combined the Python 2 and Python 3 code bases using the magic of
  ``#ifdef`` and a barrel full of ``print()`` parentheses, which should
  prevent either version from ever falling behind the other again.

- Fixed GitHub issues #35, #37, #40.

Version 3.7.5.2 (2013 December 21)
----------------------------------

- The ``separation()`` function will no longer allow hardware floating
  point rounding errors to produce a non-zero result when a position is
  compared to itself, nor return a ``NaN`` result (which one user
  reports seeing as the angle ``1389660529:33:00.8`` degrees).
  `(GitHub #31) <https://github.com/brandon-rhodes/pyephem/issues/31>`_

- PyEphem routines no longer ignore the microseconds of ``datetime``
  objects provided as input.
  `(GitHub #29) <https://github.com/brandon-rhodes/pyephem/issues/29>`_

- PyEphem is now more careful to raise an exception if angles are
  specified using strings that contain invalid characters.

- The Earth-satellite attributes ``ra`` and ``dec`` are now correctly
  referenced to the epoch-of-date, instead of being expressed in J2000
  like the astrometric attributes.

Version 3.7.5.1 (2011 November 24)
----------------------------------

- Upgraded the underlying astronomy library to 3.7.5.

- **Incompatible Change**: the transit functions are now symmetric with
  the rising and setting functions: while they still return the date and
  time of the event, they do *not* alter the ``.date`` attribute of the
  Observer which gets passed to them.  This brings their behavior into
  line with the documentation.
  `(Launchpad #861526) <https://bugs.launchpad.net/pyephem/+bug/861526>`_

- ``Date('1986-2-9')`` now means February 9th instead of meaning “the
  beginning of 1986, minus two months, minus nine days.”
  `(Launchpad #792321) <https://bugs.launchpad.net/pyephem/+bug/792321>`_

- Earth satellite positions are now computed to six additional digits,
  in an attempt to eliminate small jumps in position that some users
  were observing in their figures.
  `(Launchpad #812906) <https://bugs.launchpad.net/pyephem/+bug/812906>`_

- Coordinate pair creation no longer leaks memory.
  `(Launchpad #798155) <https://bugs.launchpad.net/pyephem/+bug/798155>`_

Version 3.7.4.1 (2011 January 5)
---------------------------------

- Renamed the ``Observer.long`` attribute to ``lon`` after realizing
  that the official syllabification of “longitude” is “lon·gi·tude.”
  Also changed ``Body`` objects so that ``hlong`` is ``hlon`` instead.
  The old names will always be supported for compatibility with older
  programs.

- Upgraded the underlying astronomy library to 3.7.4.

- **Bugfix:** repaired the ``separation()`` function so that it no
  longer leaks memory; thanks to Enno Middelburg for the bug report!

- **Bugfix:** completely rebuilt the geographic data used by ``city()``
  after Giacomo Boffi pointed out several errors.

Version 3.7.3.4 (2009 April 30)
-------------------------------

- Added a new ``next_pass()`` method to ``Observer`` that searches for
  when a satellite next rises, culminates, and sets.

- Added a ``compute_pressure()`` method to ``Observer`` which computes
  the standard atmospheric pressure at the observer's current elevation.
  This function now gets called automatically on new ``city()`` objects
  before they are returned to the user.

- Corrected the altitude of San Francisco as returned by ``city()``.

- Improved the copyright message so that two more authors are credited.

Version 3.7.3.3 (2008 October 3)
--------------------------------

- Added ``cmsI`` and ``cmsII`` attributes to ``Jupiter`` to provide the
  central meridian longitude in both System I and System II.

- **Bugfix**: Saturn was returning the wrong values for its earthward
  and sunward angle tilt.

Version 3.7.3.2 (2008 July 2)
-----------------------------

- **Bugfix**: the rising and setting functions, if called repeatedly,
  would sometimes get hung up on a single answer which they would return
  over and over again instead of progressing to the next rising or
  setting.  They should now always progress instead of getting stuck.

Version 3.7.3.1 (2008 July 1)
-----------------------------

- **Bugfix**: the rising and setting functions were attempting to
  achieve such high precision that users sometimes found circumstances
  under which they would not complete at all!  They now stop and return
  an answer once they are withing a half-second of the real time of
  rising, transit, or setting, which solves the problem without damaging
  the quality of the results when tested against the Naval Observatory.

- Upgraded to the libastro from XEphem 3.7.3.

Version 3.7.2.4 (2008 June 12)
------------------------------

- **Incompatible Change**: After feedback from users, I have changed
  the ``Observer`` methods which find risings, settings, and transits,
  so that they do not change their Observer's ``.date`` attribute.  So
  the sequence:

  .. code-block:: python

     r1 = boston.next_rising(mars)
     r2 = boston.next_rising(mars)

  now computes the same value twice!  If you want a series of calls to
  each begin when the other left off, you can use the ``start=``
  parameter described in the next item:

  .. code-block:: python

     r1 = boston.next_rising(mars)
     r2 = boston.next_rising(mars, start=r1)

- Added an optional ``start=`` argument to the rising, setting, and
  transit ``Observer`` functions, that tells them from which date and
  time to begin their search.

- **Bugfix**: Rewrote planetary moon routines so that moons of Mars,
  Jupiter, Saturn, and Uranus now return appropriate data for years
  1999-2020.  (Each moon had been returning the unmodified position of
  its planet, because I was unsure whether I could distribute the moon
  data with PyEphem.)

- You can no longer create arbitrary attributes on an ``Observer``, to
  prevent users from accidentially saying things like
  ``here.longitude`` or ``here.lon`` when they mean ``here.long``.
  Create your own subclass of ``Observer`` if you need the power to
  set your own attributes.

- The ephem module now provides a ``__version__`` symbol.

- Added test suite that tests planet and planet moon positions
  against JPL ephemeris data (needs more work).

Version 3.7.2.3 (2008 January 8)
--------------------------------

- Three new classes ``Equatorial``, ``Ecliptic``, and ``Galactic``
  allow coordinates to be transformed between the three systems
  (ability to transform coordinates was requested by Aaron Parsons).

- Added constants for popular epochs ``B1900``, ``B1950``, and
  ``J2000``.

- Added named functions for every solstice and equinox (before, only
  the vernal equinox could be asked for specifically).

- Product tests have been moved inside of the ``ephem`` module itself.

- **Bugfix**: ``Angle()`` can no longer be directly instantiated.

- **Bugfix**: San Francisco had the wrong coordinates in the cities
  database (pointed out by Randolph Bentson).

Version 3.7.2.2 (2007 December 9)
---------------------------------

- The phases of the moon can now be determined through the functions
  ``next_new_moon()``, ``next_full_moon()``, ``previous_new_moon()``,
  et cetera.

- Added a modest database of world cities; the ``city()`` function
  returns a new Observer on each call:

  .. code-block:: python

     observer = ephem.city('Boston')

- Using the old ``rise``, ``set``, and ``transit`` attributes on
  ``Body`` objects now causes a deprecation warning.

- **Bugfix**: the last release of PyEphem omitted the constants
  ``meters_per_au``, ``earth_radius``, ``moon_radius``, and
  ``sun_radius``; the constant ``c`` (the speed of light) is also now
  available.

Version 3.7.2.1 (2007 October 1)
--------------------------------

- Functions now exist to find equinoxes and solstices.

- Bodies now cleanly offer three different versions of their
  position, rather than making the user remember obscure rules for
  having each of these three values computed:

  * Astrometric geocetric right ascension and declination
  * Apparent geocentric right ascension and declination
  * Apparent topocentric right ascension and declination

- Bodies can now find their next or previous times of transit,
  anti-transit, rising, and setting.

- A ``localtime()`` function can convert PyEphem ``Date`` objects to
  local time.

- Now ``ephem.angle`` instances can survive unary ``+`` and ``-``
  without getting changed into plain floats.

- The ``elev`` Observer attribute has been renamed to ``elevation``.

- Observers now display useful information when printed.

- Added a much more extensive test suite, which, among other things,
  now compares results with the United States Naval Observatory,
  insisting upon arcsecond agreement.

- **Bugfix**: When a fixed body is repeatedly precessed to different
  dates, its original position will no longer accumulate error.

Version 3.7.2a (2007 June)
--------------------------

- Upgraded to the libastro from XEphem 3.7.2.

- Should now compile under Windows!

- **Bugfix**: rewrote date-and-time parsing to avoid the use of
  ``sscanf()``, which was breaking under Windows and requiring the
  insertion of a leading space to succeed.

- Improved the error returned when a date string cannot be parsed,
  so that it now quotes the objectionable string (so you can tell
  which of several date strings on the same line gave an error!).

Version 3.7b  (2005 August 25)
------------------------------

- **Bugfix**: in the underlying library, earth satellite objects do
  not support ``SOLSYS`` attributes like ``sun_distance``; so
  ``EarthSatellite`` must inherit from ``Body`` rather than ``Planet``
  (and lose several attributes, which were returning nonsense values).

Version 3.7a  (2005 August 22)
------------------------------

- Upgraded to the libastro from XEphem 3.7.

- **Bugfix**: after creating an earth satellite and calling
  ``compute()``, some attributes (including ``sublat`` and
  ``sublong``) would always equal zero until you had accessed a more
  mainstream attribute (like ``ra`` or ``dec``); now, all attributes
  should return correct values on their first access.

- **Bugfix**: the ``sidereal_time()`` function of an ``Observer`` now
  returns a correct floating-point number that measures in radians,
  rather than a number in the range [0,1).

- The ``Observer`` now has an ``radec_of(az=, alt=)`` function that
  returns the right ascension and declination of a point in the sky.

- You can normalize an ``Angle`` into the range [0,2pi) by requesting
  the attribute ``.norm``.

- Earth satellite objects read in from TLE files now retain their
  TLE catalog number as an attribute ``catalog_number``.

- Uninitialized bodies now start off with ``None`` for their name,
  rather than the string ``"unnamed"``.

Version 3.6.4a  (2005 July 18)
------------------------------

- Upgraded to the libastro from XEphem 3.6.4, which:

  * No longer incorrectly applies relativistic deflection to
    objects on this side of the Sun, whose light will obviously not
    go past the sun and be deflected.

  * Now correctly handles earth satellites with a negative
    ``es_decay`` parameter.

- Added several functions to the module:

  * ``moon_phases()`` computes a new and full moon following a date.

  * ``delta_t()`` computes the difference between Terrestrial Time and
    Universal Time.

  * ``julian_date()`` computes the Julian Date for a ``date`` or
    ``Observer``.

  * ``millennium_atlas()`` and
    ``uranometria()`` and
    ``uranometria2000()`` determine the star atlas page on which a
    given location falls, given as right ascension and declination.

- Added a function to the Observer class, which takes no arguments:

  ``sidereal_time()`` computes the sidereal time for the Observer

- Each ``Observer`` now has a ``horizon`` attribute, with which you
  can specify the degrees altitude at which you define an object to be
  rising or setting.  Normally, all rising and setting times are
  computed for when the object appears to be exactly at the horizon
  (at zero degrees altitude).

Version 3.6.1a  (2004 November 25)
----------------------------------

- All major moons in the solar system are now supported.

- Added ``copy()`` method to bodies, that returns a new instance of
  the body which should be identical in all properties.

- Improved the definitions of body attributes, both in their
  docstrings and in the PyEphem Manual.

- Improved access to the orbital parameters by which the user
  defines bodies in ellipical, parabolic, and hyperbolic orbits, as
  well as artificial Earth satellites; users can now create such
  objects entirely through setting their parameters, without having
  to use the ``readdb()`` function to parse a definition of the object
  in Ephem database format.

- Source distribution now includes test suites, one of which
  actually checks to see whether your version of PyEphem produces
  the same output as the examples from the PyEphem Manual (two of
  which will fail).

- Following the same adjustment in the XEphem application, PyEphem
  now uses a default atmospheric pressure of 1010 millibar, rather
  than the old value of 1013, when computing the altitude of a body
  near the horizon.

- The ``constellation()`` function now correctly forces the
  computation of a body's ``ra`` and ``dec`` before determining the
  constellation in which the body lies.

- Code should produce cleaner compiles on many platforms.

Early History
-------------

- **27 Jul 1998**: the original PyEphem, an awkward SWIG wrapper around
  the raw ``libastro`` C structures and functions, was ready to appear
  on the “Contributed.html” page on the Python web site.  (With an
  apology from the python.org webmaster, it was not actually posted
  until 1998 August 26.)
