from setuptools import setup

version = '3.7.7.0'

long_description = '''
This package is a stub
that contains no Python code,
but lists ``ephem`` as its install requirement
in case any old projects depend on this ``pyephem`` package name.
New projects are encouraged to directly require the main package,
which can be found at:

`https://pypi.org/project/ephem/ <https://pypi.org/project/ephem/>`_
'''

setup(
    name = 'pyephem',
    version = version,
    description = 'Empty package that depends on "ephem"',
    long_description = long_description,
    long_description_content_type = 'text/x-rst',
    license = 'LGPL',
    author = 'Brandon Rhodes',
    author_email = 'brandon@rhodesmill.org',
    url = 'http://rhodesmill.org/pyephem/',
    classifiers = [
        'Development Status :: 6 - Mature',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved ::'
        ' GNU Library or Lesser General Public License (LGPL)',
        'Topic :: Scientific/Engineering :: Astronomy',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.2',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
    ],
    install_requires=['ephem==' + version],
)
