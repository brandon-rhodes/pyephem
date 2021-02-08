/* code to support a set of functions which are called like X functions but
 * which may in fact be creating postscript.
 */

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>

#if defined(__NUTC__)
#include <winnutc.h>
#endif

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Separator.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

#include "xephem.h"

typedef enum {
    CLOSED=0,	/* not in use */
    ASKING,	/* in process of asking user where to save */
    OPEN, 	/* FILE * is open and ready for writing */
    XDRAWING	/* xforms set up and ready for X coords */
} XPSState;

typedef struct {
    XPSState state;		/* what's happ'nin' */
    char *lcnum;		/* LC_NUMERIC locale stack, while we use "C" */
    void (*go)();		/* user's function to call when ok to proceed */
    FILE *fp;			/* file accumulating PS commands */

    char *prfile;		/* malloced name of lpr temp file */
    int wantpr;			/* set to print (as opposed to save) */
    int wantcolor;		/* set to print in color, else B&W */
    int wantA4;			/* set to print A4, else letter size*/
    int wantThick;		/* set to print thick lines, else thin */
    int overimg;		/* hint to tweak colors over an image */
    int black;			/* if !wantcolor, whether to force black ovlay*/
    int papercolor;		/* "paper" color when in black mode */
    int papercolorset;		/* papercolor is active */

    Window win;			/* window we are capturing */
    double scale;		/* PS's points / X's pixels */
} XPSContext;

typedef struct {
    Font fid;			/* Font ID */
    char *psname;		/* malloced name of PS font to use */
    int pixsz;			/* height of font, in points */
} PSFontInfo;

/* here is the implied state behind most of the XPS* functions */
static XPSContext xpsc;

/* gray from color, ala Soc of Motion Picture and TV Engineers */
#define	SMPTE(xc)	((unsigned)(.33*xc.red + .5*xc.green + .17*xc.blue))

/* cache of info about associating a Font with its Postscript counterpart */
static PSFontInfo *psfi;	/* malloced array info about registered fonts */
static int npsfi;		/* number of entries malloced in fi */

static char printcategory[] = "Print";	/* Save category */

/* handy reference to xpsc.fp */
#define	FP	xpsc.fp 

static Widget print_w;			/* overall print setup shell */
static Widget color_w;			/* TB set when want to print in color */
static Widget A4_w;			/* TB set A4 size */
static Widget thicklw_w;		/* TB set thick lines */
static Widget prtb_w;			/* TB set when want to print */
static Widget filename_w;		/* text field holding file name */
static Widget prcmd_w;			/* text field holding print command */
static Widget title_w;			/* text field holding title */

static char deffont[] = "Helvetica";	/* default font */

static int XPSOpen (char *fn);
static void setLineStyle (Display *dsp, GC gc);
static void setColor (Display *dsp, GC gc);
static void doEllipse (int fill, Display *dsp, Drawable win, GC gc,
    int x, int y, int a0, unsigned w, unsigned h, int a1, int a2);
static void doArc (XArc *xap, int fill);
static void doPoly (XPoint xp[], int nxp, int mode, int fill);
static void doRect (int x, int y, int w, int h, int fill);
static void doSegment (int x1, int y1, int x2, int y2);
static PSFontInfo * find_psfont (Font fid);
static void checkState (char *funcname, XPSState s);
static void printTime (int x, int y);

static void create_print_w (void);
static void ok_cb (Widget w, XtPointer client, XtPointer call);
static void cancel_cb (Widget w, XtPointer client, XtPointer call);
static void help_cb (Widget w, XtPointer client, XtPointer call);
static void toggle_cb (Widget w, XtPointer client, XtPointer call);
static void saveas_confirm (void);
static void xpsc_close (void);
static void no_go (void);
static void call_go (void);
static void x_fill_circle (Display *dsp, Drawable win, GC gc, int x, int y,
    int diam);
static void x_drawstar (Display *dsp, Drawable win, GC gc, int x, int y, int d);
static Pixel haloCache (Display *dsp, Pixel p);

#define MYMIN(a,b)      ((a) < (b) ? (a) : (b))
#define MYMAX(a,b)      ((a) > (b) ? (a) : (b))

/* ask user to save or print. if ok, call his go() after we've called XPSOpen()
 */
void
XPSAsk (apname, go)
char *apname;
void (*go)();
{
	char buf[1024];

	if (xpsc.state != CLOSED) {
	    xe_msg (1, "Only one print/save request may be active at a time.");
	    return;
	}

	/* set up state */
	if (!go) {
	    printf ("XPSAsk: !go\n");
	    abort();
	}
	memset (&xpsc, 0, sizeof(xpsc));
	xpsc.go = go;
	xpsc.state = ASKING;
	xpsc.lcnum = setlocale (LC_NUMERIC, "C");

	/* bring up print dialog -- call go() if user says Ok */
	if (!print_w)
	    create_print_w();
	(void) sprintf (buf, "xephem %s Print", apname);
	set_something (print_w, XmNtitle, (XtArgVal)buf);

	XtPopup (print_w, XtGrabNone);
	set_something (print_w, XmNiconic, (XtArgVal)False);
}

/* return 1 if currently drawing in color, else 0 */
int
XPSInColor()
{
	return (xpsc.state == XDRAWING && xpsc.wantcolor);
}

/* return 1 if currently drawing, else 0 */
int
XPSDrawing()
{
	return (xpsc.state == XDRAWING);
}

/* set a color which "draws" as no ink.
 * not the same as not drawing, since this will white-out stuff.
 */
void
XPSPaperColor (p)
unsigned long p;
{
    	xpsc.papercolorset = 1;
    	xpsc.papercolor = p;
}

/* given a string to draw in postscript, look it over for characters to be
 * escaped. Return a malloced copy with any and all such characters escaped.
 * N.B. caller should make a copy of the returned sttring before calling again.
 */
char *
XPSCleanStr (s, l)
char *s;
int l;
{
	static char *lasts;
	int i, o;

	lasts = XtRealloc (lasts, l*2 + 1); /* worst case is each char escped */

	for (i = o = 0; i < l; i++) {
	    char c = s[i];
	    if (c == '(' || c == ')' || c == '\\')
		lasts[o++] = '\\';
	    lasts[o++] = c;
	}
	lasts[o] = '\0';

	return (lasts);
}

/* called to put up or remove the watch cursor.  */
void
XPS_cursor (c)
Cursor c;
{
	Window win;

	if (print_w && (win = XtWindow(print_w)) != 0) {
	    Display *dsp = XtDisplay(print_w);
	    if (c)
		XDefineCursor (dsp, win, c);
	    else
		XUndefineCursor (dsp, win);
	}
}

/* set up to convert subsequent X drawing calls to the given X window
 * into Postscript and send to xpsc.fp. [Xx0,Yy0] is the X coordinates of the
 * upper left corner of a window Xw pixels wide to be mapped to a rectangle Pw
 * points wide whose upper left corner is at [Px0,Py0], also specified in
 * points. This allows any X rectangle to me mapped into any Postscript
 * rectangle, scaled but with the same aspect ratio.
 * we also define some handy functions.
 */
