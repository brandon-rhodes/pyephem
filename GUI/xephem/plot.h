/* glue between plotmenu and plot_aux
 */


#define MAXPLTLINES     40 	/* maximum unique functions (ie tags) */
#define MAXTAG          8	/* longest tag allowed */

/* one of these gets malloced and passed to the drawing area expose callback via
 * its client parameter. be sure to free it when the parent FormDialog goes
 * away too.
 * by doing this, we can have lots of different plots up at once and yet we
 * don't have to keep track of them - they clean up after themselves.
 */
typedef struct {
    char *filename;	/* name of file being plotted (also malloced) */
    FILE *fp;		/* FILE pointer for the file */
    int flipx, flipy;	/* flip state for this instance */
    Widget fx_w, fy_w;	/* widgets for sr_unreg() */
    int grid;		/* whether to include a grid */
    int xjd_asdate;	/* whether to show x axis JDs as dates */
    int xyr_asdate;	/* whether to show x axis years as dates */
    Widget yas_w, jas_w;/* show as dates options */
    Widget g_w;		/* widgets for sr_unreg() */

    /* info needed to allow plotting mouse location */
    double data_minx, data_maxx, data_miny, data_maxy;
    int win_minx, win_maxx, win_miny, win_maxy;
} DrawInfo;

/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: plot.h,v $ $Date: 2012/12/30 17:01:02 $ $Revision: 1.2 $ $Name:  $
 */
