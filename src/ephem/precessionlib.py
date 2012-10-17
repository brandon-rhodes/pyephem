from math import cos, sin
from ephem.angles import ASEC2RAD
from ephem.timescales import T0

def precess(jd_tdb1, jd_tdb2, pos1):
    """Precesses equatorial rectangular coordinates from one epoch to another.

    One of the two epochs must be J2000.0.  The coordinates are referred
    to the mean dynamical equator and equinox of the two respective
    epochs.

    """
    eps0 = 84381.406

    # Check to be sure that either 'jd_tdb1' or 'jd_tdb2' is equal to T0.

    if jd_tdb1 != T0 and jd_tdb2 != T0:
        raise ValueError('either jd_tdb1 or jd_tdb2 must = T0')

    # 't' is time in TDB centuries between the two epochs.

    t = (jd_tdb2 - jd_tdb1) / 36525.0
    if jd_tdb2 == T0:
        t = -t

    # Numerical coefficients of psi_a, omega_a, and chi_a, along with
    # epsilon_0, the obliquity at J2000.0, are 4-angle formulation from
    # Capitaine et al. (2003), eqs. (4), (37), & (39).

    psia   = ((((-    0.0000000951  * t
                 +    0.000132851 ) * t
                 -    0.00114045  ) * t
                 -    1.0790069   ) * t
                 + 5038.481507    ) * t

    omegaa = ((((+    0.0000003337  * t
                 -    0.000000467 ) * t
                 -    0.00772503  ) * t
                 +    0.0512623   ) * t
                 -    0.025754    ) * t + eps0

    chia   = ((((-    0.0000000560  * t
                 +    0.000170663 ) * t
                 -    0.00121197  ) * t
                 -    2.3814292   ) * t
                 +   10.556403    ) * t

    eps0 = eps0 * ASEC2RAD
    psia = psia * ASEC2RAD
    omegaa = omegaa * ASEC2RAD
    chia = chia * ASEC2RAD

    sa = sin(eps0)
    ca = cos(eps0)
    sb = sin(-psia)
    cb = cos(-psia)
    sc = sin(-omegaa)
    cc = cos(-omegaa)
    sd = sin(chia)
    cd = cos(chia)

    # Compute elements of precession rotation matrix equivalent to
    # R3(chi_a) R1(-omega_a) R3(-psi_a) R1(epsilon_0).

    xx =  cd * cb - sb * sd * cc
    yx =  cd * sb * ca + sd * cc * cb * ca - sa * sd * sc
    zx =  cd * sb * sa + sd * cc * cb * sa + ca * sd * sc
    xy = -sd * cb - sb * cd * cc
    yy = -sd * sb * ca + cd * cc * cb * ca - sa * cd * sc
    zy = -sd * sb * sa + cd * cc * cb * sa + ca * cd * sc
    xz =  sb * sc
    yz = -sc * cb * ca - sa * cc
    zz = -sc * cb * sa + cc * ca

    if jd_tdb2 == T0:
        # Perform rotation from epoch to J2000.0.

        return [xx * pos1[0] + xy * pos1[1] + xz * pos1[2],
                yx * pos1[0] + yy * pos1[1] + yz * pos1[2],
                zx * pos1[0] + zy * pos1[1] + zz * pos1[2]]

    else:
        # Perform rotation from J2000.0 to epoch.

        return [xx * pos1[0] + yx * pos1[1] + zx * pos1[2],
                xy * pos1[0] + yy * pos1[1] + zy * pos1[2],
                xz * pos1[0] + yz * pos1[1] + zz * pos1[2]]
