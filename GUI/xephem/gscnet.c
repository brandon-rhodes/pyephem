/* fetch GSC stars from the network.
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xephem.h"

static int moreObjF (ObjF **opp, int nop, int nnew);

/* fetch GSC 2.3 stars around the given field of view from STScI.
 * if find some add them to the list at *opp and return new total, else
 * limit FOV to 30 arc mins -- these fields can be very dense.
 * return -1 with a diagnostic message in msg[].
 */
int
gsc23fetch (
char *url,		/* everything in the URL except the query */
Now *np,		/* current circumstances */
double ra, double dec,	/* RA/Dec of center of view, rads, J2000 */
double fov,		/* diamater of field of view, rads */
double lmag,		/* limiting magnitude desired */
ObjF **opp,		/* *opp is malloced with stars found */
int nop,		/* number already at *opp */
char msg[])		/* return diagnostic message here, if returning -1 */
{
#define	GSC23MAXFOV	degrad(30./60.0)		/* max fov */
	/* http://gsss.stsci.edu/webservices/vo/ConeSearch.aspx?RA=10.0&DEC=5.0&SR=0.2&FORMAT=CSV */
	static char ifmt[] = "%[^,],%lf,%lf,%*[^,],%*[^,],%*[^,],%*[^,],%lf,%lf,%*[^,],%*[^,],%lf,%lf,%lf,%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%d";
	static char gfmt[] = " GET http://%s%s?RA=%g&DEC=%g&SR=%g&FORMAT=CSV HTTP/1.0\r\nUser-Agent: xephem/%s\r\n\r\n";
	char host[1024];
	char buf[2048];
	char *hp1;
	int sockfd;
	int n, nstars, nnew;

	/* clamp fov */
	if (fov > GSC23MAXFOV) {
	    static int fovwarned;
	    if (!fovwarned) {
		xe_msg (1, "All GSC 2.3 downloads will be limited to %.2f degree FOV",
							raddeg(GSC23MAXFOV));
		fovwarned = 1;
	    }
	    fov = GSC23MAXFOV;
	}

	/* extract host */
	if (strncmp (url, "http://", 7)) {
	    strcpy (msg, "URL must start with http://");
	    return (-1);
	}
	if (!(hp1 = strchr (url+7, '/'))) {
	    strcpy (msg, "no host found in URL");
	    return (-1);
	}
	sprintf (host, "%.*s", (int)(hp1-(url+7)), url+7);

	/* format the GET */
	(void) sprintf (buf, gfmt, host, hp1, raddeg(ra), raddeg(dec),
						    raddeg(fov)/2, PATCHLEVEL);

	/* let user abort */
	stopd_up();

	/* send the GET method to host and connection to read response */
	sockfd = httpGET (host, buf, msg);
	if (sockfd < 0) {
	    stopd_down();
	    return (-1);
	}

	/* now read lines of stars from the socket and collect in opp array */
	nnew = 0;
	pm_up();
	pm_set(0);
	while ((n = recvlineb (sockfd, buf, sizeof(buf))) > 0) {
	    char name[1024];
	    double radeg, decdeg;
	    double fmag, jmag, bmag, vmag, rmag;
	    int class;
	    Obj *op;

	    /* look for total */
	    if (sscanf (buf, " Objects found : %d", &nstars) == 1)
		continue;

	    /* crack */
	    if (sscanf (buf, ifmt, name, &radeg, &decdeg, &fmag, &jmag, &bmag,
						&vmag, &rmag, &class) != 9)
		continue;
	    if (fmag>lmag && jmag>lmag && bmag>lmag && vmag>lmag && rmag>lmag)
		continue;

	    /* good -- grow list */
	    if (moreObjF (opp, nop, 1) < 0) {
		(void) strcpy (msg, "No memory");
		(void) close (sockfd);
		stopd_down();
		return (-1);
	    }
	    op = (Obj *)(&(*opp)[nop++]);
	    zero_mem ((void *)op, sizeof(ObjF));

	    /* add */
	    (void) sprintf (op->o_name, "GSC2.3 %.*s", MAXNM-8, name);
	    op->o_type = FIXED;
	    switch (class) {
	    case 0: op->f_class = 'S'; break;
	    case 1: op->f_class = 'G'; break;
	    default: op->f_class = 'T'; break;
	    }
	    op->f_RA = degrad (radeg);
	    op->f_dec = degrad (decdeg);
	    op->f_epoch = (float)J2000;
	    if (vmag<=lmag)
		set_fmag (op, vmag);
	    else if (bmag<=lmag)
		set_fmag (op, bmag);
	    else if (rmag<=lmag)
		set_fmag (op, rmag);
	    else if (fmag<=lmag)
		set_fmag (op, fmag);
	    else
		set_fmag (op, jmag);

	    if (nstars > 0)
		pm_set (100*nnew++/nstars);
	}

	(void) close (sockfd);
	stopd_down();
	pm_down();

	if (n < 0) {
	    (void) strcpy (msg, syserrstr());
	    return (-1);
	}
	return (nop);
}

