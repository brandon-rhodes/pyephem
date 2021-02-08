/* code to manage networking */

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

#include "xephem.h"

#define	TOUT	180		/* max secs to wait for socket data.
				 * default http timeout is 3 minutes.
				 */


static int net_save (void);
static void net_create_form (void);
static void defaultSOCKS (void);
static void net_setup (void);
static void ok_cb (Widget w, XtPointer client, XtPointer call);
static void tb_cb (Widget w, XtPointer client, XtPointer call);
static void cancel_cb (Widget w, XtPointer client, XtPointer call);
static void pw_cb (Widget w, XtPointer client, XtPointer call);
static void help_cb (Widget w, XtPointer client, XtPointer call);
static int tout (int maxt, int fd, int w);
static char *herr (char *errmsg);
static int connect_to (int sockfd, struct sockaddr *serv_addr, int addrlen);

static Widget netshell_w;	/* the main form dialog */
static Widget ndir_w;		/* net direct TB */
static Widget socks_w;		/* SOCKS option TB */
static Widget socksh_w, socksp_w;/* SOCKS host and port TF */
static Widget proxy_w;		/* Proxy option TB */
static Widget proxyh_w, proxyp_w;/* Proxy host and port TF */
static Widget auth_w;		/* Auth option TB */
static Widget authn_w, authpw_w;/* Auth name and PW TF */

/* passed to the TB callbacks to choose network access method */
typedef enum {
    NETDIRTB, NETPROXTB, NETSOCKSTB
} TB;

/* current info */
static int proxy_on;		/* whether proxy network connection is on */
static char *proxy_host;	/* proxy host */
static int proxy_port;		/* proxy port */
static int socks_on;		/* whether SOCKS network connection is on */
static char *socks_host;	/* SOCKS host */
static char *socks_port;	/* SOCKS port (as a string) */
static int auth_on;		/* whether to add Auth info */
static char *auth_name;		/* Auth name */

/* we keep the plaintext password here, but just display splats */
static char *auth_pw;		/* malloced plain text password */

/* buffer for recvlineb() */
static char rb_linebuf[512];	/* [next .. bad-1] are good */
static int rb_next;		/* index of next good char */
static int rb_unk;		/* index of first unknown char */

static char netcategory[] = "Network";	/* Save category */

/* call to set up without actually bringing up the menus.
 */
void
net_create()
{
	if (!netshell_w) {
	    net_create_form();
	    (void) net_save();	/* confirming here is just annoying */
	}
}

void
net_manage()
{
	net_create();

	if (!isUp (netshell_w))
	    net_setup();

        XtPopup (netshell_w, XtGrabNone);
	set_something (netshell_w, XmNiconic, (XtArgVal)False);
}

/* Base64 stuff lifted from downloader */

static void
three_to_four(what, where)
unsigned char *what, *where;
{
	static char Table64[64] = {
	    'A','B','C','D','E','F','G','H',
	    'I','J','K','L','M','N','O','P',
	    'Q','R','S','T','U','V','W','X',
	    'Y','Z','a','b','c','d','e','f',
	    'g','h','i','j','k','l','m','n',
	    'o','p','q','r','s','t','u','v',
	    'w','x','y','z','0','1','2','3',
	    '4','5','6','7','8','9','+','/'
	};
	int i;

	*where=(*what >> 2) & 63;
	*(where+1)=((*what << 4) | (*(what+1) >> 4)) & 63;
	*(where+2)=((*(what+1) << 2) | (*(what+2) >> 6)) & 63;
	*(where+3)=*(what+2) & 63;
	for (i=0;i<4;i++)
	    where[i]= Table64[where[i]];
}

static void
string_to_base64(plain, b64)
char *plain, *b64;
{
	unsigned char four[4];
	int len=strlen(plain),len2=0;
	int i;

	if (len%3) len2=(len/3 +1)*4 +1;
	else len2=(len/3)*4 +1;
	while (len>=3) {
	    three_to_four((unsigned char *)plain,four);
	    for (i=0;i<4;i++)
		*(b64++)=four[i];
	    len-=3;
	    plain+=3;
	}
	if (len) {
	    unsigned char three[3]={0,0,0};
	    for (i=0;i<len;i++) {
		three[i]=*((unsigned char*)plain);
		plain++;
	    }
	    three_to_four(three,four);
	    for (i=3-(len==1);i<4;i++)
		four[i]='=';
	    for (i=0;i<4;i++)
		*(b64++)=four[i];
	}
	*b64=0;
}

