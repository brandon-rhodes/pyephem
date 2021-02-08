#if 0
    liblilxml
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

/* little DOM-style XML parser.
 * only handles elements, attributes and pcdata content.
 * <! ... > and <? ... > are silently ignored.
 * pcdata is collected into one string, sans leading whitespace first line.
 *
 * #define MAIN_TST to create standalone test program
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lilxml.h"

/* used to efficiently manage growing malloced string space */
typedef struct {
    char *s;				/* malloced memory for string */
    int sl;				/* string length, sans trailing \0 */
    int sm;				/* total malloced bytes */
} String;
#define	MINMEM	64			/* starting string length */

static int oneXMLchar (LilXML *lp, int c, char errmsg[]);
static void initParser(LilXML *lp);
static void pushXMLEle(LilXML *lp);
static void popXMLEle(LilXML *lp);
static void resetEndTag(LilXML *lp);
static XMLAtt *growAtt(XMLEle *e);
static XMLEle *growEle(XMLEle *pe);
static void freeAtt (XMLAtt *a);
static int isTokenChar (int start, int c);
static void growString (String *sp, int c);
static void appendString (String *sp, char *str);
static void freeString (String *sp);
static void newString (String *sp);
static void *moremem (void *old, int n);

typedef enum  {
    LOOK4START = 0,			/* looking for first element start */
    LOOK4TAG,				/* looking for element tag */
    INTAG,				/* reading tag */
    LOOK4ATTRN,				/* looking for attr name, > or / */
    INATTRN,				/* reading attr name */
    LOOK4ATTRV,				/* looking for attr value */
    SAWSLASH,				/* saw / in element opening */
    INATTRV,				/* in attr value */
    ENTINATTRV,				/* in entity in attr value */
    LOOK4CON,				/* skipping leading content whitespc */
    INCON,				/* reading content */
    ENTINCON,				/* in entity in pcdata */
    SAWLTINCON,				/* saw < in content */
    LOOK4CLOSETAG,			/* looking for closing tag after < */
    INCLOSETAG				/* reading closing tag */
} State;				/* parsing states */

/* maintain state while parsing */
struct _LilXML {
    State cs;				/* current state */
    int ln;				/* line number for diags */
    XMLEle *ce;				/* current element being built */
    String endtag;			/* to check for match with opening tag*/
    String entity;			/* collect entity seq */
    int delim;				/* attribute value delimiter */
    int lastc;				/* last char (just used wiht skipping)*/
    int skipping;			/* in comment or declaration */
};

/* internal representation of a (possibly nested) XML element */
struct _xml_ele {
    String tag;				/* element tag */
    XMLEle *pe;				/* parent element, or NULL if root */
    XMLAtt **at;			/* list of attributes */
    int nat;				/* number of attributes */
    int ait;				/* used to iterate over at[] */
    XMLEle **el;			/* list of child elements */
    int nel;				/* number of child elements */
    int eit;				/* used to iterate over el[] */
    String pcdata;			/* character data in this element */
};

/* internal representation of an attribute */
struct _xml_att {
    String name;			/* name */
    String valu;			/* value */
    XMLEle *ce;				/* containing element */
};

/* default memory managers, override with xmlMalloc() */
static void *(*mymalloc)(size_t size) = malloc;
static void *(*myrealloc)(void *ptr, size_t size) = realloc;
static void (*myfree)(void *ptr) = free;

/* install new version of malloc/realloc/free.
 * N.B. don't call after first use of any other lilxml function
 */
void
xmlMalloc (void *(*newmalloc)(size_t size),
	   void *(*newrealloc)(void *ptr, size_t size),
	   void (*newfree)(void *ptr))
{
	mymalloc = newmalloc;
	myrealloc = newrealloc;
	myfree = newfree;
}

/* pass back a fresh handle for use with our other functions */
LilXML *
newLilXML ()
{
	LilXML *lp = (LilXML *) moremem (NULL, sizeof(LilXML));
	memset (lp, 0, sizeof(LilXML));
	initParser(lp);
	return (lp);
}

/* discard */
void
delLilXML (LilXML *lp)
{
	delXMLEle (lp->ce);
	freeString (&lp->endtag);
	(*myfree) (lp);
}

