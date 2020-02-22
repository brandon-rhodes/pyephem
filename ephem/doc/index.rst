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
   </p>

   </td>
   <td>

   <p>Download PyEphem for Windows, Linux, or as source code,
   directly from the <b>Python Package Index</b>.</p>
   <img src="_static/python.png"/>
   <p>
     <a href="https://pypi.python.org/pypi/ephem/"
      >PyPI PyEphem page</a>
     <br/>
     Includes Windows installers!
     <br/>
     <br/>
     <a href="https://pypi.python.org/packages/source/e/ephem/ephem-3.7.6.0.tar.gz#md5=0e33905844e3be5c901c13e5a9c938a3"
      >Source code</a><br/>
     As a .tar.gz file
   </p>

   </td>
   <td>

   <p>
     Ask questions on <b>Stack Overflow</b>, or use our community support
     tools on <b>GitHub</b>!
   </p>
   <img src="_static/stackoverflow.png"/>
   <p>
     <a href="http://stackoverflow.com/questions/tagged/pyephem"
        >PyEphem Q&amp;A</a><br/>
     <a href="http://stackoverflow.com/questions/ask?tags=pyephem"
        >Ask a new question</a><br/>
   </p>
   <img src="_static/github.png"/>
   <p>
     <a href="https://github.com/brandon-rhodes/pyephem"
        >Code Repository</a><br/>
     <a href="https://github.com/brandon-rhodes/pyephem/issues?state=open"
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

   <div class="sideexample">

>>> import ephem

>>> mars = ephem.Mars()
>>> mars.compute()
>>> print mars.ra, mars.dec
6:05:56.34 23:23:40.0
>>> ephem.constellation(mars)
('Gem', 'Gemini')

>>> boston = ephem.Observer()
>>> boston.lat = '42.37'
>>> boston.lon = '-71.03'
>>> mars.compute(boston)
>>> print mars.az, mars.alt
37:55:48.9 -14:23:11.8

>>> boston.next_rising(mars)
2007/10/2 02:31:51
>>> print mars.az
56:52:52.1

>>> boston.next_transit(mars)
2007/10/2 10:07:47
>>> print mars.alt
71:02:16.3

.. raw:: html

   </div>

PyEphem is in maintenance mode
==============================

The PyEphem astronomy library
has helped generations of Python programmers
locate the stars, planets, and Earth satellites.
It **will continue to receive bugfixes**
and **will be ported to new versions of Python**,
but it no longer stands at the cutting edge of astronomy in Python.

See below for newer alternatives
that offer a more Pythonic approach to astronomy in Python!

What is PyEphem?
================

**PyEphem** provides basic astronomical computations
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

Awkward edges
=============

PyEphem’s design has several flaws
that have been avoided by the newer astronomy libraries
listed in the next section.
Among the problems it creates for Python programmers are:

* Instead of offering a clear way for the user to specify units
  of radians or degrees,
  PyEphem uses the terrible and confusing convention
  that floats like ``1.23`` mean radians
  but strings like ``'1.23'`` mean degrees of declination
  and hours of right ascension.
  This has wasted many hours of programmer time and confusion over the years,
  is completely unnatural and at odds with how Python usually works,
  and can never be fixed
  because it would break all existing PyEphem programs.

* The API is also awkward
  because it mutates objects in-place instead of returning results.
  Instead of returning coordinates directly,
  ``compute()`` updates several fields on its object —
  reflecting how the underlying C library works, which I didn’t write.
  This makes a second line of code necessary
  to go fetch the coordinates from the object.

* PyEphem is difficult to release
  and difficult for many people to install,
  because some of its code is written in the C language
  and so a compiler is needed.

* PyEphem does not interoperate with NumPy
  and so is awkward to use in a modern IPython Notebook.

For all of these reasons, PyEphem might not be the best choice
for a new project.

Two Alternatives
----------------

As the principle author of PyEphem,
I — `Brandon Rhodes <https://rhodesmill.org/brandon/>`_ —
had often thought about starting over again
so I would have a second chance
to avoid the mistakes I made with PyEphem!
It was only in 2013
that I discovered an excellent excuse:
back in the year 2000, the IAU (International Astronomical Union)
had
`thoroughly upgraded how astronomical positions are measured <https://syrte.obspm.fr/IAU_resolutions/Resol-UAI.htm>`_
to allow much higher accuracy.
So a rewrite could serve two purposes!
Instead of simply rewriting what PyEphem did, but in better Python,
I could implement the newer standards of measurement
and deliver much higher precision.

Following the United States Naval Observatory’s free
“NOVAS” library’s C code as my example
(for which they
also maintain a `Python interface <http://aa.usno.navy.mil/software/novas/novas_py/novaspy_intro.php>`_),
I started writing a new library that I named “Skyfield”
with the goal of implementing the highest accuracy algorithms,
using only Python and NumPy,
behind a beautiful Pythonic API.
Version 1.0 was released in early 2017
and both I and a few other contributions continue to add new features:

`The Skyfield astronomy library <https://rhodesmill.org/skyfield/>`_

I recommend using Skyfield instead of PyEphem
if it’s possible for your new project to do so!
(The only thing missing at this point is predicting positions
from Kelperian orbital elements for comets and asteroids.)

If you are a professional astronomer
interested in writing programs that interoperate
with those of other astronomers,
you will also want to consider Astropy.
While it is not as sleek as Skyfield —
it bundles many dependencies written in other languages
and was not designed for beginners —
it is a much more comprehensive toolkit
that is very popular with professional astronomers:

`The Astropy astronomy library <http://www.astropy.org/>`_

But if you want it anyway, PyEphem is still available.

Installation
------------

Version **3.7.7.1** is the most recent release of PyEphem.
Consult the :doc:`CHANGELOG` to see the new features!

The easiest way to install PyEphem on a Linux or Mac OS machine,
after making sure that “Python.h” and the other Python header files
are installed (which on Ubuntu requires the “python-dev” package),
is to use the pip_ command, like this:

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

Documentation
-------------

.. toctree::
   :maxdepth: 2

   quick
   tutorial
   catalogs
   CHANGELOG
   rise-set
   radec
   coordinates
   date
   angle
   examples
   newton
   reference

.. raw:: html

   </td></tr>

   </table>