#if 0			/* depricated */

/* fetch GSC stars around the given field of view from the named xephem host.
 * if find some add them to the list at *opp and return new total, else
 * return -1 with a diagnostic message in msg[].
 */
int
gscnetfetch (
char *url,		/* http://<host>/xephemdbd.pl or whatever */
Now *np,		/* current circumstances */
double ra, double dec,	/* RA/Dec of center of view, rads, J2000 */
double fov,		/* diamater of field of view, rads */
double mag,		/* limiting magnitude desired */
ObjF **opp,		/* *opp is malloced with stars found */
int nop,		/* number already at *opp */
char msg[])		/* return diagnostic message here, if returning -1 */
{
static char ofmt[] = "GET http://%s%s?VERSION=3&FMT=EDB&CENTRIC=GEO&PRECES=ASTRO&GS=on&DATE=Now&TIME=&RA=%s&DEC=%s&FOV=%g&MAG=%g&LAT=0+0+0&LONG=0+0+0&ELEV=0 HTTP/1.0\r\nUser-Agent: xephem/%s\r\n\r\n";
	char *h0p, *h1p, host[1024];
	char rbuf[32], *rbp, dbuf[32], *dbp;
	char buf[2048];
	int sockfd;
	int n;

	/* confirm http and pull out the host from url */
	if ((h0p = strstr (url, "http://")) || (h0p = strstr (url, "HTTP://")))
	    h0p += 7;
	else {
	    strcpy (msg, "URL must begin with http://");
	    return (-1);
	}
	h1p = strchr (h0p, '/');
	if (!h1p) {
	    (void) strcpy (msg, "No host in xephemdbd url");
	    return (-1);
	}
	(void) sprintf (host, "%.*s", (int)(h1p-h0p), h0p);

	/* format the GET -- skip leading blanks in rbuf and dbuf */
	fs_sexa (rbuf, radhr(ra), 2, 36000);
	for (rbp = rbuf; *rbp == ' '; rbp++) continue;
	fs_sexa (dbuf, raddeg(dec), 3, 3600);
	for (dbp = dbuf; *dbp == ' '; dbp++) continue;
	(void) sprintf (buf, ofmt, host, h1p, rbp, dbp, raddeg(fov), mag, PATCHLEVEL);

	/* let user abort */
	stopd_up();

	/* send the GET method to host and connection to read response */
	sockfd = httpGET (host, buf, msg);
	if (sockfd < 0) {
	    stopd_down();
	    return (-1);
	}

	/* now read lines of stars from the socket and collect good lines in
	 * opp array
	 */
	while ((n = recvlineb (sockfd, buf, sizeof(buf))) > 0) {
	    Obj o, *op;

	    if (db_crack_line (buf, &o, NULL, 0, NULL) < 0 || o.o_type != FIXED)
		continue;

	    if (moreObjF (opp, nop, 1) < 0) {
		(void) strcpy (msg, "No memory");
		(void) close (sockfd);
		stopd_down();
		return (-1);
	    }

	    op = (Obj *)(&(*opp)[nop++]);
	    memcpy ((void *)op, (void *)&o, sizeof(ObjF));
	}

	(void) close (sockfd);
	stopd_down();

	if (n < 0) {
	    (void) strcpy (msg, syserrstr());
	    return (-1);
	}
	return (nop);
}


/* fetch GSC stars around the given field of view from ESO.
 * if find some add them to the list at *opp and return new total, else
 * return -1 with a diagnostic message in msg[].
 */
