/* xephemdbd: read lines of FOV requests and produce object lists in .edb or
 * plain text format. Requests which specify their own output file are each
 * handled by a separate child process.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>


#include "xephem.h"


extern int handle_request (char *req, int nofs);
extern void db_read (char *fn);
extern int fs_setup (char catdir[], char msg[]);

static void usage (void);
static void chklkfile (void);
static void mklkfile (void);
static void newstdin (void);
static void daemonize (void);
static void find_edb (void);
static void go(void);
static void new_process (char *req);
static void rmlkfile(void);

static char catdir_def[] = "/usr/local/xephem/catalogs";
static char moddir_def[] = "/usr/local/xephem/auxil";
static char lkfile_def[] = "/tmp/xephemdbd.pid";

static int dflag;
static int vflag;
static int fflag;
static char *pname;
static char *catdir = catdir_def;
static char *moddir = moddir_def;
static char *lkfile = lkfile_def;
static char *ifile;

/* we exit after this many minutes with no request.
 * but we allow waiting forever for that first exciting one.
 */
#define	IDLET_DEF	30
static int idlet = IDLET_DEF;

int
main (int ac, char *av[])
{
	/* save name */
	pname = av[0];

	/* scan args */
	while ((--ac > 0) && ((*++av)[0] == '-')) {
	    char *s;
	    for (s = av[0]+1; *s != '\0'; s++)
		switch (*s) {
		case 'c':
		    if (ac < 2)
			usage();
		    catdir = *++av;
		    ac--;
		    break;
		case 'd':
		    dflag++;
		    break;
		case 'f':
		    fflag++;
		    break;
		case 'i':
		    if (ac < 2)
			usage ();
		    ifile = *++av;
		    ac--;
		    break;
		case 'l':
		    if (ac < 2)
			usage ();
		    lkfile = *++av;
		    ac--;
		    break;
		case 'm':
		    if (ac < 2)
			usage ();
		    moddir = *++av;
		    ac--;
		    break;
		case 't':
		    if (ac < 2)
			usage ();
		    idlet = atoi(*++av);
		    ac--;
		    break;
		case 'v':
		    vflag++;
		    break;
		default:
		    usage();
		    break;
		}
	}
	if (ac > 0)
	    usage();

	/* see if already running */
	chklkfile ();

	/* fork/exit unless d flag */
	if (!dflag)
	    daemonize();

	/* make the lock file */
	mklkfile ();

	/* read in all edb files and check for ppm and gsc */
	find_edb();

	/* reassign stdin if given -i */
	if (ifile)
	    newstdin ();

	/* start listening for commands on stdin until eof */
	go();

	rmlkfile();
	return (0);
}

static void
usage()
{
	FILE *fp = stderr;

	fprintf(fp,"%s [options]:\n", pname);
	fprintf(fp,"Options:\n");
	fprintf(fp," -c <dir>    alternate catalogs directory;\n");
	fprintf(fp,"             default is %s\n", catdir_def);
	fprintf(fp," -d          do not fork/exit as a daemon process\n");
	fprintf(fp," -f          do not try to use field star catalogs\n");
	fprintf(fp," -i <file>   open file R/W for input; default is stdin\n");
	fprintf(fp," -l <file>   lock file; default is %s\n",lkfile_def);
	fprintf(fp," -m <file>   dir of moon models; default is %s\n",moddir_def);
	fprintf(fp," -t <secs>   max idle minutes, 0=forever. default is %d\n",
								    IDLET_DEF);
	fprintf(fp," -v          verbose to stderr\n");
	fprintf(fp,"\n");
	fprintf(fp,"Input format:\n");
	fprintf(fp," [>file]     optional input source, else stdin\n");
	fprintf(fp," outputmode  1:column; 2:topo; 4:apparent; 8:header\n");
	fprintf(fp," objtypes    15:sol sys; 16:br stars; 224:deep sky; 768:field\n");
	fprintf(fp," year        time of ephemerides, decimal year\n");
	fprintf(fp," RA,Dec      position of center, rads\n");
	fprintf(fp," FOV         field of view, rads\n");
	fprintf(fp," Mag         limiting magnitude\n");
	fprintf(fp," [lt,lg,el]  location, if topocentric\n");

	exit (1);
}

/* if lkfile indicates another xephemdbd is running exit 0;
 * if we can't tell but can't rule it out exit 1;
 * else just return, which means we appear to be alone and it's ok to run.
 */
static void
chklkfile ()
{
	char buf[1024];
	int n, fd;

	fd = open (lkfile, O_RDONLY);
	if (fd < 0) {
	    if (errno == EACCES) {
		fprintf (stderr, "%s: exists but can not read to check pid\n", 
									lkfile);
		exit (1);
	    }
	} else {
	    n = read (fd, buf, sizeof(buf));
	    close (fd);
	    if (n < 0) {
		fprintf (stderr, "%s: %s\n", lkfile, strerror(errno));
		exit (1);
	    }
	    buf[n] = '\0';
	    n = atoi (buf);
	    if (kill (n, 0) == 0 || errno != ESRCH) {
		if (vflag)
		    fprintf (stderr, "Already running\n");
		exit(0);
	    }
	}
}

