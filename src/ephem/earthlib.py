"""Formulae for specific earth behaviors and effects."""

from math import cos, fmod
from ephem.angles import ASEC2RAD, DEG2RAD, ASEC360
from ephem.timescales import T0

def earth_tilt(jd_tdb):
    """Return stuff about the earth's axis and position."""

    t = (jd_tdb - T0) / 36525.0

    dp, de = nutation_angles(t)
    c_terms = ee_ct(jd_tdb, 0.0, accuracy) / ASEC2RAD

    d_psi = dp + PSI_COR
    d_eps = de + EPS_COR

    mean_ob = mean_obliq(jd_tdb)
    true_ob = mean_ob + d_eps

    mean_ob /= 3600.0
    true_ob /= 3600.0

    eq_eq = d_psi * cos(mean_ob * DEG2RAD) + c_terms
    eq_eq /= 15.0

    return d_psi, d_eps, eq_eq, mean_ob, true_ob

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
