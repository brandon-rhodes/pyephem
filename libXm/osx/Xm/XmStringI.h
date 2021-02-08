/* 
 *  @OPENGROUP_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  Copyright (c) 1996, 1997, 1998, 1999, 2000 The Open Group
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
 *  
 *  This software is subject to an open license. It may only be
 *  used on, with or for operating systems which are themselves open
 *  source systems. You must contact The Open Group for a license
 *  allowing distribution and sublicensing of this software on, with,
 *  or for operating systems which are not Open Source programs.
 *  
 *  See http://www.opengroup.org/openmotif/license for full
 *  details of the license agreement. Any use, reproduction, or
 *  distribution of the program constitutes recipient's acceptance of
 *  this agreement.
 *  
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, THE PROGRAM IS
 *  PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 *  WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY
 *  OR FITNESS FOR A PARTICULAR PURPOSE
 *  
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT
 *  NOR ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED
 *  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE PROGRAM OR THE
 *  EXERCISE OF ANY RIGHTS GRANTED HEREUNDER, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGES.
*/ 
/* 
 * HISTORY
*/ 
/*   $TOG: XmStringI.h /main/7 1999/09/01 17:15:15 mgreess $ */

#ifndef _XmStringI_h
#define _XmStringI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * These are the fontlist structures
 */

typedef struct _XmFontListContextRec
{
  Boolean		error;			  /* something bad */
  unsigned short	index;			  /* next rendition */
  XmRenderTable		table;			  /* associated table */
} XmFontListContextRec;

/* useful macros */
#define two_byte_font(f)        (( (f)->min_byte1 != 0 || (f)->max_byte1 != 0))

#define  FontListType(r)		((_XmRendition)(*(r)))->fontType
#define  FontListFont(r)		((_XmRendition)(*(r)))->font
#define  FontListTag(r)			((_XmRendition)(*(r)))->tag

/* Convenience macros */
#define _XmStringCvtNonOpt(str)		(_XmStrOptimized(str) ? \
					 _XmStringOptToNonOpt(str) : (str))

/*
 * Macros for string internal context block data structure access
 */

#define _XmStrContString(cont)	  \
                 ((_XmStringContextRec *)(cont))->string
#define _XmStrContCurrLine(cont)  \
                 ((_XmStringContextRec *)(cont))->current_line
#define _XmStrContCurrSeg(cont)   \
                 ((_XmStringContextRec *)(cont))->current_seg
#define _XmStrContOpt(cont)	  \
                 ((_XmStringContextRec *)(cont))->optimized
#define _XmStrContError(cont)	  ((_XmStringContextRec *)(cont))->error
#define _XmStrContTabCount(cont)  ((_XmStringContextRec *)(cont))->tab_count
#define _XmStrContDir(cont) 	  ((_XmStringContextRec *)(cont))->dir
#define _XmStrContTag(cont) 	  ((_XmStringContextRec *)(cont))->tag
#define _XmStrContTagType(cont)	  ((_XmStringContextRec *)(cont))->tag_type
#define _XmStrContState(cont) 	  ((_XmStringContextRec *)(cont))->state
#define _XmStrContRendTags(cont)  ((_XmStringContextRec *)(cont))->rend_tags
#define _XmStrContRendCount(cont) ((_XmStringContextRec *)(cont))->rend_count
#define _XmStrContRendIndex(cont) ((_XmStringContextRec *)(cont))->rend_index
#define _XmStrContTmpStrDir(cont) ((_XmStringContextRec *)(cont))->tmp_str_dir
#define _XmStrContTmpDir(cont)	  ((_XmStringContextRec *)(cont))->tmp_dir

/*
 * internal context data block, for read-out
 */
enum 
{
  PUSH_STATE, BEGIN_REND_STATE, TAG_STATE, TAB_STATE, DIR_STATE,  
  TEXT_STATE, END_REND_STATE, POP_STATE, SEP_STATE
};

typedef struct __XmStringContextRec
{
    _XmString   	string;		/* pointer to internal string	*/
    short       	current_line;	/* index of current line	*/
    unsigned short      current_seg;	/* index of current segment	*/
    Boolean     	optimized;      /* is string optimized		*/
    Boolean     	error;          /* something wrong		*/
    short		tab_count;	/* tabs processed		*/
    XmStringDirection	dir;		/* last direction		*/
    XmStringTag		tag;		/* last tag seen		*/
    XmTextType		tag_type;	/* type of last tag seen	*/
    char		state;		/* current state of output	*/
    XmStringTag        *rend_tags;	/* active renditions.		*/
    short               rend_count;	/* number of rend_tags.		*/
    short		rend_index;	/* renditions processed		*/
    XmDirection		tmp_dir;	/* temporary storage            */
    XmStringDirection	tmp_str_dir;	/* temporary storage            */
} _XmStringContextRec;

/*
 * Internal representation of an XmParseMapping.
 */
typedef struct __XmParseMappingRec {
  XtPointer		pattern;
  XmTextType		pattern_type;
  XmString		substitute;
  XmParseProc		parse_proc;
  XtPointer		client_data;
  XmIncludeStatus	include_status;
  unsigned char		internal_flags;	/* reserved for unparse data */
} _XmParseMappingRec, *_XmParseMapping;

/****************************************************************
 * Symbolic values
 ****************************************************************/

#define XmSTRING_OPTIMIZED			0x0
#define XmSTRING_OTHER				0x1
#define XmSTRING_MULTIPLE_ENTRY	 		0x2

#define TAG_INDEX_BITS  	3
#define REND_INDEX_BITS		4
#define BYTE_COUNT_BITS		8
#define TEXT_BYTES_IN_STRUCT	1

#define TAG_INDEX_UNSET		((1 << TAG_INDEX_BITS) - 1)
#define TAG_INDEX_MAX		TAG_INDEX_UNSET
#define REND_INDEX_UNSET	((1 << REND_INDEX_BITS) - 1)
#define REND_INDEX_MAX		REND_INDEX_UNSET

/****************************************************************

  XmStringOpt is an optimized string containing text of less than
  256 bytes with an associated string direction and up to three 
  implicit tabs.
  
  The text is stored immediately after the header within the string.

 ****************************************************************/

typedef struct __XmStringOptHeader {	
  unsigned int type        : 2;	     /* XmSTRING_OPTIMIZED */
  unsigned int text_type   : 2;	     /* MB, WC, locale or charset text.*/
  unsigned int tag_index   : TAG_INDEX_BITS;	/* index into charset cache */
  unsigned int rend_begin  : 1;	     /* flag for RENDITION_BEGIN */
  unsigned char byte_count;	     /* size of text in this seg.*/
  unsigned int rend_end    : 1;	     /* flag for RENDITION_END */
  unsigned int rend_index  : REND_INDEX_BITS;	/* index in tag cache */
  unsigned int str_dir     : 2;      /* string direction set by app */
  unsigned int flipped     : 1;      /* whether the text has been flipped */
  unsigned int tabs	   : 2;	     /* number of tabs preceding the text */
  unsigned int refcount    : 6;      /* reference count */ 
} _XmStringOptHeader;

