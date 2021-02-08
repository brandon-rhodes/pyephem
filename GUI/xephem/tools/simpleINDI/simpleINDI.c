/* This is a stand-alone INDI server process to demonstrate a minimal implementation of
 * listening for and taking action from a Telescope GoTo from XEphem. No indiserver is
 * required which means all the intended multiple client/driver topological flexibility in
 * the INDI protocol is lost, but hopefully it helps a programmer guickly understand how
 * to talk with INDI and use the libastro circum() function call to compute ephemerides
 * and thence control a telescope mount device of the user's choice.
 *
 * This server intentionally mosty uses only standard C/POSIX library calls to be as portable
 * as possible. The only extra libraries required are the "little XML" parser and libastro
 * included with all XEphem distros. The structure is intentionally as straighforward as
 * possible for pedagogical reasons; a production version would be more flexible and robust.
 * The more glaring compromises and shortcuts are marked with the comment "TODO".
 * Connections to real hardware are, of course, not provided but local alt/az values are
 * printed once a second providing a programmer with an easy starting point
 * for their own control of real hardware.
 *
 * This server can be started at any time before, after or during running XEphem. Once
 * XEphem sends the current location and a target edb description, it continuously
 * computes and prints to stdout its topocentric ephemerides, and also sends them back to
 * XEphem for display on the Sky View with its telescope marker.
 *
 * To use:
 *
 *   build using the accompanying Makefile: make
 *   start the program: ./simpleINDI
 *
 * In XEphem:
 *
 *   On the front pane:
 *     Set Latitude and Longitude to your location.
 *     Click "RT" in Looping control.
 *
 *   Open Views -> Sky View
 *
 *   Open Sky View -> Telescope -> Configure...
 *
 *     Fill in the fields next to "Send lat and long once to" with the following:
 *       SimpleTelescope.Location.Latitude  0.174532 0
 *       SimpleTelescope.Location.Longitude 0.174532 0
 *
 *     Check "Enable sending edb" and fill in the field with:
 *       SimpleTelescope.TrackEDB.edb
 *
 *     Check "Enable Sky marker from" and fill in the field with:
 *       SimpleTelescope.Pointing.RA2K  0.2617993 0
 *       SimpleTelescope.Pointing.Dec2K 0.174532  0
 *
 *   On the main panel, open Preferences -> Save ...
 *
 *     Cick "Save now"
 *
 *   Open Sky View -> Telescope -> INDI panel...
 *
 *      Click on Connect
 *
 *   Open Sky View -> Telescope -> Configure...
 *
 *      Click "Send lat and long once to" once
 *
 *   Turn on Sky View -> Telescope -> "Keep visible"
 *
 *   Right-click on any location or target in the Sky View then
 *     release on "Telescope GoTo"
 *
 * simpleINDI now prints topocentric RA/Dec@J2000 and Alt/Az of this target once per second
 *   continuously and its location is marked on the Sky View while XEphem is connected.
 *   A different target may be sent at any time.
 */

/* system includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* local includes */
#include "astro.h"
#include "preferences.h"
#include "lilxml.h"

/* configuration variables -- set as required */
static const int indi_port = 7624;			/* listening port on localhost */
static const int poll_period = 1000;			/* device update period, ms */

/* local globals */
static Now now, *my_nowp;				/* current time&location if known */
static Obj obj, *my_objp;				/* target description, if known */
static const char my_device[] = "SimpleTelescope";	/* INDI property device name */

/* INDI property elements.
 * TODO: a real implementation would support all info, not just these core components.
 */
typedef struct {
    char *name;
    double value;
} NumberElement;
typedef struct {
    char *name;
    NumberElement *np;
    int nnp;
} NumberProperty;
typedef struct {
    char *name;
    char *value;
} TextElement;
typedef struct {
    char *name;
    TextElement *tp;
    int ntp;
} TextProperty;
#define	NELE(x)	(sizeof(x)/sizeof(x[0]))

