/* code to manage what the outside world sees as the db_ interface.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


#include <Xm/Xm.h>

#include "xephem.h"


static void db_init (void);
static Obj *db_objadd (DBCat *dbcp, Obj *newop);
static DBCat *db_catadd (char *filename);
static DBCat *db_nmfindcat (char *name);
static void dbfifo_cb (XtPointer client, int *fdp, XtInputId *idp);
static int db_catcmpf (const void *d1, const void *d2);
static Obj *DBCatObj (DBCat *dbcp, int t, int i);
static void dupaddnm (char nm[][MAXNM], int nnm, Obj *op);
static void dupdelcat (DBCat *);
static void dupsort (void);
static int dup_qsort (const void *p1, const void *p2);
static int dupchknm (char nm[][MAXNM], int nnm);

#define	MAXDBLINE	256	/* longest allowed db line */

#define	DBFIFO_MSG	'!'	/* introduces a message line from the DBFIFO */

/* Category for ours and dbmenu's resources in the Save system */
char dbcategory[] = "Data files";

/* This counter is incremented when we want to mark all the Obj derived entries
 * as being out-of-date. This works because then each of the age's will be !=
 * db_age. This is to eliminate ever calling obj_cir() under the same
 * circumstances for a given db object.
 * N.B. For this to work, call db_update() not obj_cir().
 */
static ObjAge_t db_age = 1;	/* insure all objects are initially o-o-d */

/* return true if the database has been initialized */
#define	DBINITED	(dbcat)	/* anything set by db_init() */

/* all data is kept in their respective catalogs. the first is a fake for the
 * builtin objects. the others are kept sorted by name.
 */
static DBCat *dbcat;		/* malloced array, one for each loaded catalog*/
static int ndbcat;		/* number of entries in dbcat[] */
static char dbifres[] = "DBinitialFiles";	/* init files resource name */

/* db fifo name */
static char dbfifo[] = "fifos/xephem_db_fifo";

/* names for builtin and fifo catalogs */
static char bi_catname[] = "BuiltIn";
static char fifo_catname[] = "Remote";

/* list of duplicate names and their objects */
#define MAXDUPS         20      /* max duplicate names we will catch */
static DupName *dupnames;	/* malloced list, sorted by name */
static int ndupnames;		/* n entries used in dupnames[] */
static int mdupnames;		/* total entries malloced in dupnames[] */
static int ndupsorted;		/* n entries in dupnames[] that are sorted */

/* db fifo fd and XtAddInput id */
static int db_fifofd = -1;
static XtInputId db_fifoid;

/* return number of objects in the database.
 */
int
db_n()
{
	int i, t, n;

	if (!DBINITED)
	    db_init();

	for (n = i = 0; i < ndbcat; i++)
	    for (t = 0; t < NOBJTYPES; t++)
		n += dbcat[i].tmem[t].nuse;
	return (n + fav_get_loaded (NULL));
}

/* given one of the basic ids in astro.h return pointer to its
 * updated Obj in the database.
 */
Obj *
db_basic(id)
int id;
{
	Obj *op;

	if (!DBINITED)
	    db_init();

	if (id < 0 || id >= dbcat[0].tmem[PLANET].nuse) {
	    printf ("db_basic(): bad id: %d\n", id);
	    abort();
	}

	op = DBCatObj (&dbcat[0], PLANET, id);
	db_update(op);
	return (op);
}

/* load each file of objects listed in the DBinitialFiles resource
 * and inform all modules of the update.
 * support leading ~ and / else assume in ShareDir.
 */
void
db_loadinitial()
{
	char *fns;		/* value of DBinitialFiles */
	char *dbicpy;		/* local copy of dir */
	char *fnf[128];		/* ptrs into dbicpy[] at each ' ' */
	int nfn;
	int i;

	/* get the initial list of files, if any */
	fns = getXRes (dbifres, NULL);
	if (!fns)
	    return;

	/* work on a copy since we are about to break into fields */
	splashMsg ("Parsing data file names\n");
	dbicpy = XtNewString (fns);
	nfn = get_fields (dbicpy, ' ', fnf);
	if (nfn > XtNumber(fnf)) {
	    /* we exit because we've clobbered our stack by now!
	     * TODO: pass the size of fnf to get_fields().
	     */
	    printf ("Too many entries in %s. Max is %d\n", dbifres,
							    XtNumber(fnf));
	    abort();
	}

	/* read in each catalog.
	 * N.B. get_fields() will return 1 even if there are no fields.
	 */
	for (i = 0; i < nfn && fnf[i][0] != '\0'; i++) {
	    splashMsg ("Reading %s\n", fnf[i]);
	    db_read (fnf[i]);
	}

	/* all new */
	splashMsg ("Inform all systems of new data\n");
	all_newdb(0);
	XtFree (dbicpy);
}

