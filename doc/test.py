# Test the example code in the manual.

import ephem
import fileinput, sys, traceback

inpre = 0
scope = { 'ephem' : ephem }
lines = [ line for line in fileinput.input('premanual.html') ]

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
