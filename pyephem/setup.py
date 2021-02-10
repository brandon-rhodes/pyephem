# -*- coding: utf-8 -*-

from setuptools import setup

long_description = '''\
This distribution is a stub
that doesn’t install any Python packages,
but does list ``ephem`` as an install requirement
in case any old projects depend on this ``pyephem`` package name.
New projects are encouraged to directly require the main package,
which can be found at:

`https://pypi.org/project/ephem/ <https://pypi.org/project/ephem/>`_

This package’s version 9.99 is merely a placeholder value.
This package does not pin any particular PyEphem version,
but simply installs the latest version of the library.
Switch your project to depending on ``ephem`` directly
if you want to be able to pin a specific version.
'''

setup(
    name = 'pyephem',
    version = '9.99',
    description = 'Empty package that depends on "ephem"',
    long_description = long_description,
    long_description_content_type = 'text/x-rst',
    license = 'MIT',
    author = 'Brandon Rhodes',
    author_email = 'brandon@rhodesmill.org',
    url = 'http://rhodesmill.org/pyephem/',
    classifiers = [
        'Development Status :: 6 - Mature',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: MIT License',
        'Topic :: Scientific/Engineering :: Astronomy',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.2',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
    ],
    install_requires=['ephem'],
)