/* make the current set of databases (except first for builtin objects) the
 * new default
 */
static void
db_setinitial()
{
	char buf[2048];
	int i, l;

	buf[0] = '\0';
	for (l = 0, i = 1; i < ndbcat; i++)
	    l += sprintf (buf+l, " %s", dbcat[i].name);

	setXRes (dbifres, buf);
}

/* return pointer to our duplicate names list.
 * N.B. caller must not modify
 */
int
db_dups (DupName **dnpp)
{
	*dnpp = dupnames;
	return (ndupnames);
}

/* delete the given catalog.
 * N.B. dbcp not valid on return
 */
void
db_catdel (dbcp)
DBCat *dbcp;
{
	int t, c;

	/* sanity check */
	if (dbcp < &dbcat[1] || dbcp >= &dbcat[ndbcat]) {
	    printf ("Bug! attempt to remove bogus catalog\n");
	    abort();
	}

	/* remove each dupname referring to this catalog's entries */
	dupdelcat (dbcp);

	/* reclaim mem used by dbcp then remove from dbcat */
	for (t = 0; t < NOBJTYPES; t++) {
	    DBTMem *dbtp = &dbcp->tmem[t];
	    if (dbtp->mem) {
		for (c = 0; c < dbtp->nmem/NDBCHNKO; c++)
		    free (dbtp->mem[c]);
		free ((char *)dbtp->mem);
	    }
	}

	memmove (dbcp, dbcp+1, (&dbcat[--ndbcat] - dbcp)*sizeof(DBCat));

	/* update GUI -- don't include builtin */
	db_newcatmenu (dbcat+1, ndbcat-1);
	db_setinitial();
}

/* given an object, return which catalog it is in, else NULL */
DBCat *
db_opfindcat (Obj *op)
{
	int t = op->o_type;
	int i;

	for (i = 0; i < ndbcat; i++) {
	    DBCat *dbcp = &dbcat[i];
	    int j, n = dbcp->tmem[t].nuse;

	    /* scan each chunk looking for op */
	    for (j = 0; j < n; j += NDBCHNKO) {
		Obj *oplo= DBCatObj(dbcp, t, j);
		Obj *ophi= DBCatObj(dbcp, t, j+NDBCHNKO>n ? n-1 : j+NDBCHNKO-1);
		if (oplo <= op && op <= ophi)
		    return (dbcp);
	    }
	}

	return (NULL);
}

/* search for a loaded catalog with the given name.
 * if find it return pointer to DBCat, else return NULL.
 */
static DBCat *
db_nmfindcat (name)
char *name;
{
	char *base;
	int i;

	/* find just the basename */
	while (*name == ' ')
	    name++;
	for (base = name+strlen(name);
		    base > name && base[-1] != '/' && base[-1] != '\\'; --base)
	    continue;

	for (i = 1; i < ndbcat; i++)
	    if (!strcmp (dbcat[i].name, base))
		return (&dbcat[i]);
	return (NULL);
}

/* allocate a new DBCat in dbcat[] and init with name, sorted by name but
 * leaving first for builtins.
 * name is already just the basename of a full path.
 * return pointer if ok, else NULL if no more memory.
 */