static NumberElement location_ele[] = {
    { "Latitude",  0.0},
    { "Longitude", 0.0}
};
static NumberProperty location_prop = {
    "Location",
    location_ele,
    NELE (location_ele)
};

static NumberElement pointing_ele[] = {
    { "RA2K",  0.0},
    { "Dec2K", 0.0}
};
static NumberProperty pointing_prop = {
    "Pointing",
    pointing_ele,
    NELE(pointing_ele)
};

static TextElement trackedb_ele[] = {
    { "edb", NULL }
};
static TextProperty trackedb_prop = {
    "TrackEDB",
    trackedb_ele,
    NELE(trackedb_ele)
};

/* local functions */
static void usage (const char *me);
static int openINDI(void);
static void runINDI (int listen_socket);
static int newClSocket(int listen_socket);
static int moreClientMessage (FILE *cl_fp, LilXML *lp);
static void updateDevice(FILE *cl_fp);
static void dispatchINDI (XMLEle *root, FILE *cl_fp);
static void handleLocationMessage (XMLEle *root, FILE *cl_fp);
static void handleTrackEBDMessage (XMLEle *root, FILE *cl_fp);
static void handleGetPropertiesMessage (XMLEle *root, FILE *cl_fp);
static void sendINDINumber (FILE *fp, int isdef, int isok, const char *device, NumberProperty *np,
    const char *fmt, ...);
static int getINDINumber(XMLEle *root, NumberProperty *np);
static void sendINDIText (FILE *fp, int isdef, int isok, const char *device, TextProperty *tp,
    const char *fmt, ...);
static int getINDIText(XMLEle *root, TextProperty *np);
static char *tsNow();
static double mjdNow(void);
static void vsmessage (FILE *fp, const char *fmt, va_list ap);
static int sexagesimal (const char *str0, double *dp);
static void Bye (const char *fmt, ...);

int
main (int ac, char *av[])
{
	int listen_socket;

	if (ac > 1)
	    usage (av[0]);
	
	/* start listening for connections on standard INDI port 7624 */
	listen_socket = openINDI();

	/* set preferences */
	pref_set (PREF_EQUATORIAL, PREF_TOPO);

	/* service one client at a time, forever */
	runINDI(listen_socket);

	return (0);
}

/* print usage summary for the given program and exit 
 */
static void
usage (const char *me)
{
	fprintf (stderr, "Usage: %s\n", me);
	fprintf (stderr, "Purpose: simple stand-alone INDI telescope service\n");
	exit(1);
}

/* return a public indiserver socket endpoint on indi_port
 */
static int
openINDI()
{
        struct sockaddr_in serv_socket;
        int reuse = 1;
        int sfd;

        /* make socket endpoint */
        if ((sfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
            Bye ("socket: %s\n", strerror(errno));

        /* bind to port for any IP address */
        memset (&serv_socket, 0, sizeof(serv_socket));
        serv_socket.sin_family = AF_INET;
        serv_socket.sin_addr.s_addr = htonl (INADDR_ANY);
        serv_socket.sin_port = htons ((unsigned short)indi_port);
        if (setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse)) < 0)
            Bye ("setsockopt: %s\n", strerror(errno));
        if (bind(sfd,(struct sockaddr*)&serv_socket,sizeof(serv_socket)) < 0)
            Bye ("bind: %s\n", strerror(errno));

        /* willing to accept connections with a backlog of 5 pending */
        if (listen (sfd, 5) < 0)
            Bye ("listen: %s\n", strerror(errno));

        /* ok */
	printf ("listening to port %d with fd %d\n", indi_port, sfd);
	return (sfd);
}

/* run the server forever accepting new clients on listen_socket.
 * dispatch messages while client exists, else check for new client,
 * call updateDevice if neither.
 */
