#!/bin/bash

set -e
cd "$(dirname ${BASH_SOURCE[0]})"

if [ ! -d venv2 ] ;then
    python2 ~/usr/src/virtualenv/virtualenv.py --system-site-packages venv2
    python3 ~/usr/src/virtualenv/virtualenv.py --system-site-packages venv3

    venv2/bin/pip install novas_de405
    venv3/bin/pip install novas_de405
fi

export PATH=$PWD/venv2/bin:$PWD/venv3/bin:$PATH
export PYTHONPATH=~/ephem4/src:~/sgp4:~/jplephem:~/novas/build/lib.linux-i686-2.7:$PYTHONPATH

# Since "unittest" is bad at diagnosing import errors, we try bare
# importation of the test module before setting unittest loose on it.

python3 -c 'import ephem.tests.test_vs_novas' &&
python2 -c 'import ephem.tests.test_vs_novas' &&
python3 -m unittest ephem.tests.test_vs_novas &&
python2 -m unittest ephem.tests.test_vs_novas