static DBCat *
db_catadd (name)
char *name;
{
	DBCat *dbcp;
	int i;

	/* make room for another */
	dbcp = (DBCat *) realloc (dbcat, (ndbcat+1)*sizeof(DBCat));
	if (!dbcp)
	    return (NULL);
	dbcat = dbcp;
	dbcp = &dbcat[ndbcat++];

	/* init */
	memset (dbcp, 0, sizeof(*dbcp));
	(void) sprintf (dbcp->name, "%.*s", (int)sizeof(dbcp->name)-1, name);
	dbcp->tmem[FIXED].siz = sizeof(ObjF);
	dbcp->tmem[BINARYSTAR].siz = sizeof(ObjB);
	dbcp->tmem[ELLIPTICAL].siz = sizeof(ObjE);
	dbcp->tmem[HYPERBOLIC].siz = sizeof(ObjH);
	dbcp->tmem[PARABOLIC].siz = sizeof(ObjP);
	dbcp->tmem[EARTHSAT].siz = sizeof(ObjES);
	dbcp->tmem[PLANET].siz = sizeof(ObjPl);

	/* sort by name, but leave built in at head */
	qsort (dbcat+1, ndbcat-1, sizeof(DBCat), db_catcmpf);

	/* find name again */
	for (i = 0; i < ndbcat; i++)
	    if (strcmp (dbcat[i].name, name) == 0)
		return (&dbcat[i]);

	/* eh?? */
	printf ("Bug! catalog disappeared after sorting: %s\n", name);
	abort();
	return (0);	/* for lint */
}

/* mark all db objects as out-of-date
 */
void
db_invalidate()
{
	if (!DBINITED)
	    db_init();

	db_age++;	/* ok if wraps */
}

/* initialize the given DBScan for a database scan. tmask is a collection of
 *   *M masks for the desired types of objects. op/nop describe a list of
 *   ObjF which will also be scanned in addition to what is in the database.
 * the idea is to call this once, then repeatedly call db_scan() to get all
 *   objects in the db of those types, then those in op (if any).
 * return NULL when there are no more.
 * N.B. nothing should be assumed as to the order these are returned.
 */
void
db_scaninit (sp, tmask, op, nop)
DBScan *sp;
int tmask;
ObjF *op;
int nop;
{
	if (!DBINITED)
	    db_init();

	sp->m = tmask;
	sp->t = UNDEFOBJ;
	sp->n = 0;
	sp->c = 0;
	sp->op = op;
	sp->nop = nop;
}

/* fetch the next object.
 * N.B. the s_ fields are *not* updated -- call db_update() when you need that.
 */
Obj *
db_scan (sp)
DBScan *sp;
{
	if (!DBINITED)
	    db_init();

	/* find next object, splicing in op list with FIXED */
	while (sp->c < ndbcat) {
	    while (sp->t < NOBJTYPES) {
		if (OBJTYPE2MASK(sp->t) & sp->m) {
		    if (sp->t == FIXED && sp->op) {
			/* return next in op list (all are FIXED) */
			if (sp->n < sp->nop)
			    return ((Obj*)&sp->op[sp->n++]);
			sp->op = NULL;	/* flag to turn off op list */
			sp->n = 0;
		    }
		    if (sp->n < dbcat[sp->c].tmem[sp->t].nuse) {
			/* return next in this catalog for this type */
			return (DBCatObj (&dbcat[sp->c], sp->t, sp->n++));
		    }
		}

		/* go on to next type */
		sp->t++;
		sp->n = 0;
	    }

	    /* go on to next catalog */
	    sp->c++;
	    sp->t = UNDEFOBJ;
	    sp->n = 0;
	}

	/* then any Favorites not also in db */
	return (fav_scan (&sp->n, sp->m));
}

/* see to it that all the s_* fields in the given db object are up to date.
 * N.B. it is ok to call this even if op is not actually in the database
 *   although we guarantee an actual update occurs if it's not.
 */
void
db_update(op)
Obj *op;
{
	static char me[] = "db_update()";

	if (!DBINITED)
	    db_init();

	if (op->o_type == UNDEFOBJ) {
	    printf ("%s: called with UNDEFOBJ pointer\n", me);
	    abort();
	} 
	if ((int)op->o_type >= NOBJTYPES) {
	    printf ("%s: called with bad pointer: %d\n", me, (int)op->o_type);
	    abort();
	} 

	if (op->o_age != db_age) {
	    if (obj_cir (mm_get_now(), op) < 0)
		xe_msg (0, "%s: no longer valid", op->o_name);
	    op->o_age = db_age;
	}
}

/* reload all loaded catalogs
 */
