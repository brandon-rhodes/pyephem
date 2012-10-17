"""Formulae for specific earth behaviors and effects."""

from math import cos, fmod, sin, sqrt
from ephem.angles import ASEC2RAD, DEG2RAD, ASEC360, tau
from ephem.nutationlib import iau2000a
from ephem.timescales import T0

PSI_COR = 0.0
EPS_COR = 0.0

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

def earth_tilt(jd_tdb):
    """Return stuff about the earth's axis and position."""

    dp, de = iau2000a(jd_tdb)
    dp /= ASEC2RAD
    de /= ASEC2RAD
    c_terms = equation_of_the_equinoxes_complimentary_terms(jd_tdb) / ASEC2RAD

    d_psi = dp + PSI_COR
    d_eps = de + EPS_COR

    mean_ob = mean_obliquity(jd_tdb)
    true_ob = mean_ob + d_eps

    mean_ob /= 3600.0
    true_ob /= 3600.0

    eq_eq = d_psi * cos(mean_ob * DEG2RAD) + c_terms
    eq_eq /= 15.0

    return mean_ob, true_ob, eq_eq, d_psi, d_eps

def equation_of_the_equinoxes_complimentary_terms(jd_tt):
    """Compute the "complementary terms" of the equation of the equinoxes."""

    # Interval between fundamental epoch J2000.0 and current date.

    t = (jd_tt - T0) / 36525.0

    # Fundamental Arguments.

    fa = [0] * 14

    # Mean Anomaly of the Moon.

    fa[0] = ((485868.249036 +
              (715923.2178 +
              (    31.8792 +
              (     0.051635 +
              (    -0.00024470)
              * t) * t) * t) * t) * ASEC2RAD
              + fmod(1325.0*t, 1.0) * tau) % tau;

    # Mean Anomaly of the Sun.

    fa[1] = ((1287104.793048 +
              (1292581.0481 +
              (     -0.5532 +
              (     +0.000136 +
              (     -0.00001149)
              * t) * t) * t) * t) * ASEC2RAD
              + fmod (99.0*t, 1.0) * tau) % tau;

    # Mean Longitude of the Moon minus Mean Longitude of the Ascending
    # Node of the Moon.

    fa[2] = (( 335779.526232 +
              ( 295262.8478 +
              (    -12.7512 +
              (     -0.001037 +
              (      0.00000417)
              * t) * t) * t) * t) * ASEC2RAD
              + fmod (1342.0*t, 1.0) * tau) % tau;

    # Mean Elongation of the Moon from the Sun.

    fa[3] = ((1072260.703692 +
              (1105601.2090 +
              (     -6.3706 +
              (      0.006593 +
              (     -0.00003169)
              * t) * t) * t) * t) * ASEC2RAD
              + fmod (1236.0*t, 1.0) * tau) % tau;

    # Mean Longitude of the Ascending Node of the Moon.

    fa[4] = (( 450160.398036 +
              (-482890.5431 +
              (      7.4722 +
              (      0.007702 +
              (     -0.00005939)
              * t) * t) * t) * t) * ASEC2RAD
              + fmod (-5.0*t, 1.0) * tau) % tau;

    fa[ 5] = (4.402608842 + 2608.7903141574 * t) % tau
    fa[ 6] = (3.176146697 + 1021.3285546211 * t) % tau
    fa[ 7] = (1.753470314 +  628.3075849991 * t) % tau
    fa[ 8] = (6.203480913 +  334.0612426700 * t) % tau
    fa[ 9] = (0.599546497 +   52.9690962641 * t) % tau
    fa[10] = (0.874016757 +   21.3299104960 * t) % tau
    fa[11] = (5.481293872 +    7.4781598567 * t) % tau
    fa[12] = (5.311886287 +    3.8133035638 * t) % tau
    fa[13] = (0.024381750 +    0.00000538691 * t) * t

    # Evaluate the complementary terms.

    s0 = 0.0
    s1 = 0.0

    for i in reversed(range(33)):

        a = 0.0

        for j in range(14):
            a += ke0_t[i][j] * fa[j]

        s0 += se0_t[i][0] * sin(a) + se0_t[i][1] * cos(a)

    a = 0.0

    for j in range(14):
        a += ke1[j] * fa[j]

    s1 += se1[0] * sin(a) + se1[1] * cos(a)
    c_terms = s0 + s1 * t
    c_terms *= ASEC2RAD
    return c_terms


