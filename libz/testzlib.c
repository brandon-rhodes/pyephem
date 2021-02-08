/* test compress and uncompress: read stdin, compress, uncompress, compare.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlib.h"

int
main (int ac, char *av[])
{
	Byte *raw, *comp, *uncomp;
	uLong nraw, ncomp, nuncomp;
	int n;

	/* read stdin to EOF */
	raw = malloc (4096);
	nraw = 0;
	while ((n = fread (raw+nraw, 1, 4096, stdin)) > 0)
	    raw = realloc (raw, (nraw+=n) + 4096);

	/* compress */
	comp = malloc (nraw);
	ncomp = nraw;
	if ((n = compress (comp, &ncomp, raw, nraw)) != Z_OK) {
	    fprintf (stderr, "compress returned %d\n", n);
	    exit(1);
	}

	/* uncompress */
	uncomp = malloc (nraw);
	nuncomp = nraw;
	if ((n = uncompress (uncomp, &nuncomp, comp, ncomp)) != Z_OK) {
	    fprintf (stderr, "uncompress returned %d\n", n);
	    exit(1);
	}

	/* compare */
	if (memcmp (raw, uncomp, nraw)) {
	    fprintf (stderr, "Compare failed\n");
	    exit(1);
	}

	/* success */
	printf ("Compression ratio %g\n", 100.0 * ncomp / nraw);
	return (0);
}
