#
# Simple example from the home page, so I can run it.
#

import ephem

mars = ephem.Mars()
mars.compute()
print mars.ra, mars.dec
print ephem.constellation(mars)

boston = ephem.Observer()
boston.lat = '42.37'
boston.long = '-71.03'
mars.compute(boston)
print mars.az, mars.alt

print boston.next_rising(mars)
print mars.az

print boston.next_transit(mars)
print mars.alt