typedef struct __XmStringOpt {	
  _XmStringOptHeader header;
  char               text[TEXT_BYTES_IN_STRUCT];
} _XmStringOptRec, *_XmStringOpt;

/****************************************************************

  XmStringMulti specifies a string consisting of multiple entries.
  Each entry is a segment, either an optimized single segment, an
  unoptimized segment, or an array of segments.
  
  If implicit_line is 1, each entry is treated as a single line for
  display and other purposes.
  
  If implicit_line is 0, the string is a sequence of entries, treated as
  being on one line.
  
 ****************************************************************/

/* Forward definitions */
typedef union __XmStringEntryRec *_XmStringEntry;
typedef union __XmStringNREntryRec *_XmStringNREntry;

typedef struct __XmStringMultiHeader {
  unsigned int	type          : 2;	/* XmSTRING_MULTIPLE_ENTRY */
  unsigned int	implicit_line : 1;	/* 1 => linefeed at end */
  unsigned int	entry_count   : 21;	
  unsigned char	refcount;		
} _XmStringMultiHeader;

typedef struct __XmStringMulti{	
  _XmStringMultiHeader	header;
  _XmStringEntry      * entry;	/* pointer to array of pointers to entries */
} _XmStringMultiRec, *_XmStringMulti;

typedef struct __XmStringEmptyHeader {
  unsigned int	type          : 2;	
} _XmStringEmptyHeader;

typedef union __XmStringRec {	
  _XmStringEmptyHeader  empty;
  _XmStringOptHeader	opt_str;	/* XmSTRING_OPTIMIZED */
  XtPointer		component;	/* unused */
  _XmStringMultiHeader	multi_str;      /* XmSTRING_MULTIPLE_ENTRY */
} _XmStringRec;



/****************************************************************

  Cache data structures

 ****************************************************************/
#define _XmSCANNING_CACHE  0
#define _XmRENDERING_CACHE 1
/* #define _XmHIGHLIGHT_CACHE 2 */
/* #define _XmCSTEXT_CACHE    3 */

/*
 * Header
 */
typedef struct __XmStringCacheRec {
  struct __XmStringCacheRec * next;
  unsigned char               cache_type;/* only 255 cache types supported */
  Boolean		      dirty;	 /* 1 => recompute this cache */
}  _XmStringCacheHeader, *_XmStringCache;

/*
 * Scanning cache
 */
typedef struct __XmStringScanning {
  _XmStringCacheHeader header;           /* cache_type == _XmSCANNING_CACHE */
  /* Matching fields */
  XmDirection          prim_dir;         /* primary layout direction */

  /* Cached data */
  _XmStringEntry       left;	         /* leftward segment in string */
  _XmStringEntry       right;	         /* rightward segment in string */
  XmDirection          layout_direction; /* current segment layout direction */
  unsigned short       depth;		 /* depth of layout push */
} _XmStringScanningRec, *_XmStringScanningCache;

/*
 * Rendering cache
 */
typedef struct __XmStringRendering {
  _XmStringCacheHeader header;           /* cache_type == _XmRENDERING_CACHE */
  /* Matching fields */
  XmRenderTable        rt;  

  /* Cached data */
  int  	               x;	         /* x pos of segment */
  int                  y;	         /* y pos of segment */
  int  	               width;	         /* width of segment */
  int                  height;	         /* height of segment */
  int		       ascent;		 /* ascent of segment */
  int		       descent;		 /* descent of segment */
  int                  baseline;	 /* baseline of segment */
  XmRendition          rendition;        /* Rendition used for this segment */
  char                 prev_tabs;        /* accumulates tabs on line */
} _XmStringRenderingRec, *_XmStringRenderingCache;


/****************************************************************

  Optimized segment definition. 

 ****************************************************************/

typedef struct __XmStringOptSegHdrRec {	
  unsigned int type            : 2;  /* XmSTRING_ENTRY_OPTIMIZED */
  unsigned int text_type       : 2;  /* MB, WC, locale or charset */
  unsigned int tag_index       : TAG_INDEX_BITS; /* index into charset cache */
  unsigned int rend_begin      : 1;  /* flag for RENDITION_BEGIN */
  unsigned char byte_count;
  unsigned int rend_end        : 1;  /* flag for RENDITION_END */
  unsigned int rend_index : REND_INDEX_BITS; /* index in rendition tag cache */
  unsigned int str_dir         : 2;  /* Direction of text in segment */
  unsigned int flipped         : 1;  /* 1 => data is character-flipped */
  unsigned int tabs_before     : 3;  /* number of preceding tabs */    
  unsigned int permanent       : 1;  /* 0 => Pointer data can be freed */
  unsigned int soft_line_break : 1;  /* linebreak before is soft */
  unsigned int immediate       : 1;  /* 0 => data is immediate */
  unsigned int pad             : 2;
} _XmStringOptSegHdrRec;

typedef struct __XmStringOptSegRec {
  _XmStringOptSegHdrRec header;
  union {	
    wchar_t		wchars[1];
    unsigned char	chars[1];
    XtPointer	        text;
  } data;
} _XmStringOptSegRec, *_XmStringOptSeg;

/****************************************************************

  Array 'segment' definition. 

 ****************************************************************/

typedef struct __XmStringArraySegHdrRec {	
  unsigned int type            : 2;  /* XmSTRING_ENTRY_ARRAY */
  unsigned int soft_line_break : 1;  /* linebreak before is soft */
  unsigned int pad             : 5; 
  unsigned int segment_count   : 8;  /* 256 segments per line */
  unsigned char pad2byte[2];
} _XmStringArraySegHdrRec;

typedef struct __XmStringArraySegRec {	
  _XmStringArraySegHdrRec header;
  _XmStringNREntry	* seg;	      /* array of pointers to segments */
} _XmStringArraySegRec, *_XmStringArraySeg;


/****************************************************************

  Unoptimized segment definition. 

 ****************************************************************/

typedef struct __XmStringUnoptSegHdrRec {
  unsigned int	type            : 2;  /* XmSTRING_ENTRY_UNOPTIMIZED */
  unsigned int  soft_line_break : 1;  /* linebreak before is soft */
  unsigned int	permanent       : 1;  /* 0 => Pointer data can be freed */
  unsigned int	pop_after       : 1;  /* whether a pop follows the text */
  unsigned int	str_dir         : 2;  /* Direction of text in segment */
  unsigned int  flipped         : 1;  /* 1 => data is character-flipped */
  XmDirection	push_before;	      /* if NULL => no push */
  unsigned char	tabs_before;	      /* Number of tabs preceding segment */
  XmTextType	text_type;	      /* determines type of text and tag */
}  _XmStringUnoptSegHdrRec;

