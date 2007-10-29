# The core functionalty of PyEphem lives in the C-language _libastro
# module, which packages the astronomy routines from XEphem as
# convenient Python types.

import ephem._libastro as _libastro
from math import pi

twopi = pi * 2.
halfpi = pi / 2.
quarterpi = pi / 4.
eighthpi = pi / 8.

degree = twopi / 360.
arcminute = degree / 60.
arcsecond = arcminute / 60.
half_arcsecond = arcsecond / 2.
tiny = arcsecond / 360.

# We make available several basic types from _libastro.

Angle = _libastro.Angle
degrees = _libastro.degrees
hours = _libastro.hours

Date = _libastro.Date
hour = 1. / 24.
minute = hour / 60.
second = minute / 60.

delta_t = _libastro.delta_t
julian_date = _libastro.julian_date

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
eq_ecl = _libastro.eq_ecl
now = _libastro.now

millennium_atlas = _libastro.millennium_atlas
uranometria = _libastro.uranometria
uranometria2000 = _libastro.uranometria2000

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
        _sun.compute(d)
        return (_sun.ra + eighthpi) % quarterpi - eighthpi
    d0 = Date(d0)
    _sun.compute(d0)
    angle_to_cover = motion - (_sun.ra + offset) % motion
    if abs(angle_to_cover) < tiny:
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

# Find phases of the Moon.

_moon = Moon()                  # used for computing Moon phases

def _find_moon_phase(d0, motion, target):
    """Function that assists the finding of moon phases."""

    def f(d):
        _sun.compute(d)
        _moon.compute(d)
        slong = eq_ecl(d, _sun.g_ra, _sun.g_dec)[0]
        mlong = eq_ecl(d, _moon.g_ra, _moon.g_dec)[0]
        longdiff = (mlong - slong - antitarget) % twopi - pi
        if abs(longdiff) < 1e-10:   # Moon position is not a continuous function
            return 0
        return longdiff
    antitarget = target + pi
    f0 = f(d0)
    angle_to_cover = (- f0) % motion
    if abs(angle_to_cover) < tiny:
        angle_to_cover = motion
    d = d0 + 29.53 * angle_to_cover / twopi
    return date(newton(f, d, d + hour))

def previous_new_moon(date):
    """Return the date of the previous New Moon."""
    return _find_moon_phase(date, -twopi, 0)

def next_new_moon(date):
    """Return the date of the next New Moon."""
    return _find_moon_phase(date, twopi, 0)

def previous_first_quarter_moon(date):
    """Return the date of the previous First Quarter Moon."""
    return _find_moon_phase(date, -twopi, halfpi)

def next_first_quarter_moon(date):
    """Return the date of the next First Quarter Moon."""
    return _find_moon_phase(date, twopi, halfpi)

def previous_full_moon(date):
    """Return the date of the previous Full Moon."""
    return _find_moon_phase(date, -twopi, pi)

def next_full_moon(date):
    """Return the date of the next Full Moon."""
    return _find_moon_phase(date, twopi, pi)

def previous_third_quarter_moon(date):
    """Return the date of the previous Last Quarter Moon."""
    return _find_moon_phase(date, -twopi, pi + halfpi)

def next_third_quarter_moon(date):
    """Return the date of the next Last Quarter Moon."""
    return _find_moon_phase(date, twopi, pi + halfpi)

# We provide a Python extension to our _libastro "Observer" class that
# can search for circumstances like transits.

class CircumpolarError(ValueError): pass
class NeverUpError(CircumpolarError): pass
class AlwaysUpError(CircumpolarError): pass