/* delete ep and all its children and remove from parent's list if known */
void
delXMLEle (XMLEle *ep)
{
	int i;

	/* benign if NULL */
	if (!ep)
	    return;

	/* delete all parts of ep */
	freeString (&ep->tag);
	freeString (&ep->pcdata);
	if (ep->at) {
	    for (i = 0; i < ep->nat; i++)
		freeAtt (ep->at[i]);
	    (*myfree) (ep->at);
	}
	if (ep->el) {
	    for (i = 0; i < ep->nel; i++) {
		/* forget parent so deleting doesn't modify _this_ el[] */
		ep->el[i]->pe = NULL;

		delXMLEle (ep->el[i]);
	    }
	    (*myfree) (ep->el);
	}

	/* remove from parent's list if known */
	if (ep->pe) {
	    XMLEle *pe = ep->pe;
	    for (i = 0; i < pe->nel; i++) {
		if (pe->el[i] == ep) {
		    memmove (&pe->el[i], &pe->el[i+1],
					      (--pe->nel-i)*sizeof(XMLEle*));
		    break;
		}
	    }
	}

	/* delete ep itself */
	(*myfree) (ep);
}

/* process one more character of an XML file.
 * when find closure with outter element return root of complete tree.
 * when find error return NULL with reason in errmsg[].
 * when need more return NULL with errmsg[0] = '\0'.
 * N.B. it is up to the caller to delete any tree returned with delXMLEle().
 */
XMLEle *
readXMLEle (LilXML *lp, int newc, char errmsg[])
{
	XMLEle *root;
	int s;

	/* start optimistic */
	errmsg[0] = '\0';

	/* EOF? */
	if (newc == 0) {
	    sprintf (errmsg, "Line %d: early XML EOF", lp->ln);
	    initParser(lp);
	    return (NULL);
	}

	/* new line? */
	if (newc == '\n')
	    lp->ln++;

	/* skip comments and declarations. requires 1 char history */
	if (!lp->skipping && lp->lastc == '<' && (newc == '?' || newc == '!')) {
	    lp->skipping = 1;
	    lp->lastc = newc;
	    return (NULL);
	}
	if (lp->skipping) {
	    if (newc == '>')
		lp->skipping = 0;
	    lp->lastc = newc;
	    return (NULL);
	}
	if (newc == '<') {
	    lp->lastc = '<';
	    return (NULL);
	}

	/* do a pending '<' first then newc */
	if (lp->lastc == '<') {
	    if (oneXMLchar (lp, '<', errmsg) < 0) {
		initParser(lp);
		return (NULL);
	    }
	    /* N.B. we assume '<' will never result in closure */
	}

	/* process newc (at last!) */
	s = oneXMLchar (lp, newc, errmsg);
	if (s == 0) {
	    lp->lastc = newc;
	    return (NULL);
	}
	if (s < 0) {
	    initParser(lp);
	    return (NULL);
	}

	/* Ok! return ce and we start over.
	 * N.B. up to caller to call delXMLEle with what we return.
	 */
	root = lp->ce;
	lp->ce = NULL;
	initParser(lp);
	return (root);
}

/* search ep for an attribute with given name.
 * return NULL if not found.
 */
XMLAtt *
findXMLAtt (XMLEle *ep, char *name)
{
	int i;

	for (i = 0; i < ep->nat; i++)
	    if (!strcmp (ep->at[i]->name.s, name))
		return (ep->at[i]);
	return (NULL);
}

/* search ep for an element with given tag.
 * return NULL if not found.
 */
XMLEle *
findXMLEle (XMLEle *ep, char *tag)
{
	int tl = strlen (tag);
	int i;

	for (i = 0; i < ep->nel; i++) {
	    String *sp = &ep->el[i]->tag;
	    if (sp->sl == tl && !strcmp (sp->s, tag))
		return (ep->el[i]);
	}
	return (NULL);
}

/* iterate over each child element of ep.
 * call first time with first set to 1, then 0 from then on.
 * returns NULL when no more or err
 */