/* add proxy-authorization to buf.
 * N.B. we assume buf[] ends with \r\n
 */
static void
addAuth (buf)
char *buf;
{
	char *nm = XmTextFieldGetString (authn_w);
	char plainbuf[1024], b64buf[1024];

	sprintf (plainbuf, "%s:%s", nm, auth_pw);
	string_to_base64 (plainbuf, b64buf);
	XtFree (nm);

	/* insert before last \r\n */
	sprintf (buf+strlen(buf)-2, "Proxy-Authorization: Basic %s\r\n\r\n",
								    b64buf);
}

/* open the host, do the given GET cmd, and return a socket fd for the result.
 * return -1 and with excuse in msg[], else 0 if ok.
 * N.B. can be called before we are created if net set in app defaults.
 */
int
httpGET (char *host, char *GETcmd, char msg[])
{
	char buf[2048];
	int fd;
	int n;

	/* open connection */
	if (proxy_on) {
	    fd = mkconnection (proxy_host, proxy_port, msg);
	    if (fd < 0)
		return (-1);
	} else {
	    /* SOCKS or direct are both handled by mkconnection() */
	    fd = mkconnection (host, 80, msg);
	    if (fd < 0)
		return (-1);
	}

	/* fill buf */
	(void) sprintf (buf, "%s", GETcmd);

	/* add proxy auth if enabled */
	if (!auth_w)
	    net_create_form();
	if (XmToggleButtonGetState (auth_w))
	    addAuth(buf);

	/* log it */
	xe_msg (0, "http: %s", buf);

	/* send it */
	n = strlen (buf);
	if (sendbytes(fd, (unsigned char *)buf, n) < 0) {
	    (void) sprintf (msg, "%s: send error: %s", host, syserrstr());
	    (void) close (fd);
	    return (-1);
	}

	/* caller can read response */
	return (fd);
}

/* establish a TCP connection to the named host on the given port.
 * if ok return file descriptor, else -1 with excuse in msg[].
 * try Socks if see socks_on.
 * reset recvlineb() readahead in case old stuff left behind.
 * N.B. we assume SIGPIPE is SIG_IGN
 */
