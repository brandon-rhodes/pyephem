from distutils.core import setup, Extension
from glob import glob

libastro_version = '3.6.1'
libastro_files = glob('libastro-%s/*.c' % libastro_version)

setup(name = 'pyephem',
      version = '3.6.1a',
      description = 'Python interface to the XEphem libastro library',
      author = 'Brandon Craig Rhodes',
      author_email = 'brandon@rhodesmill.org',
      url = 'http://www.rhodesmill.org/brandon/projects/pyephem.html',
      ext_modules = [Extension('ephem', ['src/ephem.c'] + libastro_files,
                               #extra_compile_args = ["-O2", "-ffast-math"],
                               include_dirs=['libastro-' + libastro_version])])