int
gscesofetch (
Now *np,		/* current circumstances */
double ra, double dec,	/* RA/Dec of center of view, rads, J2000 */
double fov,		/* diamater of field of view, rads */
double mag,		/* limiting magnitude desired */
ObjF **opp,		/* *opp is malloced with stars found */
int nop,		/* number already at *opp */
char msg[])		/* return diagnostic message here, if returning -1 */
{
	static char eso_host[] = "archive.eso.org";
static char ifmt[] = "%s %lf %lf %lf %lf %lf %lf %*s %lf %*s %*s %d";
static char ofmt[] = "GET http://%s/skycat/servers/gsc-server?%.6f%c%.6f&r=0,%.1f&m=0,%g&s=R&f=1&* HTTP/1.0\r\nUser-Agent: xephem/%s\r\n\r\n";
	static char eofcode[] = "[EOD]";
	char buf[1024];
	int sockfd;
	int n;

	/* format the GET */
	(void) sprintf (buf, ofmt, eso_host, radhr(ra), dec<0?'-':'+', fabs(raddeg(dec)),
					    raddeg(fov/2)*60, mag, PATCHLEVEL);

	/* let user abort */
	stopd_up();

	/* send the GET method to host and connection to read response */
	sockfd = httpGET (eso_host, buf, msg);
	if (sockfd < 0) {
	    stopd_down();
	    return (-1);
	}

	/* now read lines of stars from the socket and collect in opp array */
	while ((n = recvlineb (sockfd, buf, sizeof(buf))) > 0) {
	    double rh, rm, rs;
	    double dd, dm, ds;
	    char name[32];
	    double m;
	    Obj *op;
	    int c;

	    if (strncmp (buf, eofcode, sizeof(eofcode)-1) == 0)
		break;

	    if (sscanf (buf,ifmt,name,&rh,&rm,&rs,&dd,&dm,&ds,&m,&c) != 9)
		continue;
	    if (moreObjF (opp, nop, 1) < 0) {
		(void) strcpy (msg, "No memory");
		(void) close (sockfd);
		stopd_down();
		return (-1);
	    }
	    op = (Obj *)(&(*opp)[nop++]);
	    zero_mem ((void *)op, sizeof(ObjF));

	    (void) sprintf (op->o_name, "GSC %.4s-%.4s", name+1, name+6);
	    op->o_type = FIXED;
	    op->f_class = c == 0 ? 'S' : 'T';
	    op->f_RA = (float) hrrad (rs/3600.0 + rm/60.0 + rh);
	    dd = fabs(dd);
	    op->f_dec = (float) degrad (ds/3600.0 + dm/60.0 + dd);
	    if (buf[23] == '-')
		op->f_dec *= -1.0;
	    op->f_epoch = (float)J2000;
	    set_fmag (op, m);
	}

	(void) close (sockfd);
	stopd_down();

	if (n < 0) {
	    (void) strcpy (msg, syserrstr());
	    return (-1);
	}
	return (nop);
}


/* fetch GSC 2.2 stars around the given field of view from STScI.
 * if find some add them to the list at *opp and return new total, else
 * limit FOV to 30 arc mins -- these fields can be very dense.
 * return -1 with a diagnostic message in msg[].
 */
