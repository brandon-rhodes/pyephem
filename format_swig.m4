m4_divert(-1)
m4_changecom()

Copyright 1998 Brandon Craig Rhodes
Licensed under the terms of the GNU General Public License

Since the input file is essentially a SWIG file with macro calls, this
file is simpler than the HTML formatting instructions (which has to
use diversions and counters to get the typesetting and table of
contents correct).

First, we make sure that raw explanations get thrown out rather than
included in our output.

m4_define(TEXT_)
m4_define(INTRODUCTION_)

Next we define all the section and subsection macros to expand to
nothing.

m4_define(TITLE_)
m4_define(SECTION_)
m4_define(SUBSECTION_)
m4_define(SUBSUBSECTION_)

Finally, each class of C entity needs no pre or post section
formatting, but does need its name and definition formatted as if in a
C header file.

m4_define(CONSTANT_BEGIN)
m4_define(CONSTANT_, `#define $1 $2')
m4_define(CONSTANT_END)

m4_define(FIELD_BEGIN)
m4_define(FIELD_, `%name ($1) $2;')
m4_define(FIELD_END)

m4_define(FUNCTION_BEGIN)
m4_define(FUNCTION_, `%name ($1) $2;')
m4_define(ARG_, $1 $2)
m4_define(FUNCTION_END)

m4_divert(0)