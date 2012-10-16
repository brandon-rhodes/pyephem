"""Formulae for specific earth behaviors and effects."""

from math import cos
from ephem.angles import ASEC2RAD, DEG2RAD
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
