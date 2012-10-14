#!/bin/bash

export PYTHONPATH=~/ephem4/src:~/sgp4:~/jplephem
if ! python3 prototype.py
then
    echo "Error: return code $?"
    exit $?
fi
diff -u <(python2 prototype.py) <(python3 prototype.py)