XMLEle *
nextXMLEle (XMLEle *ep, int init)
{
	int eit;
	
	if (init)
	    ep->eit = 0;

	eit = ep->eit++;
	if (eit < 0 || eit >= ep->nel)
	    return (NULL);
	return (ep->el[eit]);
}

/* iterate over each attribute of ep.
 * call first time with first set to 1, then 0 from then on.
 * returns NULL when no more or err
 */
XMLAtt *
nextXMLAtt (XMLEle *ep, int init)
{
	int ait;

	if (init)
	    ep->ait = 0;

	ait = ep->ait++;
	if (ait < 0 || ait >= ep->nat)
	    return (NULL);
	return (ep->at[ait]);
}

/* return parent of given XMLEle */
XMLEle *
parentXMLEle (XMLEle *ep)
{
	return (ep->pe);
}

/* return parent element of given XMLAtt */
XMLEle *
parentXMLAtt (XMLAtt *ap)
{
	return (ap->ce);
}

/* access functions */

/* return the tag name of the given element */
char *
tagXMLEle (XMLEle *ep)
{
	return (ep->tag.s);
}

/* return the pcdata portion of the given element */
char *
pcdataXMLEle (XMLEle *ep)
{
	return (ep->pcdata.s);
}

/* return the number of characters in the pcdata portion of the given element */
int 
pcdatalenXMLEle (XMLEle *ep)
{
	return (ep->pcdata.sl);
}

/* return the nanme of the given attribute */
char *
nameXMLAtt (XMLAtt *ap)
{
	return (ap->name.s);
}

/* return the value of the given attribute */
char *
valuXMLAtt (XMLAtt *ap)
{
	return (ap->valu.s);
}

/* return the number of child elements of the given element */
int
nXMLEle (XMLEle *ep)
{
	return (ep->nel);
}

/* return the number of attributes in the given element */
int
nXMLAtt (XMLEle *ep)
{
	return (ep->nat);
}


/* search ep for an attribute with the given name and return its value.
 * return "" if not found.
 */
char *
findXMLAttValu (XMLEle *ep, char *name)
{
	XMLAtt *a = findXMLAtt (ep, name);
	return (a ? a->valu.s : "");
}

/* handy wrapper to read one xml file.
 * return root element else NULL with report in errmsg[]
 */
XMLEle *
readXMLFile (FILE *fp, LilXML *lp, char errmsg[])
{
	int c;

	while ((c = fgetc(fp)) != EOF) {
	    XMLEle *root = readXMLEle (lp, c, errmsg);
	    if (root || errmsg[0])
		return (root);
	}

	return (NULL);
}

/* add an element with the given tag to the given element.
 * parent can be NULL to make a new root.
 */
XMLEle *
addXMLEle (XMLEle *parent, char *tag)
{
	XMLEle *ep = growEle (parent);
	appendString (&ep->tag, tag);
	return (ep);
}

/* set the pcdata of the given element */
void
editXMLEle (XMLEle *ep, char *pcdata)
{
	freeString (&ep->pcdata);
	appendString (&ep->pcdata, pcdata);
}

/* add an attribute to the given XML element */
XMLAtt *
addXMLAtt (XMLEle *ep, char *name, char *valu)
{
	XMLAtt *ap = growAtt (ep);
	appendString (&ap->name, name);
	appendString (&ap->valu, valu);
	return (ap);
}

/* remove the named attribute from ep, if any */
void
rmXMLAtt (XMLEle *ep, char *name)
{
	int i;

	for (i = 0; i < ep->nat; i++) {
	    if (strcmp (ep->at[i]->name.s, name) == 0) {
		freeAtt (ep->at[i]);
		memmove (&ep->at[i],&ep->at[i+1],(--ep->nat-i)*sizeof(XMLAtt*));
		return;
	    }
	}
}

/* change the value of an attribute to str */
void
editXMLAtt (XMLAtt *ap, char *str)
{
	freeString (&ap->valu);
	appendString (&ap->valu, str);
}

/* sample print ep to fp
 * N.B. set level = 0 on first call
 */
