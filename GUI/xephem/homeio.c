/* functions to support paths relative to HOME, other misc io. */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "xephem.h"


/* if path starts with `~' replace with $HOME.
 * if path starts with '.' replace with pwd.
 * we also remove any leading or trailing blanks, and trailing / or \
 * caller should save returned string before calling us again.
 */
char *
expand_home (char *path)
{
	static char *mpath;
	static char *home;
	static char *pwd;
	static int homel;
	static int pwdl;
	char *p;
	int l;

	/* get home, if we don't already know it */
	if (!home) {
	    home = getenv ("HOME");
	    if (home)
		homel = strlen (home);
	}

	/* get pwd, if we don't already know it */
	if (!pwd) {
	    pwd = getcwd (NULL, 0);
	    if (pwd)
		pwdl = strlen (pwd);
	}

	/* start mpath so we can always just use realloc */
	if (!mpath)
	    mpath = malloc (1);

	/* skip leading blanks */
	l = strlen (path);
	while (*path == ' ') {
	    path++;
	    l--;
	}

	/* move '\0' back past any trailing baggage */
	for (p = &path[l-1]; p >= path; --l)
	    if (*p == ' ' || *p == '/' || *p == '\\')
		*p-- = '\0';
	    else
		break;

	/* prepend home if starts with ~ or pwd if starts with . */
	if (path[0] == '~' && home)
	    sprintf (mpath = realloc (mpath, homel+l), "%s%s", home, path+1);
	else if (path[0] == '.' && pwd)
	    sprintf (mpath = realloc (mpath, pwdl+l), "%s%s", pwd, path+1);
	else
	    strcpy (mpath = realloc(mpath, l+1), path);

	return (mpath);
}

/* like fopen() but substitutes HOME if name starts with '~'
 */
FILE *
fopenh (name, how)
char *name;
char *how;
{
	return (fopen (expand_home(name), how));
}

/* like fopen() but uses Private then Share/sdir dirs unless starts with / or ~.
 * return fp else NULL if not found.
 */
FILE *
fopendq (char *fn, char *sdir, char *how)
{
	char buf[1024];
	FILE *fp;

	if (fn[0] == '/' || fn[0] == '~')
	    return (fopenh(fn, how));

	if (strchr (how, 'r')) {
	    /* open for reading -- check plane then both special dirs */
	    fp = fopenh (fn, how);
	    if (fp)
		return (fp);
	    sprintf (buf, "%s/%s", getPrivateDir(), fn);
	    fp = fopenh (buf, how);
	    if (fp)
		return (fp);
	    sprintf (buf, "%s/%s/%s", getShareDir(), sdir?sdir:".", fn);
	    fp = fopenh (buf, how);
	    if (fp)
		return (fp);
	} else {
	    /* always create in private dir */
	    sprintf (buf, "%s/%s", getPrivateDir(), fn);
	    fp = fopenh (buf, how);
	    if (fp)
		return (fp);
	}

	/* nope */
	return (NULL);
}

/* like fopendq() but issues xe_msg() if fails.
 */
FILE *
fopend (char *fn, char *sdir, char *how)
{
	FILE *fp = fopendq (fn, sdir, how);

	if (!fp)
	    xe_msg (1, "%s:\n%s", fn, syserrstr());

	return (fp);
}

/* like open(2) but substitutes HOME if name starts with '~'.
 */
int
openh (char *name, int flags, ...)
{
	va_list ap;
	int perm;

	va_start (ap, flags);
	perm = va_arg (ap, int);
	va_end (ap);

	return (open (expand_home(name), flags, perm));
}

/* return 0 if the given file exists, else -1.
 * substitute HOME if name starts with '~'.
 */
int
existsh (name)
char *name;
{
	struct stat s;

	return (stat (expand_home(name), &s));
}

/* like existsh() but uses Private then Share/sdir dirs unless starts
 * with / or ~.  
 */
int
existd (char *name, char *sdir)
{
	FILE *fp = fopendq (name, sdir, "r");
	if (fp) {
	    fclose (fp);
	    return (0);
	}
	return (-1);
}

/* fill and return buf with a unique temporary file name containing the given
 * name and suffix
 */
char *
tempfilename (char *buf, char *name, char *suffix)
{
	sprintf (buf, "%s/%s%06ld%010ld%s%s", getPrivateDir(), name,
	    (long)getpid(), (long)time(NULL), *suffix=='.' ? "" : ".", suffix);
	return (buf);
}

/* get the anchor for all of xephem's support files.
 * use TELHOME env first, else ShareDir X resource, else current dir.
 */
char *
getShareDir()
{
	static char *basedir;

	if (!basedir) {
	    char *th = getenv ("TELHOME");
	    if (th) {
		basedir = malloc (strlen(th) + 10);
		if (basedir) {
		    (void) sprintf (basedir, "%s/xephem", th);
		    if (existsh(basedir) < 0) {
			(void) sprintf (basedir, "%s/archive", th);
			if (existsh(basedir) < 0) {
			    free (basedir);
			    basedir = NULL;
			}
		    }
		}
	    }
	    if (!basedir) {
		char *homebase = expand_home (getXRes ("ShareDir", "."));
		basedir = strcpy(malloc(strlen(homebase)+1), homebase);
	    }
	}

	return (basedir);

}

/* return a string for whatever is in errno right now.
 * I never would have imagined it would be so crazy to turn errno into a string!
 */
char *
syserrstr ()
{
#if defined(__STDC__)
/* some older gcc don't have strerror */
#include <errno.h>
return (strerror (errno));
#else
#if defined(VMS)
#include <errno.h>
#include <perror.h>
#else
#if !defined(__FreeBSD__) && !defined(__EMX__)
/* this is aready in stdio.h on FreeBSD */
/* this is already in stdlib.h in EMX   M. Goldberg 27 January 1997 for OS/2 */
extern char *sys_errlist[];
#endif
extern int errno;
#endif
return (sys_errlist[errno]);
#endif
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: homeio.c,v $ $Date: 2005/03/05 06:55:58 $ $Revision: 1.24 $ $Name:  $"};