void
db_rel_all()
{
	char **curn = NULL;
	int i, n;

	if (!DBINITED)
	    db_init();

	/* gather all currently loaded names so they can be reloaded */
	n = ndbcat - 1;			/* sans first for builtins */
	curn = (char **) XtMalloc (n*sizeof(char *));
	for (i = 1; i < ndbcat; i++)
	    curn[i-1] = XtNewString(dbcat[i].name);

	/* reload all, free names as we go, then list itself */
	for (i = 0; i < n; i++) {
	    db_read (curn[i]);
	    XtFree (curn[i]);
	}
	XtFree ((char *)curn);
	
	/* spread the word */
	all_newdb(0);
}

/* delete all catalogs except the basic objects.
 */
void
db_del_all()
{
	int i, n;

	if (!DBINITED)
	    db_init();

	/* free each catalog */
	n = ndbcat;			/* save, decrements after each */
	for (i = 1; i < n; i++)
	    db_catdel (&dbcat[1]);
}

/* read the given .edb or .tle file into memory.
 * add (or replace) a new catalog entry, sorted by catalog name, update GUI.
 * stop gracefully if we run out of memory.
 * keep operator informed.
 * look in several places for file.
 * if enabled and only one object in whole file, add to Favrorites
 * N.B. caller is responsible for calling all_newdb().
 */
void
db_read (fn)
char *fn;
{
	char bufs[3][MAXDBLINE];
	char *brot, *b0 = bufs[0], *b1 = bufs[1], *b2 = bufs[2];
	int alts = db_chkAltNames();
	char fullfn[1024];
	DBCat *dbcp;
	Obj *newop = NULL;
	int nobjs, nnewobjs;
	Obj o;
	char *base;
	FILE *fp;
	long len;
	int fok;

	if (!DBINITED)
	    db_init();

	/* skip any leading blanks */
	while (*fn == ' ')
	    fn++;

	/* open the file.
	 * try looking for fn in several places.
	 */
	fp = fopenh (fn, "r");
	if (fp)
	    goto ok;
	sprintf (fullfn, "%s/%s", getPrivateDir(), fn);
	fp = fopenh (fullfn, "r");
	if (fp)
	    goto ok;
	sprintf (fullfn, "%s/catalogs/%s", getShareDir(), fn);
	fp = fopenh (fullfn, "r");
	if (fp)
	    goto ok;
	xe_msg (1, "%s:\n%s", fn, syserrstr());
	return;

	/* need pure base of fn */
    ok:
	for (base = fn+strlen(fn);
		    base > fn && base[-1] != '/' && base[-1] != '\\'; --base)
	    continue;

	/* set up to run the progress meter based on file position */
	(void) fseek (fp, 0L, SEEK_END);
	len = ftell (fp);
	(void) fseek (fp, 0L, SEEK_SET);
	pm_set (0);

	/* get a /fresh/ catalog entry */
	dbcp = db_nmfindcat(base);
	if (dbcp)
	    db_catdel(dbcp);
	dbcp = db_catadd(base);
	if (!dbcp) {
	    xe_msg (1, "No memory for new catalog");
	    fclose(fp);
	    return;
	}

	/* read each line from the file and add good ones to the db and dups */
	nobjs = nnewobjs = 0;
	memset (bufs, 0, sizeof(bufs));
	while (fgets (b2, MAXDBLINE, fp)) {
	    char nm[MAXDUPS][MAXNM];
	    char wn[1024];
	    int nnm;
	    pm_set ((int)(ftell(fp)*100/len)); /* update progress meter */
	    wn[0] = '\0';
	    if ((nnm=db_crack_line (b2,&o,nm,MAXDUPS,wn)) > 0
						    || !db_tle (b0,b1,b2,&o)) {
		nobjs++;
		if (nnm <= 0) {
		    /* must have found a tle entry */
		    nnm = 1;
		    strcpy (nm[0], o.o_name);
		}
		if (!is_type(&o,PLANETM) && (!alts || !dupchknm (nm, nnm))) {
		    if (!(newop = db_objadd (dbcp, &o))) {
			xe_msg (1, "No more memory");
			fclose(fp);
			db_catdel (dbcp);
			return;
		    }
		    dupaddnm (nm, alts ? nnm : 1, newop);
		    nnewobjs++;
		}
	    } else if (wn[0])
		xe_msg (0, "%s: %s", fn, wn);

	    /* rotate for possible TLE */
	    brot = b0;
	    b0 = b1;
	    b1 = b2;
	    b2 = brot;
	}

	/* clean up */
	fok = !ferror(fp);
	fclose(fp);

	/* check for trouble */
	if (!fok) {
	    xe_msg (1, "%s:\n%s", base, syserrstr());
	    db_catdel (dbcp);
	    return;
	}

	/* reject catalog if found nothing at all */
	if (nobjs == 0) {
	    db_catdel (dbcp);
	    xe_msg (1, "%s contains no data", base);
	    return;
	}

	xe_msg (0, "%s: contained %d new objects", base, nnewobjs);
	db_newcatmenu (dbcat+1, ndbcat-1);	/* sans builtin */
	db_setinitial();
	dupsort();

	/* auto add to Favorites if exactly 1 in catalog */
	if (nnewobjs == 1 && newop && db_load1())
	    fav_add (newop);
}

