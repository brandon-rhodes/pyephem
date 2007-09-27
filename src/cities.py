
from ephem import Observer

def create(name):
    if name == 'Atlanta':
        o = Observer()
        o.name = 'Atlanta'
        o.long, o.lat = '-84.39733', '33.775867'
        o.elevation = 322
        return o
    elif name == 'Adelaide':
        o = Observer()
        o.name = 'Adelaide'
        o.long, o.lat = '138:35', '-34:55'
        return o