#define	PRINDENT	4		/* sample print indent each level */
void
prXMLEle (FILE *fp, XMLEle *ep, int level)
{
	int indent = level*PRINDENT;
	int i;

	fprintf (fp, "%*s<%s", indent, "", ep->tag.s);
	for (i = 0; i < ep->nat; i++)
	    fprintf (fp, " %s=\"%s\"", ep->at[i]->name.s,
						entityXML(ep->at[i]->valu.s));
	if (ep->nel > 0) {
	    fprintf (fp, ">\n");
	    for (i = 0; i < ep->nel; i++)
		prXMLEle (fp, ep->el[i], level+1);
	}
	if (ep->pcdata.sl > 0) {
	    char *nl;
	    if (ep->nel == 0)
		fprintf (fp, ">\n");
	    /* indent if none or one line */
	    nl = strpbrk (ep->pcdata.s, "\n\r");
	    if (!nl || nl == &ep->pcdata.s[ep->pcdata.sl-1])
		fprintf (fp, "%*s", indent+PRINDENT, "");
	    fprintf (fp, "%s", entityXML(ep->pcdata.s));
	    if (!nl)
		fprintf (fp, "\n");
	}
	if (ep->nel > 0 || ep->pcdata.sl > 0)
	    fprintf (fp, "%*s</%s>\n", indent, "", ep->tag.s);
	else
	    fprintf (fp, "/>\n");
}

/* return a string with all xml-sensitive characters within the passed string s
 * replaced with their entity sequence equivalents.
 * N.B. caller must use the returned string before calling us again.
 */
char *
entityXML (char *s)
{
	static char entities[] = "&<>'\"";
	static char *malbuf;
	int nmalbuf = 0;
	char *sret;
	char *ep;

	/* scan for each entity, if any */
	for (sret = s; (ep = strpbrk (s, entities)) != NULL; s = ep+1) {

	    /* found another entity, copy preceding to malloced buffer */
	    int nnew = ep - s;			/* all but entity itself */
	    sret = malbuf = moremem (malbuf, nmalbuf + nnew + 10);
	    memcpy (malbuf+nmalbuf, s, nnew);
	    nmalbuf += nnew;

	    /* replace with entity encoding */
	    switch (*ep) {
	    case '&':
		nmalbuf += sprintf (malbuf+nmalbuf, "&amp;");
		break;
	    case '<':
		nmalbuf += sprintf (malbuf+nmalbuf, "&lt;");
		break;
	    case '>':
		nmalbuf += sprintf (malbuf+nmalbuf, "&gt;");
		break;
	    case '\'':
		nmalbuf += sprintf (malbuf+nmalbuf, "&apos;");
		break;
	    case '"':
		nmalbuf += sprintf (malbuf+nmalbuf, "&quot;");
		break;

	    }

	}

	/* return s if no entities, else malloc cleaned-up copy */
	if (sret == s) {
	    /* using s, so free any malloced memory from last time */
	    if (malbuf) {
		free (malbuf);
		malbuf = NULL;
	    }
	} else {
	    /* put remaining part of s into malbuf */
	    int nleft = strlen (s) + 1;		/* include \0 */
	    sret = malbuf = moremem (malbuf, nmalbuf + nleft);
	    memcpy (malbuf+nmalbuf, s, nleft);
	}

	return (sret);
}




/* if ent is a recognized xml entitty sequence, set *cp to char and return 1
 * else return 0
 */
static int
decodeEntity (char *ent, int *cp)
{
	static struct {
	    char *ent;
	    char c;
	} enttable[] = {
	    {"&amp;",  '&'},
	    {"&apos;", '\''},
	    {"&lt;",   '<'},
	    {"&gt;",   '>'},
	    {"&quot;", '"'},
	};
	int i;

	for (i = 0; i < sizeof(enttable)/sizeof(enttable[0]); i++) {
	    if (strcmp (ent, enttable[i].ent) == 0) {
		*cp = enttable[i].c;
		return (1);
	    }
	}
	
	return (0);
}

/* process one more char in XML file.
 * if find final closure, return 1 and tree is in ce.
 * if need more, return 0.
 * if real trouble, return -1 and put reason in errmsg.
 */
