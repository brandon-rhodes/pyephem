/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Adapted from code written by Eric S. Raymond <esr@snark.thyrsus.com>
 *
 */

/* Pair of functions to convert to/from base64.
 * Also can be used to build a standalone utility and a loopback test.
 * see http://www.faqs.org/rfcs/rfc3548.html
 */

#include <ctype.h>

static const char base64digits[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define BAD     (-1)
static const char base64val[] = {
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD, 62, BAD,BAD,BAD, 63,
     52, 53, 54, 55,  56, 57, 58, 59,  60, 61,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,
     15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25,BAD, BAD,BAD,BAD,BAD,
    BAD, 26, 27, 28,  29, 30, 31, 32,  33, 34, 35, 36,  37, 38, 39, 40,
     41, 42, 43, 44,  45, 46, 47, 48,  49, 50, 51,BAD, BAD,BAD,BAD,BAD
};
#define DECODE64(c)  (isascii(c) ? base64val[c] : BAD)

/* convert inlen raw bytes at in to base64 string (NUL-terminated) at out. 
 * out size should be at least 4*inlen/3 + 4.
 * return length of out (sans trailing NUL).
 */
int
to64frombits(unsigned char *out, const unsigned char *in, int inlen)
{
    unsigned char *out0 = out;

    for (; inlen >= 3; inlen -= 3)
    {
        *out++ = base64digits[in[0] >> 2];
        *out++ = base64digits[((in[0] << 4) & 0x30) | (in[1] >> 4)];
        *out++ = base64digits[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
        *out++ = base64digits[in[2] & 0x3f];
        in += 3;
    }
    if (inlen > 0)
    {
        unsigned char fragment;
    
        *out++ = base64digits[in[0] >> 2];
        fragment = (in[0] << 4) & 0x30;
        if (inlen > 1)
            fragment |= in[1] >> 4;
        *out++ = base64digits[fragment];
        *out++ = (inlen < 2) ? '=' : base64digits[(in[1] << 2) & 0x3c];
        *out++ = '=';
    }
    *out = '\0';

    return (out-out0);
}

/* convert base64 at in to raw bytes out, returning count or <0 on error.
 * base64 may contain any embedded whitespace.
 * out should be at least 3/4 the length of in.
 */
int
from64tobits(char *out, const char *in)
{
    int len = 0;
    register unsigned char digit1, digit2, digit3, digit4;

    do {
	do {digit1 = *in++;} while (isspace(digit1));
        if (DECODE64(digit1) == BAD)
            return(-1);
	do {digit2 = *in++;} while (isspace(digit2));
        if (DECODE64(digit2) == BAD)
            return(-2);
	do {digit3 = *in++;} while (isspace(digit3));
        if (digit3 != '=' && DECODE64(digit3) == BAD)
            return(-3); 
	do {digit4 = *in++;} while (isspace(digit4));
        if (digit4 != '=' && DECODE64(digit4) == BAD)
            return(-4);
        *out++ = (DECODE64(digit1) << 2) | (DECODE64(digit2) >> 4);
        ++len;
        if (digit3 != '=')
        {
            *out++ = ((DECODE64(digit2) << 4) & 0xf0) | (DECODE64(digit3) >> 2);
            ++len;
            if (digit4 != '=')
            {
                *out++ = ((DECODE64(digit3) << 6) & 0xc0) | DECODE64(digit4);
                ++len;
            }
        }
	while (isspace(*in))
	    in++;
    } while (*in && digit4 != '=');

    return (len);
}

#ifdef BASE64_PROGRAM
/* standalone program that converts to/from base64.
 * cc -o base64 -DBASE64_PROGRAM base64.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
usage (char *me)
{
	fprintf (stderr, "Purpose: convert stdin to/from base64 on stdout\n");
	fprintf (stderr, "Usage: %s {-t,-f}\n", me);
	exit (1);
}

int
main (int ac, char *av[])
{
	int to64 = 1;

	/* decide whether to or from base64 */
	if (ac == 2 && strcmp (av[1], "-f") == 0)
	    to64 = 0;
	else if (ac != 1 && (ac != 2 || strcmp (av[1], "-t")))
	    usage (av[0]);

	if (to64) {
	    unsigned char *rawin, *b64;
	    int i, n, nrawin, nb64;

	    /* read raw on stdin until EOF */
	    rawin = malloc(4096);
	    nrawin = 0;
	    while ((n = fread (rawin+nrawin, 1, 4096, stdin)) > 0)
		rawin = realloc (rawin, (nrawin+=n)+4096);

	    /* convert to base64 */
	    b64 = malloc (4*nrawin/3+4);
	    nb64 = to64frombits(b64, rawin, nrawin);

	    /* pretty print */
	    for (i = 0; i < nb64; i += 72)
		printf ("%.*s\n", 72, b64+i);
	} else {
	    unsigned char *raw, *b64;
	    int n, nraw, nb64;

	    /* read base64 on stdin until EOF */
	    b64 = malloc(4096);
	    nb64 = 0;
	    while ((n = fread (b64+nb64, 1, 4096, stdin)) > 0)
		b64 = realloc (b64, (nb64+=n)+4096);
	    b64[nb64] = '\0';

	    /* convert to raw */
	    raw = malloc (3*nb64/4);
	    nraw = from64tobits(raw, b64);
	    if (nraw < 0) {
		fprintf (stderr, "base64 conversion error: %d\n", nraw);
		return (1);
	    }

	    /* write */
	    fwrite (raw, 1, nraw, stdout);
	}

	return (0);
}

#endif

#ifdef LOOPBACK_TEST
/* standalone test that reads binary on stdin, converts to base64 and back,
 * then compares. exit 0 if compares the same else 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (int ac, char *av[])
{
	unsigned char *rawin, *b64, *rawback;
	int n, nrawin, nrawback, nb64;

	/* read raw on stdin until EOF */
	rawin = malloc(4096);
	nrawin = 0;
	while ((n = fread (rawin+nrawin, 1, 4096, stdin)) > 0)
	    rawin = realloc (rawin, (nrawin+=n)+4096);

	/* convert to base64 */
	b64 = malloc (4*nrawin*3 + 4);
	nb64 = to64frombits(b64, rawin, nrawin);

	/* convert back to raw */
	rawback = malloc (3*nb64/4);
	nrawback = from64tobits(rawback, b64);
	if (nrawback < 0) {
	    fprintf (stderr, "base64 error: %d\n", nrawback);
	    return(1);
	}
	if (nrawback != nrawin) {
	    fprintf (stderr, "base64 back length %d != %d\n", nrawback, nrawin);
	    return(1);
	}

	/* compare */
	if (memcmp (rawback, rawin, nrawin)) {
	    fprintf (stderr, "compare error\n");
	    return (1);
	}

	/* success */
	return (0);
}
#endif
/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: base64.c,v $ $Date: 2005/03/14 20:25:43 $ $Revision: 1.2 $ $Name:  $"};
