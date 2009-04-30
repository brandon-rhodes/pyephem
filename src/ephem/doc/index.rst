.. raw:: html

   <table class="triad" cellspacing="0"> <!-- for IE -->

   <tr class="cap">
   <td><img class="corner2" src="_static/corner2.png"/></td>
   <td><img class="corner2" src="_static/corner2.png"/></td>
   <td><img class="corner2" src="_static/corner2.png"/></td>
   </tr>

   <tr class="sites"><td>

   <p>This site is the PyEphem <b>home page</b></p>
   <img src="_static/pyephem-logo-short.png"/>
   <p>Simply <b>scroll down</b> to find:</p>
   <p>
     Installation Guide<br/>
     Documentation<br/>
     Data Sources<br/>
   </ul>

   </td>
   <td>

   <p>Download PyEphem for Windows, Linux, or as source code,
   directly from the <b>Python Package Index</b>.</p>
   <img src="_static/python.png"/>
   <p>
     <a href="http://pypi.python.org/pypi/pyephem/"
      >PyPI PyEphem page</a><br/><br/>
     <a href="http://pypi.python.org/packages/2.5/p/pyephem/pyephem-3.7.3.4.win32-py2.5.exe"
      >Python 2.5 Windows installer<br/>
     <a href="http://pypi.python.org/packages/2.4/p/pyephem/pyephem-3.7.3.4.win32-py2.4.exe"
      >Python 2.4 Windows installer<br/>
     <a href="http://pypi.python.org/packages/source/p/pyephem/pyephem-3.7.3.4.tar.gz"
      >Source code (.tar.gz)<br/>
   </p>

   </td>
   <td>

   <p>We use <b>Launchpad</b> to host our community support tools
   and our source code repository.</p>
   <img src="_static/launchpad.png"/>
   <p>
     <a href="http://launchpad.net/pyephem"
      >LaunchPad PyEphem page</a><br/><br/>
     <a href="http://answers.launchpad.net/pyephem">Q&A Forum</a><br/>
     <a href="http://launchpad.net/pyephem/+announcements">News Feed</a><br/>
     <a href="http://bugs.launchpad.net/pyephem">Bug Tracker</a><br/>
     <a href="http://code.launchpad.net/pyephem">Code Repository</a><br/>
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

   <div class="sideexample">

>>> import ephem

>>> mars = ephem.Mars()
>>> mars.compute()
>>> print(mars.ra, mars.dec)
6:05:56.34 23:23:40.0
>>> ephem.constellation(mars)
('Gem', 'Gemini')

>>> boston = ephem.Observer()
>>> boston.lat = '42.37'
>>> boston.long = '-71.03'
>>> mars.compute(boston)
>>> print(mars.az, mars.alt)
37:55:48.9 -14:23:11.8

>>> boston.next_rising(mars)
2007/10/2 02:31:51
>>> print(mars.az)
56:52:52.1

>>> boston.next_transit(mars)
2007/10/2 10:07:47
>>> print(mars.alt)
71:02:16.3

.. raw:: html

   </div>

Welcome!
========

**PyEphem** provides scientific-grade astronomical computations
for the Python_ programming language.
Given a date and location on the Earth's surface,
it can compute the positions of the Sun and Moon,
of the planets and their moons,
and of any asteroids, comets, or earth satellites
whose orbital elements the user can provide.
Additional functions are provided to compute the angular separation
between two objects in the sky,
to determine the constellation in which an object lies,
and to find the times at which
an object rises, transits, and sets on a particular day.

.. _Python: http://www.python.org/

The numerical routines that lie behind PyEphem
are those from the wonderful XEphem_ astronomy application,
whose author, Elwood Downey, generously gave permission
for us to use them as the basis for PyEphem.

.. _XEphem: http://www.clearskyinstitute.com/xephem/

Installation
------------

Version **3.7.3.4** is the most recent release of PyEphem.
Consult the `change log`_ to see the new features,
including an **incompatible** change
in the way that the rising and settings functions operate!

.. _change log: CHANGELOG

The easiest way to install PyEphem,
if you have the easy_install_ Python command,
is to run:

.. _easy_install: http://peak.telecommunity.com/DevCenter/EasyInstall
.. code-block:: bash

   $ easy_install pyephem

If you choose to create a virtualenv_ environment
and then run its ``easy_install`` instead of your system-wide one,
then you will not even have to gain administrator rights to your machine
before performing the installation.

But if you want to download the Windows installer,
the raw ``.egg`` archives in which PyEphem is delivered,
or the source code,
then you should visit the `PyEphem entry`_
at the Python Package Index,
or use the links at the top of this page.

.. _PyEphem entry: http://pypi.python.org/pypi/pyephem/
.. _virtualenv: http://pypi.python.org/pypi/virtualenv

Documentation
-------------

.. toctree::
   :maxdepth: 2

   quick
   tutorial
   catalogs
   CHANGELOG
   radec
   coordinates
   date
   angle
   examples
   newton

.. raw:: html

   </td></tr>

   </table>
