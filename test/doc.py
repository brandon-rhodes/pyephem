# Test the example code in the manual.

import fileinput, sys, traceback
from glob import glob
from cStringIO import StringIO

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

# Try running the given Python command, and see whether the output it
# produces on the terminal is the same as the given expected result;
# and print out the two value if they differ.

def compare(lineno, command, result):
    global n_success, n_failure
    real_stdout, real_stderr = sys.stdout, sys.stderr
    sio = StringIO()
    sys.stdout = sio
    try:
        exec command in scope
        output = sio.getvalue()
    except:
        t, v, tb = sys.exc_info()
        output = '\n'.join(traceback.format_exception_only(t, v))
    sys.stdout = real_stdout
    output = output.replace('&', '&amp;').replace('<', '&lt;') \
             .replace('>', '&gt;')
    if output == result:
        n_success += 1
    else:
        n_failure += 1
        print '=' * 60
        print '%d:' % lineno, command
        print output + '--\n' + result

# This state machine determines which commands illustrated in the
# produce which outputs, and sends each command-result pair to the
# above comparison function.

pre_start = '<pre class=interactive>\n'
pre_end = '</pre>\n'

state = 'waiting-for-pre'
lineno = 0
n_success = n_failure = 0

for line in file(premanual_path):
    lineno += 1
    #print lineno
    if state == 'waiting-for-pre':
        if line == pre_start:
            state = 'waiting-for-command'

    elif state == 'waiting-for-command':
        if line == pre_end:
            state = 'waiting-for-pre'
        elif line.startswith('>>> '):
            cmdlineno = lineno
            command = line[4:]
            state = 'reading-command'

    elif state == 'reading-command':
        if line == pre_end:
            compare(cmdlineno, command, '')
            state = 'waiting-for-pre'
        elif line.startswith('>>> '):
            compare(cmdlineno, command, '')
            cmdlineno = lineno
            command = line[4:]
        elif line.startswith('... '):
            command += line[4:]
        else:
            result = line
            state = 'reading-result'

    elif state == 'reading-result':
        if line == pre_end:
            compare(cmdlineno, command, result)
            state = 'waiting-for-pre'
        elif line.startswith('>>> '):
            compare(cmdlineno, command, result)
            cmdlineno = lineno
            command = line[4:]
            state = 'reading-command'
        else:
            result += line

    else:
        raise RuntimeError, 'unknown state %r' % state

print "Test complete with %d successes and %d failures" % (
    n_success, n_failure)
