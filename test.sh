#!/bin/bash

export PYTHONPATH=~/ephem4/src:~/sgp4:~/jplephem:$PYTHONPATH

# Since "unittest" is bad at diagnosing import errors, we try bare
# importation of the test module before setting unittest loose on it.

python3 -c 'import ephem.tests.test_vs_novas' &&
python2 -c 'import ephem.tests.test_vs_novas' &&
python3 -m unittest ephem.tests.test_vs_novas &&
python2 -m unittest ephem.tests.test_vs_novas
