
============
Introduction
============


>>> import sgp4


>>> from ephem.planets import Ephemeris
>>> eph = Ephemeris()
>>> earth, mars = eph.earth, eph.mars
>>> print earth(2414993.5).observe(mars).astrometric()


>>> import numpy as np
>>> t0 = 2414993.5
>>> t = np.arange(t0, t0 + 5, 1.0)
>>> print earth(t).observe(mars).astrometric()
