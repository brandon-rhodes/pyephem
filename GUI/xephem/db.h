#ifndef _DB_H
#define _DB_H

/* used to maintain progress state with db_scanint() and db_scan */
typedef struct {
    int m;	/* mask of *N types desired */
    int t;	/* current Object type, as per ObjType */
    int n;	/* number of objects of type t "scanned" so far */
    int c;	/* index of catalog being scanned */
    ObjF *op;	/* local list to also scan */
    int nop;	/* number in op[] */
} DBScan;

/* keep track of the indeces effected for each ObjType loaded by each catalog.
 * N.B. this works because all objects from a given catalog are contiguous
 *   within each their respective ObjType.
 */
#define	MAXCATNM	32	/* max catalog file name (just the base) */

/* collect objects in each catalog, and handy macro to address entry n type t.
 * objects are collected into small arrays called chunks. this eliminates
 * them moving around since putting them into one large array would require
 * using realloc and open the possibility they could move, and the smaller
 * pieces reduces memory bloat due to fragmentation.
 */
#define	NDBCHNKO	4096	/* objects for which mem is allocated at once */
typedef struct {
    int siz;			/* bytes in each object, sizeof(ObjE) etc */
    char **mem;			/* array of pointers to memory chunks */
    int nmem;			/* n objects available in all chunks */
    int nuse;			/* n objects actually in use */
} DBTMem;			/* all objects for a given o_type */
typedef struct {
    char name[MAXCATNM];	/* name of catalog */
    DBTMem tmem[NOBJTYPES];	/* memory for each type */
} DBCat;			/* all objects in a given catalog */


/* duplicate names are detected by keeping a list of each name and the Obj to
 * which it refers.
 */
typedef struct {
    char nm[MAXNM];
    Obj *op;
} DupName;

extern char dbcategory[];

extern DBCat *db_opfindcat (Obj *op);
extern void db_newcatmenu (DBCat a[], int na);
extern void db_catdel (DBCat *dbcp);
extern void db_del_all (void);
extern void db_rel_all (void);
extern int db_dups (DupName **dnpp);

extern int db_chkAltNames(void);

#endif /* _DB_H */
