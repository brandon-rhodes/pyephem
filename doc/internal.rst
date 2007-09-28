
==================
Internal Functions
==================

Several internal functions of PyEphem are documented here,
both to remind me what they are supposed to do,
and to provide doctests to make sure they keep doing it.

    >>> import unittest
    >>> import ephem
    >>> t = unittest.TestCase('id')   # dummy TestCase for "assert"ing
    >>> def degree_range(a, b):
    ...     return [ ephem.degrees(str(n)) for n in range(a, b) ]

They are:
    
``observer.disallow_circumpolarness(declination)``
 Raises an exception if the given declination
 is circumpolar for the given observer.
 The exception raised is either ``NeverUpError`` or ``AlwaysUpError``
 depending on whether the object remains above the horizon or not.

    >>> o = ephem.city('Atlanta')
    >>> for dec in degree_range(-90, -56):
    ...     t.assertRaises(ephem.NeverUpError, o.disallow_circumpolar, dec)
    >>> for dec in degree_range(-56, +57):
    ...     o.disallow_circumpolar(dec)   # raises no exception
    >>> for dec in degree_range(+57, 91):
    ...     t.assertRaises(ephem.AlwaysUpError, o.disallow_circumpolar, dec)

 At the equator, all celestial objects should be visible:

    >>> o = ephem.Observer()
    >>> o.long = o.lat = o.elevation = 0
    >>> for dec in degree_range(-90, -91):
    ...     o.disallow_circumpolar(dec)   # raises no exception

 South of the equator,
 the exceptions returned near the poles are reversed:

    >>> o = ephem.city('Adelaide')
    >>> for dec in degree_range(-90, -55):
    ...     t.assertRaises(ephem.AlwaysUpError, o.disallow_circumpolar, dec)
    >>> for dec in degree_range(-55, +56):
    ...     o.disallow_circumpolar(dec)   # raises no exception
    >>> for dec in degree_range(+56, 91):
    ...     t.assertRaises(ephem.NeverUpError, o.disallow_circumpolar, dec)


``observer.ha_from_meridian_to_horizon(declination)``
 Given a declination,
 returns the hour angle difference (as bare radians) 
 between the intersection of that circle of declination
 with the meridian and with the ideal horizon.
 In other words,
 this computes “how far” the sky has to turn
 to move an object at that declination
 from the meridian all the way down to the horizon.

    >>> o = ephem.city('Atlanta')
    >>> for dec in degree_range(-90, -56):
    ...     t.assertRaises(ephem.NeverUpError,
    ...                    o.ha_from_meridian_to_horizon, dec)
    >>> for dec in ('-50', '-25', '0', '25', '50'):
    ...     dec = ephem.degrees(dec)
    ...     print ephem.degrees(o.ha_from_meridian_to_horizon(dec))
    37:08:51.3
    71:49:38.5
    90:00:00.0
    108:10:21.5
    142:51:08.7
    >>> for dec in degree_range(+57, 91):
    ...     t.assertRaises(ephem.AlwaysUpError,
    ...                    o.ha_from_meridian_to_horizon, dec)

 For a city in the southern hemisphere,
 the results will be the reverse:

    >>> o = ephem.city('Adelaide')
    >>> for dec in degree_range(-90, -55):
    ...     t.assertRaises(ephem.AlwaysUpError,
    ...                    o.ha_from_meridian_to_horizon, dec)
    >>> for dec in ('-50', '-25', '0', '25', '50'):
    ...     dec = ephem.degrees(dec)
    ...     print ephem.degrees(o.ha_from_meridian_to_horizon(dec))
    146:17:37.8
    108:59:45.7
    90:00:00.0
    71:00:14.3
    33:42:22.2
    >>> for dec in degree_range(+56, 91):
    ...     t.assertRaises(ephem.NeverUpError,
    ...                    o.ha_from_meridian_to_horizon, dec)