/* assuming we can open it ok, connect the db fifo to a callback.
 * we close and reopen each time we are called.
 */
void
db_connect_fifo()
{
	char fn[1024];

	/* close if currently open */
	if (db_fifofd >= 0) {
	    XtRemoveInput (db_fifoid);
	    (void) close (db_fifofd);
	    db_fifofd = -1;
	}

	/* open for read/write. this assures open will never block, that
	 * reads (and hence select()) WILL block if it's empty, and let's
	 * processes using it come and go as they please.
	 */
	(void) sprintf (fn,"%s/%s", getPrivateDir(), dbfifo);
	db_fifofd = openh (fn, O_RDWR);
	if (db_fifofd < 0) {
	    (void) sprintf (fn,"%s/%s", getShareDir(), dbfifo);
	    db_fifofd = openh (fn, O_RDWR);
	    if (db_fifofd < 0) {
		xe_msg (0, "%s: %s\n", fn, syserrstr());
		return;
	    }
	}

	/* wait for messages */
	db_fifoid = XtAppAddInput(xe_app, db_fifofd, (XtPointer)XtInputReadMask,
						    dbfifo_cb, (XtPointer)fn);
}

/* allocate *newop to the given catalog list, growing if necessary.
 * return new ptr if ok, NULL if no more memory.
 */
static Obj *
db_objadd (DBCat *dbcp, Obj *newop)
{
	DBTMem *dbtp = &dbcp->tmem[newop->o_type];
	int siz = dbtp->siz;

	/* add another chunk if no more room in current chunk */
	if (dbtp->nuse >= dbtp->nmem) {
	    int nchk = dbtp->nmem/NDBCHNKO;	/* n chunks now */
	    int ncl = (nchk+1)*sizeof(char*);	/* bytes in new chunk list */
	    char *newchk, *newcl;
	    
	    /* get new chunk and add one to chunk list */
	    newchk = malloc (NDBCHNKO*siz);
	    if (!newchk)
		return (NULL);
	    newcl = dbtp->mem ? realloc ((char*)dbtp->mem, ncl)
			      : malloc (ncl);
	    if (!newcl) {
		free (newchk);
		return (NULL);
	    }

	    /* put chunk on list and record more room */
	    dbtp->mem = (char**) newcl;
	    dbtp->mem[nchk] = newchk;
	    dbtp->nmem += NDBCHNKO;
	}

	/* copy newop to list in next available chunk */
	return(memcpy(DBCatObj(dbcp, newop->o_type, dbtp->nuse++), newop, siz));
}

/* set up the basic database.
 */
static void
db_init()
{
	char buf[256];
	char nm[1][MAXNM];
	DBCat *dbcp;
	Obj *biop;
	int n;
	int i;

	/* first catalog is for the builtin objects */
	dbcp = db_catadd (bi_catname);
	n = getBuiltInObjs (&biop);
	for (i = 0; i < n; i++) {
	    Obj *op = db_objadd (dbcp, &biop[i]);
	    strcpy (nm[0], op->o_name);
	    dupaddnm (nm, 1, op);
	}
	dupsort();

	/* inform subsystem where to find moon tables */
	sprintf (buf, "%s/auxil", getShareDir());
	setMoonDir (XtNewString(buf));	/* must provide perm storage */

	/* register the initial files list */
	sr_reg (0, dbifres, dbcategory, 1);
}