typedef struct __XmStringUnoptSegRec {
  _XmStringUnoptSegHdrRec header;
  union {
    wchar_t		* wchars;
    unsigned char	* chars;
    XtPointer             text;		   /* pointer to text. */
  } data;			           /* To conform to opt. segment */
  unsigned char           begin_count;	   /* count of rendition tag begins */
  unsigned char	          end_count;	   /* count of rendition tag ends */
  XmStringTag	        * rend_begin_tags; /* list of rendition tag begins */
  XmStringTag	        * rend_end_tags;   /* list of rendition tag ends */
  XmStringTag	          tag;		   /* locale or charset tag */
  unsigned int	          byte_count;	   /* byte count for this segment */
  unsigned int	          char_count;	   /* character count */
  _XmStringCache          cache;
}  _XmStringUnoptSegRec, *_XmStringUnoptSeg;

/****************************************************************

  XmStringEntry specifies the different 'segments' that can be part 
  of a multiple-entry XmString. 

  These entries can be an optimized segment, an segment with cache,
  also optimized, an unoptimized segment or an array of segments. 

  The array entry type can contain any of the three other entry types, 
  but not an array entry. This is because we do not want to handle 
  recursive XmStrings.  

 ****************************************************************/

#define XmSTRING_ENTRY_OPTIMIZED		0x0
#define XmSTRING_ENTRY_UNOPTIMIZED		0x1
/* #define XmSTRING_ENTRY_OPTIMIZED_CACHE  	0x2 */
#define XmSTRING_ENTRY_ARRAY			0x3

typedef union __XmStringEntryRec {	
  _XmStringEmptyHeader         empty;
  _XmStringOptSegHdrRec	       single;	       /* XmSTRING_ENTRY_OPTIMIZED */
  _XmStringUnoptSegHdrRec      unopt_single;   /* XmSTRING_ENTRY_UNOPTIMIZED */
  _XmStringArraySegHdrRec      multiple;       /* XmSTRING_ENTRY_ARRAY */
} _XmStringEntryRec;

/***************************************************************

  _XmStringNREntry: 
  Used in the XmStringArraySeg, to prevent recursive 
  definitions of XmStrings.

 ****************************************************************/

typedef union __XmStringNREntryRec {	
  _XmStringEmptyHeader         empty;
  _XmStringOptSegHdrRec	       single;		/* XmSTRING_ENTRY_OPTIMIZED */
  _XmStringUnoptSegHdrRec      unopt_single; 	/* XmSTRING_ENTRY_UNOPTIMIZED */
} _XmStringNREntryRec;

/****************************************************************
 *
 * Typedefs for old structures 
 *
 ****************************************************************/

typedef struct __XmStringUnoptSegRec _XmStringSegmentRec;
typedef struct __XmStringUnoptSegRec *_XmStringSegment;

typedef struct __XmStringArraySegRec _XmStringLineRec;
typedef struct __XmStringArraySegRec *_XmStringLine;

/****************************************************************
 *
 * Macros 
 *
 ****************************************************************/

/* General */
#define _XmStrType(str)	      ((str)->empty.type)
#define _XmStrOptimized(str)  ((str)->empty.type == XmSTRING_OPTIMIZED)
#define _XmStrMultiple(str)   ((str)->empty.type ==  XmSTRING_MULTIPLE_ENTRY)
#define _XmStrRefCountGet(str)	(_XmStrMultiple(str)  ? 		   \
			       (str)->multi_str.refcount : 		   \
			       (_XmStrOptimized(str) ?		   	   \
			        (str)->opt_str.refcount : 		   \
			        1))
#define _XmStrRefCountSet(str, val)					   \
  (_XmStrMultiple(str)  ? 		   				   \
   ((str)->multi_str.refcount = (val)) : 	  			   \
   (_XmStrOptimized(str) ?		   	  			   \
    ((str)->opt_str.refcount = (val)) : 0))

#define _XmStrRefCountInc(str)						   \
  (_XmStrMultiple(str)  ? 		   				   \
   ++((str)->multi_str.refcount) :	 	  			   \
   (_XmStrOptimized(str) ?		   	  			   \
    ++((str)->opt_str.refcount) : 0))

#define _XmStrRefCountDec(str)						   \
  (_XmStrMultiple(str)  ? 		   				   \
   --((str)->multi_str.refcount) :	 	  			   \
   (_XmStrOptimized(str) ?		   	  			   \
    --((str)->opt_str.refcount) : 0))

/* Optimized, one-segment XmStrings */
#define _XmStrTextType(str)   ((str)->opt_str.text_type) 
#define _XmStrTagIndex(str)   ((str)->opt_str.tag_index)
#define _XmStrTagGet(str)     (_XmStrTagIndex(str) == TAG_INDEX_UNSET ?    \
			       NULL :                                      \
			       _XmStringIndexGetTag(_XmStrTagIndex(str)))
#define _XmStrByteCount(str)  ((str)->opt_str.byte_count)
#define _XmStrCharCount(str)  _XmStringCharacterCount((str)->opt_str.text, \
						      _XmStrTextType(str), \
						      _XmStrByteCount(str),\
						      NULL)
#define _XmStrRendBegin(str)  ((str)->opt_str.rend_begin)
#define _XmStrRendIndex(str)  ((str)->opt_str.rend_index)
#define _XmStrRendTagGet(str) (_XmStrRendIndex(str) == REND_INDEX_UNSET ?  \
			       NULL :                                      \
			       _XmStringIndexGetTag(_XmStrRendIndex(str)))
#define _XmStrRendEnd(str)    ((str)->opt_str.rend_end)
#define _XmStrDirection(str)  ((str)->opt_str.str_dir)
#define _XmStrFlipped(str)    ((str)->opt_str.flipped)
#define _XmStrTabs(str)	      ((str)->opt_str.tabs)
#define _XmStrText(str)       (((_XmStringOpt)(str))->text)

/* Multi-segment XmStrings */
#define _XmStrImplicitLine(str) (str)->multi_str.implicit_line 
#define _XmStrAddNewline(str)   (_XmStrMultiple(str) ? 		           \
			         _XmStrImplicitLine(str) : False)
#define _XmStrEntryCount(str)	(str)->multi_str.entry_count
#define _XmStrEntryCountGet(str) (_XmStrMultiple(str) ? 		   \
				  _XmStrEntryCount(str) : 1)
#define _XmStrLineCountGet(str) (_XmStrMultiple(str)&&_XmStrAddNewline(str) ? \
				 _XmStrEntryCount(str) : 	   	   \
				  1)
