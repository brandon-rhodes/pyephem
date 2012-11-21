import os
from distutils.core import setup

# Read the current version from ephem/__init__.py itself.

path = os.path.join(os.path.dirname(__file__), 'ephem', '__init__.py')
for line in open(path):
    if line.startswith('__version__'):
        __version__ = eval(line.split(None, 2)[2]) # skip '__version__', '='

def read(*filenames):
    return open(os.path.join(os.path.dirname(__file__), *filenames)).read()

setup(
    name='pyephem',
    version=__version__,
    description='Scientific-grade astronomy routines',
    long_description=read('README'),
    license='MIT',
    author='Brandon Rhodes',
    author_email='brandon@rhodesmill.org',
    url='http://rhodesmill.org/pyephem/',
    classifiers=[
        'Development Status :: 6 - Mature',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: MIT License',
        'Topic :: Scientific/Engineering :: Astronomy',
        ],
    packages=[ 'ephem', 'ephem.tests' ],
    package_data={ 'ephem': ['doc/*.rst',
                             'tests/jpl/*.txt',
                             'tests/usno/*.txt',
                             ],},
    install_requires=['jplephem', 'numpy', 'sgp4'],
    )