static void
runINDI (int listen_socket)
{
	FILE *cl_fp = NULL;	/* client, if any. fileno read, * formatted writes */
	LilXML *lp;		/* parsing context, if client connected */

	while (1) {
	    struct timeval to;
	    fd_set rset;
	    int s, maxn;

	    /* listen for client messages else for new client connection */
	    FD_ZERO (&rset);
	    if (cl_fp) {
		FD_SET (fileno(cl_fp), &rset);
		maxn = fileno(cl_fp);
	    } else {
		FD_SET (listen_socket, &rset);
		maxn = listen_socket;
	    }

	    /* set timeout if neither */
	    to.tv_sec = poll_period/1000;
	    to.tv_usec = 1000*(poll_period%1000);

	    s = select (maxn+1, &rset, NULL, NULL, &to);
	    if (s < 0)
		Bye ("select: %s\n", strerror(errno));
	    if (s == 0) {
		updateDevice(cl_fp);
	    } else if (FD_ISSET (listen_socket, &rset)) {
		/* new client arrival. open private socket and create new parsing context */
		int cl_socket = newClSocket (listen_socket);
		cl_fp = fdopen (cl_socket, "w");
		lp = newLilXML();
	    } else if (FD_ISSET (fileno(cl_fp), &rset)) {
		/* more client message. if closes connection, close socket and delete context */
		if (moreClientMessage (cl_fp, lp) < 0) {
		    printf ("client on socket %d disconnected\n", fileno(cl_fp));
		    fclose (cl_fp);	/* also closes cl_socket */
		    cl_fp = NULL;
		    delLilXML (lp);
		}
	    } else {
		Bye ("impossible fd_set\n");
	    }

	}
}

/* accept a known pending new client arriving on listen_socket.
 * return private socket or exit
 */
static int
newClSocket(int listen_socket)
{
        struct sockaddr_in cli_socket;
        socklen_t cli_len;
        int cli_fd;

        /* get a private connection to new client */
        cli_len = sizeof(cli_socket);
        cli_fd = accept (listen_socket, (struct sockaddr *)&cli_socket, &cli_len);
        if(cli_fd < 0)
            Bye ("accept: %s\n", strerror(errno));

        /* ok */
	printf ("New client arrived, using socket %d\n", cli_fd);
        return (cli_fd);
}

/* read and dispatch more messages known to be pending on cl_fp using parser context lp.
 * return 0 if ok else -1 if closes connected.
 * exit if real trouble.
 */
static int
moreClientMessage (FILE *cl_fp, LilXML *lp)
{
	char buf[1024];
	char err[1024];
	int i, r;

	/* perform exactly one read */
	r = read (fileno(cl_fp), buf, sizeof(buf));
	if (r <= 0)
	    return (-1);

	/* parse more XML, dispatch when see closure */
	for (i = 0; i < r; i++) {
	    XMLEle *root = readXMLEle (lp, buf[i], err);
	    if (root) {
		dispatchINDI (root, cl_fp);
		delXMLEle (root);
	    } else if (err[0])
		Bye ("XML error from '%.*s': %s\n", buf, r, err);
	}

	/* finished parsing this read */
	return (0);
}

/* compute new ephemeris for my_objp at my_nowp, if known.
 * print, and send to cl_fp if open.
 * TODO: control real device!
 */
static void
updateDevice(FILE *cl_fp)
{
	if (!my_objp || !my_nowp)
	    return;

	/* update time */
	my_nowp->n_mjd = mjdNow();

	/* update my_objp @ my_nowp */
	obj_cir (my_nowp, my_objp);

	/* print */
	printf ("%13.5f: %8.5f %8.5f  %8.5f %8.5f\n",
		my_nowp->n_mjd + MJD0,
		radhr(my_objp->s_ra), raddeg(my_objp->s_dec),
		raddeg(my_objp->s_alt), raddeg(my_objp->s_az));

	/* send to client also, if open */
	if (cl_fp) {
	    pointing_prop.np[0].value = radhr(my_objp->s_ra);
	    pointing_prop.np[1].value = raddeg(my_objp->s_dec);
	    sendINDINumber (cl_fp, 0, 1, my_device, &pointing_prop, NULL);
	}
}

