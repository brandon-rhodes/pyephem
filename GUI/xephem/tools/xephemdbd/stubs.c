/* stubs to support xephem references */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "xephem.h"

void
pm_set (int p)
{
}


void
xe_msg (int loud, char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	vfprintf (stderr, fmt, ap);
	va_end (ap);
}

FILE *
fopenh (name, how)
char *name;
char *how;
{
	return (fopen (name, how));
}

int
existsh (name)
char *name;
{
	FILE *fp = fopen (name, "r");

	if (fp) {
	    fclose (fp);
	    return (0);
	}
	return (-1);
}

char *
expand_home (path)
char *path;
{
	return (strcpy (malloc (strlen(path)+1), path));
}


char *
syserrstr ()
{
	return (strerror(errno));
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: stubs.c,v $ $Date: 2004/06/21 02:48:15 $ $Revision: 1.6 $ $Name:  $"};
