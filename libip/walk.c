#include <stdlib.h>
#include <math.h>

#include "ip.h"

/* starting at the center of the given region, walk up the gradient to the
 * brightest pixel. the region just sets the starting point, the walk is not
 * confined therein. but if the walk bangs into the edge of the /image/, we
 * walk[] are the offsets to the 8 surrounding pixels, we provide if NULL.
 * return that location.
 */
void
brightWalk (ImRegion *rp, int walk[8], int *bx, int *by)
{
	CamPix *im = &rp->im[ImRCenter(rp)];
	CamPix *imend = &rp->im[rp->iw*rp->ih];
	int mywalk[8];
	int i, bi;

	/* build a pixel walk-around map if not provided */
	if (!walk) {
	    mywalk[0] =  1;
	    mywalk[1] =  1 - rp->iw;
	    mywalk[2] =    - rp->iw;
	    mywalk[3] = -1 - rp->iw;
	    mywalk[4] = -1;
	    mywalk[5] = -1 + rp->iw;
	    mywalk[6] =      rp->iw;
	    mywalk[7] =  1 + rp->iw;
	    walk = mywalk;
	} 

	do {
	    bi = 0;
	    for (i = 0; i < 8; i++)
		if (im[walk[i]] > im[bi])
		    bi = walk[i];
	    im += bi;
	    if (im < rp->im || im >= imend) {
		im -= bi;	/* step back into image */
		break;
	    }
	} while (bi);

	*bx = (im - rp->im)%rp->iw;
	*by = (im - rp->im)/rp->iw;
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: walk.c,v $ $Date: 2001/10/03 08:26:37 $ $Revision: 1.2 $ $Name:  $"};