#define _XmStrEntry(str)	((_XmStringMulti)(str))->entry
#define _XmStrEntryGet(str)     (_XmStrMultiple(str) ? 		   	   \
				 _XmStrEntry(str) :			   \
				 (_XmStringEntry*)NULL)

#define _XmStrInit(str, type)						   \
{									   \
  switch (type) { 							   \
  case XmSTRING_OPTIMIZED : 						   \
    bzero((char*)str, sizeof(_XmStringOptRec));				   \
    _XmStrType(str) = type; 						   \
    _XmStrTextType(str) = XmNO_TEXT;                                       \
    _XmStrDirection(str) = XmSTRING_DIRECTION_UNSET;			   \
    _XmStrTagIndex(str) = TAG_INDEX_UNSET;			 	   \
    _XmStrRendIndex(str) = REND_INDEX_UNSET;			 	   \
    _XmStrRefCountSet(str, 1);			 	 	 	   \
    break; 								   \
  case XmSTRING_MULTIPLE_ENTRY : 					   \
    bzero((char*)str, sizeof(_XmStringMultiRec));			   \
    _XmStrType(str) = type; 						   \
    _XmStrRefCountSet(str, 1);			 	 	 	   \
    break; 								   \
  } 									   \
}

#ifdef _XmDEBUG_XMSTRING_MEM
#define STR_OFFSET		sizeof(double)
#define _XmStrMalloc(size)	(XtMalloc((size) + STR_OFFSET) + STR_OFFSET)
#define _XmStrFree(ptr)		(XtFree(((char*)(ptr)) - STR_OFFSET))
#else
#define _XmStrMalloc(size)	(XtMalloc(size))
#define _XmStrFree(ptr)		(XtFree((char*)(ptr)))
#endif /* _XmDEBUG_XMSTRING_MEM */

#define _XmStrCreate(str, type, text_len)				   \
{									   \
  switch (type) { 							   \
  case XmSTRING_OPTIMIZED : 						   \
    (str) = (_XmString)							   \
      _XmStrMalloc(sizeof(_XmStringOptRec) +                               \
		   (text_len ? (text_len - TEXT_BYTES_IN_STRUCT) : 0));    \
    bzero((char*)str, sizeof(_XmStringOptRec)); 			   \
    _XmStrType(str) = type; 						   \
    _XmStrTextType(str) = XmNO_TEXT;                                       \
    _XmStrDirection(str) = XmSTRING_DIRECTION_UNSET;			   \
    _XmStrTagIndex(str) = TAG_INDEX_UNSET;			 	   \
    _XmStrRendIndex(str) = REND_INDEX_UNSET;			 	   \
    _XmStrRefCountSet(str, 1);			 	 	 	   \
    _XmStrByteCount(str) = text_len;                                       \
    break; 								   \
  case XmSTRING_MULTIPLE_ENTRY : 					   \
    (str) = (_XmString)_XmStrMalloc(sizeof(_XmStringMultiRec));		   \
    bzero((char*)str, sizeof(_XmStringMultiRec));			   \
    _XmStrType(str) = type; 						   \
    _XmStrRefCountSet(str, 1);			 	 	 	   \
    break; 								   \
  } 									   \
}

/* General XmString Entry macros */
#define _XmEntryType(entry)	(((_XmStringEntry)(entry))->empty.type)
#define _XmEntryOptimized(entry) 					   \
        (_XmEntryType(entry) == XmSTRING_ENTRY_OPTIMIZED)
#define _XmEntryMultiple(entry) 					   \
        (_XmEntryType(entry) == XmSTRING_ENTRY_ARRAY)
#define _XmEntryUnoptimized(entry) 					   \
	(_XmEntryType(entry) == XmSTRING_ENTRY_UNOPTIMIZED)

/* Non-array entry macros */
#define _XmEntryTextTypeSet(entry, val)					   \
	(_XmEntryOptimized(entry) ? 					   \
	 (((_XmStringEntry)(entry))->single.text_type = (val)) :	   \
	 (((_XmStringEntry)(entry))->unopt_single.text_type = (val)))
#define _XmEntryTagIndex(entry) 					   \
	(((_XmStringEntry)(entry))->single.tag_index)
#define _XmUnoptSegTag(entry) 						   \
        ((_XmStringUnoptSeg)(entry))->tag
#define _XmUnoptSegByteCount(entry)                                        \
        ((_XmStringUnoptSeg)(entry))->byte_count
#define _XmEntryByteCountSet(entry, val)				   \
	(_XmEntryOptimized(entry) ? 					   \
	 (((_XmStringEntry)(entry))->single.byte_count = (val)) : 	   \
	 (_XmUnoptSegByteCount(entry) = (val)))
#define _XmEntryCharCountSet(entry, val)			           \
	(_XmEntryUnoptimized(entry) ? 					   \
	 (((_XmStringUnoptSeg)(entry))->char_count = (val)) :	 	   \
	 0)
#define _XmEntryRendIndex(entry) 					   \
        (((_XmStringEntry)(entry))->single.rend_index)
#define _XmUnoptSegRendBeginCount(entry)				   \
 	((_XmStringUnoptSeg)(entry))->begin_count
#define _XmEntryRendBeginCountSet(entry, val) 				   \
	(_XmEntryOptimized(entry) ? 					   \
	 (((_XmStringEntry)(entry))->single.rend_begin = (val)) : 	   \
	 (_XmUnoptSegRendBeginCount(entry) = (val)))
#define _XmUnoptSegRendBegins(entry)					   \
  	((_XmStringUnoptSeg)(entry))->rend_begin_tags
#define _XmUnoptSegRendEndCount(entry)					   \
  	((_XmStringUnoptSeg)(entry))->end_count
#define _XmEntryRendEndCountSet(entry, val) 				   \
	(_XmEntryOptimized(entry) ? 					   \
	 (((_XmStringEntry)(entry))->single.rend_end = (val)) : 	   \
	 (_XmUnoptSegRendEndCount(entry) = (val)))
#define _XmUnoptSegRendEnds(entry)					   \
  	((_XmStringUnoptSeg)(entry))->rend_end_tags
#define _XmEntryTabsSet(entry, val) 					   \
	(_XmEntryOptimized(entry) ? 					   \
	 (((_XmStringEntry)(entry))->single.tabs_before = (val)) :	   \
	 (((_XmStringEntry)(entry))->unopt_single.tabs_before = (val)))
#define _XmEntryFlippedGet(entry) 					   \
	(_XmEntryOptimized(entry) ? 					   \
	 ((_XmStringEntry)(entry))->single.flipped : 			   \
	 ((_XmStringEntry)(entry))->unopt_single.flipped)
