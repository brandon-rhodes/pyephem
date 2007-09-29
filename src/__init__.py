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

# Newton's method.

def newton(f, x0, x1):
    """Return an x-value at which the given function reaches zero."""
    f0, f1 = f(x0), f(x1)
    while f1 and x1 != x0 and f1 != f0:
        x0, x1 = x1, x1 + (x1 - x0) / (f0/f1 - 1)
        f0, f1 = f1, f(x1)
    return x1

# Find equinoxes and solstices.

_sun = Sun()                    # used for computing equinoxes

def holiday(d0, motion, offset):
    """Function that assists the finding of equinoxes and solstices."""

    def f(d):
        _sun.compute(d, epoch=d)
        return (_sun.ra + eighthpi) % quarterpi - eighthpi
    _sun.compute(d0, epoch=d0)
    angle_to_cover = motion - (_sun.ra + offset) % motion
    if abs(angle_to_cover) < twentieth_arcsecond:
        angle_to_cover = motion
    d = d0 + 365.25 * angle_to_cover / twopi
    return date(newton(f, d, d + hour))

def previous_vernal_equinox(date):
    """Return the date of the previous vernal equinox."""
    return holiday(date, -twopi, 0)

def next_vernal_equinox(date):
    """Return the date of the next vernal equinox."""
    return holiday(date, twopi, 0)

def previous_equinox(date):
    """Return the date of the previous equinox."""
    return holiday(date, -pi, 0)

def next_equinox(date):
    """Return the date of the next equinox."""
    return holiday(date, pi, 0)

def previous_solstice(date):
    """Return the date of the previous solstice."""
    return holiday(date, -pi, halfpi)

def next_solstice(date):
    """Return the date of the next solstice."""
    return holiday(date, pi, halfpi)

# We provide a Python extension to our C "Observer" class that can
# find many circumstances.

class CircumpolarError(ValueError): pass
class NeverUpError(CircumpolarError): pass
class AlwaysUpError(CircumpolarError): pass

class Observer(_libastro.Observer):
    elev = _libastro.Observer.elevation

    def __repr__(self):
        """Print a useful representation of this Observer."""

        return ('<ephem.Observer date=%r epoch=%r'
                ' long=%s lat=%s elevation=%sm'
                ' horizon=%s temp=%sC pressure=%smBar>'
                % (str(self.date), str(self.epoch),
                   self.long, self.lat, self.elevation,
                   self.horizon, self.temp, self.pressure))

    def _compute_transit(self, body, sign):
        """Internal function used to compute transits."""

        def f(d):
            self.date = d
            body.compute(self)
            return degrees(body.ra - self.sidereal_time()).znorm
        body.compute(self)
        ha_to_move = (body.ra - self.sidereal_time()) % (sign * twopi)
        if abs(ha_to_move) < twentieth_arcsecond:
            ha_to_move = sign * twopi
        d = self.date + ha_to_move / twopi
        return newton(f, d, d + minute)

    def previous_transit(self, body):
        """Find the next passage of a body across the meridian."""

        return self._compute_transit(body, -1)

    def next_transit(self, body):
        """Find the next passage of a body across the meridian."""

        return self._compute_transit(body, +1)

    def disallow_circumpolar(self, declination):
        """Raise an exception if the given declination is circumpolar.

        Raises NeverUpError if an object at the given declination is
        always below this Observer's horizon, or AlwaysUpError if such
        an object would always be above the horizon.

        """
        if abs(self.lat - declination) >= halfpi:
            raise NeverUpError('The declination %s never rises'
                               ' above the horizon at latitude %s'
                               % (declination, self.lat))
        if abs(self.lat + declination) >= halfpi:
            raise AlwaysUpError('The declination %s is always'
                                ' above the horizon at latitude %s'
                                % (declination, self.lat))

    def ha_from_meridian_to_horizon(self, declination):
        """Return the HA between the meridian and ideal horizon.

        Return the angle through which the sky has to rotate (thus,
        the result could be called either a difference in RA or HA) in
        order to bring an object at the given declination from the
        meridian to the ideal horizon.

        """
        self.disallow_circumpolar(declination)
        return halfpi - asin(tan(-declination) / tan(halfpi - self.lat))

    def move_to_horizon(self, body, date):
        """Run Netwon's method to bring the given body to the horizon."""
        def f(d):
            self.date = d
            body.compute(self)
            return body.alt - self.horizon
        return Date(newton(f, date, date + minute))

    def _rising_setting(self, body, rising=False, previous=False):
        """Computation for the rising and setting functions."""

        setting = not rising
        body.compute(self)

        # Compute, for the ideal horizon in the absence of refraction,
        # how far we must turn the sky in order to place the body's
        # current RA/dec at zero degrees altitude.

        target_ha = self.ha_from_meridian_to_horizon(body.dec)
        if rising:
            target_ha = twopi - target_ha
        current_ha = self.sidereal_time() - body.ra
        ha_move = (target_ha - current_ha) % twopi

        # But because of refraction and the user's own choice of
        # self.horizon, the body, when at the ideal horizon, might
        # already have passed the circumstances for which we search.
        # So we check for this condition, and adjust our guess.

        if previous:
            ha_move = twopi - ha_move

        az = body.az
        if (rising and 0 < az < pi) or (setting and pi < az < twopi):

            alt_above_horizon = body.alt - self.horizon
            if bool(previous) == bool(rising): # Python for "not xor"
                do_big_move = alt_above_horizon < twentieth_arcsecond
            else:
                do_big_move = alt_above_horizon > - twentieth_arcsecond

            if do_big_move:
                if ha_move < pi:
                    ha_move += twopi
            else:
                if ha_move > pi:
                    ha_move -= twopi

        if previous:
            ha_move = - ha_move

        # Finally, we submit our guess to Newton's method, which
        # determines the real moment of zero altitude.

        return self.move_to_horizon(body, self.date + ha_move / twopi)

    def previous_rising(self, body):
        """Find the previous time at which the given body rises."""
        return self._rising_setting(body, rising=True, previous=True)

    def previous_setting(self, body):
        """Find the previous time at which the given body rises."""
        return self._rising_setting(body, previous=True)

    def next_rising(self, body):
        """Find the next time at which the given body rises."""
        return self._rising_setting(body, rising=True)

    def next_setting(self, body):
        """Find the next time at which the given body rises."""
        return self._rising_setting(body)


def localtime(date):
    """Convert a PyEphem date into local time, returning a Python datetime."""
    import calendar, time, datetime
    timetuple = time.localtime(calendar.timegm(date.tuple()))
    return datetime.datetime(*timetuple[:7])


def city(name):
    """Return a city from our world cities database."""
    from ephem.cities import create
    return create(name)

# THINGS TO DO:
# Better docs
# Use newton to provide risings and settings

# For backwards compatibility, provide lower-case names for our Date
# and Angle classes.

date = Date
angle = Angle
