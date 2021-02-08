/* code to manage what the outside world sees as the db_ interface.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Intrinsic.h>

#include "xephem.h"

static int db_objadd (Obj *newop);
static void db_init (void);

#define	MAXDBLINE	256	/* longest allowed db line */

/* This counter is incremented when we want to mark all the Obj derived entries
 * as being out-of-date. This works because then each of the age's will be !=
 * db_age. This is to eliminate ever calling obj_cir() under the same
 * circumstances for a given db object.
 * N.B. For this to work, call db_update() not obj_cir().
 */
static ObjAge_t db_age = 1;	/* insure all objects are initially o-o-d */

/* the "database".
 * one such struct per object type. the space for objects is malloced in
 *   seperate chunks of DBCHUNK to avoid needing big contiguous memory blocks.
 *   the space is never freed, the total is just reduced and can be reused.
 * N.B. deleting by catalog depends on all objects from a given catalog being
 *   appended contiguously within their respective ObjType list.
 * N.B. because the number is fixed and known, we use static storage for the
 *   NOBJ Objects for the PLANET type, not malloced storage; see db_init().
 */
#define	DBCHUNK	256	/* number we malloc more of at once; a power of two
			 * might help the compiler optimize the divides and
			 * modulo arithmetic.
			 */
typedef struct {
    char **dblist;	/* malloced list of malloced DBCHUNKS arrays */
    int nobj;		/* number of objects actually in use */
    int nmem;		/* total number of objects for which we have room */
    int size;		/* bytes per object */
} DBMem;
static DBMem db[NOBJTYPES];	/* this is the head for each object */

/* return true if the database has been initialized */
#define	DBINITED	(db[PLANET].dblist)	/* anything setup by db_init()*/

/* macro that returns the address of an object given its type and index */
#define	OBJP(t,n)	\
		((Obj *)(db[t].dblist[(n)/DBCHUNK] + ((n)%DBCHUNK)*db[t].size))

#define DB_SIZE_ROUND(s)        \
                          (((s) + sizeof(double) - 1) & ~(sizeof(double) - 1))

static void db_free (DBMem *dmp);

/* return number of objects in the database.
 * this includes the NOBJ basic objects.
 * N.B. this is expected to be inexpensive to call.
 */
int
db_n()
{
	DBMem *dmp;
	int n;

	if (!DBINITED)
	    db_init();

	for (n = 0, dmp = db; dmp < &db[NOBJTYPES]; dmp++)
	    n += dmp->nobj;
	return (n);
}

/* given one of the basic ids in astro.h or circum.h return pointer to its
 * updated Obj in the database.
 */
