# The core functionalty of PyEphem lives in the C-language _libastro
# module, which packages the astronomy routines from XEphem as
# convenient Python types.

import ephem._libastro as _libastro
from math import pi, asin, tan

twopi = pi * 2.
halfpi = pi / 2.
quarterpi = pi / 4.
eighthpi = pi / 8.

degree = twopi / 360.
arcminute = degree / 60.
arcsecond = arcminute / 60.
half_arcsecond = arcsecond / 2.
twentieth_arcsecond = arcsecond / 20.

# We make available several basic types from _libastro.

Angle = _libastro.Angle
degrees = _libastro.degrees
hours = _libastro.hours

Date = _libastro.Date
hour = 1. / 24.
minute = hour / 60.
second = minute / 60.

delta_t = _libastro.delta_t

Body = _libastro.Body
Planet = _libastro.Planet
PlanetMoon = _libastro.PlanetMoon
FixedBody = _libastro.FixedBody
EllipticalBody = _libastro.EllipticalBody
ParabolicBody = _libastro.ParabolicBody
HyperbolicBody = _libastro.HyperbolicBody
EarthSatellite = _libastro.EarthSatellite

readdb = _libastro.readdb
readtle = _libastro.readtle
constellation = _libastro.constellation
separation = _libastro.separation
now = _libastro.now

# We also create a Python class ("Mercury", "Venus", etcetera) for
# each planet and moon for which _libastro offers specific algorithms.

for index, classname, name in _libastro.builtin_planets():
    exec '''
class %s(_libastro.%s):
    __planet__ = %r
''' % (name, classname, index)

del index, classname, name

# We now replace two of the classes we have just created, because
# _libastro actually provides separate types for two of the bodies.

Saturn = _libastro.Saturn
Moon = _libastro.Moon

# The "newton" function finds the zero of a function.

def newton(f, x0, x1):
    f0, f1 = f(x0), f(x1)
    while f1 and x1 != x0 and f1 != f0:
        x0, x1 = x1, x1 + (x1 - x0) / (f0/f1 - 1)
        f0, f1 = f1, f(x1)
    return x1

# Find equinoxes and solstices.

_sun = Sun()                    # used for computing equinoxes

def holiday(d0, motion, offset):
    def f(d):
        _sun.compute(d, epoch=d)
        return (_sun.ra + eighthpi) % quarterpi - eighthpi
    _sun.compute(d0, epoch=d0)
    angle_to_cover = motion - (_sun.ra + offset) % motion
    if abs(angle_to_cover) < twentieth_arcsecond:
        angle_to_cover = motion
    d = d0 + 365.25 * angle_to_cover / twopi
    return date(newton(f, d, d + hour))

def previous_vernal_equinox(date): return holiday(date, -twopi, 0)
def next_vernal_equinox(date): return holiday(date, twopi, 0)

def previous_equinox(date): return holiday(date, -pi, 0)
def next_equinox(date): return holiday(date, pi, 0)

def previous_solstice(date): return holiday(date, -pi, halfpi)
def next_solstice(date): return holiday(date, pi, halfpi)

#def previous_quarter_day(date): return holiday(date, -halfpi, 0)
#def next_quarter_day(date): return holiday(date, halfpi, 0)

#def previous_cross_quarter_day(date): return holiday(date, -halfpi, quarterpi)
#def next_cross_quarter_day(date): return holiday(date, halfpi, quarterpi)

# We provide a Python extension to our C "Observer" class that can
# find many circumstances.

class CircumpolarError(ValueError): pass
class NeverUpError(CircumpolarError): pass
class AlwaysUpError(CircumpolarError): pass