ke0_t = (
      (0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  2, -2,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  2, -2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  2, -2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  2,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  2,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  0,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  1,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (1,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  1,  2, -2,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  1,  2, -2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  4, -4,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  1, -1,  1,  0, -8, 12,  0,  0,  0,  0,  0,  0),
      (0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  2,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (1,  0,  2,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (1,  0,  2,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  2, -2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  1, -2,  2, -3,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  1, -2,  2, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  0,  0,  0,  0,  8,-13,  0,  0,  0,  0,  0, -1),
      (0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (2,  0, -2,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (1,  0,  0, -2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  1,  2, -2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (1,  0,  0, -2, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  4, -2,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (0,  0,  2, -2,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (1,  0, -2,  0, -3,  0,  0,  0,  0,  0,  0,  0,  0,  0),
      (1,  0, -2,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0))

# Argument coefficients for t^1.

ke1 = (0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0)

# Sine and cosine coefficients for t^0.

se0_t = (
      (+2640.96e-6,          -0.39e-6),
      (  +63.52e-6,          -0.02e-6),
      (  +11.75e-6,          +0.01e-6),
      (  +11.21e-6,          +0.01e-6),
      (   -4.55e-6,          +0.00e-6),
      (   +2.02e-6,          +0.00e-6),
      (   +1.98e-6,          +0.00e-6),
      (   -1.72e-6,          +0.00e-6),
      (   -1.41e-6,          -0.01e-6),
      (   -1.26e-6,          -0.01e-6),
      (   -0.63e-6,          +0.00e-6),
      (   -0.63e-6,          +0.00e-6),
      (   +0.46e-6,          +0.00e-6),
      (   +0.45e-6,          +0.00e-6),
      (   +0.36e-6,          +0.00e-6),
      (   -0.24e-6,          -0.12e-6),
      (   +0.32e-6,          +0.00e-6),
      (   +0.28e-6,          +0.00e-6),
      (   +0.27e-6,          +0.00e-6),
      (   +0.26e-6,          +0.00e-6),
      (   -0.21e-6,          +0.00e-6),
      (   +0.19e-6,          +0.00e-6),
      (   +0.18e-6,          +0.00e-6),
      (   -0.10e-6,          +0.05e-6),
      (   +0.15e-6,          +0.00e-6),
      (   -0.14e-6,          +0.00e-6),
      (   +0.14e-6,          +0.00e-6),
      (   -0.14e-6,          +0.00e-6),
      (   +0.14e-6,          +0.00e-6),
      (   +0.13e-6,          +0.00e-6),
      (   -0.11e-6,          +0.00e-6),
      (   +0.11e-6,          +0.00e-6),
      (   +0.11e-6,          +0.00e-6))

# Sine and cosine coefficients for t^1.

se1 = (   -0.87e-6,          +0.00e-6)

#

def fundamental_arguments(t):
    """Compute the fundamental arguments (mean elements) of Sun and Moon.

    t - TDB time in Julian centuries since J2000.0

    Outputs fundamental arguments, in radians:
          a[0] = l (mean anomaly of the Moon)
          a[1] = l' (mean anomaly of the Sun)
          a[2] = F (mean argument of the latitude of the Moon)
          a[3] = D (mean elongation of the Moon from the Sun)
          a[4] = Omega (mean longitude of the Moon's ascending node);
                 from Simon section 3.4(b.3),
                 precession = 5028.8200 arcsec/cy)

    """
    a0 = fmod(485868.249036 +
             t * (1717915923.2178 +
             t * (        31.8792 +
             t * (         0.051635 +
             t * (       - 0.00024470)))), ASEC360) * ASEC2RAD

    a1 = fmod(1287104.79305 +
             t * ( 129596581.0481 +
             t * (       - 0.5532 +
             t * (         0.000136 +
             t * (       - 0.00001149)))), ASEC360) * ASEC2RAD

    a2 = fmod(335779.526232 +
             t * (1739527262.8478 +
             t * (      - 12.7512 +
             t * (      -  0.001037 +
             t * (         0.00000417)))), ASEC360) * ASEC2RAD

    a3 = fmod(1072260.70369 +
             t * (1602961601.2090 +
             t * (       - 6.3706 +
             t * (         0.006593 +
             t * (       - 0.00003169)))), ASEC360) * ASEC2RAD

    a4 = fmod(450160.398036 +
             t * ( - 6962890.5431 +
             t * (         7.4722 +
             t * (         0.007702 +
             t * (       - 0.00005939)))), ASEC360) * ASEC2RAD

    return a0, a1, a2, a3, a4

#

def mean_obliquity(jd_tdb):
    """Compute the mean obliquity of the ecliptic."""

    # Compute time in Julian centuries from epoch J2000.0.

    t = (jd_tdb - T0) / 36525.0;

    # Compute the mean obliquity in arcseconds.  Use expression from the
    # reference's eq. (39) with obliquity at J2000.0 taken from eq. (37)
    # or Table 8.

    epsilon = (((( -  0.0000000434   * t
                   -  0.000000576  ) * t
                   +  0.00200340   ) * t
                   -  0.0001831    ) * t
                   - 46.836769     ) * t + 84381.406;

    return epsilon