int
mkconnection (
char *host,	/* name of server */
int port,	/* TCP port */
char msg[])	/* return diagnostic message here, if returning -1 */
{

	struct sockaddr_in serv_addr;
	struct hostent  *hp;
	int sockfd;

	/* lookup host address.
	 * TODO: time out but even SIGALRM doesn't awaken this if it's stuck.
	 *   I bet that's why netscape forks a separate dnshelper process!
	 */
	hp = gethostbyname (host);
	if (!hp) {
	    (void) sprintf (msg, "Can not find IP of %s.\n%s", host, 
					    herr ("Try entering IP directly"));
	    return (-1);
	}

	/* connect -- use socks if on */
        if (socks_on) {
            /* Connection over SOCKS server */
            struct {
		unsigned char  VN;	/* version number */
		unsigned char  CD;	/* command code */
		unsigned short DSTPORT;	/* destination port */
		unsigned long  DSTIP;	/* destination IP addres */
	    } SocksPacket;

	    struct hostent *hs = gethostbyname (socks_host);
	    char *Socks_port_str = socks_port;
	    int Socks_port = Socks_port_str ? atoi (Socks_port_str) : 1080;

            SocksPacket.VN = 4;		/* version 4 */
	    SocksPacket.CD = 1;		/* Command code 1 = connect request */
            SocksPacket.DSTPORT = htons((short)port);

            if (!hs) {
		(void) sprintf (msg, "SOCKS: %s:\nCan not get server IP:\n%s.",
				socks_host, herr("Try entering IP directly"));
		return (-1);
            }

	    (void) memset ((char *)&serv_addr, 0, sizeof(serv_addr));
	    serv_addr.sin_family = AF_INET;
	    serv_addr.sin_addr.s_addr=
                              ((struct in_addr *)(hs->h_addr_list[0]))->s_addr;
	    serv_addr.sin_port = htons((short)Socks_port);
	    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
	        (void) sprintf (msg, "SOCKS: %s/%d:\n%s", socks_host, port,
								syserrstr());
	        return (-1);
	    }

	    /* Yes, again. Some variables inside are static */
	    hp = gethostbyname (host);

            SocksPacket.DSTIP=((struct in_addr *)(hp->h_addr_list[0]))->s_addr;
	    if (connect_to (sockfd, (struct sockaddr *)&serv_addr,
						    sizeof(serv_addr)) < 0) {
	        (void) sprintf (msg, "SOCKS: %s: %s", socks_host, syserrstr());
	        (void) close(sockfd);
	        return (-1);
            }
            (void)write (sockfd, &SocksPacket, sizeof (SocksPacket));
            (void)write (sockfd, "xephem", 7);	/* yes, include trailing \0 */
            (void)read  (sockfd, &SocksPacket, sizeof (SocksPacket));
            switch (SocksPacket.CD) {
                case 90:
                    break; /* yes! */
                case 92:
                    (void) sprintf (msg, "SOCKS: cannot connect to client");
		    (void) close(sockfd);
                    return (-1);
                case 93:
                    (void) sprintf (msg, "SOCKS: client program and ident report different user-ids");
		    (void) close(sockfd);
                    return (-1);
                default:
                    (void) sprintf (msg, "SOCKS: Request rejected or failed");
		    (void) close(sockfd);
                    return (-1);
            }
            
	} else {
            /* normal connection without SOCKS server */
	    /* create a socket to the host's server */
	    (void) memset ((char *)&serv_addr, 0, sizeof(serv_addr));
	    serv_addr.sin_family = AF_INET;
	    serv_addr.sin_addr.s_addr =
			((struct in_addr *)(hp->h_addr_list[0]))->s_addr;
	    serv_addr.sin_port = htons((short)port);
	    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
	        (void) sprintf (msg, "%s/%d: %s", host, port, syserrstr());
	        return (-1);
	    }
	    if (connect_to (sockfd, (struct sockaddr *)&serv_addr,
						    sizeof(serv_addr)) < 0) {
	        (void) sprintf (msg, "%s: %s", host, syserrstr());
	        (void) close(sockfd);
	        return (-1);
            }
	}

	/* reset readahead in case user uses recvlineb() */
	rb_next = rb_unk = 0;

	/* ok */
	return (sockfd);
}

/* send n bytes from buf to socket fd.
 * return 0 if ok else -1
 */
int
sendbytes (int fd, unsigned char buf[], int n)
{
	int ns, tot;

	for (tot = 0; tot < n; tot += ns) {
	    if (tout (TOUT, fd, 1) < 0)
		return (-1);
	    ns = write (fd, (void *)(buf+tot), n-tot);
	    if (ns <= 0)
		return (-1);
	}
	return (0);
}

/* receive exactly n bytes from socket fd into buf.
 * return -1, 0 or n.
 */
int
recvbytes (int fd, unsigned char buf[], int n)
{
	int ns, tot;

	for (tot = 0; tot < n; tot += ns) {
	    if (tout (TOUT, fd, 0) < 0)
		return (-1);
	    ns = read (fd, (void *)(buf+tot), n-tot);
	    if (ns <= 0)
		return (ns);
	}
	return (n);
}

/* like read(2) except we time out and allow user to cancel.
 * receive up to n bytes from socket fd into buf.
 * return count, or 0 on eof or -1 on error.
 */
int
readbytes (int fd, unsigned char buf[], int n)
{
	int ns;

	if (tout (TOUT, fd, 0) < 0)
	    return (-1);
	ns = read (fd, (void *)buf, n);
	return (ns);
}

/* read up to and including the next '\n' from socket fd into buf[max].
 * we silently ignore all '\r'. we add a trailing '\0'.
 * return line lenth (not counting \0) if all ok, else -1.
 * N.B. this never reads ahead -- if that's ok, recvlineb() is better
 */
int
recvline (int fd, char buf[], int max)
{
	unsigned char c;
	int n;

	max--;	/* leave room for trailing \0 */

	for (n = 0; n < max && recvbytes (fd, &c, 1) == 1; ) {
	    if (c != '\r') {
		buf[n++] = c;
		if (c == '\n') {
		    buf[n] = '\0';
		    return (n);
		}
	    }
	}

	return (-1);
}