class Observer(_libastro.Observer):
    elev = _libastro.Observer.elevation

    def __repr__(self):
        "Display useful information when an Observer is printed."

        return ('<ephem.Observer date=%r epoch=%r'
                ' long=%s lat=%s elevation=%sm'
                ' horizon=%s temp=%sC pressure=%smBar>'
                % (str(self.date), str(self.epoch),
                   self.long, self.lat, self.elevation,
                   self.horizon, self.temp, self.pressure))

    def next_transit(self, body):
        "Find the next passage of a body across the meridian."

        def f(d):
            self.date = d
            body.compute(self)
            return degrees(body.ra - self.sidereal_time()).znorm
        body.compute(self)
        ha_to_move = (body.ra - self.sidereal_time()) % twopi
        if ha_to_move < twentieth_arcsecond:
            ha_to_move = twopi
        d = self.date + ha_to_move / twopi
        return newton(f, d, d + minute)

    def disallow_circumpolar(self, declination):
        "Raise an exception if the given declination is circumpolar."

        if abs(self.lat - declination) >= halfpi:
            raise NeverUpError('The declination %s never rises'
                               ' above the horizon at latitude %s'
                               % (declination, self.lat))
        if abs(self.lat + declination) >= halfpi:
            raise AlwaysUpError('The declination %s is always'
                                ' above the horizon at latitude %s'
                                % (declination, self.lat))

    def ha_from_meridian_to_horizon(self, declination):
        ("Return the distince in RA between the meridian and ideal horizon"
         " along the given line of declination.")

        self.disallow_circumpolar(declination)
        return halfpi - asin(tan(-declination) / tan(halfpi - self.lat))

    def move_to_horizon(self, body, d):
        def f(d):
            self.date = d
            body.compute(self)
            return body.alt - self.horizon
        return newton(f, d, d + minute)

    def _rising_setting(self, body, rising=False, previous=False):
        """Computation for the rising and setting functions."""

        setting = not rising
        body.compute(self)

        # If we were dealing with the ideal horizon in the absence of
        # refraction, how far would we turn the sky to place the
        # object's current location at zero degrees altitude?

        target_ha = self.ha_from_meridian_to_horizon(body.dec)
        if rising:
            target_ha = twopi - target_ha
        current_ha = self.sidereal_time() - body.ra
        ha_move = (target_ha - current_ha) % twopi
        if previous:
            ha_move = ha_move - twopi

        # But since we are dealing with refraction, and the user's
        # choice of their own self.horizon, the object might already
        # have risen when we think it its rising is in the future, or
        # vice-versa.  So we check its altitude and adjust our guess.

        distance_from_horizon = body.alt - self.horizon
        up = distance_from_horizon > - twentieth_arcsecond
        down = distance_from_horizon < twentieth_arcsecond
        already = rising and up or setting and down
        not_yet = rising and down or setting and up
        if previous:
            already, not_yet = not_yet, already

        if (already and abs(ha_move) < halfpi or
            not_yet and abs(ha_move) > pi + halfpi):
            if ha_move > 0:
                ha_move -= twopi
            else:
                ha_move += twopi

        # Finally, we submit our guess to Newton's method, which
        # determines the real moment of zero altitude.

        return self.move_to_horizon(body, self.date + ha_move / twopi)

    def previous_rising(self, body):
        "Find the previous time at which the given body rises."

        self._rising_setting(body, rising=True, previous=True)

    def previous_setting(self, body):
        "Find the previous time at which the given body rises."

        self._rising_setting(body, previous=True)

    def next_rising(self, body):
        "Find the next time at which the given body rises."

        self._rising_setting(body, rising=True)

    def next_setting(self, body):
        "Find the next time at which the given body rises."

        self._rising_setting(body)


def localtime(date):
    "Convert a PyEphem date into the local time, as a Python datetime."

    import calendar, time, datetime
    timetuple = time.localtime(calendar.timegm(date.tuple()))
    return datetime.datetime(*timetuple[:7])


def city(name):
    "Return a city from our world cities database."

    from ephem.cities import create
    return create(name)

# THINGS TO DO:
# Better docs
# Use newton to provide risings and settings

# For backwards compatibility, provide lower-case names for our Date
# and Angle classes.

date = Date
angle = Angle