#define _XmEntryFlippedSet(entry, val) 					   \
	(_XmEntryOptimized(entry) ? 					   \
	 (((_XmStringEntry)(entry))->single.flipped = (val)) : 		   \
	 (((_XmStringEntry)(entry))->unopt_single.flipped = (val)))
#define _XmEntryPermGet(entry) 						   \
	(_XmEntryOptimized(entry) ? 					   \
	 ((_XmStringEntry)(entry))->single.permanent : 			   \
	 ((_XmStringEntry)(entry))->unopt_single.permanent)
#define _XmEntryPermSet(entry, val) 					   \
	(_XmEntryOptimized(entry) ? 					   \
	 (((_XmStringEntry)(entry))->single.permanent = (val)) : 	   \
	 (((_XmStringEntry)(entry))->unopt_single.permanent = (val)))
#define _XmEntrySoftNewlineGet(entry) 					   \
	(_XmEntryOptimized(entry) ? 					   \
	 ((_XmStringEntry)(entry))->single.soft_line_break : 		   \
	 (_XmEntryUnoptimized(entry) ? 					   \
	  ((_XmStringEntry)(entry))->unopt_single.soft_line_break : 	   \
	  ((_XmStringEntry)(entry))->multiple.soft_line_break))
#define _XmEntrySoftNewlineSet(entry, val) 				   \
	(_XmEntryOptimized(entry) ? 					   \
	 (((_XmStringEntry)(entry))->single.soft_line_break = (val)) : 	   \
	 (_XmEntryUnoptimized(entry) ? 					   \
	  (((_XmStringEntry)(entry))->unopt_single.soft_line_break = (val)):\
	  (((_XmStringEntry)(entry))->multiple.soft_line_break = (val))))
#define _XmEntryImm(entry) 						   \
	(((_XmStringEntry)(entry))->single.immediate)
#define _XmEntryPushSet(entry, val) 					   \
	(_XmEntryUnoptimized(entry) ? 					   \
	 (((_XmStringEntry)(entry))->unopt_single.push_before = (val)) :  \
	 0)
#define _XmEntryPopSet(entry, val) 					   \
	(_XmEntryUnoptimized(entry) ? 					   \
	 (((_XmStringEntry)(entry))->unopt_single.pop_after = (val)) :    \
	 0)
#define _XmEntryMBText(entry) 						   \
	(((_XmStringOptSeg)(entry))->data.chars)
#define _XmEntryWCText(entry) 						   \
	(((_XmStringOptSeg)(entry))->data.wchars)
#define _XmEntryCacheSet(entry, val) 					   \
	(_XmEntryUnoptimized(entry) ? 					   \
	 (((_XmStringUnoptSeg)(entry))->cache = (val)) :		   \
	 NULL)

	 
/* Array entry specific macros */
#define _XmEntrySegmentCount(entry) 					   \
  (((_XmStringEntry)(entry))->multiple.segment_count)
#define _XmEntrySegmentCountGet(entry) 					   \
        (_XmEntryMultiple(entry) ? 					   \
	 _XmEntrySegmentCount(entry) :			 		   \
	 1)
#define _XmEntrySegment(entry) 						   \
  (((_XmStringArraySeg)(entry))->seg)

#define _XmEntrySegmentGet(entry) 					   \
        (_XmEntryMultiple(entry) ? 					   \
	 _XmEntrySegment(entry) : 					   \
	 (_XmStringNREntry *)&(entry))

/* Creation macros */

#define _XmEntryInit(entry, type)                               	   \
{									   \
  switch (type) { 							   \
  case XmSTRING_ENTRY_OPTIMIZED : 					   \
    bzero((char*)entry, sizeof(_XmStringOptSegRec));			   \
    _XmEntryTagIndex(entry) = TAG_INDEX_UNSET;			 	   \
    _XmEntryRendIndex(entry) = REND_INDEX_UNSET;		 	   \
    break; 								   \
  case XmSTRING_ENTRY_ARRAY : 						   \
    bzero((char*)entry, sizeof(_XmStringArraySegRec));			   \
    break; 								   \
  case XmSTRING_ENTRY_UNOPTIMIZED :					   \
    bzero((char*)entry, sizeof(_XmStringUnoptSegRec));		   \
    break; 								   \
  } 									   \
  _XmEntryType(entry) = type; 						   \
  _XmEntryTextTypeSet(entry, XmNO_TEXT);                                   \
  if (type != XmSTRING_ENTRY_ARRAY)                                        \
    _XmEntryDirectionSet(entry, XmSTRING_DIRECTION_UNSET);		   \
}
        
#define _XmEntryCreate(entry, type)					   \
{									   \
  switch (type) { 							   \
  case XmSTRING_ENTRY_OPTIMIZED : 					   \
    (entry) = (_XmStringEntry)XtMalloc(sizeof(_XmStringOptSegRec));	   \
    bzero((char*)entry, sizeof(_XmStringOptSegRec));			   \
    _XmEntryTagIndex(entry) = TAG_INDEX_UNSET;			 	   \
    _XmEntryRendIndex(entry) = REND_INDEX_UNSET;		 	   \
    break; 								   \
  case XmSTRING_ENTRY_ARRAY : 						   \
    (entry) = (_XmStringEntry)XtMalloc(sizeof(_XmStringArraySegRec));	   \
    bzero((char*)entry, sizeof(_XmStringArraySegRec));			   \
    break; 								   \
  case XmSTRING_ENTRY_UNOPTIMIZED :					   \
    (entry) = (_XmStringEntry)XtMalloc(sizeof(_XmStringUnoptSegRec));  \
    bzero((char*)entry, sizeof(_XmStringUnoptSegRec));		   \
    break; 								   \
  } 									   \
  if (entry) {                                                             \
    _XmEntryType(entry) = type; 					   \
    _XmEntryTextTypeSet(entry, XmNO_TEXT);                                 \
    if (type != XmSTRING_ENTRY_ARRAY)                                      \
      _XmEntryDirectionSet(entry, XmSTRING_DIRECTION_UNSET);		   \
  } 									   \
}

#define _XmCACHE_DIRTY        0
#define _XmCacheDirty(cache)    (((_XmStringCache)(cache))->dirty)
#define _XmCacheNext(cache)     (((_XmStringCache)(cache))->next)

#define _XmEntryDirtyGet(entry, type, data) \
     (((type) == _XmSCANNING_CACHE) ? \
      (Boolean)(long)_XmScanningCacheGet((_XmStringNREntry)entry, \
					 (XmDirection)(long)data, \
					 _XmCACHE_DIRTY) : \
      (((type) == _XmRENDERING_CACHE) ? \
       (Boolean)(long)_XmRenderCacheGet((_XmStringEntry)entry, \
					(XmRenderTable)(long)data, \
				        _XmCACHE_DIRTY) : \
       True))