/* given a catalog, type and index, return pointer to object */
static Obj *
DBCatObj (DBCat *dbcp, int t, int i)
{
	DBTMem *dbtp = &dbcp->tmem[t];

	return ((Obj*)&dbtp->mem[(i)/NDBCHNKO][((i)%NDBCHNKO)*dbtp->siz]);
}

/* called whenever there is input readable from the db fifo.
 * read and crack what we can.
 * be prepared for partial lines split across reads.
 * N.B. do EXACTLY ONE read -- don't know that more won't block.
 * set the watch cursor while we work and call all_newdb() when we're done.
 *   we guess we are "done" when we end up without a partial line.
 */
/* ARGSUSED */
static void
dbfifo_cb (client, fdp, idp)
XtPointer client;       /* file name */
int *fdp;               /* pointer to file descriptor */
XtInputId *idp;         /* pointer to input id */
{
	static char partial[MAXDBLINE];	/* partial line from before */
	static int npartial;		/* length of stuff in partial[] */
	int alts = db_chkAltNames();	/* whether to check dup names */
	char buf[16*1024];		/* nice big read gulps */
	char *name = (char *)client;	/* fifo filename */
	int nr;				/* number of bytes read from fifo */

	/* turn on the watch cursor if there's no prior line */
	if (!npartial)
	    watch_cursor (1);

	/* catch up where we left off from last time */
	if (npartial)
	    (void) strcpy (buf, partial);

	/* read what's available up to the room we have left.
	 * if we have no room left, it will look like an EOF.
	 */
	nr = read (db_fifofd, buf+npartial, sizeof(buf)-npartial);

	if (nr > 0) {
	    char c, *lp, *bp, *ep;	/* last line, current, end */

	    /* process each whole line */
	    ep = buf + npartial + nr;
	    for (lp = bp = buf; bp < ep; ) {
		c = *bp++;
		if (c == '\n') {
		    bp[-1] = '\0';		      /* replace nl with EOS */
		    if (*lp == DBFIFO_MSG) {
			xe_msg (0, "DBFIFO message: %s", lp+1);
		    } else {
			Obj o;
			char nm[MAXDUPS][MAXNM];
			int nnm;
			if ((nnm=db_crack_line(lp,&o,nm,MAXDUPS,NULL)) < 0) {
			    xe_msg (0, "Bad DBFIFO line: %s", lp);
			} else if (!alts || !dupchknm (nm, nnm)) {
			    Obj *op;
			    if (is_type(&o, PLANETM)) {
				xe_msg (0,
				    "Planet %s ignored from DBFIFO",o.o_name);
			    } else {
				DBCat *dbcp = db_nmfindcat (fifo_catname);
				if (!dbcp)
				    dbcp = db_catadd (fifo_catname);
				if (!dbcp || !(op = db_objadd (dbcp, &o)))
				    xe_msg (0, "No more memory for DBFIFO");
				else {
				    dupaddnm (nm, alts ? nnm : 1, op);
				    dupsort();
				}
			    }
			}
		    }
		    lp = bp;
		}
	    }

	    /* save any partial line for next time */
	    npartial = ep - lp;
	    if (npartial > 0) {
		if (npartial > sizeof(partial)) {
		    xe_msg (0,"Discarding long line in %.100s.\n",name);
		    npartial = 0;
		} else {
		    *ep = '\0';
		    (void) strcpy (partial, lp);
		}
	    }

	} else {
	    if (nr < 0)
		xe_msg (1, "Error reading %.150s: %.50s.\n",
							name, syserrstr());
	    else 
		xe_msg (1, "Unexpected EOF on %.200s.\n", name);
	    XtRemoveInput (db_fifoid);
	    (void) close (db_fifofd);
	    db_fifofd = -1;
	    npartial = 0;
	}

	/* if there is not likely to be more coming inform everyone about all
	 * the new stuff and turn off the watch cursor.
	 */
	if (!npartial) {
	    all_newdb (1);
	    watch_cursor (0);
	}
}