/* rather like recvline but reads ahead in big chunk for efficiency.
 * return length if read a line ok, 0 if hit eof, -1 if error.
 * N.B. we silently swallow all '\r'.
 * N.B. we read ahead and can hide bytes after each call.
 */
int
recvlineb (int sock, char *buf, int size)
{
	char *origbuf = buf;		/* save to prevent overfilling buf */
	char c = '\0';
	int ok = 1;

	/* always leave room for trailing \n */
	size -= 1;

	/* read and copy linebuf[next] to buf until buf fills or copied a \n */
	do {

	    if (rb_next >= rb_unk) {
		/* linebuf is empty -- refill */

		int nr;

		if (tout (TOUT, sock, 0) < 0) {
		    nr = -1;
		    break;
		}
		nr = read (sock, rb_linebuf, sizeof(rb_linebuf));
		if (nr <= 0) {
		    ok = nr;
		    rb_next = 0;
		    rb_unk = 0;
		    break;
		}
		rb_next = 0;
		rb_unk = nr;
	    }

	    if ((c = rb_linebuf[rb_next++]) != '\r')
		*buf++ = c;

	} while (buf-origbuf < size && c != '\n');

	/* always give back a real line regardless, else status */
	if (ok > 0) {
	    *buf = '\0';
	    ok = buf - origbuf;
	}

	return (ok);
}

