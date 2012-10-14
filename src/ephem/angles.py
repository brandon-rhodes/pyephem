from math import pi

twopi = pi * 2.

class Angle(object):

    def __init__(self, radians):
        self.radians = radians

    def hours(self):
        return 24. / twopi * self.radians

    def hms(self):
        h, ms = divmod(self.radians / twopi * 24., 1.)
        m, ss = divmod(ms * 60., 1.)
        s = ss * 60.
        return h, m, s

    def degrees(self):
        return 360. / twopi * self.radians

    def dms(self):
        h, ms = divmod(self.radians / twopi * 360., 1.)
        m, ss = divmod(ms * 60., 1.)
        s = ss * 60.
        return h, m, s