/* crack and perform the given INDI XML message.
 * ignore if unrecognized.
 */
static void
dispatchINDI (XMLEle *root, FILE *cl_fp)
{
	char *roottag = tagXMLEle(root);
	char *dev = findXMLAttValu (root, "device");
	char *name = findXMLAttValu (root, "name");

	printf ("Parsing ncoming %s %s.%s\n", roottag, dev, name);

	/* decide type of message -- TODO */
	if (!strcmp (roottag, "getProperties"))
	    handleGetPropertiesMessage (root, cl_fp);
	else if (!strncmp (roottag, "new", 3) && !strcmp (dev, my_device)) {
	    if (!strcmp (name, "Location"))
		handleLocationMessage (root, cl_fp);
	    else if (!strcmp (name, "TrackEDB"))
		handleTrackEBDMessage (root, cl_fp);
	}

}

/* handline incoming getProperties message
 */
static void
handleGetPropertiesMessage (XMLEle *root, FILE *cl_fp)
{
	char *version = findXMLAttValu (root, "version");

	/* require at least version 1 */
	if (atoi(version) < 1)
	    return;

	/* report each supported property */
	sendINDINumber (cl_fp, 1, 1, my_device, &location_prop, NULL);
	sendINDINumber (cl_fp, 1, 1, my_device, &pointing_prop, NULL);
	sendINDIText (cl_fp, 1, 1, my_device, &trackedb_prop, NULL);
}

/* handle incoming Location message 
 */
static void
handleLocationMessage (XMLEle *root, FILE *cl_fp)
{
	if (getINDINumber(root, &location_prop) < 0)
	    return;

	/* init my_nowp to indicate it is now valid */
	my_nowp = &now;
	memset (my_nowp, 0, sizeof(*my_nowp));
	my_nowp->n_mjd = mjdNow();
	my_nowp->n_lat = degrad(location_prop.np[0].value);
	my_nowp->n_lng = degrad(location_prop.np[1].value);
	my_nowp->n_tz = 0;			/* TODO */
	my_nowp->n_temp = 0;			/* TODO */
	my_nowp->n_pressure = 1000;		/* TODO */
	my_nowp->n_elev = 0;			/* TODO */
	my_nowp->n_dip = 0;			/* TODO */
	my_nowp->n_epoch = J2000;		/* TODO */
	strcpy (my_nowp->n_tznm, "UTC");	/* TODO */

	/* ack successful */
	sendINDINumber (cl_fp, 0, 1, my_device, &location_prop, NULL);
}

/* handle incoming TrackEDB message 
 */
static void
handleTrackEBDMessage (XMLEle *root, FILE *cl_fp)
{
	char ynot[1024];
	char *edb;

	if (getINDIText (root, &trackedb_prop) < 0)
	    return;
	edb = trackedb_prop.tp[0].value;

	/* init my_objp if valid and ack */
	if (db_crack_line (edb, &obj, NULL, 0, ynot) > 0) {
	    my_objp = &obj;
	    sendINDIText (cl_fp, 0, 1, my_device, &trackedb_prop, "Now tracking %s", my_objp->o_name);
	} else {
	    my_objp = NULL;
	    sendINDIText (cl_fp, 0, 0, my_device, &trackedb_prop, "Can not grok '%s': %s", edb, ynot);
	}
}

/* send the given INDI Number to fp with an optional message
 */