#define _XmEntryDirtySet(entry, type, data, val) \
     (((type) == _XmSCANNING_CACHE) ? \
      _XmScanningCacheSet((_XmStringNREntry)entry, \
			  (XmDirection)(long)data, \
			  _XmCACHE_DIRTY, (XtPointer)(long)val) : \
      (((type) == _XmRENDERING_CACHE) ? \
       _XmRenderCacheSet((_XmStringEntry)entry, \
			 (XmRenderTable)(long)data, \
			 _XmCACHE_DIRTY, (XtPointer)(long)val) : \
       (void)NULL))
	
/* Scanning cache */
#define _XmCACHE_SCAN_LEFT    1
#define _XmCACHE_SCAN_RIGHT   2
#define _XmCACHE_SCAN_LAYOUT  3
#define _XmCACHE_SCAN_DEPTH   4
#define _XmEntryLeftGet(entry, d) 					   \
      (_XmStringEntry)_XmScanningCacheGet(entry, d, _XmCACHE_SCAN_LEFT)
#define _XmEntryRightGet(entry, d) 				   \
      (_XmStringEntry)_XmScanningCacheGet(entry, d, _XmCACHE_SCAN_RIGHT)
#define _XmEntryLayoutGet(entry, d) 				   \
      (XmDirection)(long)_XmScanningCacheGet(entry, d, _XmCACHE_SCAN_LAYOUT)
#define _XmEntryLayoutDepthGet(entry, d) 				   \
      (unsigned short)(long)_XmScanningCacheGet(entry, d, _XmCACHE_SCAN_DEPTH)
#define _XmEntryLeftSet(entry, d, val) 				   \
      _XmScanningCacheSet(entry, d, _XmCACHE_SCAN_LEFT, (XtPointer)(long)val)
#define _XmEntryRightSet(entry, d, val) 				   \
      _XmScanningCacheSet(entry, d, _XmCACHE_SCAN_RIGHT, (XtPointer)(long)val)
#define _XmEntryLayoutSet(entry, d, val) 				   \
      _XmScanningCacheSet(entry, d, _XmCACHE_SCAN_LAYOUT, (XtPointer)(long)val)
#define _XmEntryLayoutDepthSet(entry, d, val) 			   \
      _XmScanningCacheSet(entry, d, _XmCACHE_SCAN_DEPTH, (XtPointer)(long)val)

/* Rendering cache */
#define _XmCACHE_RENDER_WIDTH     1
#define _XmCACHE_RENDER_HEIGHT    2
#define _XmCACHE_RENDER_RENDITION 3
#define _XmCACHE_RENDER_X         4
#define _XmCACHE_RENDER_Y         5
#define _XmCACHE_RENDER_BASELINE  6
#define _XmCACHE_RENDER_ASCENT	  7
#define _XmCACHE_RENDER_DESCENT	  8
#define _XmCACHE_RENDER_PREV_TABS 9
#define _XmEntryXGet(entry, rt) 					   \
        (long)_XmRenderCacheGet(entry, rt, _XmCACHE_RENDER_X)
#define _XmEntryYGet(entry, rt) 					   \
        (long)_XmRenderCacheGet(entry, rt, _XmCACHE_RENDER_Y)
#define _XmEntryWidthGet(entry, rt) 					   \
        (long)_XmRenderCacheGet(entry, rt, _XmCACHE_RENDER_WIDTH)
#define _XmEntryHeightGet(entry, rt) 					   \
        (long)_XmRenderCacheGet(entry, rt, _XmCACHE_RENDER_HEIGHT)
#define _XmEntryBaselineGet(entry, rt) 				 	   \
        (long)_XmRenderCacheGet(entry, rt, _XmCACHE_RENDER_BASELINE)
#define _XmEntryAscentGet(entry, rt) 				 	   \
        (long)_XmRenderCacheGet(entry, rt, _XmCACHE_RENDER_ASCENT)
#define _XmEntryDescentGet(entry, rt) 				 	   \
        (long)_XmRenderCacheGet(entry, rt, _XmCACHE_RENDER_DESCENT)
#define _XmEntryRenditionGet(entry, rt) 				   \
        (XmRendition)_XmRenderCacheGet(entry, rt, _XmCACHE_RENDER_RENDITION)
#define _XmEntryPrevTabsGet(entry, rt)					   \
        (char)(long)_XmRenderCacheGet(entry, rt, _XmCACHE_RENDER_PREV_TABS)
#define _XmEntryXSet(entry, rt, val) 					   \
        _XmRenderCacheSet(entry, rt, _XmCACHE_RENDER_X, (XtPointer)(long)val)
#define _XmEntryYSet(entry, rt, val) 					   \
        _XmRenderCacheSet(entry, rt, _XmCACHE_RENDER_Y, (XtPointer)(long)val)
#define _XmEntryWidthSet(entry, rt, val) 				   \
        _XmRenderCacheSet(entry, rt, _XmCACHE_RENDER_WIDTH, (XtPointer)(long)val)
#define _XmEntryHeightSet(entry, rt, val) 				   \
        _XmRenderCacheSet(entry, rt, _XmCACHE_RENDER_HEIGHT, (XtPointer)(long)val)
#define _XmEntryBaselineSet(entry, rt, val) 			 	   \
        _XmRenderCacheSet(entry, rt, _XmCACHE_RENDER_BASELINE, (XtPointer)(long)val)
#define _XmEntryAscentSet(entry, rt, val) 				 	   \
        _XmRenderCacheSet(entry, rt, _XmCACHE_RENDER_ASCENT, (XtPointer)(long)val)
#define _XmEntryDescentSet(entry, rt, val) 				 	   \
        _XmRenderCacheSet(entry, rt, _XmCACHE_RENDER_DESCENT, (XtPointer)(long)val)
#define _XmEntryRenditionSet(entry, rt, val) 				   \
        _XmRenderCacheSet(entry, rt, _XmCACHE_RENDER_RENDITION, (XtPointer)(long)val)
#define _XmEntryPrevTabsSet(entry, rt, val)				   \
        _XmRenderCacheSet(entry, rt, _XmCACHE_RENDER_PREV_TABS, (XtPointer)(long)val)

/*
 * Macros for old non-optimized segment data structure access
 */

#define _XmSegTag(seg)			_XmUnoptSegTag(seg)
#define _XmSegCharCount(seg)		_XmUnoptSegByteCount(seg)
#define _XmSegText(seg)			((_XmStringUnoptSeg)(seg))->data.text
#define _XmSegDirection(seg)		_XmEntryDirectionGet(seg)
#define _XmSegLayoutDirection(seg)      _XmEntryPushGet(seg)
#define _XmSegLayout(seg)               _XmEntryPopGet(seg)

