from math import sin

J2000 = 2451545.

def tdb_minus_tt(jd_tdb):
    """Computes TT corresponding to a TDB Julian date."""

    t = (jd_tdb - J2000) / 36525.

    # Expression given in USNO Circular 179, eq. 2.6.

    return (0.001657 * sin ( 628.3076 * t + 6.2401)
          + 0.000022 * sin ( 575.3385 * t + 4.2970)
          + 0.000014 * sin (1256.6152 * t + 6.1969)
          + 0.000005 * sin ( 606.9777 * t + 4.0212)
          + 0.000005 * sin (  52.9691 * t + 0.4444)
          + 0.000002 * sin (  21.3299 * t + 5.5431)
          + 0.000010 * t * sin ( 628.3076 * t + 4.2490))
