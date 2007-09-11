try:
    from setuptools import setup, Extension
except:
    from distutils.core import setup, Extension
from glob import glob

# The `pyephem' module is built from every .c file in the libastro
# directory plus ...

libastro_version = '3.7.2'
libastro_files = glob('libastro-%s/*.c' % libastro_version)

setup(name = 'pyephem',
      version = '3.7.2a1',
      description = 'computational astronomy routines from XEphem',
      author = 'Brandon Craig Rhodes',
      author_email = 'brandon@rhodesmill.org',
      url = 'http://rhodesmill.org/brandon/projects/pyephem.html',
      packages = [ 'ephem' ],
      package_dir = { 'ephem': 'src' },
      ext_modules = [
    Extension('ephem._libastro', ['src/_libastro.c'] + libastro_files,
              include_dirs=['libastro-' + libastro_version],
              )],
      )
