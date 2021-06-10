import os
import sys
from distutils.core import setup, Extension
from glob import glob

# Work-around.

if 'bdist_wheel' in sys.argv:
    del setup, Extension
    from setuptools import setup, Extension

# Read the current version from ephem/__init__.py itself.

path = os.path.join(os.path.dirname(__file__), 'ephem', '__init__.py')
for line in open(path):
    if line.startswith('__version__'):
        __version__ = eval(line.split(None, 2)[2]) # skip '__version__', '='

# The 'ephem' module is built from every .c file in the libastro
# directory plus ...

libastro_files = glob('libastro/*.c')
libastro_data = glob('extensions/data/*.c')

here = os.path.dirname(__file__)
with open(os.path.join(here, 'README.rst')) as f:
    README = f.read()

libraries = []
if os.name != 'nt':
    # Linking against library "m" on Windows results in the error:
    # LINK : fatal error LNK1181: cannot open input file 'm.lib'
    libraries.append('m')  # Needed for Android; see GitHub issue #114.

extensions = [
    Extension(
        'ephem._libastro',
        ['extensions/_libastro.c', 'extensions/dtoa.c']
        + libastro_files + libastro_data,
        extra_compile_args=['-ffloat-store'],
        include_dirs=['libastro', '.'],
        libraries = libraries,
    ),
]

setup(name = 'ephem',
      version = __version__,
      description = 'Compute positions of the planets and stars',
      long_description = README,
      #long_description_content_type = 'text/x-rst',
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
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        ],
      packages = [ 'ephem', 'ephem.tests' ],
      package_data = { 'ephem': ['doc/*.rst',
                                 'tests/jpl/*.txt',
                                 'tests/usno/*.txt',
                                 ],},
      ext_modules = extensions,
      )