static void
net_create_form()
{
	Widget netform_w;
	Widget f, w;
	Arg args[20];
	int n;

	/* create form */
	n = 0;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNtitle,"xephem Network setup");n++;
	XtSetArg (args[n], XmNiconName, "Net"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	netshell_w = XtCreatePopupShell ("NetSetup", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (netshell_w);
	set_something (netshell_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (netshell_w, "XEphem*NetSetup.x", netcategory, 0);
	sr_reg (netshell_w, "XEphem*NetSetup.y", netcategory, 0);

	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNverticalSpacing, 10); n++;
	netform_w = XmCreateForm (netshell_w, "NetForm", args, n);
        XtAddCallback (netform_w, XmNhelpCallback, help_cb, NULL);
	XtManageChild (netform_w);

	/* make the title */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
	w = XmCreateLabel (netform_w, "NetT", args, n);
	set_xmstring (w, XmNlabelString, "Network setup:");
	XtManageChild (w);

	    /* make the Direct toggle */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    ndir_w = XmCreateToggleButton (netform_w, "Direct", args, n);
	    XtAddCallback (ndir_w, XmNvalueChangedCallback, tb_cb,
							(XtPointer)NETDIRTB);
	    set_xmstring (ndir_w, XmNlabelString, "Direct connect");
	    wtip (ndir_w, "Use direct internet connection (no proxy or SOCKS)");
	    XtManageChild (ndir_w);
	    sr_reg (ndir_w, NULL, netcategory, 1);

	    /* make the SOCKS toggle and info (first because label is wider) */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, ndir_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    socks_w = XmCreateToggleButton (netform_w, "SOCKS", args, n);
	    set_xmstring (socks_w, XmNlabelString, "via SOCKS");
	    XtAddCallback (socks_w, XmNvalueChangedCallback, tb_cb,
							(XtPointer)NETSOCKSTB);
	    wtip (socks_w, "Use SOCKS V4 to Internet");
	    XtManageChild (socks_w);
	    sr_reg (socks_w, NULL, netcategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, ndir_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, socks_w); n++;
	    XtSetArg (args[n], XmNleftOffset, 10); n++;
	    XtSetArg (args[n], XmNcolumns, 5); n++;
	    socksp_w = XmCreateTextField (netform_w, "SOCKSPort", args, n);
	    wtip (socksp_w, "SOCKS port number");
	    XtManageChild (socksp_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, ndir_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, socksp_w); n++;
	    XtSetArg (args[n], XmNleftOffset, 10); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNcolumns, 40); n++;
	    socksh_w = XmCreateTextField (netform_w, "SOCKSHost", args, n);
	    wtip (socksh_w, "Name of SOCKS host");
	    XtManageChild (socksh_w);

	    defaultSOCKS();
	    sr_reg (socksp_w, NULL, netcategory, 1);
	    sr_reg (socksh_w, NULL, netcategory, 1);

	    /* make the Proxy toggle and info */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, socksh_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    proxy_w = XmCreateToggleButton (netform_w, "Proxy", args, n);
	    set_xmstring (proxy_w, XmNlabelString, "via Proxy");
	    XtAddCallback (proxy_w, XmNvalueChangedCallback, tb_cb,
							(XtPointer)NETPROXTB);
	    wtip (proxy_w, "Reach the Internet through a proxy");
	    XtManageChild (proxy_w);
	    sr_reg (proxy_w, NULL, netcategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, socksh_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, socks_w); n++;
	    XtSetArg (args[n], XmNleftOffset, 10); n++;
	    XtSetArg (args[n], XmNcolumns, 5); n++;
	    proxyp_w = XmCreateTextField (netform_w, "ProxyPort", args, n);
	    wtip (proxyp_w, "Proxy port number");
	    XtManageChild (proxyp_w);
	    sr_reg (proxyp_w, NULL, netcategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, socksh_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, proxyp_w); n++;
	    XtSetArg (args[n], XmNleftOffset, 10); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNcolumns, 40); n++;
	    proxyh_w = XmCreateTextField (netform_w, "ProxyHost", args, n);
	    wtip (proxyh_w, "Name of Proxy host");
	    XtManageChild (proxyh_w);
	    sr_reg (proxyh_w, NULL, netcategory, 1);

	    /* make the Auth option and info */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, proxyh_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNindicatorType, XmN_OF_MANY); n++;
	    auth_w = XmCreateToggleButton (netform_w, "Auth", args, n);
	    set_xmstring (auth_w, XmNlabelString, "Auth Name");
	    wtip (auth_w, "Apply authentication name and password");
	    XtManageChild (auth_w);
	    sr_reg (auth_w, NULL, netcategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, proxyh_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, auth_w); n++;
	    XtSetArg (args[n], XmNleftOffset, 10); n++;
	    XtSetArg (args[n], XmNcolumns, 15); n++;
	    authn_w = XmCreateTextField (netform_w, "AuthName", args, n);
	    wtip (authn_w, "Authentication name");
	    XtManageChild (authn_w);
	    sr_reg (authn_w, NULL, netcategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, proxyh_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, authn_w); n++;
	    XtSetArg (args[n], XmNleftOffset, 10); n++;
	    w = XmCreateLabel (netform_w, "Password", args, n);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNtopWidget, proxyh_w); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, w); n++;
	    XtSetArg (args[n], XmNleftOffset, 10); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNcolumns, 15); n++;
	    authpw_w = XmCreateTextField (netform_w, "AuthPassword", args, n);
	    XtAddCallback (authpw_w, XmNmodifyVerifyCallback, pw_cb, 0);
	    wtip (authpw_w, "Authentication password");
	    XtManageChild (authpw_w);

	    /* malloc null string to start */
	    auth_pw = XtNewString ("");


	/* make the controls across the bottom under a separator */

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, authpw_w); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	w = XmCreateSeparator (netform_w, "Sep", args, n);
	XtManageChild (w);

	n = 0;
	XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg (args[n], XmNtopWidget, w); n++;
	XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg (args[n], XmNfractionBase, 21); n++;
	f = XmCreateForm (netform_w, "CF", args, n);
	XtManageChild (f);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 3); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 6); n++;
	    w = XmCreatePushButton (f, "Ok", args, n);
	    wtip (w, "Install changes and close dialog");
	    XtAddCallback (w, XmNactivateCallback, ok_cb, NULL);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 9); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 12); n++;
	    w = XmCreatePushButton (f, "Close", args, n);
	    wtip (w, "Close this menu without doing anything");
	    XtAddCallback (w, XmNactivateCallback, cancel_cb, NULL);
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 15); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 18); n++;
	    w = XmCreatePushButton (f, "Help", args, n);
	    wtip (w, "More detailed descriptions");
	    XtAddCallback (w, XmNactivateCallback, help_cb, NULL);
	    XtManageChild (w);
}

/* init SOCKS host and port. first check SOCKS_PORT and SOCKS_NS env variables,
 * respectively (same ones used by netscape) then X resources.
 */
static void
defaultSOCKS()
{
	char *str;

	str = getenv ("SOCKS_PORT");
	if (str)
	    XmTextFieldSetString (socksp_w, str ? str : "1080");

	str = getenv ("SOCKS_NS");
	if (str)
	    XmTextFieldSetString (socksh_w, str);
}

