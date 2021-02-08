/* functions which connect to many other functions */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>

#include "xephem.h"


/* called to set or unset the watch cursor on all menus.
 * allow for nested requests.
 */
void
watch_cursor(want)
int want;
{
	static Cursor wc;
	static int nreqs;
	Cursor c;

	if (!wc)
	    wc = XCreateFontCursor (XtD, XC_watch);

	if (want) {
	    if (nreqs++ > 0)
		return;
	    c = wc;
	} else {
	    if (--nreqs > 0)
		return;
	    c = (Cursor)0;
	}

	XPS_cursor(c);
	av_cursor(c);
	ano_cursor(c);
	c_cursor(c);
	cc_cursor(c);
	db_cursor(c);
	dm_cursor(c);
	e_cursor(c);
	fav_cursor(c);
	fs_cursor(c);
	gal_cursor(c);
	sun_cursor(c);
	hzn_cursor(c);
	jm_cursor(c);
	lst_cursor(c);
	m_cursor(c);
	main_cursor(c);
	mars_cursor(c);
	marsm_cursor(c);
	ml_cursor(c);
	msg_cursor(c);
	ng_cursor(c);
	obj_cursor(c);
	ol_cursor(c);
	plt_cursor(c);
	pm_cursor(c);
	sc_cursor(c);
	se_cursor(c);
	sf_cursor(c);
	si_cursor(c);
	sl_cursor(c);
	sm_cursor(c);
	sr_cursor(c);
	srch_cursor(c);
	ss_cursor(c);
	sv_cursor(c);
	svf_cursor(c);
	svh_cursor(c);
	um_cursor(c);
	v_cursor(c);
	wdb_cursor(c);
 
	XFlush (XtD);
	XmUpdateDisplay (toplevel_w);
}

/* update stuff on all major views because time has changed */
void
all_update(np, how_much)
Now *np;
int how_much;
{
	watch_cursor (1);

	dm_update (np, how_much);
	mars_update (np, how_much);
	marsm_update (np, how_much);
	ng_update (np, how_much);
	e_update (np, how_much);
	jm_update (np, how_much);
	sm_update (np, how_much);
	um_update (np, how_much);
	ss_update (np, how_much);
	sun_update (np, how_much);
	sv_update (np, how_much);
	m_update  (np, how_much);
	c_update (np, how_much);
	cc_update (np, how_much);

	watch_cursor (0);
}

/* tell everyone who might care that the favorites list has changed.
 */
void
all_newfavs()
{
	/* special prep */
	dm_newfavs();
	e_newfavs();
	ng_newfavs();

	/* otherwise just like changing the db */
	all_newdb (0);
}

/* tell everyone who might care that the db beyond the builtin objs has changed.
 * appended is true if it grew; else it was deleted.
 */
void
all_newdb(appended)
int appended;
{
	watch_cursor (1);

	/* fav must be first because some other modules may recheck favs
	 * list as they respond to db changes
	 */
	fav_newdb();

	obj_newdb(appended);
	sm_newdb(appended);
	jm_newdb(appended);
	ss_newdb(appended);
	um_newdb(appended);
	marsm_newdb(appended);
	sv_newdb(appended);
	db_newdb (appended);
	m_newdb (appended);
	e_newdb (appended);

	watch_cursor (0);
}

/* inform all menus that have something selectable for plotting/listing/srching
 * wether we are now in a mode that they should report when those fields are
 * selected.
 */
void
all_selection_mode(whether)
int whether;
{
	dm_selection_mode(whether);
	mm_selection_mode(whether);
	jm_selection_mode(whether);
	mars_selection_mode(whether);
	e_selection_mode(whether);
	sm_selection_mode(whether);
	um_selection_mode(whether);
	marsm_selection_mode(whether);
	srch_selection_mode(whether);
	m_selection_mode(whether);
}

/* inform all potentially interested parties of the name of a field that
 * it might want to use for latter.
 * this is just to collect in one place all the modules that gather care.
 */
void
register_selection (name)
char *name;
{
	plt_selection (name);
	lst_selection (name);
	srch_selection (name);
}

/* if we are plotting/listing/searching, send the current field info to them.
 * N.B. only send `value' to plot and search if logv is not 0.
 */
void
field_log (w, value, logv, str)
Widget w;
double value;
int logv;
char *str;
{
	char *name;

	if (!any_ison())
	    return;

	get_something (w, XmNuserData, (XtArgVal)&name);
	if (name) {
	    if (logv) {
		plt_log (name, value);
		srch_log (name, value);
	    }
	    lst_log (name, str);
	}
}

/* return !0 if any of the button/data capture tools are active, else 0.
 */
int
any_ison()
{
	return (srch_ison() || plot_ison() || listing_ison());
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: broadcast.c,v $ $Date: 2007/07/09 18:22:45 $ $Revision: 1.31 $ $Name:  $"};
