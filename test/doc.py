# Test the example code in the manual.

import fileinput, sys, traceback
from glob import glob

# Work backwords from the `test' directory in which this script sits
# to find where the distutils have placed the new module; note that
# this attempt to file the `lib.*' directory will fail if the user has
# created several by building the module for several architectures.

test_dir = sys.path[0]
(build_lib,) = glob(test_dir + '/../build/lib.*')
sys.path.insert(0, build_lib)

import ephem

premanual_path = test_dir + '/../doc/premanual.html'

inpre = 0
scope = { 'ephem' : ephem }
lines = [ line for line in fileinput.input(premanual_path) ]

i = 0
while i < len(lines):
    line = lines[i].strip()
    i += 1
    if not inpre and line == '<pre class=interactive>':
        print '=' * 60
        inpre = 1
    elif inpre and line.strip() == '</pre>':
        inpre = 0
    elif inpre and line[0:4] == '>>> ':
        command = line[4:]
        line = lines[i]
        while line[0:4] == '... ':
            command += '\n' + line[4:]
            i += 1
            line = lines[i]
        print '>>>', command
        try:
            exec command in scope
        except:
            traceback.print_exc(file=sys.stdout)
    elif inpre:
        print line
