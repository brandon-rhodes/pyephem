
.. raw:: html

   <table class="triad" cellspacing="0"> <!-- for IE -->

   <tr class="cap">
   <td><img class="corner2" src="_static/corner2.png"/></td>
   <td><img class="corner2" src="_static/corner2.png"/></td>
   <td><img class="corner2" src="_static/corner2.png"/></td>
   </tr>

   <tr class="sites"><td>

   <p>Welcome to the</p>
   <img src="_static/pyephem-logo-short.png"/>
   <p>Home Page!<br>Documentation:</p>
   <p>
     <a href="toc.html">Table of Contents</a><br>
     <a href="quick.html">Quick Reference</a><br>
     <a href="CHANGELOG.html">Changelog</a>
   </p>
   </td>
   <td>

   <p>Download PyEphem for Windows, Linux, or as source code,
   directly from the <b>Python Package Index</b>.</p>
   <img src="_static/python.png"/>
   <p>
     <a href="https://pypi.org/project/ephem/"
      >PyPI PyEphem page</a>
     <br/>
     Includes Windows installers!
   </p>
   </td>
   <td>

   <p>
     The project source code and issue tracker are hosted on <b>GitHub</b>.
   </p>
   <img src="_static/github.png"/>
   <p>
     <a href="https://github.com/brandon-rhodes/pyephem"
        >Code Repository</a><br/>
     <a href="https://github.com/brandon-rhodes/pyephem/issues"
        >Issue Tracker</a><br/>
   </p>
   </td></tr>

   <tr class="toe">
   <td></td>
   <td><img class="corner3" src="_static/corner3.png"/></td>
   <td><img class="corner3" src="_static/corner3.png"/></td>
   </tr>

   <tr class="mount">
   <td></td>
   </tr>

   <tr class="base">
   <td colspan=3>

===================
 PyEphem Home Page
===================

Welcome to the home page of the **PyEphem astronomy library** for Python!

>>> import ephem
>>> mars = ephem.Mars()
>>> mars.compute('2007/10/02 00:50:22')
>>> print mars.ra, mars.dec
6:05:56.34 23:23:40.0
>>> ephem.constellation(mars)
('Gem', 'Gemini')

Since its first release in 1998,
PyEphem has given Python programmers
the ability to compute
**planet, comet, asteroid, and Earth satellite positions**.
It wraps the “libastro” C library
that powers the XEphem_ astronomy application for UNIX —
whose author Elwood Downey generously gave permission
for PyEphem to use his library with Python.
PyEphem can also
compute the angular separation between two objects in the sky,
determine the constellation in which an object lies,
and find the times an object rises, transits, and sets.

PyEphem will continue to receive critical bugfixes
and be ported to each new version of Python.
But be warned that it has some rough edges!

* The `Skyfield astronomy library <https://rhodesmill.org/skyfield/>`_
  should be preferred over PyEphem for new projects.
  Its modern design encourages better Python code,
  and uses NumPy to accelerate its calculations.

* Because PyEphem includes a C library,
  installation issues often frustrate users.
  If the Package Index lacks a wheel for your system,
  you will need a C compiler and Python development environment
  to get PyEphem installed.

* Instead of making angular units explicit in your code,
  PyEphem tried to be clever
  but only wound up being obscure.
  An input string ``'1.23'`` is parsed as degrees of declination
  (or hours, when setting right ascension)
  but a float ``1.23`` is assumed to be in radians.
  Angles returned by PyEphem are even more confusing:
  print them, and they display degrees;
  but do math with them, and you will find they are radians.
  This causes substantial confusion and makes code much more difficult to read,
  but can never be fixed without breaking programs that already use PyEphem.

* The PyEphem ``compute()`` method mutates its object in-place
  instead of returning results.
  While this reflects how the underlying C library works,
  it makes it hard to use ``compute()`` inside a list comprehension —
  you get a list of ``None`` objects.

* PyEphem generates positions using the 1980s techniques
  popularized in |Meeus|_,
  like the IAU 1980 model of Earth nutation
  and VSOP87 planetary theory.
  These make PyEphem faster and more compact
  than modern astronomy libraries,
  but limit its accuracy to around 1 arcsecond.
  This is often sufficient for most amateur astronomy,
  but users needing higher precision should investigate
  a more modern Python astronomy library like Skyfield or AstroPy.

.. |Meeus| replace::  Jean Meeus’s *Astronomical Algorithms*
.. _Meeus: https://www.willbell.com/math/mc1.htm
.. _XEphem: http://www.clearskyinstitute.com/xephem/

Here’s more example code to illustrate how PyEphem works:

>>> boston = ephem.Observer()
>>> boston.lat = '42.37'
>>> boston.lon = '-71.03'
>>> boston.date = '2007/10/02 00:50:22'
>>> mars.compute(boston)
>>> print mars.az, mars.alt
37:55:48.9 -14:23:11.8

>>> print(boston.next_rising(mars))
2007/10/2 02:31:51
>>> print mars.az         # degrees when printed
56:52:52.1
>>> print mars.az + 0.0   # radians in math
0.992763221264

>>> print(boston.next_transit(mars))
2007/10/2 10:07:47
>>> print mars.alt        # degrees when printed
71:02:16.3
>>> print mars.alt + 0.0  # radians in math
1.23984456062

Installing PyEphem
==================

You can try installing PyEphem with:

.. _pip: http://pypi.python.org/pypi/pip
.. code-block:: bash

   $ pip install pyephem

Better yet,
you can use virtualenv_ to create a virtual environment,
and then run its ``pip`` instead of your system-wide one.
Then you will avoid having to gain administrator rights on your machine
before performing the installation.

If instead you want to download the Windows installer
or even the raw PyEphem source code,
you should visit the `PyEphem entry`_
at the Python Package Index,
or use the links at the top of this page.

.. _PyEphem entry: http://pypi.python.org/pypi/pyephem/
.. _virtualenv: http://pypi.python.org/pypi/virtualenv

.. raw:: html

   </td></tr>

   </table>