void
XPSXBegin (win, Xx0, Xy0, Xw, Xh, Px0, Py0, Pw)
Window win;	/* what window we are capturing */
int Xx0, Xy0;	/* ul origin of X rectangle */
int Xw;		/* width of X rectangle -- clipped if Xh > 0*/
int Xh;		/* height of X rectangle -- > 0 means to clip */
int Px0, Py0;	/* ul origin of Page rectangle */
int Pw;		/* width of Page rectangle */
{
	checkState ("Begin", OPEN);

	/* remember target */
	xpsc.win = win;

	/* Postscript */
	fprintf (FP, "%%!PS-Adobe\n\n");

	/* define function to set solid or nominal dashed line.
	 * By: C. P. Price <price@sedona.uafcns.alaska.edu>
	 */
	fprintf (FP, "%% Define solid and dashed\n");
	fprintf (FP, "/dashed { [4 6] 0 setdash } def\n");
	fprintf (FP, "/solid { [] 0 setdash } def\n");
	fprintf (FP, "\n");

	/* define function Spline to draw an array of points as splint */
	fprintf (FP, "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
	fprintf (FP, "%% Spline to Bezier curve\n");
	fprintf (FP, "%%   [x0 y0 x1 y1 x2 y2 ... ] Spline\n");
	fprintf (FP, "%% Chris Beecroft (chrisb@netcom.com)\n");
	fprintf (FP, "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
	fprintf (FP, "\n");
	fprintf (FP, "/SplineDict 7 dict def\n");
	fprintf (FP, "\n");
	fprintf (FP, "SplineDict begin\n");
	fprintf (FP, "\n");
	fprintf (FP, "/CPointDict 8 dict def\n");
	fprintf (FP, "/ComputeCPDict 21 dict def\n");
	fprintf (FP, "\n");
	fprintf (FP, "%% Some constants\n");
	fprintf (FP, "/Tension 0.65 def  %% 0 (loose) to 1 (tight)\n");
	fprintf (FP, "/SQR2 2 sqrt def\n");
	fprintf (FP, "/recipSQR2 1 SQR2 div def\n");
	fprintf (FP, "\n");
	fprintf (FP, "/CPoint    %% calculate control bezier points\n");
	fprintf (FP, "{          %% stack:  x y len1 len2 angle1 angle2 \n");
	fprintf (FP, "	   %% return: xa ya xb yb\n");
	fprintf (FP, "CPointDict begin\n");
	fprintf (FP, "    /theta2 exch def\n");
	fprintf (FP, "    /theta1 exch def\n");
	fprintf (FP, "\n");
	fprintf (FP, "    /theta theta1 theta2 add 2 div def\n");
	fprintf (FP, "\n");
	fprintf (FP, "    theta1 theta2 gt {\n");
	fprintf (FP, "	/s theta theta2 sub sin def\n");
	fprintf (FP, "	/theta1 theta 90 add def\n");
	fprintf (FP, "	/theta2 theta 90 sub def\n");
	fprintf (FP, "    } {\n");
	fprintf (FP, "	/s theta2 theta sub sin def\n");
	fprintf (FP, "	/theta1 theta 90 sub def\n");
	fprintf (FP, "	/theta2 theta 90 add def\n");
	fprintf (FP, "    } ifelse\n");
	fprintf (FP, "\n");
	fprintf (FP, "    s recipSQR2 gt {\n");
	fprintf (FP, "	/s SQR2 s sub def\n");
	fprintf (FP, "    } if\n");
	fprintf (FP, "\n");
	fprintf (FP, "    /s s 1 Tension sub mul def\n");
	fprintf (FP, "\n");
	fprintf (FP, "    %% stack: x y len1 len2\n");
	fprintf (FP, "\n");
	fprintf (FP, "    /len2 exch s mul def  %% len2 = len2 * s\n");
	fprintf (FP, "    /len1 exch s mul def  %% len1 = len1 * s\n");
	fprintf (FP, "\n");
	fprintf (FP, "    /y exch def\n");
	fprintf (FP, "    /x exch def\n");
	fprintf (FP, "\n");
	fprintf (FP, "  x len1 theta1 cos mul add %% x + len1 * cos(theta1)\n");
	fprintf (FP, "  y len1 theta1 sin mul sub %% y - len1 * sin(theta1)\n");
	fprintf (FP, "  x len2 theta2 cos mul add %% x + len2 * cos(theta2)\n");
	fprintf (FP, "  y len2 theta2 sin mul sub %% y - len2 * sin(theta2)\n");
	fprintf (FP, "end\n");
	fprintf (FP, "} def\n");
	fprintf (FP, "\n");
	fprintf (FP, "\n");
	fprintf (FP, "/ComputeCP %% spline through points\n");
	fprintf (FP, "{          %% stack:  [ x0 y0 x1 y1 x2 y2 .... ]\n");
	fprintf (FP, "	   %% return: nothing  %% spline path added to current path\n");
	fprintf (FP, "ComputeCPDict begin\n");
	fprintf (FP, "    /SplinePoints exch def\n");
	fprintf (FP, "    /elems SplinePoints length def\n");
	fprintf (FP, "\n");
	fprintf (FP, "    /x1 SplinePoints 0 get def\n");
	fprintf (FP, "    /y1 SplinePoints 1 get def\n");
	fprintf (FP, "    /x2 SplinePoints 2 get def\n");
	fprintf (FP, "    /y2 SplinePoints 3 get def\n");
	fprintf (FP, "    /x3 SplinePoints 4 get def\n");
	fprintf (FP, "    /y3 SplinePoints 5 get def\n");
	fprintf (FP, "\n");
	fprintf (FP, "    /dx x1 x2 sub def\n");
	fprintf (FP, "    /dy y2 y1 sub def\n");
	fprintf (FP, "    /len1 dx dx mul dy dy mul add sqrt def\n");
	fprintf (FP, "    len1 0.0 eq {\n");
	fprintf (FP, "	/theta1 0.0 def\n");
	fprintf (FP, "    } {\n");
	fprintf (FP, "	/theta1 dy dx atan def\n");
	fprintf (FP, "	theta1 0 lt {\n");
	fprintf (FP, "	    /theta1 theta1 360 add def\n");
	fprintf (FP, "	} if\n");
	fprintf (FP, "    } ifelse\n");
	fprintf (FP, "\n");
	fprintf (FP, "    /dx x3 x2 sub def\n");
	fprintf (FP, "    /dy y2 y3 sub def\n");
	fprintf (FP, "    /len2 dx dx mul dy dy mul add sqrt def\n");
	fprintf (FP, "    len2 0.0 eq {\n");
	fprintf (FP, "	/theta2 0.0 def\n");
	fprintf (FP, "    } {\n");
	fprintf (FP, "	/theta2 dy dx atan def\n");
	fprintf (FP, "	theta2 0 lt {\n");
	fprintf (FP, "	    /theta2 theta2 360 add def\n");
	fprintf (FP, "	} if\n");
	fprintf (FP, "    } ifelse\n");
	fprintf (FP, "\n");
	fprintf (FP, "    %% compute and store the left and right control points\n");
	fprintf (FP, "    x2 y2 len1 len2 theta1 theta2 CPoint\n");
	fprintf (FP, "    /yright exch def\n");
	fprintf (FP, "    /xright exch def\n");
	fprintf (FP, "    /yleft  exch def\n");
	fprintf (FP, "    /xleft  exch def\n");
	fprintf (FP, "\n");
	fprintf (FP, "    x1 y1 moveto\n");
	fprintf (FP, "\n");
	fprintf (FP, "    x1 3 xleft mul add 4 div\n");
	fprintf (FP, "    y1 3 yleft mul add 4 div\n");
	fprintf (FP, "    xleft 3 mul x2 add 4 div\n");
	fprintf (FP, "    yleft 3 mul y2 add 4 div\n");
	fprintf (FP, "    x2 y2 curveto\n");
	fprintf (FP, "\n");
	fprintf (FP, "    /xsave xright def\n");
	fprintf (FP, "    /ysave yright def\n");
	fprintf (FP, "    \n");
	fprintf (FP, "    6 2 elems 1 sub {\n");
	fprintf (FP, "	/index exch def\n");
	fprintf (FP, "\n");
	fprintf (FP, "	/x2 x3 def\n");
	fprintf (FP, "	/y2 y3 def\n");
	fprintf (FP, "	/len1 len2 def\n");
	fprintf (FP, "\n");
	fprintf (FP, "	theta2 180 ge {\n");
	fprintf (FP, "	    /theta1 theta2 180 sub def\n");
	fprintf (FP, "	} {\n");
	fprintf (FP, "	    /theta1 theta2 180 add def\n");
	fprintf (FP, "	} ifelse\n");
	fprintf (FP, "\n");
	fprintf (FP, "	/x3 SplinePoints index get def\n");
	fprintf (FP, "	/y3 SplinePoints index 1 add get def\n");
	fprintf (FP, "\n");
	fprintf (FP, "	/dx x3 x2 sub def\n");
	fprintf (FP, "	/dy y2 y3 sub def\n");
	fprintf (FP, "	/len2 dx dx mul dy dy mul add sqrt def\n");
	fprintf (FP, "	len2 0.0 eq {\n");
	fprintf (FP, "	    /theta2 0.0 def\n");
	fprintf (FP, "	} {\n");
	fprintf (FP, "	    /theta2 dy dx atan def\n");
	fprintf (FP, "	    theta2 0 lt {\n");
	fprintf (FP, "		/theta2 theta2 360 add def\n");
	fprintf (FP, "	    } if\n");
	fprintf (FP, "	} ifelse\n");
	fprintf (FP, "\n");
	fprintf (FP, "	x2 y2 len1 len2 theta1 theta2 CPoint\n");
	fprintf (FP, "	/yright exch def\n");
	fprintf (FP, "	/xright exch def\n");
	fprintf (FP, "	/yleft  exch def\n");
	fprintf (FP, "	/xleft  exch def\n");
	fprintf (FP, "\n");
	fprintf (FP, "	xsave ysave xleft yleft x2 y2 curveto\n");
	fprintf (FP, "	/xsave xright def\n");
	fprintf (FP, "	/ysave yright def\n");
	fprintf (FP, "    } for\n");
	fprintf (FP, "\n");
	fprintf (FP, "    xright 3 mul x2 add 4 div\n");
	fprintf (FP, "    yright 3 mul y2 add 4 div\n");
	fprintf (FP, "    xright 3 mul x3 add 4 div\n");
	fprintf (FP, "    yright 3 mul y3 add 4 div\n");
	fprintf (FP, "    x3 y3 curveto\n");
	fprintf (FP, "end\n");
	fprintf (FP, "} def\n");
	fprintf (FP, "\n");
	fprintf (FP, "end\n");
	fprintf (FP, "\n");
	fprintf (FP, "\n");
	fprintf (FP, "/Spline\n");
	fprintf (FP, "{\n");
	fprintf (FP, "    SplineDict begin ComputeCP end\n");
	fprintf (FP, "} def\n");

	/* function to draw rotated ellipes -- thanks Chris! */
	fprintf (FP, "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
	fprintf (FP, "%% General Rotated ellipses.\n");
	fprintf (FP, "%%   x y a0 xrad yrad a1 a2 ellipse\n");
	fprintf (FP, "%%   N.B. xrad and yrad must be != 0 \n");
	fprintf (FP, "%% Chris Beecroft (chrisb@netcom.com)\n");
	fprintf (FP, "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
	fprintf (FP, "/ellipsedict 9 dict def\n");
	fprintf (FP, "ellipsedict /mtrx matrix put\n");
	fprintf (FP, "/ellipse\n");
	fprintf (FP, "    { ellipsedict begin\n");
	fprintf (FP, "      /endangle exch def\n");
	fprintf (FP, "      /startangle exch def\n");
	fprintf (FP, "      /yrad exch def\n");
	fprintf (FP, "      /xrad exch def\n");
	fprintf (FP, "      /rotation exch def\n");
	fprintf (FP, "      /y exch def\n");
	fprintf (FP, "      /x exch def\n");
	fprintf (FP, "\n");
	fprintf (FP, "      /savematrix mtrx currentmatrix def\n");
	fprintf (FP, "      x y translate\n");
	fprintf (FP, "      rotation rotate\n");
	fprintf (FP, "      xrad yrad scale\n");
	fprintf (FP, "\n");
	fprintf (FP, "      %% convert angle/radius (radius=1) to x,y\n");
	fprintf (FP, "      %% divide in x,y scaling and convert back to polar.\n");
	fprintf (FP, "      %% subtract a bit off endangle to force 360 sin\n");
	fprintf (FP, "      %% to be a small negative number so atan results in\n");
	fprintf (FP, "      %% a degree of 360 rather than 0. If startangle is 0\n");
	fprintf (FP, "      %% then a surprise appears and no ellipse is drawn.\n");
	fprintf (FP, "\n");
	fprintf (FP, "      /endangle endangle 0.001 sub sin yrad div endangle cos xrad div atan def\n");
	fprintf (FP, "      /startangle startangle sin yrad div  startangle cos xrad div  atan def\n");
	fprintf (FP, "\n");
	fprintf (FP, "      0 0 1 startangle endangle arc\n");
	fprintf (FP, "      savematrix setmatrix\n");
	fprintf (FP, "      end\n");
	fprintf (FP, "    } def\n");

	/* define function lstr to place and show a l-justifed string */
	fprintf (FP, "%% Define l-justified string: (..) x y lstr\n");
	fprintf (FP, "/lstr { newpath moveto show } def\n");
	fprintf (FP, "\n");

	/* define function rstr to place and show a r-justifed string */
	fprintf (FP, "%% Define r-justified string: (..) x y rstr\n");
	fprintf (FP, "/rstr { newpath moveto\n");
	fprintf (FP, "  dup         %% make copies for stringwidth and show\n");
	fprintf (FP, "  stringwidth %% yields X and Y size\n");
	fprintf (FP, "  pop         %% don't need Y\n");
	fprintf (FP, "  neg 0 rmoveto %% move back\n");
	fprintf (FP, "  show        %% show the string\n");
	fprintf (FP, "} def\n");
	fprintf (FP, "\n");

	/* define function cstr to place and show a centered string */
	fprintf (FP, "%% Define centered string: (..) x y cstr\n");
	fprintf (FP, "/cstr { newpath moveto\n");
	fprintf (FP, "  dup         %% make copies for stringwidth and show\n");
	fprintf (FP, "  stringwidth %% yields X and Y size\n");
	fprintf (FP, "  pop         %% don't need Y\n");
	fprintf (FP, "  2 div neg 0 rmoveto %% move back halfway\n");
	fprintf (FP, "  show        %% show the string\n");
	fprintf (FP, "} def\n");
	fprintf (FP, "\n");

	/* define function strsz to return width and height of a string */
	fprintf (FP, "%% Define string size: (..) strsz w h\n");
	fprintf (FP, "/strsz {\n");
	fprintf (FP, "  gsave\n");
	fprintf (FP, "    newpath 0 0 moveto true charpath pathbbox\n");
	fprintf (FP, "    4 2 roll pop pop\n");
	fprintf (FP, "  grestore\n");
	fprintf (FP, "} def\n");
	fprintf (FP, "\n");

	/* all the users of this module assume they are drawing on 8.5x11"
	 * letter size paper with 1" margins all around. By setting an
	 * initial transformation matrix, we can change to anything desired,
	 * such as A4 paper with 15mm margins. Along the way I decided to
	 * change the letter margins to .75". Easy. I love postscript.
	 */
	if (xpsc.wantA4) {
	    fprintf (FP, "%% change 8.5x11@1\" margin to A4@15mm margin\n");
	    fprintf (FP, "306 396 translate\n");
	    fprintf (FP, "1.0902 1.0902 scale\n");
	    fprintf (FP, "-314 -375 translate\n");
	} else {
	    fprintf (FP, "%% change vertical margin from 1 to 3/4\"\n");
	    fprintf (FP, "306 396 translate\n");
	    fprintf (FP, "1.0556 1.0556 scale\n");
	    fprintf (FP, "-306 -396 translate\n");
	}
	fprintf (FP, "\n");

	/* set up for possibly-clipped X Windows coord system, assuming
	 * 8.5x11" letter format with 1" margins
	 */
	xpsc.scale = (double)Pw/(double)Xw;
	fprintf (FP, "%% Set up to use %sX Windows coord system.\n",
						    Xh > 0 ? "clipped " : "");
	fprintf (FP,"%% [%d,%d] maps to [%d,%d], scaled by %g and Y-flipped.\n",
					    Xx0, Xy0, Px0, Py0, xpsc.scale);
	fprintf (FP, "gsave\n");
	fprintf (FP, "%g setlinewidth\n", xpsc.wantThick ? 0.5 : 0.0);
	fprintf (FP, "%g %g translate\n", Px0 - xpsc.scale*Xx0,
							Py0 + xpsc.scale*Xy0);
	fprintf (FP, "%g %g scale\n", xpsc.scale, -xpsc.scale);
	if (Xh > 0) {
	    fprintf (FP, "newpath\n");
	    fprintf (FP,
	    "  %d %d moveto %d 0 rlineto 0 %d rlineto %d 0 rlineto closepath\n",
							Xx0, Xy0, Xw, Xh, -Xw);
	    fprintf (FP, "clip\n");
	}
	fprintf (FP, "\n");

	/* enable state */
	xpsc.state = XDRAWING;

	/* init a little cache to reduce using XQueryColor every time */
	pixCache (NULL);
}

/* return the postscript coord system back to the original.
 * this prepares for additional direct entries or closing down.
 */
void
XPSXEnd()
{
	checkState ("End", XDRAWING);

	/* back to native PS coord system */
	fprintf (FP, "\n");
	fprintf (FP, "%% Restore native PS coord system.\n");
	fprintf (FP, "grestore\n");

	/* set a font to use */
	fprintf (FP, "\n");
	fprintf (FP, "%% Set up a font to use for remaining text.\n");
	fprintf (FP, "/%s findfont %d scalefont setfont\n",deffont,ANNOT_PTSZ);

	xpsc.state = OPEN;
}

/* add a raw PS string to the file.
 */
void
XPSDirect (s)
char *s;
{
	checkState ("Direct", OPEN);

	fprintf (FP, "%s", s);
}

/* finish up with any boiler plate, close the file and reset xpsc */
void
XPSClose()
{
	char *title;

	checkState ("Close", OPEN);

	/* write the title at the top, if any */
	title = XmTextFieldGetString (title_w);
	if ((int)strlen (title) > 0) {
	    fprintf (FP, "\n");
	    fprintf (FP, "%% Title\n");
	    fprintf (FP, "(%s) 306 %d cstr\n", title, 720 + ANNOT_PTSZ + 10);
	}
	XtFree (title);

	/* add some closing boiler plate */
	fprintf (FP, "\n");
	fprintf (FP, "%% Boiler plate:\n");
	fprintf (FP, "newpath 234 %d moveto 144 0 rlineto stroke\n", AROWY(4));
	fprintf (FP, "(Created by XEphem Version %s %s) 306 %d cstr\n",
					    PATCHLEVEL, PATCHDATE, AROWY(3));
	fprintf (FP, "(%s) 306 %d cstr\n", COPYRIGHT, AROWY(2));
	fprintf (FP, "(%s) 306 %d cstr\n", "http://www.xephem.com", AROWY(1));
	printTime (306, AROWY(0));

	/* file is complete */
	fprintf (FP, "\n%% All finished\nshowpage\n");
	fclose (FP);
	FP = NULL;

	/* if we are really trying to print, start the print command and
	 * remove the temp file.
	 */
	if (xpsc.wantpr) {
	    char buf[1024];
	    char *cmd;

	    cmd = XmTextFieldGetString (prcmd_w);
	    (void) sprintf (buf, "%s '%s'", cmd, xpsc.prfile);
	    XtFree (cmd);

	    xe_msg (0, "Print command:\n%s", buf);
	    if (system (buf))
		xe_msg (1, "Error running print command:\n%s", buf);

#if defined(__STDC__) || defined(VMS)
	    (void) remove (xpsc.prfile);
#else
	    (void) unlink (xpsc.prfile);
#endif
	}

	xpsc_close();
}

/* draw a rotated ellipse.
 * easy for postscript, not so for X.
 * all angles are 64ths degree, 0 is 3oclock, positive ccw (all like in X).
 */
void
XPSDrawEllipse (dsp, win, gc, x, y, a0, w, h, a1, a2)
Display *dsp;
Drawable win;
GC gc;
int x, y;	/* bounding box upper left corner */
int a0;		/* axis rotation */
unsigned w, h;	/* bounding box width, height */
int a1, a2;	/* initial angle, additional extent */
{
	doEllipse (0, dsp, win, gc, x, y, a0, w, h, a1, a2);
}

/* fill a rotated ellipse.
 * easy for postscript, not so for X.
 * all angles are 64ths degree, 0 is 3oclock, positive ccw (all like in X).
 */
void
XPSFillEllipse (dsp, win, gc, x, y, a0, w, h, a1, a2)
Display *dsp;
Drawable win;
GC gc;
int x, y;	/* bounding box upper left corner */
int a0;		/* axis rotation */
unsigned w, h;	/* bounding box width, height */
int a1, a2;	/* initial angle, additional extent */
{
	doEllipse (1, dsp, win, gc, x, y, a0, w, h, a1, a2);
}

/* the following functions always perform their associated X Windows
 * function, then might also draw to the xpsc state if it matches the win.
 */
void
XPSDrawArc (dsp, win, gc, x, y, w, h, a1, a2)
Display *dsp;
Drawable win;
GC gc;
int x, y;
unsigned w, h;
int a1, a2;
{
	XArc xa;

	XDrawArc (dsp, win, gc, x, y, w, h, a1, a2);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	xa.x = x;
	xa.y = y;
	xa.width = w;
	xa.height = h;
	xa.angle1 = a1;
	xa.angle2 = a2;
	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	doArc (&xa, 0);
}

void
XPSDrawArcs (dsp, win, gc, xa, nxa)
Display *dsp;
Drawable win;
GC gc;
XArc xa[];
int nxa;
{
	int i;

	XDrawArcs (dsp, win, gc, xa, nxa);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	for (i = 0; i < nxa; i++)
	    doArc (&xa[i], 0);
}

void
XPSDrawLine (dsp, win, gc, x1, y1, x2, y2)
Display *dsp;
Drawable win;
GC gc;
int x1, y1;
int x2, y2;
{
	XDrawLine (dsp, win, gc, x1, y1, x2, y2);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	fprintf (FP, "newpath");
	doSegment (x1, y1, x2, y2);
	fprintf (FP, "stroke\n");
}

void
XPSDrawLines (dsp, win, gc, xp, nxp, mode)
Display *dsp;
Drawable win;
GC gc;
XPoint xp[];
int nxp;
int mode;
{
	XDrawLines (dsp, win, gc, xp, nxp, mode);
	if (xpsc.state != XDRAWING || xpsc.win != win || nxp < 2)
	    return;

	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	doPoly (xp, nxp, mode, 0);
}

void
XPSDrawPoint (dsp, win, gc, x, y)
Display *dsp;
Drawable win;
GC gc;
int x, y;
{
	XArc xa;
	XDrawPoint (dsp, win, gc, x, y);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	xa.x = x;
	xa.y = y;
	xa.width = 1;
	xa.height = 1;
	xa.angle1 = 0;
	xa.angle2 = 360*64;

	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	doArc (&xa, 1);
}

void
XPSDrawPoints (dsp, win, gc, xp, nxp, mode)
Display *dsp;
Drawable win;
GC gc;
XPoint xp[];
int nxp;
int mode;
{
	int i;

	XDrawPoints (dsp, win, gc, xp, nxp, mode);
	if (xpsc.state != XDRAWING || xpsc.win != win || nxp == 0)
	    return;

	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	for (i = 0; i < nxp; i++) {
	    XArc xa;

	    xa.x = xp[i].x;
	    xa.y = xp[i].y;
	    xa.width = 1;
	    xa.height = 1;
	    xa.angle1 = 0;
	    xa.angle2 = 360*64;
	    doArc (&xa, 1);
	}
}

void
XPSDrawRectangle (dsp, win, gc, x, y, w, h)
Display *dsp;
Drawable win;
GC gc;
int x, y;
unsigned w, h;
{
	XDrawRectangle (dsp, win, gc, x, y, w, h);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	doRect (x, y, w+1, h+1, 0); /* X'S DrawRect is one bigger than asked */
}

void
XPSDrawRectangles (dsp, win, gc, xra, nxr)
Display *dsp;
Drawable win;
GC gc;
XRectangle xra[];
int nxr;
{
	XRectangle *lastxra;

	XDrawRectangles (dsp, win, gc, xra, nxr);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	for (lastxra = xra+nxr; xra < lastxra; xra++)
	    doRect (xra->x, xra->y, xra->width+1, xra->height+1, 0);
}

void
XPSDrawSegments (dsp, win, gc, xs, nxs)
Display *dsp;
Drawable win;
GC gc;
XSegment xs[];
int nxs;
{
	int i;

	XDrawSegments (dsp, win, gc, xs, nxs);
	if (xpsc.state != XDRAWING || xpsc.win != win || nxs == 0)
	    return;

	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	fprintf (FP, "newpath");
	for (i = 0; i < nxs; i++) {
	    if (!(i & 1))
		fprintf (FP, "\n  ");
	    doSegment (xs[i].x1, xs[i].y1, xs[i].x2, xs[i].y2);
	}
	fprintf (FP, "\nstroke\n");
			
}

/* draw using the font from gc which matches a previously Registered PS font.
 */
void
XPSDrawString (dsp, win, gc, x, y, s, l)
Display *dsp;
Drawable win;
GC gc;
int x, y;
char *s;
int l;
{
	unsigned long gcm;
	XGCValues gcv;
	PSFontInfo *fi;

	XDrawString (dsp, win, gc, x, y, s, l);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	/* find gc's postscript font in preregistered list */
	gcm = GCFont;
	XGetGCValues (dsp, gc, gcm, &gcv);
	fi = find_psfont (gcv.font);
	if (!fi)
	    return;

	/* load desired font and take care not to scale the text */
	fprintf (FP, "newpath %d %d moveto gsave\n", x, y);
	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	fprintf (FP, "  %g %g scale /%s findfont %g scalefont setfont\n",
	    1.0/xpsc.scale, -1.0/xpsc.scale, fi->psname, fi->pixsz*xpsc.scale);

	/* show the string -- beware a few special characters */
	fprintf (FP, "  (%s)\n", XPSCleanStr(s,l));
	fprintf (FP, "show grestore\n");
}

/* draw a rotated string.
 * align tells where [x,y] is with respect to the string then angle tells
 *   degrees ccw to rotate about x,y.
 * mag is factor of how much larger than basic fs to draw.
 */
void
XPSRotDrawAlignedString (dsp, fs, angle, mag, win, gc, x, y, str, align)
Display *dsp;
XFontStruct *fs;
double angle;
double mag;
Drawable win;
GC gc;
int x, y;
char *str;
int align;
{
	PSFontInfo *fi;
	char *clean;

	XRotSetMagnification (mag);
	XRotDrawAlignedString (dsp, fs, angle, win, gc, x, y, str, align);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	/* find fs's postscript font in preregistered list */
	fi = find_psfont (fs->fid);
	if (!fi)
	    return;

	clean = XPSCleanStr(str, strlen(str));

	fprintf (FP, "newpath %d %d moveto gsave\n", x, y);
	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	fprintf (FP, "  %g %g scale /%s findfont %g scalefont setfont\n",
					1.0/xpsc.scale, -1.0/xpsc.scale,
					fi->psname, fi->pixsz*xpsc.scale*mag);

	/* [x,y] is the location of the `align' position of the string and
	 * is the location about which we are to appear to rotate. Since PS
	 * always rotates about the lower left corner of string we need to 
	 * move the corner to where it will be after the rotation, then let
	 * PS rotate from there. Remember PS +Y is up.
	 */
	switch (align) {
	case MLEFT:
	    fprintf (FP, "  (%s) dup strsz\n", clean);
	    fprintf (FP, "  exch pop 2 div dup %g sin mul\n", angle);
	    fprintf (FP, "  exch %g cos mul neg rmoveto\n", angle);
	    break;

	case MRIGHT:
	    fprintf (FP, "  (%s) dup strsz\n", clean);
	    fprintf (FP, "  2 div dup %g sin mul 2 index %g cos mul sub\n",
								angle, angle);
	    fprintf (FP, "  3 1 roll %g cos mul exch %g sin mul add neg\n",
								angle, angle);
	    fprintf (FP, "  rmoveto\n");
	    break;

	case BLEFT:
	    fprintf (FP, "  (%s)\n", clean);
	    break;

	case BCENTRE:
	    fprintf (FP, "  (%s) dup strsz\n", clean);
	    fprintf (FP, "  pop 2 div dup %g cos mul neg\n", angle);
	    fprintf (FP, "  exch %g sin mul neg rmoveto\n", angle);
	    break;

	case TLEFT:
	    fprintf (FP, "  (%s) dup strsz\n", clean);
	    fprintf (FP, "  exch pop dup %g sin mul\n", angle);
	    fprintf (FP, "  exch %g cos mul neg rmoveto\n", angle);
	    break;

	case TCENTRE:
	    fprintf (FP, "  (%s) dup strsz\n", clean);
	    fprintf (FP, "  dup dup mul 3 2 roll 2 div dup dup mul\n");
	    fprintf (FP, "  exch 4 1 roll add sqrt 3 1 roll atan\n");
	    fprintf (FP, "  %g sub dup cos exch sin\n", angle);
	    fprintf (FP, "  3 2 roll dup 4 1 roll mul neg 3 1 roll mul neg\n");
	    fprintf (FP, "  rmoveto\n");
	    break;

	default:
	    printf ("XPSRot: unsupported align: %d\n", align);
	    abort();
	}

	fprintf (FP, "%g rotate show grestore\n", angle);
}

/* like XPSFillArc if filling a complete circle but tries to antialias.
 */
void
XPSDrawStar (dsp, win, gc, x, y, d)
Display *dsp;
Drawable win;
GC gc;
int x, y;
int d;
{
	XArc xa;

	x_drawstar (dsp, win, gc, x, y, d);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	xa.x = x;
	xa.y = y;
	xa.width = d;
	xa.height = d;
	xa.angle1 = 0;
	xa.angle2 = 360*64;
	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	doArc (&xa, 1);
}

void
XPSFillArc (dsp, win, gc, x, y, w, h, a1, a2)
Display *dsp;
Drawable win;
GC gc;
int x, y;
unsigned w, h;
int a1, a2;
{
	XArc xa;

	if (w == h && a2 == 360*64)
	    x_fill_circle (dsp, win, gc, x, y, w);
	else
	    XFillArc (dsp, win, gc, x, y, w, h, a1, a2);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	xa.x = x;
	xa.y = y;
	xa.width = w;
	xa.height = h;
	xa.angle1 = a1;
	xa.angle2 = a2;
	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	doArc (&xa, 1);
}

void
XPSFillArcs (dsp, win, gc, xap, na)
Display *dsp;
Drawable win;
GC gc;
XArc *xap;
int na;
{
	XArc *ap, *lastap;

	/* sorry to break this up.. */
	for (ap = xap, lastap = ap+na; ap < lastap; ap++) {
	    if (ap->width == ap->height && ap->angle2 == 360*64)
		x_fill_circle (dsp, win, gc, ap->x, ap->y, ap->width);
	    else
		XFillArc (dsp, win, gc, ap->x, ap->y, ap->width, ap->height,
							ap->angle1, ap->angle2);
	}

	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	for (ap = xap, lastap = ap+na; ap < lastap; ap++)
	    doArc (ap, 1);
}

void
XPSFillPolygon (dsp, win, gc, xp, nxp, shape, mode)
Display *dsp;
Drawable win;
GC gc;
XPoint xp[];
int nxp;
int shape;
int mode;
{
	XFillPolygon (dsp, win, gc, xp, nxp, shape, mode);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;


	fprintf (FP, "gsave ");
	if (xpsc.wantcolor)
	    setColor (dsp, gc);
	else
	    fprintf (FP, "0.5 setgray\n");
	setLineStyle (dsp, gc);
	doPoly (xp, nxp, mode, 1);
	fprintf (FP, "grestore\n");
}


void
XPSFillRectangle (dsp, win, gc, x, y, w, h)
Display *dsp;
Drawable win;
GC gc;
int x, y;
unsigned w, h;
{
	XFillRectangle (dsp, win, gc, x, y, w, h);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	doRect (x, y, w, h, 1);
}

void
XPSFillRectangles (dsp, win, gc, xra, nxr)
Display *dsp;
Drawable win;
GC gc;
XRectangle xra[];
int nxr;
{
	XRectangle *lastxra;

	XFillRectangles (dsp, win, gc, xra, nxr);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	for (lastxra = xra+nxr; xra < lastxra; xra++)
	    doRect (xra->x, xra->y, xra->width, xra->height, 1);
}
/* print the given pixmap of size widxhei using colormap cm.
 * if bggc, print white for any pixel matching the bggc.
 * if black, force all overlay to be black when not in color.
 */
void
XPSPixmap (pm, wid, hei, cm, bggc, black)
Pixmap pm;		/* pixmap to print */
unsigned int wid, hei;	/* pixmap size */
Colormap cm;		/* pixels in pm are with respect to this colormap */
GC bggc;		/* background pixel to not draw, or 0 */
int black;		/* all drawing to be in black if !wantcolor */
{
	Display *dsp = XtDisplay(toplevel_w);
	Pixel white = WhitePixel (dsp, DefaultScreen (dsp));
	XImage *xim;
	Window rootwin;
	unsigned int b, d;
	unsigned bgpix = 0;
	XColor xc;
	int x, y;
	int n;


	if (xpsc.state != XDRAWING)
	    return;

	/* get the bg pixel to avoid, if any */
	if (bggc) {
	    XGCValues gcv;
	    XGetGCValues (dsp, bggc, (unsigned long)GCForeground, &gcv);
	    bgpix = gcv.foreground;
	}

	xpsc.overimg = 1;
	xpsc.black = black;

	/* suck out the pixels as an image */
	XGetGeometry (dsp, pm, &rootwin, &x, &y, &wid, &hei, &b, &d);
	xim = XGetImage (dsp, pm, 0, 0, wid, hei, ~0, ZPixmap);
	if (!xim) {
	    xe_msg (1, "Can not create image for printing");
	    return;
	}

	/* set up postscript */
	fprintf (FP, "gsave /imgw %d def /imgh %d def\n", wid, hei);
	fprintf (FP, "  %d %d translate\n", 0, 0);
	fprintf (FP, "  imgw imgh scale\n");
	fprintf (FP, "  imgw imgh 8 [imgw 0 0 imgh neg 0 imgh]\n");
	fprintf (FP, "  { currentfile imgw string readhexstring pop }");

	/* if color use 8 bits per pixel, RGB format, else 8 bit gray */
	if (xpsc.wantcolor)
	    fprintf (FP, " false 3 colorimage\n");
	else
	    fprintf (FP, " image\n");

	/* postscript draws bottom-to-top so draw bottom up */
	n = 0;
	for (y = hei-1; y >= 0; --y) {
	    for (x = 0; x < (int)wid; x++) {
		xc.pixel = XGetPixel (xim, x, y);
		if (bggc && xc.pixel == bgpix)
		    xc.pixel = white;
		pixCache (&xc);
		if (xpsc.wantcolor) {
		    fprintf (FP, "%02x%02x%02x", xc.red>>8, xc.green>>8,
								xc.blue>>8);
		    if (((++n)%12) == 0)
			fprintf (FP, "\n");
		} else {
		    fprintf (FP, "%02x", SMPTE(xc)>>8);
		    if (((++n)%36) == 0)
			fprintf (FP, "\n");
		}
	    }
	}

	fprintf (FP, "\ngrestore\n");

	/* done */
	free ((void *)xim->data);
	xim->data = NULL;
	XDestroyImage (xim);
}

/* RGB -> HSV, all [0..1) */
void
toHSV (double r, double g, double b, double *hp, double *sp, double *vp)
{
	double min, max, diff;
	double h, s, v;

	max = MYMAX (MYMAX (r, g), b);
	min = MYMIN (MYMIN (r, g), b);
	diff = max - min;
	v = max;
	s = max != 0 ? diff/max : 0;
	if (s == 0)
	    h = 0;
	else {
	    double rc = (max - r)/diff;
	    double gc = (max - g)/diff;
	    double bc = (max - b)/diff;
	    if (r == max)
		h = bc - gc;
	    else if (g == max)
		h = 2 + rc - bc;
	    else
		h = 4 + gc - rc;
	    if (h < 0)
		h += 6;
	    h /= 6;
	}

	*hp = h;
	*sp = s;
	*vp = v;
}

/* HSV -> RGB, all [0..1) */
void
toRGB (double h, double s, double v, double *rp, double *gp, double *bp)
{
	double r, g, b;
	double f, p, q, t;
	int i;

	if (v == 0) 
	    r = g = b = 0.0;
	else if (s == 0)
	    r = g = b = v;
	else {
	    h *= 6;
	    i = (int)floor(h);
	    f = h - i;
	    p = v * (1.0 - s);
	    q = v * (1.0 - s * f);
	    t = v * (1.0 - s * (1.0 - f));

	    switch (i) {
	    default: /* FALLTHRU */
	    case 0: r = v; g = t; b = p; break;
	    case 1: r = q; g = v; b = p; break;
	    case 2: r = p; g = v; b = t; break;
	    case 3: r = p; g = q; b = v; break;
	    case 4: r = t; g = p; b = v; break;
	    case 5: r = v; g = p; b = q; break;
	    }
	}

	*rp = r;
	*gp = g;
	*bp = b;
}

/* check gc's LineStyle and emit "solid" or "dashed" macro.
 * also set line width.
 * TODO: check dash details.
 */
static void
setLineStyle (dsp, gc)
Display *dsp;
GC gc;
{
	XGCValues gcv;
	unsigned long gcm;
	double lw;

	gcm = GCLineStyle | GCLineWidth;
	XGetGCValues (dsp, gc, gcm, &gcv);
	fprintf (FP, "%s ", gcv.line_style == LineSolid ? "solid" : "dashed");
	lw = gcv.line_width + (xpsc.wantThick ? 0.5 : 0.0);
	fprintf (FP, "%g setlinewidth ", lw);
}

/* set postscript value for FG color in gc. */
static void
setColor (dsp, gc)
Display *dsp;
GC gc;
{
	XGCValues gcv;
	XColor xc;

	XGetGCValues (dsp, gc, (unsigned long) GCForeground, &gcv);
	xc.pixel = gcv.foreground;

	if (xpsc.papercolorset && xc.pixel == xpsc.papercolor) {
	    fprintf (FP, "1 setgray\n");		/* draw as "paper" */
	} else if (xpsc.wantcolor) {
	    double r, g, b;
	    pixCache (&xc);
	    r = (double)xc.red/65535.0;
	    g = (double)xc.green/65535.0;
	    b = (double)xc.blue/65535.0;
	    if (!xpsc.overimg) {
		/* enrich colors when drawing on white paper */
		double h, s, v;
		toHSV (r, g, b, &h, &s, &v);
		s = sqrt(s);
		v = .5*sqrt(v);
		toRGB (h, s, v, &r, &g, &b);
	    }
	    fprintf (FP, "%g %g %g setrgbcolor\n", r, g, b);
	} else if (!xpsc.overimg || xpsc.black) {
	    fprintf (FP, "0 setgray\n");		/* draw black */
	} else {
	    pixCache (&xc);
	    fprintf (FP, "%g setgray\n", SMPTE(xc)/65535.0);
	}
}

/* XFill and XDraw arc. same goofyness with whether it includes outter edge.
 */
static void
doArc (xap, fill)
XArc *xap;
int fill;
{
	int x = xap->x;
	int y = xap->y;
	int w = xap->width;
	int h = xap->height;

	/* seems ellipse can't handle the thin cases */
	if (w == 0 && h == 0)
	    return;
	if (w == 0 || h == 0)
	    doSegment (x, y, x+w, y+h);
	else {
	    double hp = (w&1) ? 0 : 0.5; /* add half pixel for even widths */
	    fprintf (FP, "newpath %g %g 0 %g %g %g %g ellipse %s\n",
			    x+(w-fill)/2.0+hp, y+(h-fill)/2.0+hp, w/2.0, h/2.0,
			    360-(xap->angle1 + xap->angle2)/64.0,
			    360-xap->angle1/64.0,
			    fill ? "fill" : "stroke");
	}
}

/* draw or fill a rotated ellipse.
 * easy for postscript, not so for X.
 * all angles are 64ths degree, 0 is 3oclock, positive ccw (all like in X).
 */
static void
doEllipse (fill, dsp, win, gc, x, y, a0, w, h, a1, a2)
int fill;
Display *dsp;
Drawable win;
GC gc;
int x, y;	/* bounding box upper left corner */
int a0;		/* axis rotation */
unsigned w, h;	/* bounding box width, height */
int a1, a2;	/* initial angle, additional extent */
{
#define	MAXEPTS	50	/* n points to draw a full ellipse */
#ifndef PI
#define	PI	3.14159
#endif
	double ca0 = cos(a0/64.0*PI/180.0);
	double sa0 = sin(a0/64.0*PI/180.0);
	double a = (double)w/2.0;	/* semi-major axis */
	double b = (double)h/2.0;	/* semi-minor axis */
	XPoint xpt[MAXEPTS];
	int nxpt, maxpt;
	int i;

	/* find number of points to draw an extent of a2 */
	if (a2 == 0)
	    return;
	if (a2 > 360*64)
	    a2 = 360*64;
	else if (a2 < -360*64)
	    a2 = -360*64;
	maxpt = a2*MAXEPTS/(360*64);
	if (maxpt < 0)
	    maxpt = -maxpt;

	/* compute each point, from a1 through a1+a2 */
	for (nxpt = i = 0; i < maxpt; i++) {
	    XPoint *xptp = &xpt[nxpt];
	    double theta = PI/180.0*(a1 + i*a2/(maxpt-1))/64.0;
	    double ex = a*cos(theta);
	    double ey = -b*sin(theta);
	    double xp =  ex*ca0 + ey*sa0;
	    double yp = -ex*sa0 + ey*ca0;
	    xptp->x = (short)floor(xp + x + a + 0.5);
	    xptp->y = (short)floor(yp + y + b + 0.5);
	    if (nxpt == 0 || (xptp[-1].x != xptp->x || xptp[-1].y != xptp->y))
		nxpt++;
	}
	if (fill)
	    XFillPolygon (dsp, win, gc, xpt, nxpt, Convex, CoordModeOrigin);
	else
	    XDrawLines (dsp, win, gc, xpt, nxpt, CoordModeOrigin);
	if (xpsc.state != XDRAWING || xpsc.win != win)
	    return;

	/* ellipse can't handle the thin cases */
	setColor (dsp, gc);
	setLineStyle (dsp, gc);
	if ((int)w == 0 || (int)h == 0)
	    doSegment (x, y, x+(int)w, y+(int)h);
	else {
	    fprintf (FP, "newpath %g %g %g %g %g %g %g ellipse ", x + a,
			y + b, -a0/64.0, a, b, 360-(a1 + a2)/64.0, 360-a1/64.0);
	    fprintf (FP, "%s\n", fill ? "fill" : "stroke");
	}
}

static void
doPoly (xp, nxp, mode, fill)
XPoint xp[];
int nxp;
int mode;
int fill;
{
	int i;

	if (nxp < 1)
	    return;

	if (nxp < 3 || mode == CoordModePrevious) {
	    char *lt = mode == CoordModeOrigin ? "lineto" : "rlineto";

	    fprintf (FP, "newpath %d %d moveto\n", xp[0].x, xp[0].y);
	    for (i = 1; i < nxp; i++)
		fprintf (FP, "  %d %d %s\n", xp[i].x, xp[i].y, lt);
	    fprintf (FP, "%s\n", fill ? "fill" : "stroke");
	} else if (nxp < 8) {
	    /* not enough points for a sensible spline */
	    fprintf (FP, "newpath %d %d moveto", xp[0].x, xp[0].y);
	    for (i = 1; i < nxp; i++)
		fprintf (FP, " %d %d lineto", xp[i].x, xp[i].y);
	    fprintf (FP, " %s\n", fill ? "fill" : "stroke");
	} else {
	    /* Spline requires at least 3 absolute points */
	    fprintf (FP, "newpath\n  [");
	    for (i = 0; i < nxp; i++)
		fprintf (FP, " %d %d ", xp[i].x, xp[i].y);
	    fprintf (FP, "]\nSpline %s\n", fill ? "fill" : "stroke");
	}
}

static void
doRect (x, y, w, h, fill)
int x, y, w, h;
int fill;
{
	fprintf (FP, "newpath %d %d moveto %d %d lineto %d %d lineto\n",
				    x, y,   x+w, y,   x+w, y+h);
	fprintf (FP, "%d %d lineto %d %d lineto", x, y+h, x, y);
	fprintf (FP, " %s\n", fill ? "fill" : "stroke");
}

/* draw a line from [x1,y1] to [x2,y2] */
static void
doSegment (x1, y1, x2, y2)
int x1, y1, x2, y2;
{
	fprintf (FP, " %4d %4d moveto %4d %4d lineto ", x1, y1, x2, y2);
}

/* ARGSUSED */
static int IgnoreXError(disp, event)
Display *disp;
XErrorEvent *event;
{
	return (0);
}

/* find the postscipt info for fid, creating new one if necessary.
 * xe_msg() why and return NULL if trouble.
 */
static PSFontInfo *
find_psfont (fid)
Font fid;
{
#define	PSDEFFAM	4		/* psmaps[] when no match found */
#define	PSDEFSIZ	12		/* default pixel size */
#define	XAT(a)		XInternAtom(dsp,a,1)	/* handy */
	typedef struct {
	    char *famkey;		/* lowercase FONT family key string */
	    char *psfam;		/* ps family */
	    char *pslight;		/* ps light suffix, if any */
	    char *psbold;		/* ps bold suffix, if any */
	    char *psital;		/* ps italic suffix, if any */
	} PSMap;
	static PSMap psmaps[] = {
	    {"avantgarde", "AvantGarde",       "Book",  "Demi", "Oblique"},
	    {"bookman",    "Bookman",          "Light", "Demi", "Italic"},
	    {"century",    "NewCenturySchlbk", 0,       "Bold", "Italic"},
	    {"lucidatypewriter", "Courier",    0,       "Bold", "Oblique"},
	    {"courier",    "Courier",          0,       "Bold", "Oblique"},
	    {"helvetica",  "Helvetica",        0,       "Bold", "Oblique"},
	    {"palatino",   "Palatino",         0,       "Bold", "Italic"},
	    {"symbol",     "Symbol",           0,       0,      0},
	    {"times",      "Times",            0,       "Bold", "Italic"},
	};
	Display *dsp = XtDisplay(toplevel_w);
	int (*oldhandler)();
	int bold, ital;
	char *fam;
	char psname[64];
	PSFontInfo *fi;
	XFontStruct *fsp;
	PSMap *psm;
	unsigned long card32;
	int pixsz;
	int i;

	/* see if already have info */
	for (i = 0; i < npsfi; i++)
	    if (fid == psfi[i].fid)
		return (&psfi[i]);

	/* get font descriptor so we can get font properties */
	oldhandler = XSetErrorHandler (IgnoreXError);
	fsp = XQueryFont (dsp, fid);
	if (!fsp) {
	    xe_msg (1, "No font info!");
	    XSetErrorHandler (oldhandler);
	    return (NULL);
	}

	/* get string containing family and whether bold and italic */
	if (XGetFontProperty (fsp, XAT("FONT"), &card32)) {
	    /* get all from xlfd if available */
	    char *xlfd = XGetAtomName (dsp, card32);
	    strtolower (xlfd);
	    fam = XtNewString (xlfd);
	    bold = strstr (xlfd, "bold") || strstr (xlfd, "demi");
	    ital = strstr (xlfd, "-o-") || strstr (xlfd, "-i-");
	    XFree (xlfd);
	} else {
	    /* find family, or default */
	    if (XGetFontProperty (fsp, XAT("FAMILY_NAME"), &card32)) {
		char *fn = XGetAtomName (dsp, card32);
		fam = XtNewString (fn);
		strtolower (fam);
		XFree (fn);
	    } else {
		fam = XtNewString (psmaps[PSDEFFAM].famkey);
	    }

	    /* decide whether bold */
	    if (XGetFontProperty (fsp, XAT("WEIGHT_NAME"), &card32)) {
		char *wt = XGetAtomName (dsp, card32);
		strtolower (wt);
		bold = strstr (wt, "bold") || strstr (wt, "demi");
		XFree (wt);
	    } else
		bold = 0;

	    /* decide whether italic */
	    if (XGetFontProperty (fsp, XAT("SLANT"), &card32)) {
		char *sl = XGetAtomName (dsp, card32);
		strtolower (sl);
		ital = strstr (sl, "o") || strstr (sl, "i");
		XFree (sl);
	    } else if (XGetFontProperty (fsp, XAT("ITALIC_ANGLE"), &card32)) {
		ital = card32 != 90*64;
	    } else
		ital = 0;
	}

	/* find psmaps[] that matches family, or use default */
	for (i = 0; i < XtNumber(psmaps); i++)
	    if (strstr (fam, psmaps[i].famkey))
		break;
	if (i ==  XtNumber(psmaps))
	    i = PSDEFFAM;
	psm = &psmaps[i];

	/* assemble ps name */
	strcpy (psname, psm->psfam);
	if (bold) {
	    if (psm->psbold) {
		strcat (psname, "-");
		strcat (psname, psm->psbold);
	    }
	} else {
	    if (psm->pslight) {
		strcat (psname, "-");
		strcat (psname, psm->pslight);
	    }
	}
	if (ital) {
	    if (psm->psital) {
		if (!strchr (psname, '-'))
		    strcat (psname, "-");
		strcat (psname, psm->psital);
	    }
	}

	/* get pixel size */
	if (XGetFontProperty (fsp, XAT("PIXEL_SIZE"), &card32))
	    pixsz = card32;
	else if(XGetFontProperty(fsp,XAT("POINT_SIZE"),&card32)) {
	    unsigned long res;
	    if (XGetFontProperty(fsp, XAT("RESOLUTION"), &res))
		pixsz = card32*res/72/10;
	    else
		pixsz = PSDEFSIZ;
	} else
	    pixsz = PSDEFSIZ;

	/* TODO: how to free memory but retain fid?
	XFreeFont (dsp, fsp);
	*/

	/* add to psfi[] cache */
	psfi = (PSFontInfo*)XtRealloc((void*)psfi,(npsfi+1)*sizeof(PSFontInfo));
	fi = &psfi[npsfi++];
	fi->psname = XtNewString (psname);
	fi->pixsz = pixsz;
	fi->fid = fid;

	/* done */
	XSetErrorHandler (oldhandler);
	XtFree (fam);
	return (fi);
}

static void
checkState (funcname, s)
char *funcname;
XPSState s;
{
	if (xpsc.state != s) {
	    printf ("XPS%s(): state is %d but expected %d\n",
					funcname, (int)xpsc.state, (int)s);
	    abort();
	}
}

static void
printTime (x, y)
int x, y;
{
	char *ct;
	int ctl;

#if defined(__STDC__)
	struct tm *tmp;
	time_t t;

	(void) time (&t);
	tmp = gmtime (&t);
	if (tmp) {
	    static char mons[12][4] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	    };
	    static char days[7][4] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	    };
	    char buf[128];

	    (void) sprintf (buf, "%s %s %2d %02d:%02d:%02d %d",
	    	days[tmp->tm_wday], mons[tmp->tm_mon], tmp->tm_mday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec, tmp->tm_year+1900);
	    fprintf (FP, "(Generated %s UTC) %d %d cstr\n", buf , x, y);
	    return;
	}
#else
	long t;

	time (&t);
#endif

	/* use local time if not standard C or no gmtime() */
	ct = ctime (&t);
	ctl = strlen (ct)-1;	/* chop trailing '\n' */

	fprintf (FP, "(Generated %.*s Local Time) %d %d cstr\n", ctl, ct, x, y);
}

/* prepare the named file for writing postscript into.
 * return 0 if ok, else report why with xe_msg(), reset xpsc and return -1.
 */
static int
XPSOpen (fn)
char *fn;
{
	checkState ("Open", ASKING);

	xpsc.fp = fopend (fn, NULL, "w");
	if (!xpsc.fp) {
	    xpsc_close();
	    return (-1);
	}

	xpsc.state = OPEN;

	return (0);
}

/* create the print dialog */
static void
create_print_w()
{
	Widget w;
	Widget f_w, rc_w;
	Widget thinlw_w, letter_w, bandw_w, filetb_w;
	Widget rb_w;
	Arg args[20];
	int n;

	/* create the form dialog -- title gets set later */
	n = 0;
	XtSetArg (args[n], XmNallowShellResize, True); n++;
	XtSetArg (args[n], XmNcolormap, xe_cm); n++;
	XtSetArg (args[n], XmNiconName, "Print"); n++;
	XtSetArg (args[n], XmNdeleteResponse, XmUNMAP); n++;
	print_w = XtCreatePopupShell ("Print", topLevelShellWidgetClass,
							toplevel_w, args, n);
	setup_icon (print_w);
	set_something (print_w, XmNcolormap, (XtArgVal)xe_cm);
	sr_reg (print_w, "XEphem*Print.x", printcategory, 0);
	sr_reg (print_w, "XEphem*Print.y", printcategory, 0);

	/* make a rowcolumn for stuff */

	n = 0;
	XtSetArg (args[n], XmNmarginHeight, 10); n++;
	XtSetArg (args[n], XmNmarginWidth, 10); n++;
	XtSetArg (args[n], XmNspacing, 5); n++;
	rc_w = XmCreateRowColumn (print_w, "VRC", args, n);
	XtAddCallback (rc_w, XmNhelpCallback, help_cb, 0);
	XtManageChild (rc_w);

	/* make a prompt for the title */

	n = 0;
	w = XmCreateLabel (rc_w, "TitleL", args, n);
	set_xmstring (w, XmNlabelString, "Title:");
	XtManageChild (w);

	n = 0;
	title_w = XmCreateTextField (rc_w, "Title", args, n);
	wtip (title_w, "Enter desired text to appear across top of print file");
	XtManageChild (title_w);

	/* add a separator */

	n = 0;
	w = XmCreateSeparator (rc_w, "Sep0", args, n);
	XtManageChild (w);

	/* form for two side-by-side radio boxes */

	n = 0;
	f_w = XmCreateForm (rc_w, "PF", args, n);
	XtManageChild (f_w);

	    /* make the color/B&W radio box */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    rb_w = XmCreateRadioBox (f_w, "CGRB", args, n);
	    XtManageChild (rb_w);

		n = 0;
		color_w = XmCreateToggleButton (rb_w, "Color", args, n);
		set_xmstring (color_w, XmNlabelString, " Color");
		wtip (color_w, "When on, attempt to print in full color");
		XtManageChild (color_w);
		sr_reg (color_w, NULL, printcategory, 1);

		n = 0;
		bandw_w = XmCreateToggleButton (rb_w, "BW", args, n);
		set_xmstring (bandw_w, XmNlabelString, " Black");
		wtip (bandw_w, "When on, print in black only (no color)");
		XtManageChild (bandw_w);

		/* color inits both */
		XmToggleButtonSetState(bandw_w,
				    !XmToggleButtonGetState(color_w), False);

	    /* make the letter/A4 radio box */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 30); n++;
	    rb_w = XmCreateRadioBox (f_w, "PSZ", args, n);
	    XtManageChild (rb_w);

		n = 0;
		letter_w = XmCreateToggleButton (rb_w, "Letter", args, n);
		set_xmstring (letter_w, XmNlabelString, " Letter");
		wtip (letter_w, "Set size for US Letter (8½x11 inch) paper");
		XtManageChild (letter_w);

		n = 0;
		A4_w = XmCreateToggleButton (rb_w, "A4", args, n);
		set_xmstring (A4_w, XmNlabelString, " A4");
		wtip (A4_w, "Set size for A4 (210x297 mm) paper");
		XtManageChild (A4_w);
		sr_reg (A4_w, NULL, printcategory, 1);

		/* A4 inits both */
		XmToggleButtonSetState(letter_w,
				    !XmToggleButtonGetState(A4_w), False);

	    /* make the line width radio box */

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 60); n++;
	    rb_w = XmCreateRadioBox (f_w, "LWSZ", args, n);
	    XtManageChild (rb_w);

		n = 0;
		thinlw_w = XmCreateToggleButton (rb_w, "Thin", args, n);
		set_xmstring (thinlw_w, XmNlabelString, " Thin lines");
		wtip (thinlw_w, "Use thin lines");
		XtManageChild (thinlw_w);

		n = 0;
		thicklw_w = XmCreateToggleButton (rb_w, "Thick", args, n);
		set_xmstring (thicklw_w, XmNlabelString, " Thick lines");
		wtip (thicklw_w, "Use thick lines");
		XtManageChild (thicklw_w);
		sr_reg (thicklw_w, NULL, printcategory, 1);

		/* thick inits both */
		XmToggleButtonSetState(thinlw_w,
				    !XmToggleButtonGetState(thicklw_w), False);

	/* add a separator */

	n = 0;
	w = XmCreateSeparator (rc_w, "Sep1", args, n);
	XtManageChild (w);

	/* make the fake "to file" toggle button and filename text field */

	n = 0;
	f_w = XmCreateForm (rc_w, "FileF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    filetb_w = XmCreateToggleButton (f_w, "Save", args, n);
	    set_xmstring (filetb_w, XmNlabelString, " Save to file: ");
	    wtip(filetb_w,"When on, plot will be saved to file named at right");
	    XtManageChild (filetb_w);
	    sr_reg (filetb_w, NULL, printcategory, 1);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, filetb_w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    filename_w = XmCreateTextField (f_w, "Filename", args, n);
	    defaultTextFN (filename_w, 1, "xephem.ps", NULL);
	    wtip (filename_w, "Name of postscript file to create");
	    XtManageChild (filename_w);
	    sr_reg (filename_w, NULL, printcategory, 1);

	/* make the fake "print" toggle button and print command text field */

	n = 0;
	f_w = XmCreateForm (rc_w, "PrintF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY); n++;
	    prtb_w = XmCreateToggleButton (f_w, "Print", args, n);
	    set_xmstring (prtb_w, XmNlabelString, " Print command:");
	    wtip (prtb_w, "When set, send postscript directly to printer using command at right");
	    XtManageChild (prtb_w);

	    n = 0;
	    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg (args[n], XmNleftWidget, prtb_w); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    prcmd_w = XmCreateTextField (f_w, "PrintCmd", args, n);
	    wtip (prcmd_w, "Command which will print a postscript file given name of file as first and only argument");
	    XtManageChild (prcmd_w);
	    sr_reg (prcmd_w, NULL, printcategory, 1);

	/* connect them together so they work like a radio box */

	XtAddCallback (filetb_w, XmNvalueChangedCallback, toggle_cb,
							    (XtPointer)prtb_w);
	XtAddCallback (prtb_w, XmNvalueChangedCallback, toggle_cb,
							(XtPointer)filetb_w);

	/* value of Save sets both */
	XmToggleButtonSetState(prtb_w, !XmToggleButtonGetState(filetb_w),False);

	/* add another separator */

	n = 0;
	w = XmCreateSeparator (rc_w, "Sep2", args, n);
	XtManageChild (w);

	/* add the command buttons across the bottom */

	n = 0;
	XtSetArg (args[n], XmNfractionBase, 10); n++;
	f_w = XmCreateForm (rc_w, "CmdF", args, n);
	XtManageChild (f_w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 1); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 3); n++;
	    w = XmCreatePushButton (f_w, "Ok", args, n);
	    XtAddCallback (w, XmNactivateCallback, ok_cb, 0);
	    wtip (w, "Commence printing and close this dialog");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 4); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 6); n++;
	    w = XmCreatePushButton (f_w, "Cancel", args, n);
	    XtAddCallback (w, XmNactivateCallback, cancel_cb, 0);
	    wtip (w, "Close this dialog and do nothing");
	    XtManageChild (w);

	    n = 0;
	    XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNleftPosition, 7); n++;
	    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	    XtSetArg (args[n], XmNrightPosition, 9); n++;
	    w = XmCreatePushButton (f_w, "Help", args, n);
	    XtAddCallback (w, XmNactivateCallback, help_cb, 0);
	    wtip (w, "Additional information");
	    XtManageChild (w);
}