/* set up the dialog according to our static state */
static void
net_setup ()
{
	/* Net */
	XmToggleButtonSetState (ndir_w, !socks_on && !proxy_on, False);

	XmToggleButtonSetState (proxy_w, proxy_on, False);
	if (proxy_host)
	    XmTextFieldSetString (proxyh_w, proxy_host);
	if (proxy_port) {
	    char buf[32];
	    (void) sprintf (buf, "%d", proxy_port);
	    XmTextFieldSetString (proxyp_w, buf);
	}

	XmToggleButtonSetState (socks_w, socks_on, False);
	if (socks_host)
	    XmTextFieldSetString (socksh_w, socks_host);
	if (socks_port)
	    XmTextFieldSetString (socksp_w, socks_port);

	XmToggleButtonSetState (auth_w, auth_on, False);
	if (auth_name)
	    XmTextFieldSetString (authn_w, auth_name);
}

/* save the dialog as our static state.
 * if any major trouble, issue xe_msg and return -1, else return 0.
 */
static int
net_save ()
{
	char *str, msg[1024];
	int allok = 1;
	int fd;

	watch_cursor (1);

	/* Network setup.
	 * N.B. do this before using the network :-)
	 */
	proxy_on = XmToggleButtonGetState (proxy_w);
	if (proxy_host)
	    XtFree (proxy_host);
	proxy_host = XmTextFieldGetString (proxyh_w);
	str = XmTextFieldGetString (proxyp_w);
	proxy_port = atoi (str);
	XtFree (str);
	if (proxy_on) {
	    fd = mkconnection (proxy_host, proxy_port, msg);
	    if (fd < 0) {
		xe_msg (1, "%s", msg);
		proxy_on = 0;
		net_setup ();
		allok = 0;
	    } else
		(void) close (fd);
	}
	socks_on = XmToggleButtonGetState (socks_w);
	if (socks_host)
	    XtFree (socks_host);
	socks_host = XmTextFieldGetString (socksh_w);
	if (socks_port)
	    XtFree (socks_port);
	socks_port = XmTextFieldGetString (socksp_w);
	if (socks_on) {
	    /* TODO: how to test? */
	    fd = mkconnection (socks_host, atoi(socks_port), msg);
	    if (fd < 0) {
		xe_msg (1, "%s", msg);
		socks_on = 0;
		net_setup ();
		allok = 0;
	    } else
		(void) close (fd);
	}
	auth_on = XmToggleButtonGetState (auth_w);
	if (auth_name)
	    XtFree (auth_name);
	auth_name = XmTextFieldGetString (authn_w);
	/* TODO: how to test? */

	watch_cursor (0);

	return (allok ? 0 : -1);
}

/* called from Ok */
/* ARGSUSED */
static void
ok_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (net_save() == 0)
	    XtPopdown (netshell_w);
}

/* called from Ok */
/* ARGSUSED */
static void
cancel_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/* outta here */
	XtPopdown (netshell_w);
}

/* called whenever the Password text field is edited.
 * add to auth_pw but echo as *
 */
/* ARGSUSED */
static void
pw_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmTextVerifyCallbackStruct *vp = (XmTextVerifyCallbackStruct *)call;
	int l = XmTextFieldGetLastPosition(w);
	int sp = vp->startPos;
	int ep = vp->endPos;

	if (sp < ep) {
	    /* cut sp .. ep-1 */
	    memmove (auth_pw+sp, auth_pw+ep, l-ep+1);		/* EOS too */
	    l -= ep-sp;
	}

	if (vp->text->length > 0) {
	    /* insert 1 @ sp */
	    auth_pw = XtRealloc (auth_pw, l+2);			/* new + EOS */
	    memmove (auth_pw+sp+1, auth_pw+sp, l-sp+1);		/* EOS too */
	    auth_pw[sp] = vp->text->ptr[0];

	    /* echo as '*' */
	    vp->text->ptr[0] = '*';
	}
}

/* called from any of the choice toggle buttons.
 * client is one of the TB enums to tell us which.
 */