#define _XmSegType(seg)			_XmEntryTextTypeGet(seg)
#define _XmSegTextGet(seg)		(unsigned char *)_XmSegText(seg)
#define _XmSegTextSet(seg,val)		_XmEntryTextSet(seg, (XtPointer)(val))
#define _XmSegMBTextGet(seg)		_XmEntryMBText(seg)
#define _XmSegWCTextGet(seg)		_XmEntryWCText(seg)
#define _XmSegTab(seg)			_XmEntryTabsGet(seg)
#define _XmSegLeft(seg)			((_XmStringUnoptSeg)seg)->cache
#define _XmSegRight(seg)		((_XmStringUnoptSeg)seg)->cache
#define _XmSegRendBegins(seg)		_XmUnoptSegRendBegins(seg)
#define _XmSegRendBeginGet(seg,n)	_XmEntryRendBeginGet(seg,n)
#define _XmSegRendBeginCount(seg)	_XmUnoptSegRendBeginCount(seg)
#define _XmSegRendEnds(seg)		_XmUnoptSegRendEnds(seg)
#define _XmSegRendEndGet(seg,n)		_XmEntryRendEndGet(seg,n)
#define _XmSegRendEndCount(seg)		_XmUnoptSegRendEndCount(seg)

/*
 * Macros for line data structure access
 */

#define _XmStrLineSegCount(line)        _XmEntrySegmentCount(line)
#define _XmStrLineSegment(line)						\
  ((_XmStringSegment *)_XmEntrySegment(line))
#define _XmStrLineSegmentSet(line, val)	_XmEntrySegment(line) = 	\
  (_XmStringNREntry *)(val)

/*
 * Macros for internal string data structure access
 */

#define _XmStrLineCnt(str)		_XmStrEntryCount(str)
#define _XmStrLineLine(str)		((_XmStringLine *)_XmStrEntry(str))
#define _XmStrLineLineSet(str, val)	_XmStrEntry(str) = 		\
  (_XmStringEntry *)(val)


/****************************************************************
 *
 * Function headers
 *
 ****************************************************************/

/**** Private Defines, Typedefs, and Function Declarations for XmString.c ****/

extern XFontStruct * _XmGetFirstFont( 
                        XmFontListEntry entry) ;
extern Boolean _XmFontListSearch( 
                        XmFontList fontlist,
                        XmStringCharSet charset,
                        short *indx,
                        XFontStruct **font_struct) ;

extern int _XmStringIndexCacheTag( 
                        XmStringTag tag,
                        int length) ;
extern XmStringTag _XmStringCacheTag( 
                        XmStringTag tag,
                        int length) ;


extern Boolean _XmStringInitContext( 
                        _XmStringContext *context,
                        _XmString string) ;
extern Boolean _XmStringGetNextSegment( 
                        _XmStringContext context,
                        XmStringCharSet *charset,
                        XmStringDirection *direction,
                        char **text,
                        short *char_count,
                        Boolean *separator) ;
extern void _XmStringFreeContext( 
                        _XmStringContext context) ;
extern Dimension _XmStringWidth( 
                        XmFontList fontlist,
                        _XmString string) ;
extern Dimension _XmStringHeight( 
                        XmFontList fontlist,
                        _XmString string) ;
extern void _XmStringExtent( 
                        XmFontList fontlist,
                        _XmString string,
                        Dimension *width,
                        Dimension *height) ;
extern Boolean _XmStringEmpty( 
                        _XmString string) ;
extern void _XmStringDraw( 
                        Display *d,
                        Window w,
                        XmFontList fontlist,
                        _XmString string,
                        GC gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        unsigned int align,
                        unsigned int lay_dir,
#else
                        Position x,
                        Position y,
                        Dimension width,
                        unsigned char align,
                        unsigned char lay_dir,
#endif /* NeedWidePrototypes */
                        XRectangle *clip) ;
extern void _XmStringDrawImage( 
                        Display *d,
                        Window w,
                        XmFontList fontlist,
                        _XmString string,
                        GC gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        unsigned int align,
                        unsigned int lay_dir,
#else
                        Position x,
                        Position y,
                        Dimension width,
                        unsigned char align,
                        unsigned char lay_dir,
#endif /* NeedWidePrototypes */
                        XRectangle *clip) ;
extern void _XmStringDrawUnderline( 
                        Display *d,
                        Window w,
                        XmFontList f,
                        _XmString s,
                        GC gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        unsigned int align,
                        unsigned int lay_dir,
#else
                        Position x,
                        Position y,
                        Dimension width,
                        unsigned char align,
                        unsigned char lay_dir,
#endif /* NeedWidePrototypes */
                        XRectangle *clip,
                        _XmString u) ;
extern void _XmStringDrawMnemonic( 
                        Display *d,
                        Window w,
                        XmFontList fontlist,
                        _XmString string,
                        GC gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        unsigned int align,
                        unsigned int lay_dir,
#else
                        Position x,
                        Position y,
                        Dimension width,
                        unsigned char align,
                        unsigned char lay_dir,
#endif /* NeedWidePrototypes */
                        XRectangle *clip,
                        String mnemonic,
                        XmStringCharSet charset) ;
extern void _XmStringRender(Display *d,
                            Drawable w,
                            XmRenderTable rendertable,
                            XmRendition rend,
                            _XmString string,
#if NeedWidePrototypes
                            int x,
                            int y,
                            int width,
                            unsigned int align,
                            unsigned int lay_dir
#else
                            Position x,
                            Position y,
                            Dimension width,
                            unsigned char align,
                            unsigned char lay_dir
#endif	/* NeedWidePrototypes */
                            ) ;
extern _XmString _XmStringCreate( 
                        XmString cs) ;
extern void _XmStringFree( 
                        _XmString string) ;
extern char * _XmStringGetCurrentCharset( void ) ;
extern char * _XmCharsetCanonicalize( 
                        String charset) ;
extern _XmString _XmStringCopy( 
                        _XmString string) ;
extern Boolean _XmStringByteCompare( 
                        _XmString a,
                        _XmString b) ;
extern Boolean _XmStringHasSubstring( 
                        _XmString string,
                        _XmString substring) ;
extern XmString _XmStringCreateExternal( 
                        XmFontList fontlist,
                        _XmString cs) ;
extern Dimension _XmStringBaseline( 
                        XmFontList fontlist,
                        _XmString string) ;
extern void _XmStringGetBaselines(XmRenderTable rendertable,
                                  _XmString string,
                                  Dimension **baselines,
                                  Cardinal *line_count);
extern int _XmStringLineCount( 
                        _XmString string) ;
extern char * _XmStringGetTextConcat( 
                        XmString string) ;
extern Boolean _XmStringIsCurrentCharset(
			XmStringCharSet c) ;
