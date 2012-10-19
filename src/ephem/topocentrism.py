from ephem import earthlib, nutationlib, precessionlib, timescales
from ephem.angles import interpret_longitude, interpret_latitude
from ephem.coordinates import ICRS, frame_tie
from ephem.earthlib import AU_KM
from ephem.planets import earth
from ephem.relativity import aberration
from ephem.timescales import T0

class Topos(object):

    def __init__(self, longitude, latitude, elevation=0.,
                 temperature=10.0, pressure=1010.0):
        self.longitude = interpret_longitude(longitude)
        self.latitude = interpret_latitude(latitude)
        self.elevation = elevation

    def __call__(self, jd_tt):
        e = earth(jd_tt)
        tpos, tvel = self.geocentric_position_and_velocity(jd_tt)
        seconds_per_day = 24.0 * 60.0 * 60.0
        return ToposXYZ(e.position + tpos * AU_KM,
                        e.velocity + tvel * AU_KM / seconds_per_day,
                        jd_tt)

    def geocentric_position_and_velocity(self, jd_tt):
        delta_t = 0
        jd_tdb = jd_tt + timescales.tdb_minus_tt(jd_tt)
        jd_ut1 = jd_tt - (delta_t / 86400.)

        gmst = timescales.sidereal_time(jd_ut1, delta_t)
        x1, x2, eqeq, x3, x4 = earthlib.earth_tilt(jd_tdb)
        gast = gmst + eqeq / 3600.0

        pos1, vel1 = earthlib.terra(self, gast)

        pos2 = nutationlib.nutation(jd_tdb, pos1, invert=True)
        pos3 = precessionlib.precess(jd_tdb, T0, pos2)
        pos = frame_tie(pos3, -1)

        vel2 = nutationlib.nutation(jd_tdb, vel1, invert=True)
        vel3 = precessionlib.precess(jd_tdb, T0, vel2)
        vel = frame_tie(vel3, -1)

        return pos, vel

class ToposXYZ(ICRS):
    """In ICRS, right?"""

    def observe(self, body):
        """Make geocentric apparent coord."""

        pv = super(ToposXYZ, self).observe(body)
        print('here1', pv.position)

        # TODO: deflection near sun
        print(pv.lighttime)
        print(aberration(pv.position, self.velocity, pv.lighttime))
        # TODO: frame_tie
        # TODO: precession
        # TODO: nutation
        # print('here2', xyz.x, xyz.y, xyz.z)

        return pv