static void
sendINDINumber (FILE *fp, int isdef, int isok, const char *device, NumberProperty *np,
const char *fmt, ...)
{
	char *tv = isdef ? "def" : "set";
        int i;

        fprintf (fp, "<%sNumberVector\n", tv);
        fprintf (fp, "  device='%s'\n", device);
        fprintf (fp, "  name='%s'\n", np->name);
        fprintf (fp, "  state='%s'\n", isok ? "Ok" : "Alert");
        fprintf (fp, "  timestamp='%s'\n", tsNow());
	if (isdef) {
	    fprintf (fp, "  label='%s'\n", "");				/* TODO */
	    fprintf (fp, "  group='%s'\n", "");				/* TODO */
	    fprintf (fp, "  perm='%s'\n", "rw");			/* TODO */
	    fprintf (fp, "  timeout='%g'\n", 0.0);			/* TODO */
	}

        if (fmt) {
	    va_list ap;
	    va_start (ap, fmt);
            vsmessage (fp, fmt, ap);
	    va_end (ap);
        }
	fprintf (fp, ">\n");

        for (i = 0; i < np->nnp; i++) {
	    if (isdef) {
		fprintf (fp, "<defNumber\n");
		fprintf (fp, "  name='%s'\n", np->np[i].name);
		fprintf (fp, "  label='%s'\n", "");			/* TODO */
		fprintf (fp, "  format='%s'\n", "%10.5g");		/* TODO */
		fprintf (fp, "  min='%g'\n", 0.0);			/* TODO */
		fprintf (fp, "  max='%g'\n", 0.0);			/* TODO */
		fprintf (fp, "  step='%g'>\n", 0.0);			/* TODO */
		fprintf (fp, "    %.20g\n", np->np[i].value);
		fprintf (fp, "</defNumber>\n");
	    } else {
		fprintf (fp, "  <oneNumber name='%s'>\n", np->np[i].name);
		fprintf (fp, "      %.20g\n", np->np[i].value);
		fprintf (fp, "  </oneNumber>\n");
	    }
        }

        fprintf (fp, "</%sNumberVector>\n", tv);
        fflush (fp);
}


/* extract the given INDI Number values from the XML message.
 * return 0 if all ok, else -1
 */
static int
getINDINumber(XMLEle *root, NumberProperty *np)
{
	XMLEle *ep;
	int i, nfound = 0;

	for (ep = nextXMLEle(root,1); ep; ep = nextXMLEle(root,0)) {
	    if (strcmp (tagXMLEle(ep)+3, "Number") == 0) {	// def or set ok
		char *en = findXMLAttValu (ep, "name");
		for (i = 0; i < np->nnp; i++) {
		    if (!strcmp (en, np->np[i].name)) {
			if (sexagesimal (pcdataXMLEle(ep), &np->np[i].value) < 0)
			    return (-1);	/* bad format */
			break;
		    }
		}
		if (i == np->nnp)
		    return (-1);		/* unknown name in message */
		nfound++;
	    }
	}
	if (nfound != np->nnp)
	    return (-1);			/* some numbers missing */
	return (0);				/* ok */
}

/* send the given INDI Text to fp
 */
static void
sendINDIText (FILE *fp, int isdef, int isok, const char *device, TextProperty *tp,
const char *fmt, ...)
{
	char *tv = isdef ? "def" : "set";
        int i;

        fprintf (fp, "<%sTextVector\n", tv);
        fprintf (fp, "  device='%s'\n", device);
        fprintf (fp, "  name='%s'\n", tp->name);
        fprintf (fp, "  state='%s'\n", isok ? "Ok" : "Alert");
	if (isdef) {
	    fprintf (fp, "  label='%s'\n", "");				/* TODO */
	    fprintf (fp, "  group='%s'\n", "");				/* TODO */
	    fprintf (fp, "  perm='%s'\n", "rw");			/* TODO */
	    fprintf (fp, "  timeout='%g'\n", 0.0);			/* TODO */
	}
        fprintf (fp, "  timestamp='%s'\n", tsNow());
        if (fmt) {
	    va_list ap;
	    va_start (ap, fmt);
            vsmessage (fp, fmt, ap);
	    va_end (ap);
        }
	fprintf (fp, ">\n");

        for (i = 0; i < tp->ntp; i++) {
	    char *v = tp->tp[i].value;
	    if (isdef) {
		fprintf (fp, "  <defText\n");
		fprintf (fp, "    name='%s'\n", tp->tp[i].name);
		fprintf (fp, "    label='%s'>\n", "");			/* TODO */
		fprintf (fp, "      %s\n", v ? entityXML(v) : "");
		fprintf (fp, "  </defText>\n");
	    } else {
		fprintf (fp, "  <oneText name='%s'>\n", tp->tp[i].name);
		fprintf (fp, "      %s\n", v ? entityXML(v) : "");
		fprintf (fp, "  </oneText>\n");
	    }
        }

        fprintf (fp, "</%sTextVector>\n", tv);
        fflush (fp);
}


