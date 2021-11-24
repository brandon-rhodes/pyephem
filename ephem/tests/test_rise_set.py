import unittest
import ephem
from ephem import Angle, degrees, hours

class RiseSetTests(unittest.TestCase):
    def test_sun(self):
        s = ephem.Sun()
        o = ephem.Observer()
        o.lat = '36.4072'
        o.lon = '-105.5734'
        o.date = '2021/11/24'

        self.assertEqual(str(o.previous_setting(s)), '2021/11/23 23:49:36')
        self.assertEqual(str(o.previous_rising(s)), '2021/11/23 13:47:44')
        self.assertEqual(str(o.next_rising(s)), '2021/11/24 13:48:43')
        self.assertEqual(str(o.next_setting(s)), '2021/11/24 23:49:12')

    def test_moon(self):
        s = ephem.Moon()
        o = ephem.Observer()
        o.lat = '36.4072'
        o.lon = '-105.5734'
        o.date = '2021/11/24'

        self.assertEqual(str(o.previous_setting(s)), '2021/11/23 17:38:32')
        self.assertEqual(str(o.previous_rising(s)), '2021/11/23 02:17:52')
        self.assertEqual(str(o.next_rising(s)), '2021/11/24 03:12:00')
        self.assertEqual(str(o.next_setting(s)), '2021/11/24 18:22:25')