/* called by either of the Save or Print togle buttons to enforce their
 * radio behavior. client is the opposite Widget.
 */
static void
/* ARGSUSED */
toggle_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	if (XmToggleButtonGetState(w)) {
	    Widget otherw = (Widget)client;
	    XmToggleButtonSetState (otherw, False, False);
	} else
	    XmToggleButtonSetState (w, True, False); /* just like a RB :-) */
}

/* called to reset the pending print query sequence */
static void
no_go()
{
	checkState ("no_go", ASKING);
	xpsc_close();
}

/* called when the Ok button is hit in the print dialog */
/* ARGSUSED */
static void
ok_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	xpsc.wantcolor = XmToggleButtonGetState (color_w);
	xpsc.wantA4 = XmToggleButtonGetState (A4_w);
	xpsc.wantThick = XmToggleButtonGetState (thicklw_w);
	xpsc.wantpr = XmToggleButtonGetState (prtb_w);

	if (xpsc.wantpr) {
	    /* print */
	    char name[2048];

	    tempfilename (name, "xepr", ".ps");
#if defined(__NUTC__)
	    xpsc.prfile = XtMalloc (PATH_MAX+1);
	    _NutPathToWin32 (name, xpsc.prfile, 1);
#else
	    xpsc.prfile = XtNewString (name);
#endif
	    if (XPSOpen (xpsc.prfile) == 0)
		call_go();

	} else {
	    /* save to file -- ask whether to clobber if it already exits */
	    char *name = XmTextFieldGetString (filename_w);

	    if (existd (name,NULL) == 0 && confirm()) {
		char buf[1024];
		(void) sprintf (buf, "%s exists:\nOk to Overwrite?", name);
		query (print_w, buf, "Yes -- Overwrite", "No -- Cancel",
					    NULL, saveas_confirm, no_go, NULL);
	    } else
		saveas_confirm();

	    XtFree (name);
	}

	XtPopdown (print_w);
}

