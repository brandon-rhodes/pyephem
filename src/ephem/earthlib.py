"""Formulae for specific earth behaviors and effects."""

from math import cos, sin, sqrt
from ephem.angles import DEG2RAD

AU_KM = 1.4959787069098932e+8

ANGVEL = 7.2921150e-5
ERAD = 6378136.6
ERAD_KM = ERAD / 1000.0
F = 0.003352819697896

def terra(location, st):
    """Compute the position and velocity of a terrestrial observer.

    The resulting vectors are measured with respect to the center of the
    Earth.

    """
    # Compute parameters relating to geodetic to geocentric conversion.

    df = 1.0 - F
    df2 = df * df

    phi = location.latitude
    sinphi = sin(phi)
    cosphi = cos(phi)
    c = 1.0 / sqrt(cosphi * cosphi + df2 * sinphi * sinphi)
    s = df2 * c
    ht_km = location.elevation / 1000.0
    ach = ERAD_KM * c + ht_km
    ash = ERAD_KM * s + ht_km

    # Compute local sidereal time factors at the observer's longitude.

    stlocl = st * 15.0 * DEG2RAD + location.longitude
    sinst = sin(stlocl)
    cosst = cos(stlocl)

    # Compute position vector components in kilometers.

    pos = [ach * cosphi * cosst, ach * cosphi * sinst, ash * sinphi]

    # Compute velocity vector components in kilometers/sec.

    vel = [-ANGVEL * ach * cosphi * sinst, ANGVEL * ach * cosphi * cosst, 0.0]

    # Convert position and velocity components to AU and AU/DAY.

    for j in range(3):
        pos[j] /= AU_KM
        vel[j] /= AU_KM
        vel[j] *= 86400.0

    return pos, vel