/* compare 2 pointers to DBCat's by name in qsort fashion */
static int
db_catcmpf (const void *d1, const void *d2)
{
	return (strcmp (((DBCat *)d1)->name, ((DBCat *)d2)->name));
}

/* append nm's to the dup list, each pointing to op.
 * we increment ndupnames but not ndupsorted.
 * N.B. result is not sorted .. use dupsort() when finished
 */
static void
dupaddnm (char nm[][MAXNM], int nnm, Obj *op)
{
	int i;

	if (mdupnames < ndupnames + nnm)
	    dupnames = (DupName *) XtRealloc ((char *)dupnames,
				(mdupnames = ndupnames+10*nnm)*sizeof(DupName));

	for (i = 0; i < nnm; i++) {
	    strcpy (dupnames[ndupnames+i].nm, nm[i]);
	    dupnames[ndupnames+i].op = op;
	}

	ndupnames += nnm;
}

/* delete each dupnames[] IN PLACE that points to catalog dbcp.
 * N.B. we assume this will never be called to remove a builtin object
 */
static void
dupdelcat (DBCat *dbcp)
{
	int from, to;

	/* scan the dupnames list, copy over any entries pointing to dbcp */
	for (to = from = 0; from < ndupnames; from++) {
	    Obj *op = dupnames[from].op;
	    int t = op->o_type;
	    int n = dbcp->tmem[t].nuse;
	    int i;

	    /* see if op is within any chunk of type t in catalog dbcp */
	    for (i = 0; i < n; i += NDBCHNKO) {
		Obj *oplo= DBCatObj(dbcp, t, i);
		Obj *ophi= DBCatObj(dbcp, t, i+NDBCHNKO>n ? n-1 : i+NDBCHNKO-1);
		if (oplo <= op && op <= ophi)
		    break;	/* yes, this dupname refers to catalog dbcp */
	    }

	    /* keep if not in dbcp */
	    if (i >= n) {
		if (from > to)
		    memcpy (&dupnames[to], &dupnames[from], sizeof(DupName));
		to++;
	    }
	}

	/* new count */
	ndupnames = to;

	/* unsorted, if any, are at end of list */
	if (ndupnames < ndupsorted)
	    ndupsorted = ndupnames;

	/* cut back to ndupnames memory */
	dupnames = (DupName *) XtRealloc ((char *)dupnames,
				    (mdupnames = ndupnames)*sizeof(DupName));
}

/* check whether any of the nm[] names are already in the sorted portion of
 * dup list. if at least one is, add all the ones that aren't and return -1
 * else don't add anything and return 0 (meaning "no dups")
 */
static int
dupchknm (char nm[][MAXNM], int nnm)
{
	Obj *op = NULL;
	int i;

	for (i = 0; i < nnm; i++) {
	    /* binary search to find a matching dupname */
	    int l = 0;
	    int u = ndupsorted - 1;
	    int m = -1, diff = -1;

	    while (l <= u) {
		m = (l+u)/2;
		diff = strnncmp (nm[i], dupnames[m].nm);
		if (diff == 0)
		    break;		/* found dup */
		if (diff < 0)
		    u = m-1;
		else
		    l = m+1;
	    }

	    if (diff == 0) {
		/* found a matching dup, add all prev if first */
		if (!op) {
		    op = dupnames[m].op;
		    dupaddnm (nm, i, op);
		}
	    } else {
		/* no match, add if found a dup already */
		if (op)
		    dupaddnm (&nm[i], 1, op);
	    }
	}

	/* return 0 if found no matches */
	return (op ? -1 : 0);
}

/* compare pointers to two DupNames by nm, qsort-style */
static int
dup_qsort (const void *p1, const void *p2)
{
	return (strnncmp (((DupName*)p1)->nm, ((DupName*)p2)->nm));
}

/* sort dupnames[] by name */
static void
dupsort (void) 
{
	qsort (dupnames, ndupnames, sizeof(dupnames[0]), dup_qsort);
	ndupsorted = ndupnames;
}


/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: db.c,v $ $Date: 2010/01/18 01:47:02 $ $Revision: 1.46 $ $Name:  $"};
