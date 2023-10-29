
Newton's Method
===============

PyEphem comes with a simple implementation of Newton's Method,
named ``newton``.

    >>> from ephem import newton

Given a continuous function and two *x* coordinates
near which the function crosses zero,
it returns the *x* coordinate of the actual zero crossing.
For example,
if asked to find the zero-crossing of the ``sin()`` function
in the vicinity of the number three,
it returns a quite good appoximation of π (“pi”):

    >>> import math
    >>> n = newton(math.sin, 3.0, 3.1)
    >>> print('%.11f' % n)
    3.14159265359

Here is a real-world example of using Newton’s method:
trying to figure out when the longitude of the Sun on Mars
crosses one of the values 0°, 90°, 180°, or 270°
that mark the solstices and equinoxes
(inspired by `a Stack Overflow question`_).

::

    # The angle that we call "the longitude of the Sun, as
    # seen from Mars" should grow at the same rate as the
    # "longitude of Mars as seen from the Sun" (since the
    # two are the same line but viewed in opposite
    # directions).
    #
    # The only problem is knowing what point to name "zero",
    # so we have to learn what .hlon was when the first
    # Martian year started:

    from ephem import Mars, Date, degrees, newton
    m = Mars()
    m.compute('1955/4/11 23:00')
    Ls0 = m.hlon

    def Ls(date):
        m.compute(date)
        return degrees(m.hlon - Ls0).norm

    # There!  Ls() should give Martian solar latitude.
    # So the first round of seasons listed at the page
    # http://www.planetary.org/explore/space-topics/mars/mars-calendar.html
    # should give 90 degrees, 180 degrees, and 270 degrees:

    for date in '1955/10/27', '1956/4/27', '1956/9/21':
        print Ls(date)

    # The output is close to what we would expect:
    #
    # 90:11:58.3
    # 179:57:32.2
    # 270:13:22.6
    #
    # Great!  So what if we want to know, say, the date
    # of the upcoming Spring Equinox or Summer Solstice?
    # We need functions that are smooth, well-behaved,
    # and cross zero at those two times, so that we can
    # unleash Newton's Method upon them:

    def spring_equinox(date):
        return Ls(date).znorm

    def summer_solstice(date):
        return Ls(date) - degrees('90:00:00')

    def find_spring_equinox(start_date):
        start_date = Date(start_date)
        y0 = Ls(start_date)
        y1 = Ls(start_date + 1)
        rate = y1 - y0
        angle_to_go = degrees(0.0 - y0).norm
        closer_date = start_date + angle_to_go / rate
        d = newton(spring_equinox, closer_date, closer_date + 1)
        return Date(d)

    def find_summer_solstice(start_date):
        start_date = Date(start_date)
        y0 = Ls(start_date)
        y1 = Ls(start_date + 1)
        rate = y1 - y0
        angle_to_go = degrees(degrees('90:00:00') - y0).norm
        closer_date = start_date + angle_to_go / rate
        d = newton(summer_solstice, closer_date, closer_date + 1)
        return Date(d)

    d = find_spring_equinox('2015/1/22')
    print d, Ls(d)

    d = find_summer_solstice('2015/1/22')
    print d, Ls(d)

    # Output:
    # 2015/6/16 15:03:15 0:00:00.0
    # 2015/12/31 21:12:07 90:00:00.0

.. _a Stack Overflow question: https://stackoverflow.com/questions/25538926/solar-longitude-from-pyephem/