static int
oneXMLchar (LilXML *lp, int c, char errmsg[])
{
	switch (lp->cs) {
	case LOOK4START:		/* looking for first element start */
	    if (c == '<') {
		pushXMLEle(lp);
		lp->cs = LOOK4TAG;
	    }
	    /* silently ignore until resync */
	    break;

	case LOOK4TAG:			/* looking for element tag */
	    if (isTokenChar (1, c)) {
		growString (&lp->ce->tag, c);
		lp->cs = INTAG;
	    } else if (!isspace(c)) {
		sprintf (errmsg, "Line %d: Bogus tag char %c", lp->ln, c);
		return (-1);
	    }
	    break;
		
	case INTAG:			/* reading tag */
	    if (isTokenChar (0, c))
		growString (&lp->ce->tag, c);
	    else if (c == '>')
		lp->cs = LOOK4CON;
	    else if (c == '/')
		lp->cs = SAWSLASH;
	    else 
		lp->cs = LOOK4ATTRN;
	    break;

	case LOOK4ATTRN:		/* looking for attr name, > or / */
	    if (c == '>')
		lp->cs = LOOK4CON;
	    else if (c == '/')
		lp->cs = SAWSLASH;
	    else if (isTokenChar (1, c)) {
		XMLAtt *ap = growAtt(lp->ce);
		growString (&ap->name, c);
		lp->cs = INATTRN;
	    } else if (!isspace(c)) {
		sprintf (errmsg, "Line %d: Bogus leading attr name char: %c",
								    lp->ln, c);
		return (-1);
	    }
	    break;

	case SAWSLASH:			/* saw / in element opening */
	    if (c == '>') {
		if (!lp->ce->pe)
		    return(1);		/* root has no content */
		popXMLEle(lp);
		lp->cs = LOOK4CON;
	    } else {
		sprintf (errmsg, "Line %d: Bogus char %c before >", lp->ln, c);
		return (-1);
	    }
	    break;
		
	case INATTRN:			/* reading attr name */
	    if (isTokenChar (0, c))
		growString (&lp->ce->at[lp->ce->nat-1]->name, c);
	    else if (isspace(c) || c == '=')
		lp->cs = LOOK4ATTRV;
	    else {
		sprintf (errmsg, "Line %d: Bogus attr name char: %c", lp->ln,c);
		return (-1);
	    }
	    break;

	case LOOK4ATTRV:		/* looking for attr value */
	    if (c == '\'' || c == '"') {
		lp->delim = c;
		lp->cs = INATTRV;
	    } else if (!(isspace(c) || c == '=')) {
		sprintf (errmsg, "Line %d: No value for attribute %s", lp->ln,
					lp->ce->at[lp->ce->nat-1]->name.s);
		return (-1);
	    }
	    break;

	case INATTRV:			/* in attr value */
	    BADATTRENT:			/* come here if & but no ; then delim */
	    if (c == '&') {
		newString (&lp->entity);
		growString (&lp->entity, c);
		lp->cs = ENTINATTRV;
	    } else if (c == lp->delim)
		lp->cs = LOOK4ATTRN;
	    else if (!iscntrl(c))
		growString (&lp->ce->at[lp->ce->nat-1]->valu, c);
	    break;

	case ENTINATTRV:		/* working on entity in attr valu */
	    if (c == ';') {
		/* if find a recognized entity, add equiv char else raw seq */
		growString (&lp->entity, c);
		if (decodeEntity (lp->entity.s, &c))
		    growString (&lp->ce->at[lp->ce->nat-1]->valu, c);
		else
		    appendString(&lp->ce->at[lp->ce->nat-1]->valu,lp->entity.s);
		freeString (&lp->entity);
		lp->cs = INATTRV;
	    } else if (c == lp->delim) {
		/* saw & without ; */
		appendString(&lp->ce->at[lp->ce->nat-1]->valu,lp->entity.s);
		freeString (&lp->entity);
		lp->cs = INATTRV;
		goto BADATTRENT;
	    } else
		growString (&lp->entity, c);
	    break;

	case LOOK4CON:			/* skipping leading content whitespace*/
	    if (c == '<')
		lp->cs = SAWLTINCON;
	    else if (c == '&') {
		newString (&lp->entity);
		growString (&lp->entity, c);
		lp->cs = ENTINCON;
	    } else if (!isspace(c)) {
		growString (&lp->ce->pcdata, c);
		lp->cs = INCON;
	    }
	    break;

	case INCON:			/* reading content */
	    BADCONENT:			/* come here if see & but no ; then < */
	    if (c == '&') {
		newString (&lp->entity);
		growString (&lp->entity, c);
		lp->cs = ENTINCON;
	    } else if (c == '<') {
		/* found closure. if text contains a nl trim trailing blanks.
		 * chomp trailing nl if it's the only one.
		 */
		char *nl = strpbrk (lp->ce->pcdata.s, "\n\r");
		if (nl)
		    while (lp->ce->pcdata.sl > 0 &&
				lp->ce->pcdata.s[lp->ce->pcdata.sl-1] == ' ')
			lp->ce->pcdata.s[--(lp->ce->pcdata.sl)] = '\0';
		if (nl == &lp->ce->pcdata.s[lp->ce->pcdata.sl-1])
		    lp->ce->pcdata.s[--(lp->ce->pcdata.sl)] = '\0';/*safe!*/
		lp->cs = SAWLTINCON;
	    } else {
		growString (&lp->ce->pcdata, c);
	    }
	    break;

	case ENTINCON:			/* working on entity in content */
	    if (c == ';') {
		/* if find a recognized entity, add equiv char else raw seq */
		growString (&lp->entity, c);
		if (decodeEntity (lp->entity.s, &c))
		    growString (&lp->ce->pcdata, c);
		else
		    appendString(&lp->ce->pcdata, lp->entity.s);
		freeString (&lp->entity);
		lp->cs = INCON;
	    } else if (c == '<') {
		/* saw & without ; */
		appendString(&lp->ce->pcdata, lp->entity.s);
		freeString (&lp->entity);
		lp->cs = INCON;
		goto BADCONENT;
	    } else
		growString (&lp->entity, c);
	    break;

	case SAWLTINCON:		/* saw < in content */
	    if (c == '/') {
		resetEndTag(lp);
		lp->cs = LOOK4CLOSETAG;
	    } else {
		pushXMLEle(lp);
		if (isTokenChar(1,c)) {
		    growString (&lp->ce->tag, c);
		    lp->cs = INTAG;
		} else
		    lp->cs = LOOK4TAG;
	    }
	    break;

	case LOOK4CLOSETAG:		/* looking for closing tag after < */
	    if (isTokenChar (1, c)) {
		growString (&lp->endtag, c);
		lp->cs = INCLOSETAG;
	    } else if (!isspace(c)) {
		sprintf (errmsg, "Line %d: Bogus preend tag char %c", lp->ln,c);
		return (-1);
	    }
	    break;

	case INCLOSETAG:		/* reading closing tag */
	    if (isTokenChar(0, c))
		growString (&lp->endtag, c);
	    else if (c == '>') {
		if (strcmp (lp->ce->tag.s, lp->endtag.s)) {
		    sprintf (errmsg,"Line %d: closing tag %s does not match %s",
				    lp->ln, lp->endtag.s, lp->ce->tag.s);
		    return (-1);
		} else if (lp->ce->pe) {
		    popXMLEle(lp);
		    lp->cs = LOOK4CON;	/* back to content after nested elem */
		} else
		    return (1);		/* yes! */
	    } else if (!isspace(c)) {
		sprintf (errmsg, "Line %d: Bogus end tag char %c", lp->ln, c);
		return (-1);
	    }
	    break;
	}

	return (0);
}