/* called by Cancel */
/* ARGSUSED */
static void
cancel_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	no_go();
	XtPopdown (print_w);
}

/* called by Help */
/* ARGSUSED */
static void
help_cb (w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *msg[] = {
"Select Color or Grayscale;",
"Select Letter or A4 size;",
"Select to save to file (and give filename) or print (and give command);",
"Then press Ok.",
};

	hlp_dialog ("Print", msg, sizeof(msg)/sizeof(msg[0]));
}

/* called when we have confirmed it's ok to clobber an existing save file */
static void
saveas_confirm()
{
	char *name = XmTextFieldGetString (filename_w);
	if (XPSOpen (name) == 0)
	    call_go();
	XtFree (name);
}


/* finished with xpsc */
static void
xpsc_close()
{
	if (xpsc.fp)
	    fclose (xpsc.fp);

	if (xpsc.prfile)
	    XtFree (xpsc.prfile);

	if (xpsc.lcnum)
	    setlocale (LC_NUMERIC, xpsc.lcnum);

	memset (&xpsc, 0, sizeof(xpsc));

}

/* called when it's confirmed to try and print or save */
/* ARGSUSED */
static void
call_go ()
{
	/* call the user's function */
	if (xpsc.go)
	    (*xpsc.go) ();
	else {
	    printf ("call_go() but no (*go)(). state = %d\n", (int)xpsc.state);
	    abort();
	}

	/* clean up if the user forgot */
	switch (xpsc.state) {
	case CLOSED:
	    break;
	case ASKING:
	    printf ("call_go() state = %d\n", (int)xpsc.state);
	    abort();
	    break;	/* :-) */
	case OPEN:
	    XPSClose ();
	    break;
	case XDRAWING:
	    XPSXEnd();
	    XPSClose();
	    break;
	}
}

