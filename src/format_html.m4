m4_divert(-1)
m4_changecom()

Copyright 1998 Brandon Craig Rhodes
Licensed under the terms of the GNU General Public License

Of course, much of the source file might be SWIG-specific directives
(such as typemaps) that have nothing to do with the documentation.
Rather than make the author engage in the messy business of marking
all of these regions as `out of bounds' for the documentation module,
we will simply read the entire input into diversion -1 (remembering
that negative diversions are m4's equivalent of /dev/null).  So, when
a macro is called that produces documentation, we will need to switch
back to the main diversion; rather than do this manually, we here
define macros to `turn on' output to the documentation file.  The OFF_
macro starts with a linefeed so I do not have to manually put
linefeeds at the end of all the documentation strings.

m4_define(ONH_, `m4_divert(0)')  for the page heading
m4_define(ONC_, `m4_divert(1)')  for the table of contents
m4_define(ON_, `m4_divert(2)')   for the main diversion
m4_define(ONE_, `m4_divert(3)')  for the end of the document
m4_define(OFF_, `
m4_divert(-1)')

We want to include text sections (which are stipped out of the SWIG
version of the file) in the documentation.

m4_define(TEXT_, `ON_<P>$1</P>OFF_')
m4_define(INTRODUCTION_, `ONH_()$1'`OFF_')

These three routines reset the given section counter to zero (the
counters are incremented when used, so this correctly yields first
sections designated by the number 1.)

m4_define(S1_RESET_, `m4_define(`S1_COUNT_', 0)')
m4_define(S2_RESET_, `m4_define(`S2_COUNT_', 0)')
m4_define(S3_RESET_, `m4_define(`S3_COUNT_', 0)')

And of course we want to start with all the counters reset.

S1_RESET_ S2_RESET_ S3_RESET_

Each of these routines increments the given counter, and resets all
higher-order counters in preparation for enumerating the subsections
of the new section.

m4_define(S1INC_,
	`m4_define(`S1_COUNT_', m4_eval(S1_COUNT_+1))S2_RESET_()S3_RESET_()')
m4_define(S2INC_, `m4_define(`S2_COUNT_', m4_eval(S2_COUNT_+1))S3_RESET_()')
m4_define(S3INC_, `m4_define(`S3_COUNT_', m4_eval(S3_COUNT_+1))')

These three routines give the full designations of sections and
subsections (without trailing periods).

m4_define(S1NUM_, `S1_COUNT_')
m4_define(S2NUM_, `S1_COUNT_.S2_COUNT_')
m4_define(S3NUM_, `S1_COUNT_.S2_COUNT_.S3_COUNT_')

These routines not only print their title, but generate a hyperlinked
table of contents in between the title and the main document.  This is
accomplished by using diversion 1 for the main document, and diversion
0 for the contents.  (Ah, the wonders of m4!)

m4_define(TITLE_, `ONH_<HEAD><TITLE>$1</TITLE></HEAD>
<BODY><H1>$1</H1>OFF_')
m4_define(SECTION_,
	`S1INC_()
	ONC_()<B><A HREF="#S1NUM_()">S1NUM_. $1</A></B><BR>OFF_
	ON_<HR><A NAME="S1NUM_"><H2>S1NUM_. $1</H2></A>OFF_')
m4_define(SUBSECTION_,
	`S2INC_()
	ONC_()<A HREF="#S2NUM_()">S2NUM_. $1</A><BR>OFF_
	ON_<H3><A NAME=S2NUM_>S2NUM_. $1</H3></A>OFF_')
m4_define(SUBSUBSECTION_,
	`S3INC_()
	ONC_()<A HREF="#S3NUM_">S3NUM_. $1</A><BR>OFF_
	ON_<H4><A NAME="S3NUM_">S3NUM_. $1</H4></A>OFF_')

These macro sets are used respectively for constants, structure
fields, and functions.

m4_define(CONSTANT_BEGIN, `ON_<UL>OFF_')
m4_define(CONSTANT_, `ON_<LI><B><TT>$1</TT></B> - $3OFF_')
m4_define(CONSTANT_END, `ON_</UL>OFF_')

m4_define(FIELD_BEGIN, `ON_</P>m4_ifelse($1,,,<B>$1:</B>)<UL>OFF_')
m4_define(FIELD_, `ON_<LI><B>$1</B> - $3OFF_')
m4_define(FIELD_END, `ON_</UL>OFF_')

m4_define(FUNCTION_BEGIN, `ON_<DL>OFF_')
m4_define(ARGIZE_, `m4_patsubst($1, `\([^ )]*\)\(,\|)\|$\)', `<B>\1</B>\2')')
m4_define(FUNCTION_,
 `ON_<DT><TABLE WIDTH=100%><TR><TD><B>$1</B> ARGIZE_($3)<TD ALIGN=RIGHT>-&gt; ARGIZE_($4)</TABLE><DD>$5</P>OFF_')
m4_define(FUNCTION_END, `ON_</DL>OFF_')

Titles the table of contents, and sets up the head and foot of the
HTML page.

ONH_<HTML>OFF_
ONC_<HR><H2>Table of Contents</H2>OFF_
ONE_</BODY></HTML>OFF_
