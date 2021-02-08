#ifndef _INDIAPI_H
#define _INDIAPI_H

#if 0
    INDI
    Copyright (C) 2003 Elwood C. Downey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#endif

/*******************************************************************************
 * These are the Constants and Data structure definitions for the interface to
 * the reference INDI C API implementation. These may be used on both the
 * Client and Device side.
 */

/*******************************************************************************
 * INDI wire protocol version implemented by this API.
 * N.B. this is indepedent of the API itself.
 */

#define	INDIV	1.5

/*******************************************************************************
 * Manifest constants
 */

typedef enum {
    ISS_OFF, ISS_ON
} ISState;				/* switch state */

typedef enum {
    IPS_IDLE, IPS_OK, IPS_BUSY, IPS_ALERT
} IPState;				/* property state */

typedef enum {
    ISR_1OFMANY, ISR_ATMOST1, ISR_NOFMANY
} ISRule;				/* switch vector rule hint */

typedef enum {
    IP_RO, IP_WO, IP_RW
} IPerm;				/* permission hint, WRT client */

/* the XML strings for these attributes may be any length but implementations
 * are only obligued to support these lengths for the various string attributes.
 */
#define	MAXINDINAME	32
#define	MAXINDILABEL	32
#define	MAXINDIDEVICE	32
#define	MAXINDIGROUP	32
#define	MAXINDIFORMAT	32
#define	MAXINDIBLOBFMT	32
#define	MAXINDITSTAMP	32

/*******************************************************************************
 * Typedefs for each INDI Property type.
 *
 * INumber.format may be any printf-style appropriate for double
 * or style "m" to create sexigesimal using the form "%<w>.<f>m" where
 *   <w> is the total field width.
 *   <f> is the width of the fraction. valid values are:
 *      9  ->  :mm:ss.ss
 *      8  ->  :mm:ss.s
 *      6  ->  :mm:ss
 *      5  ->  :mm.m
 *      3  ->  :mm
 *
 * examples:
 *
 *   to produce     use
 *
 *    "-123:45"    %7.3m
 *  "  0:01:02"    %9.6m
 */

typedef struct {			/* one text descriptor */
    char name[MAXINDINAME];		/* index name */
    char label[MAXINDILABEL];		/* short description */
    char *text;				/* malloced text string */
    struct _ITextVectorProperty *tvp;	/* pointer to parent */
    void *aux0, *aux1;			/* handy place to hang helper info */
} IText;

typedef struct _ITextVectorProperty {	/* text vector property descriptor */
    char device[MAXINDIDEVICE];		/* device name */
    char name[MAXINDINAME];		/* property name */
    char label[MAXINDILABEL];		/* short description */
    char group[MAXINDIGROUP];		/* GUI grouping hint */
    IPerm p;				/* client accessibility hint */
    double timeout;			/* current max time to change, secs */
    IPState s;				/* current property state */
    IText *tp;				/* texts comprising this vector */
    int ntp;				/* dimension of tp[] */
    char timestamp[MAXINDITSTAMP];	/* ISO 8601 timestamp of this event */
    void *aux;				/* handy place to hang helper info */
} ITextVectorProperty;

typedef struct {			/* one number descriptor */
    char name[MAXINDINAME];		/* index name */
    char label[MAXINDILABEL];		/* short description */
    char format[MAXINDIFORMAT];		/* GUI display format, see above */
    double min, max;			/* range, ignore if min == max */
    double step;			/* step size, ignore if step == 0 */
    double value;			/* current value */
    struct _INumberVectorProperty *nvp;	/* pointer to parent */
    void *aux0, *aux1;			/* handy place to hang helper info */
} INumber;