/* fill a circle with upper left bounding box at [x,y] with diameter diam.
 * use custom bitmaps because many X servers get it poorly.
 */
static void
x_fill_circle (dsp, win, gc, x, y, diam)
Display *dsp;
Drawable win;
GC gc;
int x, y;
int diam;
{
	static unsigned char star2_bits[] = {
	   0x03, 0x03};
	static unsigned char star3_bits[] = {
	   0x07, 0x07, 0x07};
	static unsigned char star4_bits[] = {
	   0x06, 0x0f, 0x0f, 0x06};
	static unsigned char star5_bits[] = {
	   0x0e, 0x1f, 0x1f, 0x1f, 0x0e};
	static unsigned char star6_bits[] = {
	   0x1e, 0x3f, 0x3f, 0x3f, 0x3f, 0x1e};
	static unsigned char star7_bits[] = {
	   0x1c, 0x3e, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c};
	static unsigned char star8_bits[] = {
	   0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c};
	static unsigned char star9_bits[] = {
	   0x38, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xff, 0x01, 0xff, 0x01, 0xff,
	   0x01, 0xfe, 0x00, 0xfe, 0x00, 0x38, 0x00};
	static unsigned char star10_bits[] = {
	   0x78, 0x00, 0xfe, 0x01, 0xfe, 0x01, 0xff, 0x03, 0xff, 0x03, 0xff,
	   0x03, 0xff, 0x03, 0xfe, 0x01, 0xfe, 0x01, 0x78, 0x00};
	static unsigned char star11_bits[] = {
	   0xf8, 0x00, 0xfe, 0x03, 0xfe, 0x03, 0xff, 0x07, 0xff, 0x07, 0xff,
	   0x07, 0xff, 0x07, 0xff, 0x07, 0xfe, 0x03, 0xfe, 0x03, 0xf8, 0x00};
	static unsigned char star12_bits[] = {
	   0xf0, 0x00, 0xfe, 0x07, 0xfe, 0x07, 0xfe, 0x07, 0xff, 0x0f, 0xff,
	   0x0f, 0xff, 0x0f, 0xff, 0x0f, 0xfe, 0x07, 0xfe, 0x07, 0xfe, 0x07,
	   0xf0, 0x00};
	static unsigned char *star_bits[] = {
	    NULL,
	    NULL,
	    star2_bits,
	    star3_bits,
	    star4_bits,
	    star5_bits,
	    star6_bits,
	    star7_bits,
	    star8_bits,
	    star9_bits,
	    star10_bits,
	    star11_bits,
	    star12_bits,
	};
	static Pixmap pms[XtNumber(star_bits)];

	/* build the pixmaps the first time through */
	if (!pms[2]) {
	    int i;
	    for (i = 0; i < XtNumber(pms); i++)
		if (star_bits[i])
		    pms[i] = XCreateBitmapFromData (dsp, win,
						(char *)(star_bits[i]), i, i);
	}

	/* see whether the special cases apply */
	if (diam < 1)
	    return;
	if (diam < 2) {
	    XDrawPoint (dsp, win, gc, x, y);
	    return;
	}
	if (diam >= XtNumber(pms)) {
	    XFillArc (dsp, win, gc, x, y, diam, diam, 0, 360*64);
	    return;
	}

	/* draw smaller dots with the bitmaps */
	XSetClipMask (dsp, gc, pms[diam]);
	XSetClipOrigin (dsp, gc, x, y);
	XFillRectangle (dsp, win, gc, x, y, diam, diam);
	XSetClipMask (dsp, gc, None); /* TODO: probably should put back old */
}

