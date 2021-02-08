/* this file implements two handy fifo interfaces to sky view.
 * 1. Inbound: A marker may be placed on the Sky View by writing a text string
 *   to the fifo named fifos/xephem_in_fifo. The format is "RA:X Dec:Y",
 *   where X and Y are in radians at epoch 2000.0. This fifo is opened and
 *   closed each time the Sky View window is opened and closed.
 * 2. Outbound: If a process has the fifo fifos/xephem_loc_fifo open for reading
 *   then sky view popup menu will offer a button to send the cursor postion to
 *   this fifo. The format is the same as a line in .edb format. This fifo is
 *   attempted opened each time the popup is activated, and remains open until
 *   writing to it fails.
 * Both fifos must be in the ShareDir directory.
 * We also implement a common query and command interface to this facility and
 *   INDI.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <Xm/Xm.h>

#include "xephem.h"

static int sfifo_outready(void);
static void sfifo_sendout (Obj *op);
static void ififo_cb (XtPointer client, int *fdp, XtInputId *idp);

/* fifo names, with respect to ShareDir  */
static char iname[] = "fifos/xephem_in_fifo";	/* inbound name */
static char oname[] = "fifos/xephem_loc_fifo";	/* outbound name */

/* inbound fifo fd and XtAddInput id */
static int ififo_fd = -1;
static XtInputId ififo_id;

/* outbound fifo fd */
static int ofifo_fd = -1;

/* return whether either our fifos or INDI is currently accepting commands */
int
telIsOn()
{
	return (sc_isGotoOn() || !sfifo_outready());
}

/* send object to our fifos and INDI, ok if either not open */
void
telGoto (Obj *op)
{
	/* check horizons */
	if (op->s_alt < 0) {
	    xe_msg (1, "%s is below the horizon", op->o_name);
	    return;
	} else if (!sv_hznOpOk(op)) {
	    xe_msg (1, "%s is below Sky View user horizon map", op->o_name);
	    return;
	}

	/* ok */
	sfifo_sendout (op);
	(void) sc_goto (op);
}

/* return 0 if the outbound fifo is ready, else -1.
 * if successful, leave open. harmless if already open.
 */
static int
sfifo_outready()
{
	char fn[1024];

	/* success if already open */
	if (ofifo_fd >= 0)
	    return(0);

	/* open for write, non-blocking. only succeeds if a reader is active */
	sprintf (fn, "%s/%s", getPrivateDir(), oname);
	ofifo_fd = open (fn, O_WRONLY|O_NONBLOCK);
	if (ofifo_fd < 0) {
	    sprintf (fn, "%s/%s", getShareDir(), oname);
	    ofifo_fd = open (fn, O_WRONLY|O_NONBLOCK);
	    if (ofifo_fd < 0)
		return (-1);
	}
	return (0);
}

/* send op to ofifo, must be open already. close if trouble. */
static void
sfifo_sendout (Obj *op)
{
	char buf[256];

	/* must already be open */
	if (ofifo_fd < 0)
	    return;

	/* format and write */
	db_write_line (op, buf);
	strcat (buf, "\n");
	if (write (ofifo_fd, buf, strlen(buf)) < 0) {
	    close (ofifo_fd);
	    ofifo_fd = -1;
	}
}

/* connect to the inbound fifo. arrange for a marker to be drawn on skyview if
 * a legal string is ever received. harmless to call if already open.
 */
void
sfifo_openin()
{
	char fn[1024];

	/* do nothing if already open */
	if (ififo_fd >= 0)
	    return;

	/* open r/w so we never block on the open but do block on reads */
	sprintf (fn, "%s/%s", getPrivateDir(), iname);
	ififo_fd = open (fn, O_RDWR);
	if (ififo_fd < 0) {
	    sprintf (fn, "%s/%s", getShareDir(), iname);
	    ififo_fd = open (fn, O_RDWR);
	    if (ififo_fd < 0)
		return;
	}

	/* connect a callback to monitor */
	ififo_id = XtAppAddInput(xe_app, ififo_fd, (XtPointer)XtInputReadMask,
							    ififo_cb, NULL);
}

/* close the inbound fifo, if open */
void
sfifo_closein()
{
	if (ififo_fd >= 0) {
	    close (ififo_id);
	    ififo_fd = -1;
	    XtRemoveInput (ififo_id);
	}
}

/* called when reading from ififo will not block */
static void
ififo_cb (XtPointer client, int *fdp, XtInputId *idp)
{
	char buf[64];
	double ra, dec;
	double alt, az;
	int nr;

	/* read */
	nr = read (ififo_fd, buf, sizeof(buf));
	if (nr <= 0) {
	    sfifo_closein();
	    return;
	}
	buf[nr] = '\0';

	/* parse and mark */
	if (sscanf (buf, "RA:%lf Dec:%lf", &ra, &dec) == 2) {
	    Now *np = mm_get_now();
	    Obj o;

	    memset (&o, 0, sizeof(o));
	    o.o_type = FIXED;
	    o.f_RA = ra;
	    o.f_dec = dec;
	    o.f_epoch = J2000;
	    obj_cir (np, &o);
	    sv_scopeMark (&o);
	} else if (sscanf (buf, "Alt:%lf Az:%lf", &alt, &az) == 2) {
	    Now *np = mm_get_now();
	    Obj o;

	    sv_other (alt, az, 1, &dec, &ra);
	    memset (&o, 0, sizeof(o));
	    o.o_type = FIXED;
	    o.f_RA = ra;
	    o.f_dec = dec;
	    o.f_epoch = epoch == EOD ? mjd : epoch;
	    obj_cir (np, &o);
	    sv_scopeMark (&o);
	}
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: skyfifos.c,v $ $Date: 2011/09/23 01:52:39 $ $Revision: 1.8 $ $Name:  $"};
