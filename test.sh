#!/bin/bash

export PYTHONPATH=~/ephem4/src:~/sgp4:~/jplephem:$PYTHONPATH
python3 -m unittest ephem.tests.test_vs_novas &&
python2 -m unittest ephem.tests.test_vs_novas