/* set up for a fresh start again */
static void
initParser(LilXML *lp)
{
	delXMLEle (lp->ce);
	freeString (&lp->endtag);
	memset (lp, 0, sizeof(*lp));
	newString (&lp->endtag);
	lp->cs = LOOK4START;
	lp->ln = 1;
}

/* start a new XMLEle.
 * point ce to a new XMLEle.
 * if ce already set up, add to its list of child elements too.
 * endtag no longer valid.
 */
static void
pushXMLEle(LilXML *lp)
{
	lp->ce = growEle (lp->ce);
	resetEndTag(lp);
}

/* point ce to parent of current ce.
 * endtag no longer valid.
 */
static void
popXMLEle(LilXML *lp)
{
	lp->ce = lp->ce->pe;
	resetEndTag(lp);
}

/* return one new XMLEle, added to the given element if given */
static XMLEle *
growEle (XMLEle *pe)
{
	XMLEle *newe = (XMLEle *) moremem (NULL, sizeof(XMLEle));

	memset (newe, 0, sizeof(XMLEle));
	newString (&newe->tag);
	newString (&newe->pcdata);
	newe->pe = pe;

	if (pe) {
	    pe->el = (XMLEle **) moremem (pe->el, (pe->nel+1)*sizeof(XMLEle *));
	    pe->el[pe->nel++] = newe;
	}

	return (newe);
}

