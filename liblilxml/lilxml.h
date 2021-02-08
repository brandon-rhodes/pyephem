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

/* this is a little DOM-style library to handle parsing and processing an XML file.
 * it only handles elements, attributes and pcdata content.
 * <! ... > and <? ... > are silently ignored.
 * pcdata is collected into one string, sans leading whitespace first line.
 * see the end for example usage.
 */

#ifndef LILXML_H
#define LILXML_H

/* opaque handle types */
typedef struct _xml_att XMLAtt;
typedef struct _xml_ele XMLEle;
typedef struct _LilXML LilXML;

/* creation and destruction functions */
extern LilXML *newLilXML(void);
extern void delLilXML (LilXML *lp);
extern void delXMLEle (XMLEle *e);

/* process an XML one char at a time */
extern XMLEle *readXMLEle (LilXML *lp, int c, char errmsg[]);

/* search functions */
extern XMLAtt *findXMLAtt (XMLEle *e, char *name);
extern XMLEle *findXMLEle (XMLEle *e, char *tag);

/* iteration functions */
extern XMLEle *nextXMLEle (XMLEle *ep, int first);
extern XMLAtt *nextXMLAtt (XMLEle *ep, int first);

/* tree functions */
extern XMLEle *parentXMLEle (XMLEle *ep);
extern XMLEle *parentXMLAtt (XMLAtt *ap);

/* access functions */
extern char *tagXMLEle (XMLEle *ep);
extern char *pcdataXMLEle (XMLEle *ep);
extern char *nameXMLAtt (XMLAtt *ap);
extern char *valuXMLAtt (XMLAtt *ap);
extern int pcdatalenXMLEle (XMLEle *ep);
extern int nXMLEle (XMLEle *ep);
extern int nXMLAtt (XMLEle *ep);

/* editing functions */
extern XMLEle *addXMLEle (XMLEle *parent, char *tag);
extern void editXMLEle (XMLEle *ep, char *pcdata);
extern XMLAtt *addXMLAtt (XMLEle *ep, char *name, char *value);
extern void rmXMLAtt (XMLEle *ep, char *name);
extern void editXMLAtt (XMLAtt *ap, char *str);
extern char *entityXML (char *str);

/* convenience functions */
extern char *findXMLAttValu (XMLEle *ep, char *name);
extern XMLEle *readXMLFile (FILE *fp, LilXML *lp, char errmsg[]);
extern void prXMLEle (FILE *fp, XMLEle *e, int level);

/* install alternatives to malloc/realloc/free */
extern void xmlMalloc (void *(*newmalloc)(size_t size),
    void *(*newrealloc)(void *ptr, size_t size), void (*newfree)(void *ptr));


/* examples.

        initialize a lil xml context and read an XML file in a root element

	LilXML *lp = newLilXML();
	char errmsg[1024];
	XMLEle *root, *ep;
	int c;

	while ((c = fgetc(stdin)) != EOF) {
	    root = readXMLEle (lp, c, errmsg);
	    if (root)
		break;
	    if (errmsg[0])
		error ("Error: %s\n", errmsg);
	}
 
        print the tag and pcdata content of each child element within the root

        for (ep = nextXMLEle (root, 1); ep != NULL; ep = nextXMLEle (root, 0))
	    printf ("%s: %s\n", tagXMLEle(ep), pcdataXMLEle(ep));


	finished with root element and with lil xml context

	delXMLEle (root);
	delLilXML (lp);
 */

/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: lilxml.h,v $ $Date: 2006/05/27 17:37:34 $ $Revision: 1.21 $ $Name:  $
 */

#endif	/* LILXML_H */