extern Boolean _XmStringSingleSegment(
			XmString str,
			char **pTextOut,
			XmStringCharSet *pCharsetOut ) ;
extern NextTabResult _XmStringGetNextTabWidth(XmStringContext ctx,
				Widget widget,
                                unsigned char units,
                                XmRenderTable rt,
				float *width,
				XmRendition *rend); 
extern XtPointer _XmStringUngenerate (XmString string,
			XmStringTag tag,
			XmTextType tag_type,
			XmTextType output_type);

extern void _XmStringDrawSegment(Display *d,
				 Drawable w, 
#if NeedWidePrototypes
				 int x,
				 int y,
				 int width,
				 int height,
#else
				 Position x,
				 Position y,
				 Dimension width,
				 Dimension height,
#endif /* NeedWidePrototypes */
				 _XmStringNREntry seg, 
				 XmRendition rend, 
				 XmRenderTable rendertable,
#if NeedWidePrototypes 
				 int image,
#else
				 Boolean image, 
#endif /* NeedWidePrototypes */
				 XmString *underline, 
#if NeedWidePrototypes
				 unsigned int descender
#else 
				 Dimension descender
#endif /* NeedWidePrototypes */
				 ); 
extern void _XmStringDrawLining(Display *d,
				Drawable w,
				Position x,
				Position y,
				Dimension width,
				Dimension height,
				Dimension descender,
				XmRendition rend,
				Pixel select_color,
				XmHighlightMode mode,
				Boolean colors_set);

extern Boolean _XmStringSegmentExtents(_XmStringEntry entry,
				       XmRenderTable rendertable, 
				       XmRendition *rend_in_out,
				       XmRendition base,
				       Dimension *width,
				       Dimension *height,
				       Dimension *ascent,
				       Dimension *descent); 

extern void _XmStringLayout(_XmString string,
#if NeedWidePrototypes
			    int direction
#else
                            XmDirection direction
#endif /* NeedWidePrototypes */
			    );
extern _XmString _XmStringOptToNonOpt(_XmStringOpt string);
extern XmString _XmStringCvtOptToMulti(XmString str);
extern XmString _XmStringOptimize(XmString str);
extern XmString _XmStringMakeXmString(_XmStringEntry **entries, 
				      int count);
extern void _XmStringEntryFree(_XmStringEntry entry);
extern _XmStringEntry _XmStringEntryCopy(_XmStringEntry entry);
extern unsigned char _XmStringCharacterCount(XtPointer text,
					     XmTextType text_type,
					     int byte_count,
					     XFontStruct *font);
extern unsigned char _XmEntryCharCountGet(_XmStringEntry entry, 
					  XmRenderTable rt);
extern _XmStringCache _XmStringCacheGet(_XmStringCache caches, 
					int type);
extern void _XmStringCacheFree(_XmStringCache caches);
extern XtPointer _XmScanningCacheGet(_XmStringNREntry entry, 
#if NeedWidePrototypes
				     int d,
#else				     
				     XmDirection d, 
#endif /* NeedWidePrototypes */
				     int field);
extern void      _XmScanningCacheSet(_XmStringNREntry entry, 
#if NeedWidePrototypes
				     int d,
#else				     
				     XmDirection d, 
#endif /* NeedWidePrototypes */
				     int field,
				     XtPointer value);
/* Rendering cache */
extern XtPointer _XmRenderCacheGet(_XmStringEntry entry,
				   XmRenderTable rt, 
				   int field);
extern void      _XmRenderCacheSet(_XmStringEntry entry, 
				   XmRenderTable rt, 
				   int field, 
				   XtPointer value);

extern XmStringTag _XmStringIndexGetTag(int index);

extern Boolean _XmStringGetSegment(_XmStringContext   context, 
				   Boolean	      update_context,
				   Boolean	      copy_data,
				   XtPointer         *text, 
				   XmStringTag       *tag, 
				   XmTextType        *type, 
				   XmStringTag      **rendition_tags, 
				   unsigned int      *tag_count,
				   XmStringDirection *direction,
				   Boolean           *separator, 
				   unsigned char     *tabs,
				   short             *char_count,
				   XmDirection       *push_before,
				   Boolean	     *pop_after);

/* Declarations for macro to function switchover. */
extern _XmStringCache _XmEntryCacheGet(_XmStringEntry entry); 
extern XmStringTag _XmEntryTag(_XmStringEntry entry); 
extern void _XmEntryTagSet(_XmStringEntry entry, XmStringTag tag); 
extern XtPointer _XmEntryTextGet(_XmStringEntry entry); 
extern XmDirection _XmEntryPushGet(_XmStringEntry entry); 
extern Boolean _XmEntryPopGet(_XmStringEntry entry); 
extern unsigned int _XmEntryByteCountGet(_XmStringEntry entry); 
extern unsigned int _XmEntryDirectionGet(_XmStringEntry entry);
extern void _XmEntryDirectionSet(_XmStringEntry entry, XmDirection val);
extern unsigned char _XmEntryRendEndCountGet(_XmStringEntry entry);
extern unsigned char _XmEntryRendBeginCountGet(_XmStringEntry entry);
extern XmStringTag _XmEntryRendEndGet(_XmStringEntry entry,
				      int n);
extern XmStringTag _XmEntryRendBeginGet(_XmStringEntry entry,
					int n);
extern void _XmEntryRendEndSet(_XmStringEntry entry,
			       XmStringTag tag,
			       int n);
extern void _XmEntryRendBeginSet(_XmStringEntry entry,
				 XmStringTag tag,
				 int n);
extern unsigned char _XmEntryTabsGet(_XmStringEntry entry);
extern unsigned int _XmEntryTextTypeGet(_XmStringEntry entry); 
extern void _XmEntryTextSet(_XmStringEntry entry, XtPointer val);

extern unsigned char *_XmStringTruncateASN1(unsigned char *str, int n); 
extern void _XmStringContextCopy(_XmStringContext target,
				 _XmStringContext source);
extern void _XmStringContextFree(_XmStringContext target);
extern XmString _XmStringNCreate(char *text, XmStringTag tag, int len);
extern void _XmStringSegmentNew(_XmString string,
				int line_index,
				_XmStringEntry value,
				int copy) ;
extern void _XmStringContextReInit(_XmStringContext context,
			 _XmString	  string);
extern int _XmConvertFactor(unsigned char units,
			    float *factor);



#ifdef _XmDEBUG_XMSTRING
extern void _Xm_dump_fontlist(XmFontList f) ;
extern void _Xm_dump_fontlist_cache( void ) ;
extern void _Xm_dump_stream( unsigned char *cs) ;
extern void _Xm_dump_internal(_XmString string) ;
#endif /* _XmDEBUG_XMSTRING */
/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmStringI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