/* fill a star with upper left bounding box at [x,y] with diameter diam.
 * try to simulate antialiasing.
 */
static void
x_drawstar (dsp, win, gc, x, y, diam)
Display *dsp;
Drawable win;
GC gc;
int x, y;
int diam;
{
	static GC gc1;
	XGCValues gcv;
	Pixel hp;

	/* skip if invisible */
	if (diam <= 0)
	    return;

	/* build helper GC first time */
	if (!gc1)
	    gc1 = XCreateGC (dsp, win, 0L, NULL);

	/* 1 and 2 are single pixels */
	if (diam == 1) {
	    XDrawPoint (dsp, win, gc, x, y);	/* already dimmed */
	    return;
	}
	if (diam == 2) {
	    XDrawPoint (dsp, win, gc, x+1, y+1);
	    return;
	}

	/* halo is dimmed version of main color */
	XGetGCValues (dsp, gc, (unsigned long) GCForeground, &gcv);
	hp = haloCache (dsp, gcv.foreground);
	XSetForeground (dsp, gc1, hp);

	/* center with surrounding halo */
	switch (diam) {
	case 3:
	    XFillRectangle (dsp, win, gc1, x, y, 3, 2);
	    XDrawLine (dsp, win, gc, x+1, y, x+1, y+1);
	    break;
	case 4:
	    x += 1;
	    y += 1;
	    diam -= 1;
	    XFillArc (dsp, win, gc, x, y, diam, diam, 0, 360*64);
	    XDrawArc (dsp, win, gc1, x, y, diam, diam, 0, 360*64);
	    break;
	default:
	    XFillArc (dsp, win, gc, x, y, diam, diam, 0, 360*64);
	    XDrawArc (dsp, win, gc1, x, y, diam, diam, 0, 360*64);
	}
}