typedef struct _INumberVectorProperty {	/* number vector property descriptor */
    char device[MAXINDIDEVICE];		/* device name */
    char name[MAXINDINAME];		/* property name */
    char label[MAXINDILABEL];		/* short description */
    char group[MAXINDIGROUP];		/* GUI grouping hint */
    IPerm p;				/* client accessibility hint */
    double timeout;			/* current max time to change, secs */
    IPState s;				/* current property state */
    INumber *np;			/* numbers comprising this vector */
    int nnp;				/* dimension of np[] */
    char timestamp[MAXINDITSTAMP];	/* ISO 8601 timestamp of this event */
    void *aux;				/* handy place to hang helper info */
} INumberVectorProperty;

typedef struct {			/* one switch descriptor */
    char name[MAXINDINAME];		/* index name */
    char label[MAXINDILABEL];		/* this switch's label */
    ISState s;				/* this switch's state */
    struct _ISwitchVectorProperty *svp;	/* pointer to parent */
    void *aux;				/* handy place to hang helper info */
} ISwitch;

typedef struct _ISwitchVectorProperty {	/* switch vector property descriptor */
    char device[MAXINDIDEVICE];		/* device name */
    char name[MAXINDINAME];		/* property name */
    char label[MAXINDILABEL];		/* short description */
    char group[MAXINDIGROUP];		/* GUI grouping hint */
    IPerm p;				/* client accessibility hint */
    ISRule r;				/* switch behavior hint */
    double timeout;			/* current max time to change, secs */
    IPState s;				/* current property state */
    ISwitch *sp;			/* switches comprising this vector */
    int nsp;				/* dimension of sp[] */
    char timestamp[MAXINDITSTAMP];	/* ISO 8601 timestamp of this event */
    void *aux;				/* handy place to hang helper info */
} ISwitchVectorProperty;

typedef struct {			/* one light descriptor */
    char name[MAXINDINAME];		/* index name */
    char label[MAXINDILABEL];		/* this lights's label */
    IPState s;				/* this lights's state */
    struct _ILightVectorProperty *lvp;	/* pointer to parent */
    void *aux;				/* handy place to hang helper info */
} ILight;

typedef struct _ILightVectorProperty {	/* light vector property descriptor */
    char device[MAXINDIDEVICE];		/* device name */
    char name[MAXINDINAME];		/* property name */
    char label[MAXINDILABEL];		/* short description */
    char group[MAXINDIGROUP];		/* GUI grouping hint */
    IPState s;				/* current property state */
    ILight *lp;				/* lights comprising this vector */
    int nlp;				/* dimension of lp[] */
    char timestamp[MAXINDITSTAMP];	/* ISO 8601 timestamp of this event */
    void *aux;				/* handy place to hang helper info */
} ILightVectorProperty;

typedef struct {			/* one BLOB descriptor */
    char name[MAXINDINAME];		/* index name */
    char label[MAXINDILABEL];		/* this BLOB's label */
    char format[MAXINDIBLOBFMT];	/* format attr */
    void *blob;				/* malloced binary large object bytes */
    int bloblen;			/* bytes in blob */
    int size;				/* n uncompressed bytes */
    struct _IBLOBVectorProperty *bvp;	/* pointer to parent */
    void *aux0, *aux1, *aux2;		/* handy place to hang helper info */
} IBLOB;

typedef struct _IBLOBVectorProperty {	/* BLOB vector property descriptor */
    char device[MAXINDIDEVICE];		/* device name */
    char name[MAXINDINAME];		/* property name */
    char label[MAXINDILABEL];		/* short description */
    char group[MAXINDIGROUP];		/* GUI grouping hint */
    IPerm p;				/* client accessibility hint */
    double timeout;			/* current max time to change, secs */
    IPState s;				/* current property state */
    IBLOB *bp;				/* BLOBs comprising this vector */
    int nbp;				/* dimension of bp[] */
    char timestamp[MAXINDITSTAMP];	/* ISO 8601 timestamp of this event */
    void *aux;				/* handy place to hang helper info */
} IBLOBVectorProperty;

/*******************************************************************************
 * Handy macro to find the number of elements in array a[].
 * N.B. must be used with actual array, not pointer.
 */

#define NARRAY(a)       (sizeof(a)/sizeof(a[0]))

#endif /* _INDIAPI_H */