class Observer(_libastro.Observer):
    elev = _libastro.Observer.elevation

    def __repr__(self):
        """Return a useful textual representation of this Observer."""

        return ('<ephem.Observer date=%r epoch=%r'
                ' long=%s lat=%s elevation=%sm'
                ' horizon=%s temp=%sC pressure=%smBar>'
                % (str(self.date), str(self.epoch),
                   self.long, self.lat, self.elevation,
                   self.horizon, self.temp, self.pressure))

    def _compute_transit(self, body, sign, offset=0.):
        """Internal function used to compute transits."""

        def f(d):
            self.date = d
            body.compute(self)
            return degrees(offset - sidereal_time() + body.g_ra).znorm
        sidereal_time = self.sidereal_time
        body.compute(self)
        ha = sidereal_time() - body.g_ra
        ha_to_move = (offset - ha) % (sign * twopi)
        if abs(ha_to_move) < tiny:
            ha_to_move = sign * twopi
        d = self.date + ha_to_move / twopi
        return Date(newton(f, d, d + minute))

    def previous_transit(self, body):
        """Find the previous passage of a body across the meridian."""

        return self._compute_transit(body, -1., 0.)

    def next_transit(self, body):
        """Find the next passage of a body across the meridian."""

        return self._compute_transit(body, +1., 0.)

    def previous_antitransit(self, body):
        """Find the previous passage of a body across the anti-meridian."""

        return self._compute_transit(body, -1., pi)

    def next_antitransit(self, body):
        """Find the next passage of a body across the anti-meridian."""

        return self._compute_transit(body, +1., pi)

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

    def _prev_helper(self, body, rising, previous):
        """Internal function for finding risings and settings."""

        def visit_transit():
            d = (previous and self.previous_transit(body)
                 or self.next_transit(body)) # if-then
            if body.alt + body.radius - self.horizon <= 0:
                raise NeverUpError('%r transits below the horizon at %s'
                                   % (body.name, d))
            return d

        def visit_antitransit():
            d = (previous and self.previous_antitransit(body)
                 or self.next_antitransit(body)) # if-then
            if body.alt + body.radius - self.horizon >= 0:
                raise AlwaysUpError('%r is still above the horizon at %s'
                                    % (body.name, d))
            return d

        body.compute(self)
        heading_downward = (rising == previous) # "==" is inverted "xor"
        if heading_downward:
            on_lower_cusp = body.alt + body.radius - self.horizon > tiny
        else:
            on_lower_cusp = body.alt + body.radius - self.horizon < - tiny

        az = body.az
        on_right_side_of_sky = ((rising == (az < pi)) # inverted "xor"
                                or (az < tiny
                                    or pi - tiny < az < pi + tiny
                                    or twopi - tiny < az))

        if on_lower_cusp and on_right_side_of_sky:
            d0 = self.date
        elif heading_downward:
            d0 = visit_transit()
        else:
            d0 = visit_antitransit()
        if heading_downward:
            d1 = visit_antitransit()
        else:
            d1 = visit_transit()

        def f(d):
            self.date = d
            body.compute(self)
            return body.alt + body.radius - self.horizon

        d = (d0 + d1) / 2.
        return Date(newton(f, d, d + minute))

    def previous_rising(self, body):
        """Move to the given body's previous rising, returning the date."""
        return self._prev_helper(body, True, True)

    def previous_setting(self, body):
        """Move to the given body's previous setting, returning the date."""
        return self._prev_helper(body, False, True)

    def next_rising(self, body):
        """Move to the given body's next rising, returning the date."""
        return self._prev_helper(body, True, False)

    def next_setting(self, body):
        """Move to the given body's next setting, returning the date."""
        return self._prev_helper(body, False, False)


def localtime(date):
    """Convert a PyEphem date into local time, returning a Python datetime."""
    import calendar, time, datetime
    timetuple = time.localtime(calendar.timegm(date.tuple()))
    return datetime.datetime(*timetuple[:7])


# For backwards compatibility, provide lower-case names for our Date
# and Angle classes.

date = Date
angle = Angle

# Catalog boostraps.  Each of these functions imports a catalog
# module, then replaces itself with the function of the same name that
# lives inside of the catalog.

def star(name, *args, **kwargs):
    global star
    import ephem.stars
    star = ephem.stars.star  # by the function in the "stars" module
    return star(name, *args, **kwargs)

def city(name):
    """Return a city from our world cities database."""
    global city
    import ephem.cities
    city = ephem.cities.city
    return city(name)
