#
# Convert the planetary moon datasets from text into C arrays of
# structs.
#

import os, sys

class Moon(object):
    def __init__(self, planeti, mooni):
        self.planeti = planeti
        self.mooni = mooni
        self.data = {}

    def store(self, jd, coefficients):
        self.data[jd] = coefficients

    def pr(self):
        """Grab our bits out of the first line of a BDL file."""
        print 'planet', self.planeti
        print 'moon', self.mooni + 1
        print 'freq', self.frequency
        print 'delt', self.delta

moons = {}

path = sys.argv[1]
f = open(path)
planeti = None

# The first line of the file tells us which planet's moons are
# described in this file, as well as meta-information about the data
# itself.

firstline = f.readline()
fields = firstline.split()
planet_id = int(fields[0])  # planet id
mooncount = int(fields[1])  # number of moons

#moons = [ Moon(planeti, mooni) for mooni in range(mooncount) ]

    #for moon in moons:
    #    i = moon.mooni
    #    # ignore fields[2+i], which is the line# of moon's first entry
    #    moon.frequency = float(fields[2+mooncount+i])
    #    moon.delta = float(fields[2+2*mooncount+i])

    #ienrf = int(fields[-3])
    #djj = float(fields[-2])
    #jan = int(fields[-1])
    #for moon in moons:
    #    moon.pr()
    #print ienrf, djj, jan
    
    # Subsequent lines have the actual orbital data itself.

for line in f:
    fields = line.split()
    moon_id = int(line[0:1])

    key = (planet_id, moon_id)
    moon = moons.get(key, None)
    if moon is None:
        moons[key] = moon = Moon(planet_id, moon_id)

    jd = float(line[22:31])
    coefficients = [ float(f.replace('D', 'e')) for f in fields[2:] ]
    moon.store(jd, coefficients)

# Function to interpret floats from satxyz file.

def f(s):
    return str(float(s.replace('D', 'e')))

# How to print out a data line.

space9 = " " * 9

def print_data_line(line, comma=','):
    def print_data_group(name, numfields, comma=','):
        print space9,
        print "{%s}%s" % (','.join([ f(line[p + 17*i : p + 17*(i+1)])
                                     for i in range(numfields) ]), comma),
        print "/*%s*/" % name
        return p + 17*numfields

    print "     {"
    p = 1 + 5 + 8 + 8; # skip first four fields
    print space9, "%s," % f(line[p:p+9])
    p += 9
    p = print_data_group('cmx', 6)
    p = print_data_group('cfx', 4)
    p = print_data_group('cmy', 6)
    p = print_data_group('cfy', 4)
    p = print_data_group('cmz', 6)
    p = print_data_group('cfz', 4, '')
    print "     }%s" % comma

# Read and output the actual data.

lines = open(path).readlines()
line0 = lines[0]
nsat = int(line0[2:4]) # number of moons in this file
p = 4
idn_list = []
for i in range(nsat):
    idn_list.append(str(int(line0[p:p+5])))
    p += 5
freq_list = []
for i in range(nsat):
    freq_list.append(f(line0[p:p+8]))
    p += 8
delt_list = []
for i in range(nsat):
    delt_list.append(f(line0[p:p+5]))
    p += 5
djj = f(line0[p+5:p+20]) # beginning Julian date for this file

print """
#include "bdl.h"

static BDL_Record moonrecords[] = {
"""

for line in lines[1:-1]:
    print_data_line(line)
print_data_line(lines[-1], '')

print """
};
"""

def print_list(typ, name, lst):
    print "static %s %s_list[] = {%s};" % (typ, name, ','.join(lst))

print_list('unsigned', 'idn', idn_list)
print_list('double', 'freq', freq_list)
print_list('double', 'delt', delt_list)

print """
BDL_Dataset %s = {
     %d, /*nsat*/
     %s, /*djj*/
     idn_list,
     freq_list,
     delt_list,
     moonrecords
};
""" % (os.path.basename(path).replace('.', '_'), nsat, djj)