int
gsc22fetch (
Now *np,		/* current circumstances */
double ra, double dec,	/* RA/Dec of center of view, rads, J2000 */
double fov,		/* diamater of field of view, rads */
double mag,		/* limiting magnitude desired */
ObjF **opp,		/* *opp is malloced with stars found */
int nop,		/* number already at *opp */
char msg[])		/* return diagnostic message here, if returning -1 */
{
#define	GSC22MAXFOV	degrad(0.5)		/* max fov */
	/* http://www-gsss.stsci.edu/cgi-bin/gsc22query.exe?ra=1%3A2%3A3&dec=4%3A5%3A6&r1=0&r2=5&m1=0.0&m2=19.5&n=20000&submit2=Submit+Request */
	static char host[] = "www-gsss.stsci.edu";
	static char ifmt[] = "%s %lf %lf %*lf %*lf %*lf %lf %lf %*lf %*lf %lf %*lf %lf %*lf %lf %*lf %lf %*lf %*lf %*lf %*lf %d";
	static char ofmt[] = "GET http://%s/cgi-bin/gsc22query.exe?ra=%d%%3A%d%%3A%d&dec=%c%d%%3A%d%%3A%d&r1=0&r2=%g&m1=0.0&m2=%g&n=100000&submit2=Submit+Request HTTP/1.0\r\nUser-Agent: xephem/%s\r\n\r\n";
	static char eofcode[] = "[EOD]";
	char buf[1024];
	char dsign;
	int rh, rm, rs, dd, dm, ds;
	double fmag, jmag, vmag, nmag, pmRA, pmdec;
	int class;
	int sockfd;
	int n;

	/* clamp fov */
	if (fov > GSC22MAXFOV) {
	    sprintf (msg, "GSC 22 FOV is limited to %.2f degree",
							raddeg(GSC22MAXFOV));
	    return (-1);
	}

	/* format the GET */
	fs_sexa (buf, radhr(ra), 3, 3600);
	sscanf (buf, "%d:%d:%d", &rh, &rm, &rs);
	fs_sexa (buf, raddeg(fabs(dec)), 4, 3600);
	sscanf (buf, "%d:%d:%d", &dd, &dm, &ds);
	dsign = dec < 0 ? '-' : '+';
	(void) sprintf (buf, ofmt, host, rh, rm, rs, dsign, dd, dm, ds, 
					    raddeg(fov/2)*60, mag, PATCHLEVEL);

	/* let user abort */
	stopd_up();

	/* send the GET method to host and connection to read response */
	sockfd = httpGET (host, buf, msg);
	if (sockfd < 0) {
	    stopd_down();
	    return (-1);
	}

	/* now read lines of stars from the socket and collect in opp array */
	while ((n = recvlineb (sockfd, buf, sizeof(buf))) > 0) {
	    char name[32];
	    Obj *op;

	    if (strncmp (buf, eofcode, sizeof(eofcode)-1) == 0)
		break;

	    if (sscanf (buf, ifmt, name, &radeg, &decdeg, &pmRA, &pmdec,
				    &fmag, &jmag, &vmag, &nmag, &class) != 10)
		continue;
	    if (moreObjF (opp, nop, 1) < 0) {
		(void) strcpy (msg, "No memory");
		(void) close (sockfd);
		stopd_down();
		return (-1);
	    }
	    op = (Obj *)(&(*opp)[nop++]);
	    zero_mem ((void *)op, sizeof(ObjF));

	    (void) sprintf (op->o_name, "GSC2 %.*s", MAXNM-6, name);
	    op->o_type = FIXED;
	    switch (class) {
	    case 0: op->f_class = 'S'; break;
	    case 1: op->f_class = 'G'; break;
	    default: op->f_class = 'T'; break;
	    }
	    op->f_RA = degrad (radeg);
	    op->f_pmRA = (float) 1.327e-11*pmRA; /* mas/yr -> rad/dy */
	    op->f_dec = degrad (decdeg);
	    op->f_pmdec = (float) 1.327e-11*pmdec; /* mas/yr -> rad/dy */
	    if (fabs(op->f_dec) < PI/2)
		op->f_pmRA /= cos (op->f_dec);
	    op->f_epoch = (float)J2000;
	    if (vmag < 30)
		set_fmag (op, vmag);
	    else if (fmag < 30)
		set_fmag (op, fmag);
	    else if (jmag < 30)
		set_fmag (op, jmag);
	    else if (nmag < 30)
		set_fmag (op, nmag);
	    else
		set_fmag (op, 0.0);
	}

	(void) close (sockfd);
	stopd_down();

	if (n < 0) {
	    (void) strcpy (msg, syserrstr());
	    return (-1);
	}
	return (nop);
}

#endif

/* grow *opp, which already contains nop, to hold nnew more.
 * if ok update *opp and return 0, else -1.
 */
static int
moreObjF (opp, nop, nnew)
ObjF **opp;
int nop;
int nnew;
{
	char *newmem;

	/* extend *opp for nnew more ObjF's */
	if (*opp)
	    newmem = realloc ((void *)*opp, (nop+nnew)*sizeof(ObjF));
	else
	    newmem = malloc (nnew * sizeof(ObjF));
	if (!newmem)
	    return (-1);
	*opp = (ObjF *)newmem;
	return (0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: gscnet.c,v $ $Date: 2010/10/06 21:12:16 $ $Revision: 1.22 $ $Name:  $"};