/* make the lock file with our pid in it.
 * exit if trouble.
 */
static void
mklkfile ()
{
	char buf[1024];
	int n, fd;

	fd = open (lkfile, O_CREAT|O_WRONLY, 0644);
	if (fd < 0) {
	    fprintf (stderr, "%s: %s\n", lkfile, strerror(errno));
	    exit (1);
	}
	n = sprintf (buf, "%d\n", getpid());
	if (write (fd, buf, n) < 0) {
	    fprintf (stderr, "%s: %s\n", lkfile, strerror(errno));
	    exit (1);
	}
	close (fd);
}

static void
rmlkfile()
{
	remove (lkfile);
}

/* reopen stdin as ifile.
 * N.B. open R/W in case it is a fifo so we will never see EOF and can
 *   stand by forever waiting for requests.
 */
static void
newstdin ()
{
	if (!freopen (ifile, "r+", stdin)) {
	    fprintf (stderr, "%s: %s\n", ifile, strerror(errno));
	    rmlkfile();
	    exit (1);
	}
}

/* make a new process to serve as the daemon.
 * this only returns if we are the new daemon process.
 */
static void
daemonize()
{
	int i;
	long n;

	switch (fork()) {
	case -1:
	    perror ("fork");
	    exit (1);
	    break;
	case 0:
	    /* close all, but preserve out/err for messages */
	    n = sysconf (_SC_OPEN_MAX);
	    for (i = 3; i < n; i++)
		(void) close (i);
	    (void) setsid();
	    break;
	default:
	    exit (0);
	}
}

/* read in all edb we can find.
 * also check for ppm and gsc files.
 */
static void
find_edb()
{
	struct dirent *dirent;
	char buf[1024];
	DIR *dir;

	/* register moon model dir */
	setMoonDir (moddir);

	/* check for field star catalogs unless disabled */
	if (!fflag) {
	    if (vflag)
		fprintf (stderr, "Checking GSC and PPM\n");
	    if (fs_setup(catdir, buf) < 0) {
		fprintf (stderr, "%s\n", buf);
		rmlkfile();
		exit (1);
	    }
	}

	/* open dir */
	dir = opendir (catdir);
	if (!dir) {
	    fprintf (stderr, "%s: %s", catdir, strerror(errno));
	    rmlkfile();
	    exit (1);
	}

	/* scan for and read each .edb catalog -- skip sao! */
	if (vflag)
	    fprintf (stderr, "Loading %s/*.edb (except sao.edb)\n", catdir);
	while ((dirent = readdir (dir)) != NULL) {
	    char *name = dirent->d_name;
	    int l = strlen (name);

	    if (l > 4 && !strcasecmp (name+(l-4), ".edb")
					    && strncasecmp (name, "sao", 3)) {
		(void) sprintf (buf, "%s/%s", catdir, name);
		if (vflag)
		    fprintf (stderr, "    %s\n", name);
		db_read (buf);
	    }
	}

	if (vflag)
	    fprintf (stderr, "done\n");

	(void) closedir (dir);
}

static void
on_alarm (int signo)
{
	if (vflag)
	    fprintf (stderr, "Idle time-out\n");
	rmlkfile();
	exit (0);
}

/* loop reading stdin until see eof, sending results to stdout.
 * if nothing after idlet minutes, exit (but allow first read to wait forever).
 */
static void
go()
{
	char request[1024];

	/* no zombies */
	signal (SIGCHLD, SIG_IGN);

	/* prepare for SIGALRM */
	signal (SIGALRM, on_alarm);

	if (vflag)
	    fprintf (stderr, "%6d: Master daemon pid\n", getpid());

	while (fgets (request, sizeof(request), stdin) != NULL) {
	    alarm (0);	/* cancel timeout */
	    request[strlen(request)-1] = '\0';	/* strip \n */
	    if (vflag)
		fprintf (stderr, "Request: %s\n", request);

	    if (request[0] == '>')
		new_process (request);
	    else
		(void) handle_request (request, fflag);
	    alarm (60*idlet);	/* arm timeout */
	}
}

/* handle the given request in its own process.
 */
static void
new_process (char *req)
{
	int pid;

	pid = fork();

	if (pid == 0) {
	    /* child */
	    char *fnp;

	    /* after '>' and up to ',' is new file name */
	    fnp = strchr (req++, ',');
	    if (!fnp) {
		fprintf (stderr, "No file name\n");
		exit(1);
	    }
	    *fnp = '\0';

	    /* open req file as stdout, no buffering */
	    if (vflag)
		fprintf (stderr, "Opening %s\n", req);
	    if (!freopen (req, "w", stdout)) {
		fprintf (stderr, "%s: %s", req, strerror(errno));
		exit(1);
	    }
	    setbuf (stdout, NULL);

	    /* handle remainder of string as the request as usual */
	    (void) handle_request (fnp + 1, fflag);
	    exit(0);

	} else if (pid < 0) {

	    perror ("fork");
	    rmlkfile();
	    exit (1);
	}

	/* parent just resumes */
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: xephemdbd.c,v $ $Date: 2003/12/05 06:27:38 $ $Revision: 1.6 $ $Name:  $"};