/* ARGSUSED */
static void
tb_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (XmToggleButtonGetState(w)) {
	    switch ((long int)client) {
	    case NETDIRTB:
		/* turn off proxy, socks and auth */
		XmToggleButtonSetState (proxy_w, False, False);
		XmToggleButtonSetState (socks_w, False, False);
		XmToggleButtonSetState (auth_w, False, False);
		break;
	    case NETPROXTB:
		/* turn off direct and socks */
		XmToggleButtonSetState (ndir_w, False, False);
		XmToggleButtonSetState (socks_w, False, False);
		break;
	    case NETSOCKSTB:
		/* turn off direct and proxy */
		XmToggleButtonSetState (ndir_w, False, False);
		XmToggleButtonSetState (proxy_w, False, False);
		break;
	    default:
		printf ("FS: bad client: %d\n", (int)(long int)client);
		abort();
	    }
	}
}

/* called from Ok */
/* ARGSUSED */
static void
help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
        static char *msg[] = {"Set up network connectivity options."};

	hlp_dialog ("NetSetup", msg, sizeof(msg)/sizeof(msg[0]));

}

/* wait at most maxt secs for the ability to read/write using fd and allow X
 *   processing in the mean time.
 * w is 0 is for reading, 1 for writing, 2 for either.
 * return 0 if ok to proceed, else -1 if trouble or timeout.
 */
static int
tout (maxt, fd, w)
int maxt;
int fd;
int w;
{
	int i;
	    
	for (i = 0; stopd_check() == 0 && i < maxt; i++) {
	    fd_set rset, wset;
	    struct timeval tv;
	    int ret;

	    FD_ZERO (&rset);
	    FD_ZERO (&wset);
	    switch (w) {
	    case 0:
	    	FD_SET (fd, &rset);
		break;
	    case 1:
	    	FD_SET (fd, &wset);
		break;
	    case 2:
	    	FD_SET (fd, &rset);
	    	FD_SET (fd, &wset);
		break;
	    default:
		printf ("Bug: tout() called with %d\n", w);
		abort();
	    }

	    tv.tv_sec = 1;
	    tv.tv_usec = 0;

	    ret = select (fd+1, &rset, &wset, NULL, &tv);
	    if (ret > 0)
		return (0);
	    if (ret < 0)
		return (-1);
	}

	errno = i == maxt ? ETIMEDOUT : EINTR;
	return (-1);
}

/* a networking error has occured. if we can dig out more details about why
 * using h_errno, return its message, otherwise just return errmsg unchanged.
 * we do this because we don't know how portable is h_errno?
 */
static char *
herr (errmsg)
char *errmsg;
{
#if defined(HOST_NOT_FOUND) && defined(TRY_AGAIN)
	switch (h_errno) {
	case HOST_NOT_FOUND:
	    errmsg = "Host Not Found";
	    break;
	case TRY_AGAIN:
	    errmsg = "Might be a temporary condition -- try again later";
	    break;
	}
#endif
	return (errmsg);
}

/* just like connect(2) but tries to time out after TOUT yet let X continue.
 * return 0 if ok, else -1.
 */
static int
connect_to (sockfd, serv_addr, addrlen)
int sockfd;
struct sockaddr *serv_addr;
int addrlen;
{
#ifdef O_NONBLOCK               /* _POSIX_SOURCE */
#define NOBLOCK O_NONBLOCK
#else
#define NOBLOCK O_NDELAY
#endif
	unsigned int len;
	int err;
	int flags;
	int ret;

	/* set socket non-blocking */
	flags = fcntl (sockfd, F_GETFL, 0);
	(void) fcntl (sockfd, F_SETFL, flags | NOBLOCK);

	/* start the connect */
	ret = connect (sockfd, serv_addr, addrlen);
	if (ret < 0 && errno != EINPROGRESS)
	    return (-1);

	/* wait for sockfd to become useable */
	ret = tout (TOUT, sockfd, 2);
	if (ret < 0)
	    return (-1);

	/* verify connection really completed */
	len = sizeof(err);
	err = 0;
	ret = getsockopt (sockfd, SOL_SOCKET, SO_ERROR, (char *) &err, &len);
	if (ret < 0)
	    return (-1);
	if (err != 0) {
	    errno = err;
	    return (-1);
	}

	/* looks good - restore blocking */
	(void) fcntl (sockfd, F_SETFL, flags);
	return (0);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: netmenu.c,v $ $Date: 2010/10/06 21:12:16 $ $Revision: 1.30 $ $Name:  $"};