/* given a pixel, find a dimmed halo color.
 * cache for later.
 */
static Pixel
haloCache (Display *dsp, Pixel p)
{
	static struct _hc { Pixel p, hp; } *hcache;
	static int nhcache;
	XColor xc;
	double r, g, b;
	double h, s, v;
	int i;

	/* check cache */
	for (i = 0; i < nhcache; i++)
	    if (hcache[i].p == p)
		return (hcache[i].hp);

	/* nope, create halo pixel */
	xc.pixel = p;
	XQueryColor (dsp, xe_cm, &xc);
	r = (double)xc.red/65535.0;
	g = (double)xc.green/65535.0;
	b = (double)xc.blue/65535.0;
	toHSV (r, g, b, &h, &s, &v);
	v *= 0.40;
	toRGB (h, s, v, &r, &g, &b);
	xc.red = (int)(r*65535);
	xc.green = (int)(g*65535);
	xc.blue = (int)(b*65535);
	XAllocColor (dsp, xe_cm, &xc);

	/* add to hcache and return */
	hcache = (struct _hc *) realloc(hcache, (nhcache+1)*sizeof(struct _hc));
	hcache[nhcache].p = p;
	hcache[nhcache].hp = xc.pixel;
	return (hcache[nhcache++].hp);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: ps.c,v $ $Date: 2006/08/21 09:18:56 $ $Revision: 1.67 $ $Name:  $"};