Obj *
db_basic(id)
int id;
{
	Obj *op;

	if (!DBINITED)
	    db_init();

	if (id < 0 || id >= db[PLANET].nobj) {
	    printf ("db_basic(): bad id: %d\n", id);
	    exit (1);
	}

	op = OBJP(PLANET,id);
	if (op->o_type != UNDEFOBJ)
	    db_update(op);
	return (op);
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

/* initialize the given DBScan for a database scan. mask is a collection of
 *   *M masks for the desired types of objects. op/nop describe a list of
 *   ObjF which will also be scanned in addition to what is in the database.
 * the idea is to call this once, then repeatedly call db_scan() to get all
 *   objects in the db of those types, then those is op (if any).
 * return NULL when there are no more.
 * N.B. nothing should be assumed as to the order these are returned.
 */
void
db_scaninit (sp, mask, op, nop)
DBScan *sp;
int mask;
ObjF *op;
int nop;
{
	if (!DBINITED)
	    db_init();

	sp->t = UNDEFOBJ;
	sp->n = 0;
	sp->m = mask;
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

	/* find next object, splicing in ObjF list with FIXED */
	while (sp->t < NOBJTYPES) {
	    if ((1<<sp->t) & sp->m) {
		if (sp->t == FIXED && sp->op) {
		    if (sp->n < sp->nop)
			return ((Obj*)&sp->op[sp->n++]);
		    sp->op = NULL;		/* flag to turn off op list */
		    sp->n = 0;
		}
		/* return next for this type, skipping any UNDEF user objects */
		while (sp->n < db[sp->t].nobj) {
		    Obj *op = OBJP(sp->t, sp->n);	/* MACRO 2nd arg twice*/
		    sp->n++;
		    if (op->o_type != UNDEFOBJ)
			return (op);
		}
	    }
	    sp->t++;
	    sp->n = 0;
	}
	return (NULL);
}

/* see to it that all the s_* fields in the given db object are up to date.
 * always recompute the user defined objects because we don't know when
 * they might have been changed.
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
	    exit (1);
	} 
	if ((int)op->o_type >= NOBJTYPES) {
	    printf ("%s: called with bad pointer\n", me);
	    exit (1);
	} 

	if (op->o_age != db_age) {
	    if (obj_cir (mm_get_now(), op) < 0)
		xe_msg (0, "%s: no longer valid", op->o_name);
	    op->o_age = db_age;
	}
}

/* delete all but the basic objects.
 */
void
db_del_all()
{
	DBMem *dmp;

	if (!DBINITED)
	    db_init();

	/* free memory for each type */
	for (dmp = db; dmp < &db[NOBJTYPES]; dmp++) {
	    /* N.B. PLANET entries are fixed -- not malloced */
	    if (dmp == &db[PLANET])
		continue;	/* N.B. except planets! */
	    db_free (dmp);
	}
}

/* read the given .edb or .tle file into memory.
 * add a new catalog entry, sorted by catalog name, update GUI.
 * stop gracefully if we run out of memory.
 * keep operator informed.
 * look in several places for file.
 * if enabled and only one object in whole file, preload into an ObjXYZ.
 */
void
db_read (fn)
char *fn;
{
	char bufs[3][MAXDBLINE];
	char *brot, *b0 = bufs[0], *b1 = bufs[1], *b2 = bufs[2];
	int nobjs;
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

	/* read each line from the file and add good ones to the db */
	nobjs = 0;
	while (fgets (b2, MAXDBLINE, fp)) {
	    if (db_crack_line (b2,&o,0,0,0) > 0 || !db_tle (b0,b1,b2,&o)) {
		if (db_objadd (&o) < 0) {
		    xe_msg (1, "No more memory");
		    fclose(fp);
		    return;
		}
		nobjs++;
	    }

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
	    return;
	}

	/* record result */
	if (nobjs == 0)
	    xe_msg (1, "%s contains no data", base);
	else
	    xe_msg (0, "%s: contained %d objects", base, nobjs);
}

/* allocate *newop to the appropriate list, growing if necessary.
 * return 0 if ok, -1 if no more memory.
 * N.B we do *not* validate newop in any way.
 */
static int
db_objadd (newop)
Obj *newop;
{
	int t = newop->o_type;
	DBMem *dmp = &db[t];
	Obj *op;

	/* allocate another chunk if this type can't hold another one */
	if (dmp->nmem <= dmp->nobj) {
	    int ndbl = dmp->nmem/DBCHUNK;
	    int newdblsz = (ndbl + 1) * sizeof(char *);
	    char **newdbl;
	    char *newchk;

	    /* grow list of chunks */
	    newdbl = dmp->dblist ? (char **) realloc (dmp->dblist, newdblsz)
				 : (char **) malloc (newdblsz);
	    if (!newdbl)
		return (-1);

	    /* add 1 chunk */
	    newchk = malloc (dmp->size * DBCHUNK);
	    if (!newchk) {
		free ((char *)newdbl);
		return (-1);
	    }
	    newdbl[ndbl] = newchk;

	    dmp->dblist = newdbl;
	    dmp->nmem += DBCHUNK;
	}

	op = OBJP (t, dmp->nobj);
	dmp->nobj++;
	memcpy ((void *)op, (void *)newop, dmp->size);

	return (0);
}

/* set up the basic database.
 */
static void
db_init()
{
	static Obj *plan_dblist[1];

	/* init the object sizes.
	 * N.B. must do this before using the OBJP macro
	 */
	db[UNDEFOBJ].size	= 0;
	db[FIXED].size		= DB_SIZE_ROUND(sizeof(ObjF));
	db[BINARYSTAR].size	= DB_SIZE_ROUND(sizeof(ObjB));
	db[ELLIPTICAL].size	= DB_SIZE_ROUND(sizeof(ObjE));
	db[HYPERBOLIC].size	= DB_SIZE_ROUND(sizeof(ObjH));
	db[PARABOLIC].size	= DB_SIZE_ROUND(sizeof(ObjP));
	db[EARTHSAT].size	= DB_SIZE_ROUND(sizeof(ObjES));

	/* init access to the built-in objects
	 */
	db[PLANET].nmem = db[PLANET].nobj = getBuiltInObjs (&plan_dblist[0]);
	db[PLANET].dblist = (char **) plan_dblist;
	db[PLANET].size		= sizeof(Obj); /* *NOT* ObjPl */
}

/* free all the memory associated with the given DBMem */
static void
db_free (dmp)
DBMem *dmp;
{
	if (dmp->dblist) {
	    int i;
	    for (i = 0; i < dmp->nmem/DBCHUNK; i++)
		free (dmp->dblist[i]);
	    free ((char *)dmp->dblist);
	    dmp->dblist = NULL;
	}

	dmp->nobj = 0;
	dmp->nmem = 0;
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: db.c,v $ $Date: 2004/06/21 02:48:15 $ $Revision: 1.5 $ $Name:  $"};
