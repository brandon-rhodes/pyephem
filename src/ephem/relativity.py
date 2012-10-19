from math import fabs, sqrt
from ephem import planets

C = 299792458.0
AU = 1.4959787069098932e+11
C_AUDAY = 173.1446326846693

# Heliocentric gravitational constant in meters^3 / second^2, from DE-405.

GS = 1.32712440017987e+20

deflectors = [
    planets.sun, planets.jupiter, planets.saturn, planets.moon,
    planets.venus, planets.uranus, planets.neptune,
    ]
rmasses = {
    # earth-moon barycenter: 328900.561400
    planets.mercury: 6023600.0,
    planets.venus: 408523.71,
    planets.earth: 332946.050895,
    planets.mars: 3098708.0,
    planets.jupiter: 1047.3486,
    planets.saturn: 3497.898,
    planets.uranus: 22902.98,
    planets.neptune: 19412.24,
    planets.pluto: 135200000.0,
    planets.sun: 1.0,
    planets.moon: 27068700.387534,
    }

def deflect(position, observer_position, jd, apply_earth, deflector_count=3):
    """Compute the gravitational deflection of light for the observed object.

    Based on novas.c:grav_def().

      loc_code (short int)
         Code for location of observer, determining whether the
         gravitational deflection due to the earth itself is applied.
            = 0 ... No earth deflection (normally means observer
                          is at geocenter)
            = 1 ... Add in earth deflection (normally means
         observer is on or above surface of earth, including earth
         orbit)

    """
    # Compute light-time to observed object.

    tlt = sqrt(position.dot(position)) / C_AUDAY

    # Cycle through gravitating bodies.

    for planet in deflectors[:deflector_count]:

        # Get position of gravitating body wrt ss barycenter at time 'jd_tdb'.

        bpv = planet(jd)

        # Get position of gravitating body wrt observer at time 'jd_tdb'.

        gpv = bpv.position - observer_position

        # Compute light-time from point on incoming light ray that is closest
        # to gravitating body.

        dlt = d_light(position, gpv)

        # Get position of gravitating body wrt ss barycenter at time when
        # incoming photons were closest to it.

        tclose = jd

        if dlt > 0.0:
            tclose = jd - dlt

        if tlt < dlt:
            tclose = jd - tlt

        bpv = planet(tclose)
        rmass = rmasses[planet]
        grav_vec(position, observer_position, bpv.position, rmass)

    # If observer is not at geocenter, add in deflection due to Earth.

    if apply_earth:
        bpv = planets.earth(jd)
        rmass = rmasses[planets.earth]
        grav_vec(position, observer_position, bpv.position, rmass)

#

def d_light(position, observer_position):
    """Returns the difference in light-time, for a star,
      between the barycenter of the solar system and the observer (or
      the geocenter).

    """
    # From 'pos1', form unit vector 'u1' in direction of star or light
    # source.

    dis = sqrt(position.dot(position))
    u1 = position / dis

    # Light-time returned is the projection of vector 'pos_obs' onto the
    # unit vector 'u1' (formed from 'pos1'), divided by the speed of light.

    diflt = observer_position.dot(u1) / C_AUDAY;
    return diflt

#

def grav_vec(position, observer_position, deflector_position, rmass):
    """Correct a position vector for the deflection of light.

    Based on novas.c:grav_vec().

    """
    # Construct vector 'pq' from gravitating body to observed object and
    # construct vector 'pe' from gravitating body to observer.

    pq = observer_position + position - deflector_position
    pe = observer_position - deflector_position

    # Compute vector magnitudes and unit vectors.

    pmag = sqrt(position.dot(position))
    qmag = sqrt(pq.dot(pq))
    emag = sqrt(pe.dot(pe))

    phat = position / pmag
    qhat = pq / qmag
    ehat = pe / emag

    # Compute dot products of vectors.

    pdotq = phat.dot(qhat)
    qdote = qhat.dot(ehat)
    edotp = ehat.dot(phat)

    # If gravitating body is observed object, or is on a straight line
    # toward or away from observed object to within 1 arcsec, deflection
    # is set to zero set 'pos2' equal to 'pos1'.

    if fabs(edotp) > 0.99999999999:
        return

    # Compute scalar factors.

    fac1 = 2.0 * GS / (C * C * emag * AU * rmass)
    fac2 = 1.0 + qdote

    # Correct position vector.

    position += fac1 * (pdotq * ehat - edotp * qhat) / fac2 * pmag
    print("P INTERMEDIATE:", repr(pdotq))

#

def aberration(position, velocity, lighttime):
    """Corrects a relative position vector for aberration of light."""

    pos, ve = position, velocity

    if lighttime:
        p1mag = lighttime * C_AUDAY
    else:
        p1mag = sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2])
        lighttime = p1mag / C_AUDAY

    vemag = sqrt(ve[0] * ve[0] + ve[1] * ve[1] + ve[2] * ve[2])
    beta = vemag / C_AUDAY
    dot = pos[0] * ve[0] + pos[1] * ve[1] + pos[2] * ve[2]

    cosd = dot / (p1mag * vemag)
    gammai = sqrt(1.0 - beta * beta)
    p = beta * cosd
    q = (1.0 + p / (1.0 + gammai)) * lighttime
    r = 1.0 + p

    return [(gammai * pos[0] + q * ve[0]) / r,
            (gammai * pos[1] + q * ve[1]) / r,
            (gammai * pos[2] + q * ve[2]) / r]
