from distutils.core import setup, Extension
from glob import glob

# The `pyephem' module is built from every .c file in the libastro
# directory plus ...

libastro_version = '3.6.1'
libastro_files = glob('libastro-%s/*.c' % libastro_version)

ext_modules = [
    Extension('ephem', ['ephem.c'] + libastro_files,
              include_dirs=['libastro-' + libastro_version],
              # These make the library more efficient on some platforms:
              #extra_compile_args = ["-O2", "-ffast-math"],
              )]

setup(name = 'pyephem',
      version = '3.6.1a',
      description = 'computational astronomy routines from XEphem',
      author = 'Brandon Craig Rhodes',
      author_email = 'brandon@rhodesmill.org',
      url = 'http://www.rhodesmill.org/brandon/projects/pyephem.html',
      ext_modules = ext_modules)