/* extract the given INDI Text values from the XML message.
 * return 0 if all ok, else -1
 * N.B. strings are re/malloc()ed so caller must eventually free()
 */
static int
getINDIText(XMLEle *root, TextProperty *tp)
{
	XMLEle *ep;
	int i, nfound = 0;

	for (ep = nextXMLEle(root,1); ep; ep = nextXMLEle(root,0)) {
	    if (strcmp (tagXMLEle(ep)+3, "Text") == 0) {
		char *en = findXMLAttValu (ep, "name");
		for (i = 0; i < tp->ntp; i++) {
		    if (!strcmp (en, tp->tp[i].name)) {
			char *v = pcdataXMLEle(ep);
			tp->tp[i].value = strcpy (realloc (tp->tp[i].value, strlen(v)+1), v);
			break;
		    }
		}
		if (i == tp->ntp)
		    return (-1);		/* unknown name in message */
		nfound++;
	    }
	}
	if (nfound != tp->ntp)
	    return (-1);			/* some values missing */
	return (0);
}


/* print message to fp
 * N.B. can not vfprint directly, must go through entityXML()
 */
static void
vsmessage (FILE *fp, const char *fmt, va_list ap)
{
        char msg[2048];
        vsnprintf (msg, sizeof(msg), fmt, ap);
        fprintf (fp, "  message='%s'\n", entityXML(msg));
}


/* return current MJD from computer time
 */
static double
mjdNow()
{
        struct timeval tv;
        gettimeofday (&tv, NULL);
        return (2440587.5 + (tv.tv_sec+tv.tv_usec/1e6)/SPD - MJD0);
}

/* return current UTC in ISO-8601 format.
 * N.B. returned string is always the same static, will be reused on each subsequent call
 */
static char *
tsNow()
{
	static char timestamp[32];
	time_t t0 = time(NULL);
	struct tm *tmp = gmtime (&t0);

	strftime (timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", tmp);
	return(timestamp);
}

/* convert sexagesimal string str AxBxC to double.
 *   x can be anything non-numeric. Any missing A, B or C will be assumed 0.
 *   optional - and + can be anywhere.
 * return 0 if ok, -1 if can't find a thing.
 */
static int
sexagesimal (
const char *str0,	/* input string */
double *dp)		/* cracked value, if return 0 */
{
	double a, b, c;
	char str[256];
	char *neg;
	int i, l, isneg;

	/* copy str0 so we can play with it */
	strncpy (str, str0, sizeof(str)-1);
	str[sizeof(str)-1] = '\0';

	/* note first negative (but not fooled by neg exponent) */
	isneg = 0;
	neg = strchr(str, '-');
	if (neg && (neg == str || (neg[-1] != 'E' && neg[-1] != 'e'))) {
	    *neg = ' ';
	    isneg = 1;
	}

	/* crack up to three components -- treat blank as 0 */
	a = b = c = 0.0;
	l = strlen(str);
	for (i = 0; i < l; i++) {
	    if (!isspace(str[i])) {
		if (sscanf (str, "%lf%*[^0-9]%lf%*[^0-9]%lf", &a, &b, &c) < 1)
		    return (-1);
		break;
	    }
	}

	/* back to one value, restoring neg */
	*dp = (c/60.0 + b)/60.0 + a;
	if (isneg)
	    *dp *= -1;
	return (0);
}

/* print the given error message to stderr and exit
 */
static void
Bye (const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	vfprintf (stderr, fmt, ap);
	va_end (ap);

	exit (1);
}