/* add room for and return one new XMLAtt to the given element */
static XMLAtt *
growAtt(XMLEle *ep)
{
	XMLAtt *newa = (XMLAtt *) moremem (NULL, sizeof(XMLAtt));

	memset (newa, 0, sizeof(*newa));
	newString(&newa->name);
	newString(&newa->valu);
	newa->ce = ep;

	ep->at = (XMLAtt **) moremem (ep->at, (ep->nat+1)*sizeof(XMLAtt *));
	ep->at[ep->nat++] = newa;

	return (newa);
}

/* free a and all it holds */
static void
freeAtt (XMLAtt *a)
{
	if (!a)
	    return;
	freeString (&a->name);
	freeString (&a->valu);
	(*myfree)(a);
}

/* reset endtag */
static void
resetEndTag(LilXML *lp)
{
	freeString (&lp->endtag);
	newString (&lp->endtag);
}

/* 1 if c is a valid token character, else 0.
 * it can be alpha or '_' or numeric unless start.
 */
static int
isTokenChar (int start, int c)
{
	return (isalpha(c) || c == '_' || (!start && isdigit(c)));
}

/* grow the String storage at *sp to append c */
static void
growString (String *sp, int c)
{
	int l = sp->sl + 2;		/* need room for '\0' plus c */

	if (l > sp->sm) {
	    if (!sp->s)
		newString (sp);
	    else
		sp->s = (char *) moremem (sp->s, sp->sm *= 2);
	}
	sp->s[--l] = '\0';
	sp->s[--l] = (char)c;
	sp->sl++;
}

/* append str to the String storage at *sp */
static void
appendString (String *sp, char *str)
{
	int strl = strlen (str);
	int l = sp->sl + strl + 1;	/* need room for '\0' */

	if (l > sp->sm) {
	    if (!sp->s)
		newString (sp);
	    if (l > sp->sm)
		sp->s = (char *) moremem (sp->s, (sp->sm = l));
	}
	strcpy (&sp->s[sp->sl], str);
	sp->sl += strl;		
}

/* init a String with a malloced string containing just \0 */
static void
newString(String *sp)
{
	sp->s = (char *)moremem(NULL, MINMEM);
	sp->sm = MINMEM;
	*sp->s = '\0';
	sp->sl = 0;
}

/* free memory used by the given String */
static void
freeString (String *sp)
{
	if (sp->s)
	    (*myfree) (sp->s);
	sp->s = NULL;
	sp->sl = 0;
	sp->sm = 0;
}

/* like malloc but knows to use realloc if already started */
static void *
moremem (void *old, int n)
{
	return (old ? (*myrealloc)(old, n) : (*mymalloc)(n));
}

#if defined(MAIN_TST)
int
main (int ac, char *av[])
{
	LilXML *lp = newLilXML();
	char errmsg[1024];
	XMLEle *root;

	root = readXMLFile (stdin, lp, errmsg);
	if (root) {
	    if (ac > 1) {
		XMLEle *theend = addXMLEle (root, "theend");
		editXMLEle (theend, "Added to test editing");
		addXMLAtt (theend, "hello", "world");
	    }

	    fprintf (stderr, "::::::::::::: %s\n", tagXMLEle(root));
	    prXMLEle (stdout, root, 0);
	    delXMLEle (root);
	} else if (errmsg[0]) {
	    fprintf (stderr, "Error: %s\n", errmsg);
	}

	delLilXML (lp);

	return (0);
}
#endif

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: lilxml.c,v $ $Date: 2007/01/03 07:26:12 $ $Revision: 1.44 $ $Name:  $"};
