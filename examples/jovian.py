#!/usr/bin/env python
#
# Print out how the Jovian moons will be distributed around the planet
# for the next three days.

import ephem

moons = ((ephem.Io(), 'i'),
         (ephem.Europa(), 'e'),
         (ephem.Ganymede(), 'g'),
         (ephem.Callisto(), 'c'))

# How to place discrete characters on a line that actually represents
# the real numbers -maxradii to +maxradii.

linelen = 65
maxradii = 30.

def put(line, character, radii):
    if abs(radii) > maxradii:
        return
    offset = radii / maxradii * (linelen - 1) / 2
    i = int(linelen / 2 + offset)
    line[i] = character


# Loop over the next two days, displaying the moons every three hours.

interval = ephem.hour * 3
now = ephem.now()
now -= now % interval

t = now
while t < now + 2:
    line = [' '] * linelen
    put(line, 'J', 0)
    for moon, character in moons:
        moon.compute(t)
        put(line, character, moon.x)
    print str(ephem.date(t))[5:], ''.join(line)
    t += interval

print 'East is to the right;',
print ', '.join([ '%s = %s' % (c, m.name) for m, c in moons ])
